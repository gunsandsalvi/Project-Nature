package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException
import java.net.HttpURLConnection
import java.net.URL
import java.security.MessageDigest

/**
 * B73, Gemma: downloads one large public file, resuming after any interruption, then checks its size and
 * SHA-256 before using it. The file grows as "<name>.part" and is renamed only once checked, with a small
 * "<name>.ok" mark beside it, so a later run (or round) reuses it without checking it again.
 * Everything the phone decides (Wi-Fi, the Skip button, the clock) comes in from outside, so the JVM tests
 * can drive it against a local server.
 */
class ModelDownload(
    private val url: String,
    val dest: File,
    private val size: Long,
    private val sha256: String,
    /** Is the network allowed now (the phone on Wi-Fi)? */
    private val canUseNetwork: () -> Boolean = { true },
    /** A reason to stop now (the owner tapped Skip, or time ran out), or null to go on. */
    private val stop: () -> String? = { null },
    /** Progress: phase ("wifi", "download" or "check"), bytes done, bytes in all, bytes a second. */
    private val progress: (String, Long, Long, Double) -> Unit = { _, _, _, _ -> },
    private val sleepMs: (Long) -> Unit = { Thread.sleep(it) },
    private val maxFailures: Int = 8,
    private val networkWaitMs: Long = 180_000L,
    private val connectTimeoutMs: Int = 30_000,
    private val readTimeoutMs: Int = 60_000,
) {
    val part = File(dest.path + ".part")
    val okFile = File(dest.path + ".ok")

    /** What happened, for the results: status, bytes, seconds, speed, resumes, restarts, errors. */
    val log = JSONObject()
    private val errors = JSONArray()

    private class Stop(why: String) : IOException(why)
    private class Fatal(why: String) : IOException(why)

    private fun now() = System.nanoTime() / 1_000_000L

    /** Is the complete, checked file already there? */
    fun ready(): Boolean = try {
        dest.isFile && dest.length() == size && okFile.isFile && okFile.readText().trim() == "$size $sha256"
    } catch (_: Throwable) {
        false
    }

    /** Deletes the file, any partial download and the check mark; returns the model bytes freed. */
    fun deleteAll(): Long {
        var freed = 0L
        for (f in listOf(dest, part)) if (f.exists()) { freed += f.length(); f.delete() }
        okFile.delete()
        return freed
    }

    private var t0 = 0L
    private var got = 0L

    private fun finish(status: String): String {
        val s = (now() - t0) / 1000.0
        log.put("status", status).put("got", got).put("s", Logic.dp(s, 1))
            .put("mbps", if (s > 0) Logic.dp(got / s / 1e6, 1) else JSONObject.NULL)
        if (errors.length() > 0) log.put("errs", errors)
        return status
    }

    private fun note(e: Throwable) {
        errors.put(e.toString().take(160))
        while (errors.length() > 5) errors.remove(0)
    }

    /** Returns "ready", "done", "stopped: why", "no-network", "bad-hash" or "failed: why". */
    fun run(): String {
        t0 = now()
        got = 0L
        if (ready()) return finish("ready")
        dest.parentFile?.mkdirs()
        // A complete file without its mark: the app died while checking it. Check it again.
        if (dest.isFile) {
            when (if (dest.length() == size) check(dest) else false) {
                true -> { mark(); return finish("ready") }
                null -> return finish("stopped: ${stop() ?: "while checking"}")
                false -> dest.delete()
            }
        }
        log.put("have0", if (part.isFile) part.length() else 0L)
        var failures = 0
        while (true) {
            stop()?.let { return finish("stopped: $it") }
            if (!canUseNetwork() && !waitForNetwork()) return finish(stop()?.let { "stopped: $it" } ?: "no-network")
            var have = if (part.isFile) part.length() else 0L
            if (have > size) { part.delete(); have = 0L }
            if (have == size) break
            val gotBefore = got
            try {
                fetch(have)
                failures = 0
            } catch (e: Stop) {
                return finish("stopped: ${e.message}")
            } catch (e: Fatal) {
                note(e)
                return finish("failed: ${e.message}")
            } catch (e: IOException) {
                note(e)
                // Only connections that bring nothing count towards giving up.
                if (got > gotBefore) failures = 0
                if (++failures > maxFailures) return finish("failed: ${e.toString().take(120)}")
                log.put("retries", log.optInt("retries") + 1)
                sleepMs(minOf(30_000L, 1000L shl minOf(failures, 5)))
            }
        }
        return when (check(part)) {
            null -> finish("stopped: ${stop() ?: "while checking"}")
            false -> { part.delete(); finish("bad-hash") }
            true -> {
                dest.delete()
                if (!part.renameTo(dest)) return finish("failed: could not rename the checked file")
                mark()
                finish("done")
            }
        }
    }

    private fun mark() {
        okFile.writeText("$size $sha256")
    }

    private fun waitForNetwork(): Boolean {
        val start = now()
        while (now() - start < networkWaitMs) {
            if (stop() != null) return false
            progress("wifi", (now() - start) / 1000, networkWaitMs / 1000, 0.0)
            sleepMs(5_000)
            if (canUseNetwork()) return true
        }
        return canUseNetwork()
    }

    /** Opens the file at byte `have`, following redirects by hand so every hop carries the Range header. */
    private fun open(have: Long): HttpURLConnection {
        var target = URL(url)
        repeat(8) {
            val c = target.openConnection() as HttpURLConnection
            c.instanceFollowRedirects = false
            c.connectTimeout = connectTimeoutMs
            c.readTimeout = readTimeoutMs
            c.setRequestProperty("Accept-Encoding", "identity")
            c.setRequestProperty("User-Agent", "KindlingPretests/r2")
            if (have > 0) c.setRequestProperty("Range", "bytes=$have-")
            val code = c.responseCode
            if (code in 300..399 && code != 304) {
                val loc = c.getHeaderField("Location")
                c.disconnect()
                if (loc.isNullOrEmpty()) throw IOException("HTTP $code without Location")
                target = URL(target, loc)
                log.put("host", target.host)
            } else {
                return c
            }
        }
        throw IOException("too many redirects")
    }

    /** One connection: appends to the partial file (counting into `got`); returns the bytes written. */
    private fun fetch(have: Long): Long {
        val conn = open(have)
        try {
            val code = conn.responseCode
            val append: Boolean
            val expect: Long
            when {
                code == 206 -> {
                    val cr = Logic.parseContentRange(conn.getHeaderField("Content-Range"))
                        ?: throw IOException("206 without Content-Range")
                    if (cr.third != size) throw Fatal("the server's file is ${cr.third} bytes, not $size")
                    if (cr.first != have) throw IOException("the server resumed at ${cr.first}, not $have")
                    append = true
                    expect = cr.second - cr.first + 1
                    if (have > 0) log.put("resumes", log.optInt("resumes") + 1)
                }
                code == 200 -> {
                    val len = conn.contentLengthLong
                    if (len >= 0 && len != size) throw Fatal("the server's file is $len bytes, not $size")
                    if (have > 0) log.put("restarts", log.optInt("restarts") + 1) // the server ignored the range
                    append = false
                    expect = size
                }
                code == 416 -> {
                    part.delete()
                    throw IOException("HTTP 416 at $have")
                }
                code == 408 || code == 429 || code >= 500 -> throw IOException("HTTP $code")
                else -> throw Fatal("HTTP $code")
            }
            val base = if (append) have else 0L
            var written = 0L
            val window = ArrayDeque<Pair<Long, Long>>() // (time, bytes) over the last 10 s, for the speed shown
            conn.inputStream.use { input ->
                FileOutputStream(part, append).use { out ->
                    val buf = ByteArray(1 shl 20)
                    var sinceSync = 0L
                    var lastReport = 0L
                    while (true) {
                        stop()?.let { why -> out.fd.sync(); throw Stop(why) }
                        val n = input.read(buf)
                        if (n < 0) break
                        try {
                            out.write(buf, 0, n)
                        } catch (e: IOException) {
                            if ((e.message ?: "").let { "ENOSPC" in it || "No space" in it }) throw Fatal("storage full")
                            throw e
                        }
                        written += n
                        got += n
                        sinceSync += n
                        if (base + written > size) throw IOException("more data than expected")
                        if (sinceSync >= (256L shl 20)) { out.fd.sync(); sinceSync = 0 }
                        val t = now()
                        if (t - lastReport >= 500) {
                            lastReport = t
                            window.addLast(t to written)
                            while (window.size > 1 && t - window.first().first > 10_000) window.removeFirst()
                            val (t1, b1) = window.first()
                            val bps = if (t > t1) (written - b1) * 1000.0 / (t - t1) else 0.0
                            progress("download", base + written, size, bps)
                        }
                    }
                    out.fd.sync()
                }
            }
            if (written < expect) throw IOException("connection closed at ${base + written} of $size bytes")
            return written
        } finally {
            conn.disconnect()
        }
    }

    /** True if the file has the expected size and SHA-256, false if not, null if stopped part-way. */
    fun check(f: File): Boolean? {
        if (f.length() != size) return false
        val md = MessageDigest.getInstance("SHA-256")
        val t = now()
        var done = 0L
        var lastReport = 0L
        FileInputStream(f).use { input ->
            val buf = ByteArray(4 shl 20)
            while (true) {
                val n = input.read(buf)
                if (n < 0) break
                md.update(buf, 0, n)
                done += n
                val now = now()
                if (now - lastReport >= 500) {
                    lastReport = now
                    if (stop() != null) return null
                    progress("check", done, size, if (now > t) done * 1000.0 / (now - t) else 0.0)
                }
            }
        }
        val hex = md.digest().joinToString("") { "%02x".format(it) }
        log.put("hashS", Logic.dp((now() - t) / 1000.0, 1))
        return hex.equals(sha256, ignoreCase = true)
    }
}
