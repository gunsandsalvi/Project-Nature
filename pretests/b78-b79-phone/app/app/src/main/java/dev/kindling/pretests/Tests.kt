package dev.kindling.pretests

import android.os.SystemClock
import org.json.JSONArray
import org.json.JSONObject
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import kotlin.math.abs

/** B79 and the phone side of B01/B02: the tests that need no screen. */
class Tests(private val probe: Probe, private val store: Store) {

    class Run(
        val ok: Boolean, val opsPerSec: Double, val checksum: String, val consistent: Boolean,
        val cpusSeen: List<Int>, val meanUa: Double, val mv: Int, val wallS: Double, val error: String?,
    ) {
        /** Average battery power during the run, in mW (current sign ignored). */
        val mw: Double get() = if (meanUa.isNaN() || mv <= 0) Double.NaN else abs(meanUa) * mv / 1e6
    }

    private fun failed(why: String, mv: Int, wall: Double) =
        Run(false, Double.NaN, "", false, emptyList(), Double.NaN, mv, wall, why)

    /**
     * One kernel run on a fresh thread: pinned to `cpu`, or with one worker per CPU when `cpu` is null.
     * The battery current is sampled every 100 ms meanwhile, for energy per operation.
     */
    fun runOne(base: JSONObject, cpu: Int?, threads: Int, seconds: Double, managed: Boolean): Run {
        val cfg = JSONObject(base.toString()).put("threads", threads).put("seconds", Logic.dp(seconds, 2))
        if (!managed) cfg.put("cpus", if (cpu != null) JSONArray().put(cpu) else Logic.jsonInts(0 until probe.ncpu))
        val cfgText = cfg.toString()
        var out: String? = null
        var err: Throwable? = null
        val worker = Thread {
            try {
                if (cpu != null) Bench.pinToCpu(cpu) // threads the library starts inherit this pin
                out = if (managed) Managed.run(cfgText) else Bench.run(cfgText)
            } catch (t: Throwable) {
                err = t
            }
        }
        worker.isDaemon = true
        val mv = probe.voltageMv()
        val t0 = SystemClock.elapsedRealtime()
        worker.start()
        var sum = 0.0
        var n = 0
        val limitMs = ((seconds * 4 + 30) * 1000).toLong()
        while (worker.isAlive && SystemClock.elapsedRealtime() - t0 < limitMs) {
            val ua = probe.currentUa()
            if (ua != Int.MIN_VALUE) { sum += ua; n++ }
            worker.join(100)
        }
        val wall = (SystemClock.elapsedRealtime() - t0) / 1000.0
        if (worker.isAlive) return failed("timeout", mv, wall)
        err?.let { return failed(it.toString().take(120), mv, wall) }
        val r = try { JSONObject(out ?: "{}") } catch (t: Throwable) { return failed("bad json", mv, wall) }
        if (r.has("error")) return failed(r.optString("error").take(120), mv, wall)
        val seen = r.optJSONArray("cpus_seen")?.let { a -> (0 until a.length()).map { a.optInt(it, -1) } } ?: emptyList()
        return Run(
            true, r.optDouble("ops_per_sec", Double.NaN), r.optString("checksum", ""),
            r.optBoolean("reps_consistent", true), seen, if (n > 0) sum / n else Double.NaN, mv, wall, null,
        )
    }

    /** Idle baseline: battery current with the screen on and nothing running, for net energy. */
    fun idle(seconds: Int): JSONObject {
        var sum = 0.0
        var n = 0
        val mv = probe.voltageMv()
        val end = SystemClock.elapsedRealtime() + seconds * 1000L
        while (SystemClock.elapsedRealtime() < end) {
            val ua = probe.currentUa()
            if (ua != Int.MIN_VALUE) { sum += ua; n++ }
            Thread.sleep(200)
        }
        val ua = if (n > 0) sum / n else Double.NaN
        return JSONObject().put("mA", Logic.num(ua / 1000)).put("mV", mv).put("n", n)
            .put("mw", Logic.num(if (ua.isNaN()) Double.NaN else abs(ua) * mv / 1e6))
    }

