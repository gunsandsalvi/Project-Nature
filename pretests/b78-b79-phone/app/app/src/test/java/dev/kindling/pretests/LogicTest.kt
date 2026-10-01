package dev.kindling.pretests

import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import kotlin.random.Random

/** B78/B79: the app's plain logic, checked on the JVM (no phone needed). */
class LogicTest {
    @Test fun clustersByFrequency() {
        // A made-up 7-core phone: 2 slow, 4 middle, 1 fast.
        val f = mapOf(0 to 1_800_000L, 1 to 1_800_000L, 2 to 2_600_000L, 3 to 2_600_000L, 4 to 2_600_000L, 5 to 2_600_000L, 6 to 3_700_000L)
        val c = Logic.clustersByFreq(f, 7)!!
        assertEquals(listOf(listOf(0, 1), listOf(2, 3, 4, 5), listOf(6)), c.map { it.cpus })
        assertEquals(3_700_000L, c.last().maxKHz)
        assertEquals(listOf(1, 5, 6), c.map { Logic.representative(it) })
        assertNull("an unreadable CPU gives no answer", Logic.clustersByFreq(f - 3, 7))
    }

    @Test fun clustersByCoreType() {
        val info = (0 until 4).joinToString("\n\n") { "processor\t: $it\nCPU part\t: ${if (it < 3) "0xd80" else "0xd85"}" }
        assertEquals(listOf(listOf(0, 1, 2), listOf(3)), Logic.clustersByPart(info, 4)!!.map { it.cpus })
        assertNull(Logic.clustersByPart("processor : 0", 2))
    }

    @Test fun clustersBySpeed() {
        val c = Logic.clustersBySpeed(doubleArrayOf(1.0, 1.02, 2.0, 2.05, 1.99, 3.5))
        assertEquals(listOf(listOf(0, 1), listOf(2, 3, 4), listOf(5)), c.map { it.cpus })
    }

    @Test fun percentilesAndRounding() {
        val xs = (1..100).map { it.toDouble() }.toDoubleArray()
        assertEquals(50.0, Logic.percentile(xs, 50.0), 0.0)
        assertEquals(99.0, Logic.percentile(xs, 99.0), 0.0)
        assertEquals(100.0, Logic.percentile(xs, 100.0), 0.0)
        assertEquals(1.23e9, Logic.sig(1_234_567_890.0), 1.0)
        assertEquals(JSONObject.NULL, Logic.num(Double.NaN))
        assertEquals(8.33, Logic.dp(8.3333, 2) as Double, 0.0)
    }

    @Test fun planFromTheRealLibraryList() {
        val list = JSONObject(javaClass.getResource("/kbench-list.json")!!.readText())
        val plan = Logic.planFrom(list)
        assertEquals(list.getJSONArray("plan").length(), plan.size)
        assertEquals("rust:heat/f32", Logic.label(plan[0]))
        assertTrue(plan.any { Logic.label(it) == "cpp:rng/philox" })
        // Without a plan, one run per kernel and format, skipping comparison-only kernels.
        list.remove("plan")
        val derived = Logic.planFrom(list)
        assertTrue(derived.size > 20)
        assertTrue(derived.none { it.getString("kernel").startsWith("cppnc") })
    }

    @Test fun planFromManagedList() {
        val managed = JSONObject("""{"kernels":[{"kernel":"java:heat","formats":["f32","fx32"]},{"kernel":"java:rng","rngs":["splitmix"]}]}""")
        assertEquals(listOf("java:heat/f32", "java:heat/fx32", "java:rng/splitmix"), Logic.planFrom(managed).map { Logic.label(it) })
    }

    @Test fun budgetGivesAboutOneSecondRuns() {
        // 45 configs, 3 clusters, 300 s: each config gets about 6.7 s.
        val s = Logic.secondsPerRun(300.0, 45, 3, 2.5)
        assertTrue("got $s", s in 0.9..1.2)
        assertEquals(0.4, Logic.secondsPerRun(10.0, 45, 3, 2.5), 0.0)
        assertEquals(3.0, Logic.secondsPerRun(1000.0, 5, 3, 2.5), 0.0)
    }

    /** A full-size round-1 result must fit the ~4 KB result code, and decode back exactly. */
    @Test fun fullSizeResultCodeFitsAndRoundTrips() {
        val r = syntheticResults(configs = 45, clusters = 3)
        val json = r.toString()
        val code = Logic.resultCode(r)
        println("results JSON ${json.length} chars -> code ${code.length} chars, trim=${JSONObject(Logic.unBase64Gunzip(code)).optInt("trim")}")
        assertTrue("code is ${code.length} chars", code.length <= 4000)
        val back = JSONObject(Logic.unBase64Gunzip(code))
        assertEquals(r.getJSONObject("dev").toString(), back.getJSONObject("dev").toString())
        assertEquals(r.getJSONObject("sus").getDouble("r10"), back.getJSONObject("sus").getDouble("r10"), 0.0)
    }

