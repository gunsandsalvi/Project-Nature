package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.math.BigDecimal
import kotlin.random.Random

/** Round 2's plain logic, checked on the JVM (no phone needed). */
class LogicTest {
    // ------------------------------------------------------------------ synthetic results, with real texts

    /** Real model texts: the writer test's cloud stand-in runs (B73), 20 texts each. */
    private fun texts(file: String): List<String> {
        val dir = File(System.getProperty("b73.results") ?: error("b73.results not set"))
        val runs = JSONObject(File(dir, file).readText()).getJSONArray("runs")
        return (0 until runs.length()).map { runs.getJSONObject(it).getString("text") }
    }

    private val rnd = Random(73)

    private fun run(id: String, text: String?, gemma: Boolean = false) = JSONObject().put("id", id)
        .put("voice", if (id.endsWith("doc")) "documentary" else "tradition").put("dark", id.startsWith("r02") || id.startsWith("r04") || id.startsWith("r09"))
        .put("ttfw_ms", rnd.nextLong(300, 4000)).put("total_ms", rnd.nextLong(3000, 15000)).put("words", rnd.nextInt(60, 110))
        .put("wps", rnd.nextDouble(5.0, 30.0)).put("wps_all", rnd.nextDouble(5.0, 20.0)).put("finish", if (gemma) "done" else "stop")
        .put("battery_c", 30 + rnd.nextInt(80) / 10.0).put("thermal", rnd.nextInt(0, 2)).apply {
            if (text != null) put("text", text) else put("error_code", "BUSY").put("error", "busy, try later")
            if (gemma) put("mem_mb", rnd.nextLong(700, 2500)).put("tok", rnd.nextInt(600, 800)).put("chunks", rnd.nextInt(100, 200)).put("cv_ms", 12)
        }

    private fun ids() = WriterTest.PROMPTS.map { it.id }

    private fun nanoPass(pref: String, texts: List<String?>) = JSONObject().put("preference", pref).put("status_at_start", "available")
        .put("status", "available").put("base_model", "gemini-nano-v4").put("token_limit", 4000).put("warmup_ms", 812).put("warmup_ok", true)
        .put("runs", JSONArray(ids().zip(texts).map { (id, t) -> run(id, t) }))
        .put("summary", JSONObject().put("done", 20).put("tried", 20).put("median_ttfw_ms", 512).put("median_wps", 17.3))

    private fun vitals() = JSONObject().put("battery_c", 31.2).put("thermal", 0).put("headroom_10s", 0.41).put("avail_mem_mb", 6123).put("low_memory", false)

