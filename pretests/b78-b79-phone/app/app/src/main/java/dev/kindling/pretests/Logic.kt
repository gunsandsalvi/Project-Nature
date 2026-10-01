package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.util.Base64
import java.util.zip.GZIPInputStream
import java.util.zip.GZIPOutputStream
import kotlin.math.abs
import kotlin.math.ceil
import kotlin.math.floor
import kotlin.math.log10
import kotlin.math.pow
import kotlin.math.roundToLong

/** B79: plain logic with no Android calls, unit-tested on the JVM (app/src/test). */
object Logic {
    data class Cluster(val cpus: List<Int>, val maxKHz: Long)

    /** CPUs grouped by maximum frequency, slowest first. Null if any CPU's frequency is unknown. */
    fun clustersByFreq(maxKHz: Map<Int, Long>, ncpu: Int): List<Cluster>? {
        if (ncpu <= 0 || (0 until ncpu).any { (maxKHz[it] ?: 0L) <= 0L }) return null
        return (0 until ncpu).groupBy { maxKHz.getValue(it) }
            .map { (f, cpus) -> Cluster(cpus.sorted(), f) }
            .sortedBy { it.maxKHz }
    }

    /** CPUs grouped by core type ("CPU part" in /proc/cpuinfo), in CPU order. Null if incomplete. */
    fun clustersByPart(cpuinfo: String, ncpu: Int): List<Cluster>? {
        val part = HashMap<Int, String>()
        var cur = -1
        for (line in cpuinfo.lineSequence()) {
            val k = line.substringBefore(':').trim()
            val v = line.substringAfter(':', "").trim()
            if (k == "processor") cur = v.toIntOrNull() ?: -1
            if (k == "CPU part" && cur >= 0) part[cur] = v
        }
        if (ncpu <= 0 || (0 until ncpu).any { part[it] == null }) return null
        val order = (0 until ncpu).map { part.getValue(it) }.distinct()
        return order.map { p -> Cluster((0 until ncpu).filter { part[it] == p }, 0L) }
    }

    /** CPUs grouped by measured single-thread speed (within 8%), slowest first. */
    fun clustersBySpeed(speed: DoubleArray): List<Cluster> {
        val idx = speed.indices.sortedBy { speed[it] }
        val out = ArrayList<MutableList<Int>>()
        var base = -1.0
        for (i in idx) {
            if (out.isEmpty() || speed[i] > base * 1.08) { out.add(mutableListOf()); base = speed[i] }
            out.last().add(i)
        }
        return out.map { Cluster(it.sorted(), 0L) }
    }

    /** The core used for one cluster's single-thread runs: its highest-numbered CPU (cpu0 is busiest). */
    fun representative(c: Cluster): Int = c.cpus.last()

    /** Nearest-rank percentile of an ascending array. */
    fun percentile(sorted: DoubleArray, p: Double): Double {
        if (sorted.isEmpty()) return Double.NaN
        val i = (ceil(p / 100.0 * sorted.size) - 1).toInt().coerceIn(0, sorted.size - 1)
        return sorted[i]
    }

    /** Rounds to `digits` significant digits. */
    fun sig(x: Double, digits: Int = 3): Double {
        if (x == 0.0 || x.isNaN() || x.isInfinite()) return x
        val e = floor(log10(abs(x))).toInt() - digits + 1
        val m = 10.0.pow(e)
        return (x / m).roundToLong() * m
    }

    /** A JSON-safe rounded number (JSON has no NaN or infinity). */
    fun num(x: Double, digits: Int = 3): Any = if (x.isNaN() || x.isInfinite()) JSONObject.NULL else sig(x, digits)

    /** Rounds to `dp` decimal places, JSON-safe. */
    fun dp(x: Double, dp: Int): Any {
        if (x.isNaN() || x.isInfinite()) return JSONObject.NULL
        val m = 10.0.pow(dp)
        return (x * m).roundToLong() / m
    }

    /** The run list from kbench's {"list":true}: its "plan" if it has one, else one run per kernel and format or generator. */
    fun planFrom(list: JSONObject): List<JSONObject> {
        val plan = list.optJSONArray("plan")
        if (plan != null && plan.length() > 0) return (0 until plan.length()).mapNotNull { plan.optJSONObject(it) }
        val out = ArrayList<JSONObject>()
        val kernels = list.optJSONArray("kernels") ?: return out
        for (i in 0 until kernels.length()) {
            val k = kernels.optJSONObject(i) ?: continue
            val name = k.optString("kernel", k.optString("name", ""))
            if (name.isEmpty() || k.optString("note").contains("only")) continue
            val formats = k.optJSONArray("formats")
            val rngs = k.optJSONArray("rngs")
            when {
                formats != null && formats.length() > 0 -> for (f in 0 until formats.length())
                    out.add(JSONObject().put("kernel", name).put("format", formats.getString(f)).also {
                        if (rngs != null && rngs.length() > 0) it.put("rng", rngs.getString(0))
                    })
                rngs != null && rngs.length() > 0 -> for (r in 0 until rngs.length())
                    out.add(JSONObject().put("kernel", name).put("rng", rngs.getString(r)))
                else -> out.add(JSONObject().put("kernel", name))
            }
        }
        return out
    }

