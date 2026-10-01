package dev.kindling.pretests

import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import java.io.File
import java.io.IOException
import java.io.OutputStream
import java.net.InetAddress
import java.net.ServerSocket
import java.net.Socket
import java.nio.file.Files
import java.security.MessageDigest
import java.util.concurrent.atomic.AtomicInteger
import kotlin.random.Random

/**
 * B73, the Gemma download, against a local server shaped like Hugging Face's: the file's link answers with a
 * redirect to a storage host, which serves byte ranges. Checks resuming after a cut, a server that ignores
 * ranges, a damaged file, the owner's Skip, a missing file and no Wi-Fi.
 */
class DownloadTest {
    private val blob = Random(7).nextBytes(5_000_000 + 123)
    private val sha = MessageDigest.getInstance("SHA-256").digest(blob).joinToString("") { "%02x".format(it) }
    private lateinit var server: ServerSocket
    private lateinit var base: String
    private val requests = AtomicInteger(0)
    private val ranged = AtomicInteger(0)
    @Volatile private var cutFirstAt = -1 // cut the first body after this many bytes
    @Volatile private var ignoreRange = false

    /** A minimal HTTP/1.1 server on a local port: one request per connection, then close. */
    @Before fun start() {
        server = ServerSocket(0, 50, InetAddress.getByName("127.0.0.1"))
        base = "http://127.0.0.1:${server.localPort}"
        Thread {
            while (!server.isClosed) {
                val s = try { server.accept() } catch (_: IOException) { break }
                Thread { handle(s) }.apply { isDaemon = true }.start()
            }
        }.apply { isDaemon = true }.start()
    }

    @After fun stop() = server.close()

    private fun handle(sock: Socket) {
        sock.use { s ->
            val input = s.getInputStream().bufferedReader(Charsets.ISO_8859_1)
            val path = (input.readLine() ?: return).split(" ").getOrNull(1) ?: return
            val headers = HashMap<String, String>()
            while (true) {
                val line = input.readLine() ?: break
                if (line.isEmpty()) break
                headers[line.substringBefore(':').trim().lowercase()] = line.substringAfter(':').trim()
            }
            val out = s.getOutputStream()
            when {
                path == "/repo/resolve/main/model.bin" -> head(out, "302 Found", "Location: /cdn/abc?signed=1", 0)
                path.startsWith("/cdn/abc") -> serve(out, headers["range"])
                else -> head(out, "404 Not Found", null, 0)
            }
            out.flush()
        }
    }

    private fun head(out: OutputStream, status: String, extra: String?, length: Long) {
        val h = StringBuilder("HTTP/1.1 $status\r\nContent-Length: $length\r\nConnection: close\r\n")
        if (extra != null) h.append(extra).append("\r\n")
        out.write(h.append("\r\n").toString().toByteArray(Charsets.ISO_8859_1))
    }

    private fun serve(out: OutputStream, range: String?) {
        val n = requests.incrementAndGet()
        var from = 0
        if (range != null && !ignoreRange) {
            from = range.removePrefix("bytes=").substringBefore('-').toInt()
            ranged.incrementAndGet()
            head(out, "206 Partial Content", "Content-Range: bytes $from-${blob.size - 1}/${blob.size}", (blob.size - from).toLong())
        } else {
            head(out, "200 OK", null, blob.size.toLong())
        }
        // The first body can be cut part-way, as a dropped connection.
        val end = if (n == 1 && cutFirstAt > 0) cutFirstAt else blob.size
        try {
            var i = from
            while (i < end) {
                val m = minOf(64 * 1024, end - i)
                out.write(blob, i, m)
                i += m
            }
        } catch (_: IOException) {}
    }

    private fun dir() = Files.createTempDirectory("dl").toFile()

    private fun download(dest: File, hash: String = sha, wifi: () -> Boolean = { true }, stop: () -> String? = { null }) =
        ModelDownload("$base/repo/resolve/main/model.bin", dest, blob.size.toLong(), hash,
            canUseNetwork = wifi, stop = stop, sleepMs = {}, networkWaitMs = 50, readTimeoutMs = 5_000)

    @Test fun downloadsThroughTheRedirectAndChecksTheFile() {
        val dest = File(dir(), "models/model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertTrue(dest.readBytes().contentEquals(blob))
        assertTrue(d.ready())
        assertFalse(d.part.exists())
        assertEquals("127.0.0.1", d.log.getString("host"))
        // A second run finds it ready, without the network.
        val again = download(dest, wifi = { false })
        assertEquals("ready", again.run())
    }

    @Test fun resumesAfterTheConnectionIsCut() {
        cutFirstAt = 1_500_000
        val dest = File(dir(), "model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertTrue(dest.readBytes().contentEquals(blob))
        assertEquals(1, d.log.getInt("resumes"))
        assertTrue(ranged.get() >= 1)
        assertEquals(blob.size.toLong(), d.log.getLong("got"))
    }

    @Test fun resumesAPartialFileFromAnEarlierRun() {
        val dest = File(dir(), "model.bin")
        File(dest.path + ".part").writeBytes(blob.copyOf(2_000_000))
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(2_000_000L, d.log.getLong("have0"))
        assertEquals((blob.size - 2_000_000).toLong(), d.log.getLong("got"))
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun startsAgainWhenTheServerIgnoresTheRange() {
        ignoreRange = true
        val dest = File(dir(), "model.bin")
        File(dest.path + ".part").writeBytes(blob.copyOf(1_000_000))
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(1, d.log.getInt("restarts"))
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun aDamagedFileIsDeletedNotUsed() {
        val dest = File(dir(), "model.bin")
        val d = download(dest, hash = "0".repeat(64))
        assertEquals("bad-hash", d.run())
        assertFalse(dest.exists())
        assertFalse(d.part.exists())
        assertFalse(d.ready())
    }

    @Test fun skipKeepsWhatCameForNextTime() {
        cutFirstAt = 1_000_000
        val dest = File(dir(), "model.bin")
        var calls = 0
        val d = download(dest, stop = { if (++calls > 3) "you tapped Skip" else null })
        assertTrue(d.run().startsWith("stopped: you tapped Skip"))
        assertFalse(dest.exists())
        assertTrue(d.part.length() > 0)
        // Next time it carries on and finishes.
        assertEquals("done", download(dest).run())
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun aMissingFileFailsAtOnceAndNoWifiWaitsThenGivesUp() {
        val missing = ModelDownload("$base/repo/resolve/main/nothing.bin", File(dir(), "x.bin"), 10, sha, sleepMs = {})
        assertEquals("failed: HTTP 404", missing.run())
        val noWifi = download(File(dir(), "model.bin"), wifi = { false })
        assertEquals("no-network", noWifi.run())
        assertEquals(0, requests.get())
    }

    @Test fun deleteRemovesTheFileAndAnyPart() {
        val dest = File(dir(), "model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(blob.size.toLong(), d.deleteAll())
        assertFalse(dest.exists() || d.part.exists() || d.okFile.exists())
    }
}