    private fun results(withFast: Boolean, withGemma: Boolean = true): JSONObject {
        val nanoTexts = texts("standin-qwen2.5-1.5b-v2.json")
        val passes = JSONArray().put(nanoPass("FULL", nanoTexts))
        if (withFast) passes.put(nanoPass("FAST", texts("standin-qwen2.5-1.5b-v1.json")))
        val nano = JSONObject().put("b73", "b73-r1").put("prompts", "v2").put("api", "mlkit genai-prompt 1.0.0-beta4")
            .put("device", JSONObject().put("model", "Google Pixel 11 Pro XL").put("soc", "Tensor G6").put("android", "17").put("sdk", 37)
                .put("security_patch", "2026-09-01").put("aicore", "1.2.3").put("play_services", "26.38.31").put("private_compute_services", "6.1"))
            .put("before", vitals()).put("after", vitals()).put("passes", passes).put("wall_ms", 251234)
        val r = JSONObject().put("v", 2).put("app", "r2").put("t0", 1790900000L)
            .put("dev", JSONObject().put("model", "Pixel 11 Pro XL").put("soc", "Google Tensor G6").put("rel", "17").put("sdk", 37).put("freeGB", 210.4))
            .put("st", JSONObject(File(System.getProperty("b04.cloud")!!).readText()))
            .put("snd", sound())
            .put("nano", nano)
            .put("time", JSONObject().put("dev", 0.1).put("st", 31.2).put("snd", 37.9).put("nano", 251.3).put("gm", 612.0))
            .put("done", JSONArray(listOf("dev", "st", "snd", "nano", "gm")))
        if (withGemma) {
            val gm = JSONObject().put("file", GemmaTest.FILE).put("free0", 120.5).put("need", 4.0)
                .put("dl", JSONObject().put("status", "done").put("got", 3313938293L).put("s", 140.2).put("mbps", 23.6).put("hashS", 6.1))
                .put("tries", JSONArray().put(JSONObject().put("b", "GOOGLE_TENSOR").put("ok", false).put("ms", 1234)
                    .put("e", "com.google.ai.edge.litertlm.LiteRtLmJniException: Failed to create engine: NOT_FOUND: No available engine for backend")
                    .put("log", JSONArray((1..12).map { "E litert: line $it about the dispatch library not being found in the native library folder" }))
                ).put(JSONObject().put("b", "GPU").put("ok", true).put("ms", 9876)))
                .put("be", "GPU").put("init_ms", 9876).put("cfg", 0)
                .put("runs", JSONArray(ids().zip(texts("standin-gemma-4-e2b-v2.json")).map { (id, t) -> run(id, t, gemma = true) }))
                .put("summary", JSONObject().put("done", 20)).put("before", vitals()).put("after", vitals())
                .put("mem", JSONObject().put("init", JSONArray(listOf(1500, 300, 1200, 900, 400))).put("write", JSONArray(listOf(1800, 400, 1400, 1100, 500))))
                .put("status", "done")
            r.put("gm", gm)
        }
        r.put("rate", JSONArray().put(JSONObject().put("m", "n").put("i", "r04-doc").put("r", 1)))
        return r.put("auto", true).put("finished", true)
    }

    /** A sound result shaped like the B74 library's (offline, per core, the stream and three phases). */
    private fun sound(): JSONObject {
        fun d() = rnd.nextDouble()
        val offline = JSONArray(listOf(8, 32, 128).map { JSONObject().put("voices", it).put("core_share", d() / 10).put("voices_per_core", 2900 + d()).put("active", it.toDouble()) })
        val perCpu = JSONArray((0 until 7).map { JSONObject().put("cpu", it).put("pinned", true).put("seen_on", it).put("voices", 32).put("voices_per_core", 1000 + 3000 * d()) })
        val phases = JSONArray(listOf(8, 32, 128).map {
            JSONObject().put("voices", it).put("callbacks", 8000).put("xruns", 0).put("work_us_mean", 30 * d()).put("work_us_p50", 30 * d())
                .put("work_us_p99", 90 * d()).put("work_us_max", 300 * d()).put("budget_us", 1000.0).put("load_mean", d() / 10).put("load_p99", d() / 5)
                .put("thread_cpu_share", d() / 10).put("latency_ms_p50", 20 + d()).put("latency_ms_min", 18 + d()).put("latency_ms_max", 25 + d())
                .put("latency_samples", 160).put("cpus", JSONArray().put(JSONArray(listOf(5, 8000))))
        })
        val stream = JSONObject().put("sample_rate", 48000).put("channels", 2).put("format_float", true).put("frames_per_burst", 96)
            .put("buffer_frames", 192).put("buffer_capacity", 3072).put("exclusive", true).put("low_latency", true)
        return JSONObject().put("block", "B74").put("approach", "Modal").put("sample_rate_offline", 48000.0).put("offline", offline)
            .put("per_cpu", perCpu).put("audio", JSONObject().put("stream", stream).put("phases", phases).put("stream_error", JSONObject.NULL))
            .put("elapsed_s", 33.123456789).put("vol", "7/15")
    }

    // ------------------------------------------------------------------ the result code

