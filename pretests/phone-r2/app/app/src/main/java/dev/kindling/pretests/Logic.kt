package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.math.BigDecimal
import java.math.BigInteger
import java.util.Base64
import java.util.zip.GZIPInputStream
import java.util.zip.GZIPOutputStream
import kotlin.math.abs
import kotlin.math.floor
import kotlin.math.log10
import kotlin.math.pow
import kotlin.math.roundToLong

/** Round 2's plain logic, with no Android calls, unit-tested on the JVM (app/src/test). */
object Logic {
    // ------------------------------------------------------------------ numbers (from round 1)

    /** Rounds to `digits` significant digits. */
    fun sig(x: Double, digits: Int = 3): Double {
        if (x == 0.0 || x.isNaN() || x.isInfinite()) return x
        val e = floor(log10(abs(x))).toInt() - digits + 1
        val m = 10.0.pow(e)
        return if (e >= 0) (x / m).roundToLong() * m else (x / m).roundToLong() / 10.0.pow(-e)
    }

    /** A JSON-safe rounded number (JSON has no NaN or infinity). */
    fun num(x: Double, digits: Int = 3): Any = if (x.isNaN() || x.isInfinite()) JSONObject.NULL else sig(x, digits)

    /** Rounds to `dp` decimal places, JSON-safe. */
    fun dp(x: Double, dp: Int): Any {
        if (x.isNaN() || x.isInfinite()) return JSONObject.NULL
        val m = 10.0.pow(dp)
        return (x * m).roundToLong() / m
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

    // ------------------------------------------------------------------ the writer tests (B73)

    /** Words in a text, counted exactly as WriterTest counts them, so Gemma and Gemini Nano compare. */
    fun words(text: String): Int = text.split(Regex("\\s+")).count { it.isNotEmpty() }

    /**
     * WriterTest's per-prompt measures for a run timed elsewhere (Gemma): time to the first word,
     * total time, words, words a second after the first word (`wps`) and including the wait (`wps_all`).
     */
    fun measures(r: JSONObject, startMs: Long, firstMs: Long, endMs: Long, text: String): JSONObject {
        val words = words(text)
        val firstAt = if (firstMs > 0) firstMs else endMs
        return r.put("ttfw_ms", firstAt - startMs).put("total_ms", endMs - startMs).put("words", words)
            .put("wps", if (endMs > firstAt) round2(words * 1000.0 / (endMs - firstAt)) else JSONObject.NULL)
            .put("wps_all", round2(words * 1000.0 / maxOf(1L, endMs - startMs)))
    }

    private fun round2(x: Double) = Math.round(x * 100) / 100.0

    /** The same summary WriterTest gives each pass. */
    fun summary(runs: JSONArray): JSONObject {
        val ok = (0 until runs.length()).map { runs.getJSONObject(it) }.filter { !it.has("error_code") }
        fun median(xs: List<Double>): Any = if (xs.isEmpty()) JSONObject.NULL else xs.sorted()[xs.size / 2]
        return JSONObject().put("done", ok.size).put("tried", runs.length())
            .put("median_ttfw_ms", median(ok.map { it.optDouble("ttfw_ms") }))
            .put("median_wps", median(ok.mapNotNull { if (it.isNull("wps")) null else it.optDouble("wps") }))
            .put("max_total_ms", ok.maxOfOrNull { it.optLong("total_ms") } ?: JSONObject.NULL)
            .put("words", ok.sumOf { it.optInt("words") })
    }

    // ------------------------------------------------------------------ the owner's rating (B73 rule 6)

    /** The 3 records (feud killing, dream, sky-fire sign) in both voices. */
    val RATE_IDS = listOf("r04-doc", "r04-tra", "r06-doc", "r06-tra", "r08-doc", "r08-tra")
    val RECORD_TITLES = mapOf("r04" to "The feud killing", "r06" to "The dream", "r08" to "The sky-fire sign")

    data class RateItem(val model: String, val id: String, val text: String)

    private fun usableTexts(runs: JSONArray?): Map<String, String> {
        val out = LinkedHashMap<String, String>()
        if (runs == null) return out
        for (i in 0 until runs.length()) {
            val r = runs.optJSONObject(i) ?: continue
            val t = r.optString("text").trim()
            if (t.isNotEmpty() && !r.has("error_code")) out[r.optString("id")] = t
        }
        return out
    }

    /** Gemini Nano's texts: the main (FULL) pass, or the FAST pass if FULL wrote nothing at all. */
    fun nanoTexts(nano: JSONObject?): Map<String, String> {
        val passes = nano?.optJSONArray("passes") ?: return emptyMap()
        val byPref = (0 until passes.length()).mapNotNull { passes.optJSONObject(it) }
            .associate { it.optString("preference") to usableTexts(it.optJSONArray("runs")) }
        return byPref["FULL"]?.takeIf { it.isNotEmpty() } ?: byPref["FAST"] ?: emptyMap()
    }

    fun gemmaTexts(gm: JSONObject?): Map<String, String> = usableTexts(gm?.optJSONArray("runs"))

    /**
     * What the owner rates: Gemini Nano's texts for the 6 prompts, or Gemma's if Nano wrote none;
     * if both models wrote, both (12 texts at most). Model "n" is Gemini Nano, "g" Gemma.
     */
    fun ratingPlan(nano: JSONObject?, gm: JSONObject?): List<RateItem> {
        val n = nanoTexts(nano)
        val g = gemmaTexts(gm)
        val out = ArrayList<RateItem>()
        for ((model, texts) in listOf("n" to n, "g" to g)) {
            if (texts.isEmpty()) continue
            for (id in RATE_IDS) texts[id]?.let { out.add(RateItem(model, id, it)) }
        }
        return out
    }

    /** The facts a prompt gave the model: its DATA block, shown to the owner on request. */
    fun dataBlock(prompt: String): String = prompt.substringAfter("DATA:\n", prompt).trim()

    // ------------------------------------------------------------------ the Gemma download (B73)

    /** Free storage needed before downloading: 4 GB from scratch, else what is left plus a margin. */
    fun needFreeBytes(total: Long, have: Long, fresh: Long = 4_000_000_000L, margin: Long = 700_000_000L): Long =
        if (have <= 0L) fresh else maxOf(0L, total - have) + margin

    /** "bytes 100-199/1000" gives (100, 199, 1000); a star for the range gives (-1, -1, 1000); null if unreadable. */
    fun parseContentRange(h: String?): Triple<Long, Long, Long>? {
        val m = Regex("""bytes\s+(?:(\d+)-(\d+)|\*)/(\d+|\*)""").find(h?.trim() ?: return null) ?: return null
        val total = m.groupValues[3].toLongOrNull() ?: -1L
        val a = m.groupValues[1].toLongOrNull() ?: -1L
        val b = m.groupValues[2].toLongOrNull() ?: -1L
        return Triple(a, b, total)
    }

    /** The next Gemma backend to try: the first not yet tried, unless one already worked. */
    fun nextBackend(order: List<String>, tries: JSONArray?): String? {
        val tried = HashSet<String>()
        if (tries != null) for (i in 0 until tries.length()) {
            val t = tries.optJSONObject(i) ?: continue
            if (t.optBoolean("ok")) return t.optString("b")
            tried.add(t.optString("b"))
        }
        return order.firstOrNull { it !in tried }
    }

    fun gb(bytes: Long): String = "%.2f GB".format(bytes / 1e9)

    fun minutes(seconds: Double): String = when {
        seconds < 60 -> "under a minute"
        seconds < 90 -> "about 1 minute"
        else -> "about ${Math.round(seconds / 60.0)} minutes"
    }

    // ------------------------------------------------------------------ the result code

    /** Short keys for one writer run (Gemini Nano and Gemma); "voice" and "dark" follow from the id. */
    val RUN_KEYS = linkedMapOf(
        "id" to "i", "ttfw_ms" to "f", "total_ms" to "n", "words" to "w", "wps" to "s", "wps_all" to "a",
        "finish" to "e", "text" to "t", "battery_c" to "b", "thermal" to "h", "error_code" to "c", "error" to "x",
        "mem_mb" to "m", "tok" to "k", "chunks" to "ch",
    )
    private val DROPPED_RUN_KEYS = setOf("voice", "dark")

    /** Rounds every decimal number to `digits` significant digits, in place (integers and strings untouched). */
    fun roundDeep(v: Any?, digits: Int = 4): Any? = when (v) {
        is JSONObject -> {
            for (k in v.keys().asSequence().toList()) v.put(k, roundDeep(v.opt(k), digits))
            v
        }
        is JSONArray -> {
            for (i in 0 until v.length()) v.put(i, roundDeep(v.opt(i), digits))
            v
        }
        is Double -> num(v, digits)
        is Float -> num(v.toDouble(), digits)
        is BigDecimal -> if (v.signum() == 0 || v.scale() <= 0 || v.stripTrailingZeros().scale() <= 0) v else num(v.toDouble(), digits)
        is BigInteger, is Int, is Long -> v
        else -> v
    }

    /** One run with short keys, its text replaced by an index into the shared text table. */
    private fun compactRun(r: JSONObject, texts: TextTable, pass: String): JSONObject {
        val o = JSONObject()
        for (k in r.keys()) {
            if (k in DROPPED_RUN_KEYS) continue
            val v = r.opt(k)
            if (k == "text") {
                val t = (v as? String)?.trim().orEmpty()
                if (t.isNotEmpty()) o.put("t", texts.add(t, pass))
                continue
            }
            o.put(RUN_KEYS[k] ?: k, v)
        }
        return o
    }

    /** Texts, each kept once, with the passes that use it (to drop the least needed ones first). */
    class TextTable {
        val texts = ArrayList<String>()
        val users = ArrayList<MutableSet<String>>()
        private val index = HashMap<String, Int>()
        fun add(t: String, pass: String): Int {
            val i = index.getOrPut(t) { texts.add(t); users.add(HashSet()); texts.size - 1 }
            users[i].add(pass)
            return i
        }
    }

    private fun compactRuns(holder: JSONObject?, texts: TextTable, pass: String) {
        val runs = holder?.optJSONArray("runs") ?: return
        val out = JSONArray()
        for (i in 0 until runs.length()) runs.optJSONObject(i)?.let { out.put(compactRun(it, texts, pass)) }
        holder.put("runs", out)
    }

    /** The compact form of the results, before any trimming. */
    fun compact(results: JSONObject): Pair<JSONObject, TextTable> {
        val r = JSONObject(results.toString())
        // Step bookkeeping stays on the phone; the step times say which steps finished.
        r.remove("done")
        val texts = TextTable()
        r.optJSONObject("nano")?.optJSONArray("passes")?.let { passes ->
            for (i in 0 until passes.length()) passes.optJSONObject(i)?.let { p ->
                compactRuns(p, texts, "nano-" + p.optString("preference", "?"))
            }
        }
        compactRuns(r.optJSONObject("gm"), texts, "gemma")
        r.put("tx", JSONArray(texts.texts))
        roundDeep(r, 4)
        return r to texts
    }

    /** Removes texts used by no pass in `keep`, renumbering the others; dropped ones become -1. */
    private fun dropTexts(r: JSONObject, texts: TextTable, keep: (Set<String>) -> Boolean) {
        val newIndex = IntArray(texts.texts.size) { -1 }
        val kept = JSONArray()
        for (i in texts.texts.indices) if (keep(texts.users[i])) { newIndex[i] = kept.length(); kept.put(texts.texts[i]) }
        fun fix(holder: JSONObject?) {
            val runs = holder?.optJSONArray("runs") ?: return
            for (i in 0 until runs.length()) {
                val o = runs.optJSONObject(i) ?: continue
                if (o.has("t")) o.put("t", newIndex.getOrElse(o.optInt("t", -1)) { -1 })
            }
        }
        r.optJSONObject("nano")?.optJSONArray("passes")?.let { p -> for (i in 0 until p.length()) fix(p.optJSONObject(i)) }
        fix(r.optJSONObject("gm"))
        r.put("tx", kept)
    }

    private fun eachRun(r: JSONObject, f: (JSONObject) -> Unit) {
        fun each(holder: JSONObject?) {
            val runs = holder?.optJSONArray("runs") ?: return
            for (i in 0 until runs.length()) runs.optJSONObject(i)?.let(f)
        }
        r.optJSONObject("nano")?.optJSONArray("passes")?.let { p -> for (i in 0 until p.length()) each(p.optJSONObject(i)) }
        each(r.optJSONObject("gm"))
    }

    /**
     * Trim steps, least needed first, used only while the code is over its limit. The main texts
     * (Gemini Nano's FULL pass and Gemma) are never dropped: the lead fact-checks them.
     */
    private fun trims(texts: TextTable): List<(JSONObject) -> Unit> = listOf(
        { r -> dropTexts(r, texts) { users -> users.any { it != "nano-FAST" } } },
        { r ->
            r.optJSONObject("gm")?.optJSONArray("tries")?.let { a ->
                for (i in 0 until a.length()) a.optJSONObject(i)?.optJSONArray("log")?.let { log ->
                    val short = JSONArray(); for (j in 0 until minOf(2, log.length())) short.put(log.opt(j)); a.optJSONObject(i).put("log", short)
                }
            }
        },
        { r -> r.optJSONObject("snd")?.remove("per_cpu") },
        { r -> eachRun(r) { it.remove("b"); it.remove("h") } },
        { r -> r.optJSONObject("snd")?.remove("offline") },
        { r ->
            r.optJSONObject("nano")?.let { it.remove("device"); it.remove("before"); it.remove("after") }
            r.optJSONObject("gm")?.let { it.remove("before"); it.remove("after"); it.remove("memSeries") }
        },
        { r ->
            r.optJSONObject("st")?.let { st ->
                val zstd = st.optJSONObject("save")?.optJSONObject("custom-zstd")
                val keep = JSONObject().put("gen", st.opt("gen")).put("detail", st.opt("detail")).put("total_s", st.opt("total_s"))
                    .put("readZstdMs", zstd?.optJSONObject("read_ms")?.opt("median")).put("error", st.opt("error"))
                r.put("st", keep)
            }
        },
    )

    class Code(val code: String, val trim: Int, val json: String)

    /** The result code: gzip then base64 of the compact results, trimmed in a fixed order if over `limit`. */
    fun resultCode(results: JSONObject, limit: Int = 15_500): Code {
        val (r, texts) = compact(results)
        var json = r.toString()
        var code = gzipBase64(json)
        var level = 0
        val steps = trims(texts)
        while (code.length > limit && level < steps.size) {
            steps[level](r)
            level++
            r.put("trim", level)
            json = r.toString()
            code = gzipBase64(json)
        }
        return Code(code, level, json)
    }
}