    /**
     * B79 sustained load: all cores on one kernel for `durationS` seconds, sampled once a second
     * (throughput, thermal headroom and status, battery level, current and temperature, core clocks).
     * Returns (compact summary for the result code, per-second detail for full.json).
     */
    fun sustained(base: JSONObject, durationS: Int, clusters: List<Logic.Cluster>, tick: (Int) -> Unit): Pair<JSONObject, JSONObject> {
        val ncpu = probe.ncpu
        val cfgText = JSONObject(base.toString()).put("threads", ncpu).put("seconds", 1.0)
            .put("cpus", Logic.jsonInts(0 until ncpu)).toString()
        val stop = AtomicBoolean(false)
        val calls = ArrayList<DoubleArray>() // [end s, ops, elapsed s]
        val loadErrors = AtomicInteger(0)
        var lastError = ""
        val t0 = SystemClock.elapsedRealtime()
        val loader = Thread {
            while (!stop.get()) {
                try {
                    val r = JSONObject(Bench.run(cfgText))
                    if (r.has("error")) {
                        loadErrors.incrementAndGet(); lastError = r.optString("error").take(100); Thread.sleep(500)
                    } else synchronized(calls) {
                        calls.add(doubleArrayOf((SystemClock.elapsedRealtime() - t0) / 1000.0, r.optDouble("ops", 0.0), r.optDouble("elapsed_s", 0.0)))
                    }
                } catch (t: Throwable) {
                    loadErrors.incrementAndGet(); lastError = t.toString().take(100)
                    try { Thread.sleep(500) } catch (_: InterruptedException) { return@Thread }
                }
            }
        }
        loader.isDaemon = true
        loader.start()
        val lvl0 = probe.level()
        val q0 = probe.chargeUah()
        // per second: t, headroom, status, level, current uA, mV, temp C, then one clock (MHz) per CPU
        val samples = ArrayList<DoubleArray>()
        for (s in 1..durationS) {
            val wait = t0 + s * 1000L - SystemClock.elapsedRealtime()
            if (wait > 0) Thread.sleep(wait)
            val row = DoubleArray(7 + ncpu)
            row[0] = (SystemClock.elapsedRealtime() - t0) / 1000.0
            row[1] = probe.headroom().toDouble()
            row[2] = probe.thermalStatus().toDouble()
            row[3] = probe.level().toDouble()
            row[4] = probe.currentUa().let { if (it == Int.MIN_VALUE) Double.NaN else it.toDouble() }
            row[5] = probe.voltageMv().toDouble()
            row[6] = probe.tempDeciC() / 10.0
            for (c in 0 until ncpu) row[7 + c] = probe.curFreqKHz(c) / 1000.0
            samples.add(row)
            tick(s)
        }
        stop.set(true)
        loader.join(5000)
        val lvl1 = probe.level()
        val q1 = probe.chargeUah()
        val callList = synchronized(calls) { ArrayList(calls) }

        val w = 30
        val nw = (durationS + w - 1) / w
        fun opsRate(from: Double, to: Double): Double {
            val cs = callList.filter { it[0] > from && it[0] <= to }
            val el = cs.sumOf { it[2] }
            return if (el > 0) cs.sumOf { it[1] } / el else Double.NaN
        }
        fun win(i: Int, col: Int, f: (List<Double>) -> Double): Double {
            val xs = samples.filter { it[0] > i * w && it[0] <= (i + 1) * w }.map { it[col] }.filter { !it.isNaN() }
            return if (xs.isEmpty()) Double.NaN else f(xs)
        }
        val ops = JSONArray(); val hr = JSONArray(); val st = JSONArray(); val tC = JSONArray()
        val mA = JSONArray(); val lvl = JSONArray()
        for (i in 0 until nw) {
            ops.put(Logic.num(opsRate(i * w.toDouble(), (i + 1) * w.toDouble())))
            hr.put(Logic.dp(win(i, 1) { it.average() }, 2))
            st.put(Logic.num(win(i, 2) { it.max() }))
            tC.put(Logic.dp(win(i, 6) { it.average() }, 1))
            mA.put(Logic.num(win(i, 4) { it.average() } / 1000))
            lvl.put(Logic.num(win(i, 3) { it.last() }))
        }
        val clocksReadable = samples.any { r -> (0 until ncpu).any { r[7 + it] > 0 } }
        val mhz = JSONArray()
        if (clocksReadable) for (c in clusters) {
            val a = JSONArray()
            for (i in 0 until nw) a.put(Logic.num(c.cpus.map { cpu -> win(i, 7 + cpu) { it.average() } }.filter { !it.isNaN() }.average(), 3))
            mhz.put(a)
        }
        val first = opsRate(0.0, 60.0)
        val last = opsRate(durationS - 60.0, durationS.toDouble())
        val tModerate = samples.firstOrNull { it[2] >= 2 }?.get(0)
        val hrs = samples.map { it[1] }.filter { !it.isNaN() }
        val capUah = if (lvl0 > 0 && q0 > 0) q0 * 100.0 / lvl0 else Double.NaN
        val meanUa = samples.map { it[4] }.filter { !it.isNaN() }.let { if (it.isEmpty()) Double.NaN else it.average() }
        val summary = JSONObject()
            .put("k", Logic.label(base)).put("s", durationS).put("w", w).put("calls", callList.size)
            .put("ops", ops).put("hr", hr).put("st", st).put("tC", tC).put("mA", mA).put("lvl", lvl)
            .put("r10", Logic.dp(last / first, 3))
            .put("tMod", if (tModerate == null) JSONObject.NULL else Logic.dp(tModerate, 0))
            .put("stMax", Logic.num(samples.maxOfOrNull { it[2] } ?: Double.NaN))
            .put("hrMax", Logic.dp(hrs.maxOrNull() ?: Double.NaN, 3))
            .put("hrNaN", samples.size - hrs.size)
            .put("dLvl", lvl0 - lvl1).put("dmAh", Logic.num((q0 - q1) / 1000.0))
            .put("pctH", Logic.dp((q0 - q1) / capUah * 100 * 3600 / durationS, 1))
            .put("pctHi", Logic.dp(abs(meanUa) / capUah * 100, 1))
            .put("capmAh", Logic.num(capUah / 1000))
            .put("errs", loadErrors.get())
        if (clocksReadable) summary.put("mhz", mhz)
        if (lastError.isNotEmpty()) summary.put("lastErr", lastError)
        val detail = JSONObject()
            .put("samples", JSONArray(samples.map { r -> JSONArray(r.map { Logic.num(it, 4) }) }))
            .put("calls", JSONArray(callList.map { c -> JSONArray(c.map { Logic.num(it, 5) }) }))
        return summary to detail
    }