    @Test fun fullSizeCodeFitsAndKeepsEveryMainText() {
        val r = results(withFast = false)
        val c = Logic.resultCode(r)
        println("round-2 code, both models, no FAST pass: results ${r.toString().length} chars -> code ${c.code.length} chars, trim ${c.trim}")
        assertTrue("code is ${c.code.length} chars", c.code.length <= 15_500)
        assertEquals(0, c.trim)
        val back = JSONObject(Logic.unBase64Gunzip(c.code))
        val tx = back.getJSONArray("tx")
        // Each distinct text once (the small stand-in model sometimes wrote the same text for both voices).
        val distinct = (texts("standin-qwen2.5-1.5b-v2.json") + texts("standin-gemma-4-e2b-v2.json")).map { it.trim() }.toSet()
        assertEquals(distinct.size, tx.length())
        // Every text comes back exactly, through its index.
        val nanoRuns = back.getJSONObject("nano").getJSONArray("passes").getJSONObject(0).getJSONArray("runs")
        val want = texts("standin-qwen2.5-1.5b-v2.json")
        for (i in 0 until 20) {
            val run = nanoRuns.getJSONObject(i)
            assertEquals(WriterTest.PROMPTS[i].id, run.getString("i"))
            assertEquals(want[i].trim(), tx.getString(run.getInt("t")))
        }
        val gemmaRuns = back.getJSONObject("gm").getJSONArray("runs")
        assertEquals(texts("standin-gemma-4-e2b-v2.json")[3].trim(), tx.getString(gemmaRuns.getJSONObject(3).getInt("t")))
        // The storage hashes the lead compares come back as they are.
        assertEquals("895e636495687a48", back.getJSONObject("st").getJSONObject("gen").getJSONObject("plates").getString("hash"))
        assertEquals("5e3b0c482d789a49", back.getJSONObject("st").getJSONObject("detail").getJSONObject("threads_1").getString("hash"))
        assertEquals("GPU", back.getJSONObject("gm").getString("be"))
        assertFalse(back.has("done"))
        assertTrue(back.getBoolean("finished"))
    }

    /** Writes a sample code (both models, a FAST pass, a rating) for checking tools/decode-result.py. */
    @Test fun writesASampleCodeForTheDecoder() {
        val dir = File(System.getProperty("r2.sampleDir") ?: return).apply { mkdirs() }
        val r = results(withFast = true)
        File(dir, "code.txt").writeText(Logic.resultCode(r).code)
        File(dir, "results.json").writeText(r.toString())
        File(dir, "code-trimmed.txt").writeText(Logic.resultCode(r, limit = 9_000).code)
    }

    @Test fun aFastPassStillFitsButAnOversizedCodeDropsItsTextsFirst() {
        val r = results(withFast = true)
        val fits = Logic.resultCode(r)
        println("round-2 code with a full FAST pass too: code ${fits.code.length} chars, trim ${fits.trim}")
        assertTrue("code is ${fits.code.length} chars", fits.code.length <= 15_500)
        // Over a (lower) limit, the FAST pass's texts go first.
        val c = Logic.resultCode(r, limit = fits.code.length - 1000)
        val back = JSONObject(Logic.unBase64Gunzip(c.code))
        assertEquals(1, c.trim)
        assertEquals(c.trim, back.getInt("trim"))
        val tx = back.getJSONArray("tx")
        val main = (texts("standin-qwen2.5-1.5b-v2.json") + texts("standin-gemma-4-e2b-v2.json")).map { it.trim() }.toSet()
        assertEquals(main.size, tx.length()) // FULL and Gemma kept; FAST dropped
        val fast = back.getJSONObject("nano").getJSONArray("passes").getJSONObject(1).getJSONArray("runs")
        for (i in 0 until fast.length()) {
            // A FAST text left in the code is one a main pass also wrote (kept once, shared).
            val ti = fast.getJSONObject(i).getInt("t")
            assertTrue(ti == -1 || tx.getString(ti) in main)
        }
        assertTrue((0 until fast.length()).count { fast.getJSONObject(it).getInt("t") == -1 } >= 15)
        val full = back.getJSONObject("nano").getJSONArray("passes").getJSONObject(0).getJSONArray("runs")
        for (i in 0 until full.length()) assertTrue(full.getJSONObject(i).getInt("t") >= 0)
        val gm = back.getJSONObject("gm").getJSONArray("runs")
        for (i in 0 until gm.length()) assertTrue(gm.getJSONObject(i).getInt("t") >= 0)
        // However small the limit, the main texts stay; every trim step is applied, in order.
        val tiny = Logic.resultCode(r, limit = 100)
        val t = JSONObject(Logic.unBase64Gunzip(tiny.code))
        assertEquals(7, t.getInt("trim"))
        assertEquals(main.size, t.getJSONArray("tx").length())
        assertTrue(t.getJSONObject("st").has("gen") && t.getJSONObject("st").has("detail"))
        assertFalse(t.getJSONObject("snd").has("per_cpu") || t.getJSONObject("snd").has("offline"))
        assertTrue(t.getJSONObject("snd").has("audio"))
    }