    @Test fun oversizedResultIsTrimmedInOrder() {
        val r = syntheticResults(configs = 200, clusters = 4)
        val code = Logic.resultCode(r, limit = 4000)
        val back = JSONObject(Logic.unBase64Gunzip(code))
        assertTrue(back.optInt("trim") >= 1)
        assertTrue(back.has("dev") && back.has("sus") && back.has("fp"))
    }

    private fun syntheticResults(configs: Int, clusters: Int): JSONObject {
        val rnd = Random(7)
        fun ops() = Logic.num(rnd.nextDouble(1e7, 4e9))
        val k = JSONObject().put("cl", JSONArray((0 until clusters).map { JSONArray().put(it * 2 + 1).put(1800 + 900 * it) }))
            .put("lab", JSONArray()).put("s", JSONArray()).put("ops", JSONArray()).put("mw", JSONArray()).put("ck", JSONArray())
        val sb = StringBuilder()
        repeat(configs) { i ->
            k.getJSONArray("lab").put(listOf("rust", "cpp")[i % 2] + ":" + listOf("heat", "walk", "sum", "learn", "rng")[i % 5] + "/" + listOf("f32", "f64", "fx32", "fx64")[i % 4])
            k.getJSONArray("s").put(0.95)
            k.getJSONArray("ops").put(JSONArray((0..clusters).map { ops() }))
            k.getJSONArray("mw").put(JSONArray((0..clusters).map { Logic.num(rnd.nextDouble(300.0, 9000.0), 2) }))
            k.getJSONArray("ck").put("%08x".format(rnd.nextInt()))
            sb.append('1')
        }
        k.put("rep", sb.toString()).put("x", sb.toString()).put("pinMiss", 0)
        val w = 20
        fun series(f: () -> Any) = JSONArray((0 until w).map { f() })
        val sus = JSONObject().put("k", "rust:heat/f32").put("s", 600).put("w", 30).put("calls", 540)
            .put("ops", series { ops() }).put("hr", series { Logic.dp(rnd.nextDouble(0.3, 1.1), 2) })
            .put("st", series { rnd.nextInt(0, 4) }).put("tC", series { Logic.dp(rnd.nextDouble(28.0, 45.0), 1) })
            .put("mA", series { Logic.num(-rnd.nextDouble(800.0, 2500.0)) }).put("lvl", series { rnd.nextInt(60, 90) })
            .put("mhz", JSONArray((0 until clusters).map { series { Logic.num(rnd.nextDouble(800.0, 3700.0)) } }))
            .put("r10", 0.712).put("tMod", 214).put("stMax", 3).put("hrMax", 1.04).put("hrNaN", 0)
            .put("dLvl", 5).put("dmAh", 238).put("pctH", 29.5).put("pctHi", 31.2).put("capmAh", 4950).put("errs", 0)
        val dev = JSONObject("""{"model":"Pixel 11 Pro XL","mfr":"Google","dev":"xxxx","soc":"Google Tensor G6","rel":"17","sdk":37,
            "patch":"2026-09-05","page":16384,"ncpu":7,"jcpu":7,"maxFreqReadable":7,"curFreqReadable":7,"clSrc":"freq",
            "cl":[{"cpus":[0,1],"mhz":2000},{"cpus":[2,3,4,5],"mhz":2900},{"cpus":[6],"mhz":3800}],"parts":["0xd8a","0xd8a","0xd87","0xd87","0xd87","0xd87","0xd85"],
            "mc":256,"lmc":1024,"ramMiB":15400,"availMiB":9800,"thresholdMiB":800,"lowRam":false,"th0":0,"hr0":0.42,
            "hrThresholds":{"1":0.6,"2":0.8,"3":1,"4":1.1,"5":1.2,"6":1.3},"bat0":84,"plug":0,"tC0":29.5,"mV0":4210,"chargeUah0":4100000,"ua0":-420000,
            "disp":{"w":1344,"h":2992,"hz":120,"modes":[1,10,60,120]}}""")
        return JSONObject().put("v", 1).put("app", "r1").put("lib", "real").put("started", 1790000000L)
            .put("dev", dev)
            .put("lib", JSONObject("""{"loaded":true,"kphone":true,"managed":true,"plan":45,"clusters":[[0,1],[2,3,4,5],[6]],"selftest":true}"""))
            .put("idle", JSONObject("""{"mA":-410,"mV":4200,"n":50,"mw":1722}"""))
            .put("fp", JSONObject("""{"req":120,"hz":120,"n":2397,"fps":119.8,"p50":8.33,"p90":8.4,"p99":9.1,"p999":16.6,"max":25,"miss":3,"missPct":0.13,"gl":"PowerVR D-Series DXT-48-1536 | OpenGL ES 3.2 build 25.1","done":true}"""))
            .put("k", k).put("sus", sus)
            .put("mem", JSONObject("""{"end":"killed","mib":9728,"dyingAt":9984,"lowAt":7168,"rssMax":9850,"availLast":620,"s":41.2,"availGiB":[9000,8000,7000,6000,5000,4000,3000,2000,1000]}"""))
            .put("done", JSONArray((0 until configs + 6).map { "k$it" }))
            .put("time", JSONObject().apply { repeat(configs) { put("k$it", 6.6) } })
            .put("finished", true)
    }
}