    /**
     * B79 memory: map and touch native memory in steps up to `maxMiB`, writing progress before each
     * step. Stops on a critical memory warning (trimLevel) or a failed mapping. If Android kills the
     * app instead, the next launch reads the progress file (see memoryFromProgress).
     */
    fun memory(trimLevel: AtomicInteger, stepMiB: Int = 256, maxMiB: Int = 10240, tick: (Int) -> Unit): JSONObject {
        val t0 = SystemClock.elapsedRealtime()
        val log = StringBuilder()
        var total = 0
        var reason = "max"
        var lowAt = -1
        val availPerGiB = JSONArray()
        while (total < maxMiB) {
            val mi = probe.memInfo()
            if (mi.lowMemory && lowAt < 0) lowAt = total
            log.append(JSONObject().put("s", "start").put("target", total + stepMiB).put("reached", total)
                .put("avail", mi.availMem shr 20).put("rss", probe.rssMiB()).put("low", mi.lowMemory)
                .put("ms", SystemClock.elapsedRealtime() - t0).toString()).append('\n')
            store.writeAtomic(store.memFile, log.toString())
            if (trimLevel.get() >= 15) { reason = "trim${trimLevel.get()}"; break }
            if (!Native.memAllocMiB(stepMiB)) { reason = "alloc"; break }
            total += stepMiB
            if (total % 1024 == 0) availPerGiB.put(probe.memInfo().availMem shr 20)
            tick(total)
        }
        val ms = SystemClock.elapsedRealtime() - t0
        val rss = probe.rssMiB()
        val freed = Native.memFreeAll()
        log.append(JSONObject().put("s", "end").put("reason", reason).put("reached", total).toString()).append('\n')
        store.writeAtomic(store.memFile, log.toString())
        return JSONObject().put("end", reason).put("mib", total).put("lowAt", lowAt).put("rssMax", rss)
            .put("freed", freed).put("s", Logic.dp(ms / 1000.0, 1)).put("availGiB", availPerGiB)
            .put("trim", trimLevel.get())
    }

    /** After the app died during the memory test: the last step reached, from the progress file. */
    fun memoryFromProgress(): JSONObject {
        val lines = try { store.memFile.readLines().filter { it.isNotBlank() } } catch (_: Throwable) { emptyList() }
        val starts = lines.mapNotNull { try { JSONObject(it) } catch (_: Throwable) { null } }.filter { it.optString("s") == "start" }
        val last = starts.lastOrNull() ?: return JSONObject().put("end", "killed").put("mib", -1)
        val availPerGiB = JSONArray()
        starts.filter { it.optInt("reached") > 0 && it.optInt("reached") % 1024 == 0 }.forEach { availPerGiB.put(it.optLong("avail")) }
        val low = starts.firstOrNull { it.optBoolean("low") }
        return JSONObject().put("end", "killed").put("mib", last.optInt("reached")).put("dyingAt", last.optInt("target"))
            .put("lowAt", low?.optInt("reached") ?: -1).put("rssMax", last.optLong("rss"))
            .put("availLast", last.optLong("avail")).put("s", Logic.dp(last.optLong("ms") / 1000.0, 1)).put("availGiB", availPerGiB)
    }
}