    @Test fun textsAreKeptOnceAndKeysAreShort() {
        val t = texts("standin-gemma-4-e2b-v2.json")
        val r = results(withFast = false)
        // Gemini Nano writing the same text as Gemma for one prompt: the text is kept once.
        r.getJSONObject("nano").getJSONArray("passes").getJSONObject(0).getJSONArray("runs").getJSONObject(0).put("text", t[0])
        val (c, table) = Logic.compact(r)
        val distinct = (listOf(t[0]) + texts("standin-qwen2.5-1.5b-v2.json").drop(1) + t).map { it.trim() }.toSet()
        assertEquals(distinct.size, c.getJSONArray("tx").length())
        assertEquals(setOf("nano-FULL", "gemma"), table.users[table.texts.indexOf(t[0].trim())])
        val run = c.getJSONObject("gm").getJSONArray("runs").getJSONObject(0)
        assertFalse(run.has("text") || run.has("voice") || run.has("dark") || run.has("ttfw_ms"))
        assertTrue(run.has("f") && run.has("n") && run.has("w") && run.has("m") && run.has("k"))
        // An error run keeps its code and message, and has no text.
        r.getJSONObject("nano").getJSONArray("passes").getJSONObject(0).getJSONArray("runs").put(run("r10-tra", null))
        val (c2, _) = Logic.compact(r)
        val last = c2.getJSONObject("nano").getJSONArray("passes").getJSONObject(0).getJSONArray("runs").let { it.getJSONObject(it.length() - 1) }
        assertEquals("BUSY", last.getString("c"))
        assertFalse(last.has("t"))
    }

    @Test fun decimalsAreRoundedNotIntegersOrStrings() {
        val o = JSONObject().put("a", 0.002724963000000006).put("b", 1790900000L).put("c", "895e636495687a48")
            .put("d", BigDecimal("117.560488")).put("e", BigDecimal("20")).put("f", JSONArray().put(33.123456789))
        Logic.roundDeep(o, 4)
        assertEquals(0.002725, o.getDouble("a"), 0.0)
        assertEquals(1790900000L, o.getLong("b"))
        assertEquals("895e636495687a48", o.getString("c"))
        assertEquals(117.6, o.getDouble("d"), 1e-9)
        assertEquals(20, o.getInt("e"))
        assertEquals(33.12, o.getJSONArray("f").getDouble(0), 1e-9)
    }

    // ------------------------------------------------------------------ rating plan

    @Test fun ratingUsesNanoThenGemmaAndBothWhenBothWrote() {
        val r = results(withFast = false)
        val both = Logic.ratingPlan(r.getJSONObject("nano"), r.getJSONObject("gm"))
        assertEquals(12, both.size)
        assertEquals(Logic.RATE_IDS, both.take(6).map { it.id })
        assertEquals(List(6) { "n" } + List(6) { "g" }, both.map { it.model })
        val onlyNano = Logic.ratingPlan(r.getJSONObject("nano"), null)
        assertEquals(6, onlyNano.size)
        val onlyGemma = Logic.ratingPlan(JSONObject().put("passes", JSONArray().put(nanoPass("FULL", List(20) { null }))), r.getJSONObject("gm"))
        assertEquals(List(6) { "g" }, onlyGemma.map { it.model })
        assertTrue(Logic.ratingPlan(null, JSONObject().put("status", "skipped: not on Wi-Fi")).isEmpty())
    }

