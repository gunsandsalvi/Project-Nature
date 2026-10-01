package dev.kindling.pretests

import android.app.ActivityManager
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.media.AudioManager
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.BatteryManager
import android.os.Build
import android.os.Debug
import android.os.PowerManager
import android.os.StatFs
import android.os.SystemClock
import android.system.Os
import android.system.OsConstants
import org.json.JSONObject
import java.io.File

/** What the phone reports about itself: battery, heat, memory, storage, network. No permission prompts. */
class Probe(private val ctx: Context) {
    private val bm = ctx.getSystemService(BatteryManager::class.java)
    private val pm = ctx.getSystemService(PowerManager::class.java)
    private val am = ctx.getSystemService(ActivityManager::class.java)

    fun level(): Int = try { bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CAPACITY) } catch (_: Throwable) { -1 }

    /** The sticky battery broadcast (temperature, plugged); no receiver is registered. */
    private fun battery(): Intent? = try { ctx.registerReceiver(null, IntentFilter(Intent.ACTION_BATTERY_CHANGED)) } catch (_: Throwable) { null }
    fun plugged(): Int = battery()?.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1) ?: -1
    fun tempC(): Any = battery()?.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, Int.MIN_VALUE)
        ?.takeIf { it != Int.MIN_VALUE }?.let { it / 10.0 } ?: JSONObject.NULL

    fun thermalStatus(): Int = try { pm.currentThermalStatus } catch (_: Throwable) { -1 }

    private var lastHeadroomAt = 0L
    private var lastHeadroom = Float.NaN

    /** Thermal headroom forecast 10 s ahead (1.0 = throttling). Called at most once a second, as the API asks. */
    @Synchronized fun headroom(): Float {
        val now = SystemClock.elapsedRealtime()
        if (now - lastHeadroomAt >= 1000) {
            lastHeadroom = try { pm.getThermalHeadroom(10) } catch (_: Throwable) { Float.NaN }
            lastHeadroomAt = now
        }
        return lastHeadroom
    }

    fun memInfo(): ActivityManager.MemoryInfo = ActivityManager.MemoryInfo().also { am.getMemoryInfo(it) }

    fun freeBytes(dir: File = ctx.filesDir): Long = try { StatFs(dir.path).availableBytes } catch (_: Throwable) { -1L }

    /** Is the phone on Wi-Fi (or a wired network) right now? */
    fun onWifi(): Boolean = try {
        val cm = ctx.getSystemService(ConnectivityManager::class.java)
        val caps = cm.getNetworkCapabilities(cm.activeNetwork)
        caps != null && (caps.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) || caps.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET))
    } catch (_: Throwable) {
        false
    }

    /** Media volume as "current/max", for the sound test (B74). */
    fun mediaVolume(): String = try {
        val a = ctx.getSystemService(AudioManager::class.java)
        "${a.getStreamVolume(AudioManager.STREAM_MUSIC)}/${a.getStreamMaxVolume(AudioManager.STREAM_MUSIC)}"
    } catch (_: Throwable) {
        "?"
    }

    fun versionOf(pkg: String): String = try {
        ctx.packageManager.getPackageInfo(pkg, 0).versionName ?: "?"
    } catch (_: Throwable) {
        "not found"
    }

    /** This process's memory from /proc/self/status, in MiB: resident (RSS), its anonymous and file-backed parts, peak. */
    fun procMem(): LongArray {
        val out = longArrayOf(-1, -1, -1, -1)
        try {
            File("/proc/self/status").forEachLine { line ->
                val i = when {
                    line.startsWith("VmRSS:") -> 0
                    line.startsWith("RssAnon:") -> 1
                    line.startsWith("RssFile:") -> 2
                    line.startsWith("VmHWM:") -> 3
                    else -> -1
                }
                if (i >= 0) out[i] = (line.split(Regex("\\s+")).getOrNull(1)?.toLongOrNull() ?: -1L) / 1024
            }
        } catch (_: Throwable) {}
        return out
    }

    /** Proportional memory (PSS) and graphics memory of this process, in MiB. Slow-ish: call from a worker thread. */
    fun pssAndGraphics(): Pair<Long, Long> = try {
        val mi = Debug.MemoryInfo()
        Debug.getMemoryInfo(mi)
        val gfx = mi.getMemoryStat("summary.graphics")?.toLongOrNull() ?: -1L
        (mi.totalPss / 1024L) to (if (gfx >= 0) gfx / 1024 else -1L)
    } catch (_: Throwable) {
        -1L to -1L
    }

    /** Heat and free memory now, as WriterTest records them before and after its run (B73). */
    fun vitals(): JSONObject {
        val mi = memInfo()
        val hr = headroom()
        return JSONObject().put("battery_c", tempC()).put("thermal", thermalStatus())
            .put("headroom_10s", if (hr.isNaN()) JSONObject.NULL else Logic.dp(hr.toDouble(), 2))
            .put("avail_mem_mb", mi.availMem / (1024 * 1024)).put("low_memory", mi.lowMemory)
    }

    /** Device facts for the results ("dev"). */
    fun device(): JSONObject {
        val d = JSONObject()
        d.put("model", Build.MODEL).put("mfr", Build.MANUFACTURER).put("dev", Build.DEVICE)
        d.put("soc", "${Build.SOC_MANUFACTURER} ${Build.SOC_MODEL}")
        d.put("rel", Build.VERSION.RELEASE).put("sdk", Build.VERSION.SDK_INT).put("patch", Build.VERSION.SECURITY_PATCH)
        d.put("page", try { Os.sysconf(OsConstants._SC_PAGESIZE) } catch (_: Throwable) { -1L })
        d.put("ncpu", Runtime.getRuntime().availableProcessors())
        val mi = memInfo()
        d.put("ramMiB", mi.totalMem shr 20).put("availMiB", mi.availMem shr 20).put("lowRam", am.isLowRamDevice)
        d.put("th0", thermalStatus()).put("hr0", Logic.dp(headroom().toDouble(), 3))
        d.put("bat0", level()).put("plug", plugged()).put("tC0", tempC())
        d.put("freeGB", Logic.dp(freeBytes() / 1e9, 1)).put("wifi", onWifi()).put("vol", mediaVolume())
        d.put("aicore", versionOf("com.google.android.aicore")).put("gms", versionOf("com.google.android.gms"))
            .put("pcs", versionOf("com.google.android.as.oss"))
        return d
    }
}
