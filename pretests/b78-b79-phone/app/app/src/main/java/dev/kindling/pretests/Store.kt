package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream

/**
 * B78/B79 robustness: every result is written as soon as its test ends, and a marker
 * names the test that is running, so a crash is found and reported on the next launch.
 * Files (in the app's files directory, kept when a later round installs over this one):
 * - results.json: the results; the result code is made from it.
 * - full.json: extra detail (per-second samples, full checksums) that stays on the phone.
 * - running.txt: the step running now; left behind only if the app died during it.
 * - memory-progress.txt: the memory test's last step, written before each step.
 */
class Store(private val dir: File) {
    val resultsFile = File(dir, "results.json")
    val fullFile = File(dir, "full.json")
    val markerFile = File(dir, "running.txt")
    val memFile = File(dir, "memory-progress.txt")
    val uncaughtFile = File(dir, "uncaught.txt")

    var results: JSONObject = read(resultsFile)
        private set
    var full: JSONObject = read(fullFile)
        private set

    private fun read(f: File): JSONObject = try {
        if (f.exists()) JSONObject(f.readText()) else JSONObject()
    } catch (t: Throwable) {
        JSONObject().put("readError", t.toString())
    }

    /** Writes a file so that a crash leaves either the old or the new version, never half of one. */
    fun writeAtomic(f: File, text: String) {
        val tmp = File(f.path + ".tmp")
        FileOutputStream(tmp).use { out ->
            out.write(text.toByteArray(Charsets.UTF_8))
            out.fd.sync()
        }
        if (!tmp.renameTo(f)) {
            f.delete()
            tmp.renameTo(f)
        }
    }

    @Synchronized fun save() {
        writeAtomic(resultsFile, results.toString())
        writeAtomic(fullFile, full.toString())
    }

    @Synchronized fun put(key: String, value: Any, detail: Any? = null) {
        results.put(key, value)
        if (detail != null) full.put(key, detail)
        save()
    }

    @Synchronized fun reset(meta: JSONObject) {
        results = meta
        full = JSONObject()
        save()
    }

    private fun list(key: String): JSONArray =
        results.optJSONArray(key) ?: JSONArray().also { results.put(key, it) }

    private fun has(key: String, step: String): Boolean {
        val a = results.optJSONArray(key) ?: return false
        return (0 until a.length()).any { a.optString(it) == step }
    }

    fun isDone(step: String) = has("done", step)
    fun isCrashed(step: String) = has("crash", step)

    @Synchronized fun markDone(step: String, seconds: Double) {
        if (!isDone(step)) list("done").put(step)
        (results.optJSONObject("time") ?: JSONObject().also { results.put("time", it) })
            .put(step, Logic.dp(seconds, 1))
        save()
    }

    @Synchronized fun error(step: String, t: Throwable) {
        val e = results.optJSONObject("err") ?: JSONObject().also { results.put("err", it) }
        e.put(step, t.toString().take(160))
        save()
    }

    /** Written (and synced) before a step starts. */
    fun markStart(step: String) = writeAtomic(markerFile, step)

    fun markEnd() {
        markerFile.delete()
    }

    /** The step that was running when the app last died, if any. */
    fun leftoverMarker(): String? = try {
        if (markerFile.exists()) markerFile.readText().trim().ifEmpty { null } else null
    } catch (_: Throwable) {
        null
    }

    @Synchronized fun recordCrash(step: String) {
        if (!isCrashed(step)) list("crash").put(step)
        save()
    }
}