    @Test fun ratingFallsBackToTheFastPassOnlyIfFullWroteNothing() {
        val fullEmpty = nanoPass("FULL", List(20) { null })
        val fast = nanoPass("FAST", texts("standin-qwen2.5-1.5b-v1.json"))
        val nano = JSONObject().put("passes", JSONArray().put(fullEmpty).put(fast))
        val plan = Logic.ratingPlan(nano, null)
        assertEquals(6, plan.size)
        assertEquals(texts("standin-qwen2.5-1.5b-v1.json")[6].trim(), plan[0].text) // r04-doc is the 7th prompt
        // A refused dark text is simply left out of the rating.
        val texts = texts("standin-qwen2.5-1.5b-v2.json").toMutableList<String?>()
        texts[6] = null
        val plan2 = Logic.ratingPlan(JSONObject().put("passes", JSONArray().put(nanoPass("FULL", texts))), null)
        assertEquals(listOf("r04-tra", "r06-doc", "r06-tra", "r08-doc", "r08-tra"), plan2.map { it.id })
    }

    // ------------------------------------------------------------------ writer measures, download helpers

    @Test fun gemmaIsMeasuredLikeGeminiNano() {
        assertEquals(5, Logic.words("  The band left   Oru behind.\n "))
        val r = Logic.measures(JSONObject(), startMs = 1000, firstMs = 1500, endMs = 6500, text = "one two three four five six seven eight nine ten")
        assertEquals(500L, r.getLong("ttfw_ms"))
        assertEquals(5500L, r.getLong("total_ms"))
        assertEquals(10, r.getInt("words"))
        assertEquals(2.0, r.getDouble("wps"), 0.0)
        assertEquals(1.82, r.getDouble("wps_all"), 0.0)
        val none = Logic.measures(JSONObject(), 1000, -1, 2000, "")
        assertEquals(1000L, none.getLong("ttfw_ms"))
        assertTrue(none.isNull("wps"))
        val runs = JSONArray().put(JSONObject().put("ttfw_ms", 400).put("wps", 10.0).put("total_ms", 5000).put("words", 50))
            .put(JSONObject().put("ttfw_ms", 600).put("wps", 12.0).put("total_ms", 7000).put("words", 70))
            .put(JSONObject().put("error_code", "TIMEOUT"))
        val s = Logic.summary(runs)
        assertEquals(2, s.getInt("done"))
        assertEquals(3, s.getInt("tried"))
        assertEquals(600.0, s.getDouble("median_ttfw_ms"), 0.0)
        assertEquals(120, s.getInt("words"))
    }

    @Test fun downloadHelpers() {
        assertEquals(Triple(100L, 199L, 1000L), Logic.parseContentRange("bytes 100-199/1000"))
        assertEquals(Triple(-1L, -1L, 3313938293L), Logic.parseContentRange("bytes */3313938293"))
        assertNull(Logic.parseContentRange("nonsense"))
        assertNull(Logic.parseContentRange(null))
        assertEquals(4_000_000_000L, Logic.needFreeBytes(3_313_938_293L, 0))
        assertEquals(1_000_000_000L + 700_000_000L, Logic.needFreeBytes(3_000_000_000L, 2_000_000_000L))
        assertEquals("about 3 minutes", Logic.minutes(170.0))
        assertEquals("under a minute", Logic.minutes(20.0))
    }

    @Test fun backendOrderAndResumingAfterACrash() {
        val order = listOf("GOOGLE_TENSOR", "GPU")
        assertEquals("GOOGLE_TENSOR", Logic.nextBackend(order, null))
        val tries = JSONArray().put(JSONObject().put("b", "GOOGLE_TENSOR").put("ok", false))
        assertEquals("GPU", Logic.nextBackend(order, tries))
        tries.put(JSONObject().put("b", "GPU").put("ok", false))
        assertNull(Logic.nextBackend(order, tries))
        // A backend that worked before the app closed is started again.
        val worked = JSONArray().put(JSONObject().put("b", "GOOGLE_TENSOR").put("ok", true))
        assertEquals("GOOGLE_TENSOR", Logic.nextBackend(order, worked))
    }
}
