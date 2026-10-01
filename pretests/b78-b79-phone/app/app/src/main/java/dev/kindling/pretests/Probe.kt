package dev.kindling.pretests

import android.app.ActivityManager
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.BatteryManager
import android.os.Build
import android.os.PowerManager
import android.os.SystemClock
import android.system.Os
import android.system.OsConstants
import android.hardware.display.DisplayManager
import android.view.Display
import android.view.WindowManager
import org.json.JSONArray
import org.json.JSONObject
import java.io.File

/** B79: what the phone reports about itself: battery, heat, CPUs, memory, display. No permissions needed. */
class Probe(private val ctx: Context) {
    private val bm = ctx.getSystemService(BatteryManager::class.java)
    private val pm = ctx.getSystemService(PowerManager::class.java)
    private val am = ctx.getSystemService(ActivityManager::class.java)
    val ncpu: Int = countCpus()

    private fun countCpus(): Int {
        val fromSys = File("/sys/devices/system/cpu").list()?.count { it.matches(Regex("cpu[0-9]+")) } ?: 0
        return maxOf(fromSys, Runtime.getRuntime().availableProcessors())
    }

    /** Battery current in microamps; negative while discharging on most phones. Int.MIN_VALUE if unsupported. */
    fun currentUa(): Int = bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW)
    fun level(): Int = bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CAPACITY)
    fun chargeUah(): Int = bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CHARGE_COUNTER)

    /** The sticky battery broadcast (voltage, temperature, plugged); no receiver is registered. */
    fun battery(): Intent? = ctx.registerReceiver(null, IntentFilter(Intent.ACTION_BATTERY_CHANGED))
    fun voltageMv(): Int = battery()?.getIntExtra(BatteryManager.EXTRA_VOLTAGE, -1) ?: -1
    fun tempDeciC(): Int = battery()?.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, Int.MIN_VALUE) ?: Int.MIN_VALUE
    fun plugged(): Int = battery()?.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1) ?: -1

    fun thermalStatus(): Int = pm.currentThermalStatus

    private var lastHeadroomAt = 0L
    private var lastHeadroom = Float.NaN

    /** Thermal headroom forecast 10 s ahead (1.0 = throttling). Called at most once a second, as the API asks. */
    fun headroom(): Float {
        val now = SystemClock.elapsedRealtime()
        if (now - lastHeadroomAt >= 1000) {
            lastHeadroom = try { pm.getThermalHeadroom(10) } catch (_: Throwable) { Float.NaN }
            lastHeadroomAt = now
        }
        return lastHeadroom
    }

    private fun readLong(path: String): Long = try {
        File(path).readText().trim().toLong()
    } catch (_: Throwable) {
        -1L
    }

    fun maxFreqKHz(cpu: Int) = readLong("/sys/devices/system/cpu/cpu$cpu/cpufreq/cpuinfo_max_freq")
    fun curFreqKHz(cpu: Int) = readLong("/sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_cur_freq")

    fun memInfo(): ActivityManager.MemoryInfo = ActivityManager.MemoryInfo().also { am.getMemoryInfo(it) }

    fun rssMiB(): Long = try {
        File("/proc/self/status").readLines().firstOrNull { it.startsWith("VmRSS:") }
            ?.split(Regex("\\s+"))?.getOrNull(1)?.toLong()?.div(1024) ?: -1L
    } catch (_: Throwable) {
        -1L
    }

    /** CPU clusters: by max frequency if readable, else by core type, else null (the kernel phase then measures speed). */
    fun clusters(): Pair<List<Logic.Cluster>?, String> {
        val freqs = (0 until ncpu).associateWith { maxFreqKHz(it) }
        Logic.clustersByFreq(freqs, ncpu)?.let { return it to "freq" }
        val cpuinfo = try { File("/proc/cpuinfo").readText() } catch (_: Throwable) { "" }
        Logic.clustersByPart(cpuinfo, ncpu)?.let { return it to "part" }
        return null to "none"
    }

    /** Device facts for the results ("dev"). */
    fun device(): JSONObject {
        val d = JSONObject()
        d.put("model", Build.MODEL).put("mfr", Build.MANUFACTURER).put("dev", Build.DEVICE)
        d.put("soc", "${Build.SOC_MANUFACTURER} ${Build.SOC_MODEL}")
        d.put("rel", Build.VERSION.RELEASE).put("sdk", Build.VERSION.SDK_INT).put("patch", Build.VERSION.SECURITY_PATCH)
        d.put("page", try { Os.sysconf(OsConstants._SC_PAGESIZE) } catch (_: Throwable) { -1L })
        d.put("ncpu", ncpu).put("jcpu", Runtime.getRuntime().availableProcessors())
        val maxF = (0 until ncpu).map { maxFreqKHz(it) }
        d.put("maxFreqReadable", maxF.count { it > 0 }).put("curFreqReadable", (0 until ncpu).count { curFreqKHz(it) > 0 })
        val (cl, src) = clusters()
        d.put("clSrc", src)
        if (cl != null) d.put("cl", JSONArray().also { a ->
            cl.forEach { c -> a.put(JSONObject().put("cpus", Logic.jsonInts(c.cpus)).put("mhz", c.maxKHz / 1000)) }
        })
        val parts = try {
            File("/proc/cpuinfo").readLines().filter { it.startsWith("CPU part") }.map { it.substringAfter(':').trim() }
        } catch (_: Throwable) { emptyList() }
        d.put("parts", JSONArray(parts))
        val mi = memInfo()
        d.put("mc", am.memoryClass).put("lmc", am.largeMemoryClass)
        d.put("ramMiB", mi.totalMem shr 20).put("availMiB", mi.availMem shr 20).put("thresholdMiB", mi.threshold shr 20)
        d.put("lowRam", am.isLowRamDevice)
        d.put("th0", thermalStatus()).put("hr0", Logic.dp(headroom().toDouble(), 3))
        if (Build.VERSION.SDK_INT >= 35) try {
            val t = pm.thermalHeadroomThresholds
            d.put("hrThresholds", JSONObject().also { o -> t.forEach { (k, v) -> o.put(k.toString(), Logic.dp(v.toDouble(), 3)) } })
        } catch (_: Throwable) {}
        d.put("bat0", level()).put("plug", plugged()).put("tC0", tempDeciC() / 10.0).put("mV0", voltageMv())
        d.put("chargeUah0", chargeUah()).put("ua0", currentUa())
        try {
            val disp = ctx.getSystemService(DisplayManager::class.java).getDisplay(Display.DEFAULT_DISPLAY)
            val modes = disp.supportedModes.map { it.refreshRate }.distinct().sorted()
            val b = ctx.getSystemService(WindowManager::class.java).maximumWindowMetrics.bounds
            d.put("disp", JSONObject().put("w", b.width()).put("h", b.height()).put("hz", Logic.dp(disp.refreshRate.toDouble(), 1))
                .put("modes", JSONArray(modes.map { Logic.dp(it.toDouble(), 1) })))
        } catch (t: Throwable) {
            d.put("disp", t.toString().take(80))
        }
        return d
    }
}