    /** Short label of one run config, e.g. "rust:heat/f32" or "cpp:rng/philox". */
    fun label(cfg: JSONObject): String {
        val k = cfg.optString("kernel", "?")
        val f = if (k.endsWith(":rng")) cfg.optString("rng", "") else cfg.optString("format", cfg.optString("rng", ""))
        return if (f.isEmpty()) k else "$k/$f"
    }

    /**
     * Seconds for each timed run so the kernel phase ends near its budget (B01/B02 phone side).
     * Per config: on each cluster one timed run plus a short repeat, then one all-core run.
     * `fixedS` is the measured cost of one config apart from its timed seconds.
     */
    fun secondsPerRun(remainingS: Double, remainingConfigs: Int, clusters: Int, fixedS: Double): Double {
        if (remainingConfigs <= 0) return 1.0
        val perConfig = remainingS / remainingConfigs
        return ((perConfig - fixedS) / (clusters + 1)).coerceIn(0.4, 3.0)
    }

    fun gzipBase64(text: String): String {
        val bytes = ByteArrayOutputStream()
        GZIPOutputStream(bytes).use { it.write(text.toByteArray(Charsets.UTF_8)) }
        return Base64.getEncoder().encodeToString(bytes.toByteArray())
    }

    fun unBase64Gunzip(code: String): String {
        val raw = Base64.getMimeDecoder().decode(code.trim())
        return GZIPInputStream(ByteArrayInputStream(raw)).use { String(it.readBytes(), Charsets.UTF_8) }
    }

    /**
     * The result code: gzip then base64 of the results JSON, under `limit` characters.
     * If too long, detail is dropped in a fixed order, and "trim" says how much.
     */
    fun resultCode(results: JSONObject, limit: Int = 4000): String {
        val r = JSONObject(results.toString())
        // Step bookkeeping stays on the phone; only the kernel phase's total time goes in the code.
        r.remove("done")
        r.optJSONObject("time")?.let { t ->
            val short = JSONObject()
            var kSum = 0.0
            for (key in t.keys()) if (key.startsWith("k")) kSum += t.optDouble(key, 0.0) else short.put(key, t.opt(key))
            short.put("k", dp(kSum, 0))
            r.put("time", short)
        }
        var code = gzipBase64(r.toString())
        val cuts = listOf<(JSONObject) -> Unit>(
            { it.optJSONObject("k")?.remove("mw") },
            { it.optJSONObject("sus")?.let { s -> s.remove("mhz"); s.remove("mA") } },
            { it.remove("err") },
            { it.optJSONObject("k")?.remove("ck") },
            { it.optJSONObject("sus")?.let { s -> for (key in listOf("hr", "st", "tC", "lvl")) s.remove(key) } },
            { it.optJSONObject("k")?.remove("ops") },
        )
        var level = 0
        while (code.length > limit && level < cuts.size) {
            cuts[level](r)
            level++
            r.put("trim", level)
            code = gzipBase64(r.toString())
        }
        return code
    }

    /**
     * B01/X11: checksums the phone should give, from the other pre-test's determinism.csv
     * (its ARM column; for Java, the x86 column, since Java arithmetic is the same everywhere).
     * Key: "lang:kernel|format|rng", as made by predictionKey.
     */
    fun predictions(csv: String): Map<String, String> {
        val lines = csv.lines().filter { it.isNotBlank() }
        if (lines.isEmpty()) return emptyMap()
        val head = lines.first().split(',')
        val ki = head.indexOf("kernel"); val fi = head.indexOf("format"); val ri = head.indexOf("rng"); val li = head.indexOf("lang")
        val ai = head.indexOf("arm_qemu_checksum"); val xi = head.indexOf("x86_checksum")
        if (listOf(ki, fi, ri, li, ai, xi).any { it < 0 }) return emptyMap()
        val out = HashMap<String, String>()
        for (line in lines.drop(1)) {
            val c = line.split(',')
            if (c.size < head.size) continue
            val pred = c[ai].ifEmpty { if (c[li] == "java") c[xi] else "" }
            if (pred.isNotEmpty()) out["${c[li]}:${c[ki]}|${c[fi]}|${c[ri]}"] = pred
        }
        return out
    }

    fun predictionKey(cfg: JSONObject): String {
        val kernel = cfg.optString("kernel", "?:?")
        val name = kernel.substringAfter(':')
        val format = if (name == "rng") "-" else cfg.optString("format", "-")
        val rng = if (name == "rng" || name == "walk") cfg.optString("rng", "splitmix") else "-"
        return "$kernel|$format|$rng"
    }

    /** '1' the phone gave the predicted checksum, '0' it didn't, '?' no prediction or no checksum. */
    fun predictionMark(predicted: String?, got: String?): Char = when {
        predicted.isNullOrEmpty() || got.isNullOrEmpty() -> '?'
        predicted.equals(got, ignoreCase = true) -> '1'
        else -> '0'
    }

    fun jsonInts(xs: Iterable<Int>): JSONArray = JSONArray().also { a -> xs.forEach { a.put(it) } }
}
