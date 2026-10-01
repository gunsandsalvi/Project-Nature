package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream

/**
 * B78 robustness, as in round 1: every result is written as soon as its test ends, and a marker
 * names the test that is running, so a crash is found and reported on the next launch.
 * Round 2 keeps its files in their own folder, so round 1's results (kept on the phone) are never touched.
 * Files (in files/r2/, kept when a later round installs over this one):
 * - results.json: everything the result code is made from, texts included.
 * - full.json: extra detail that stays on the phone unless shared (memory samples, logs).
 * - running.txt: the step running now, as "step" or "step/part"; left behind only if the app died.
 * - uncaught.txt: the last uncaught exception, written as the app dies.
 */
class Store(val dir: File) {
    init { dir.mkdirs() }

    val resultsFile = File(dir, "results.json")
    val fullFile = File(dir, "full.json")
    val markerFile = File(dir, "running.txt")
    val uncaughtFile = File(dir, "uncaught.txt")

    var results: JSONObject = read(resultsFile)
        private set
    var full: JSONObject = read(fullFile)
        private set

    private fun read(f: File): JSONObject = try {
        if (f.exists()) JSONObject(f.readText()) else JSONObject()
    } catch (t: Throwable) {
        JSONObject().put("readError", t.toString().take(200))
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

    /** A copy of one result, safe to read while the test thread keeps writing. */
    @Synchronized fun copyOf(key: String): JSONObject? = results.optJSONObject(key)?.let { JSONObject(it.toString()) }

    /** A copy of everything, for the result code and the shared file. */
    @Synchronized fun snapshot(): Pair<JSONObject, JSONObject> = JSONObject(results.toString()) to JSONObject(full.toString())

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

    @Synchronized fun isDone(step: String) = has("done", step)
    @Synchronized fun isCrashed(step: String) = has("crash", step)

    @Synchronized fun markDone(step: String, seconds: Double) {
        if (!isDone(step)) list("done").put(step)
        (results.optJSONObject("time") ?: JSONObject().also { results.put("time", it) })
            .put(step, Logic.dp(seconds, 1))
        save()
    }

    @Synchronized fun error(step: String, t: Throwable) = errorText(step, t.toString())

    @Synchronized fun errorText(step: String, text: String) {
        val e = results.optJSONObject("err") ?: JSONObject().also { results.put("err", it) }
        e.put(step, text.take(200))
        save()
    }

    /** Written (and synced) before a step, or a part of one, starts. */
    fun markStart(step: String) = writeAtomic(markerFile, step)

    fun markEnd() {
        markerFile.delete()
    }

    /** The step (or "step/part") that was running when the app last died, if any. */
    fun leftoverMarker(): String? = try {
        if (markerFile.exists()) markerFile.readText().trim().ifEmpty { null } else null
    } catch (_: Throwable) {
        null
    }

    @Synchronized fun recordCrash(step: String) {
        if (!isCrashed(step)) list("crash").put(step)
        save()
    }

    /**
     * A crash inside a part of a step ("gm/init-GPU"): the step itself carries on next time, from the
     * next part. Returns how many times this step has now crashed; past a few, the caller gives up on it.
     */
    @Synchronized fun recordPartCrash(step: String, part: String): Int {
        val all = results.optJSONObject("partCrash") ?: JSONObject().also { results.put("partCrash", it) }
        val a = all.optJSONArray(step) ?: JSONArray().also { all.put(step, it) }
        a.put(part)
        save()
        return a.length()
    }

    @Synchronized fun partCrashes(step: String): List<String> {
        val a = results.optJSONObject("partCrash")?.optJSONArray(step) ?: return emptyList()
        return (0 until a.length()).map { a.optString(it) }
    }
}
