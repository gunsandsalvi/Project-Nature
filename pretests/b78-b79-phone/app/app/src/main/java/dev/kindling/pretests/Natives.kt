package dev.kindling.pretests

/**
 * B01/B02 benchmark library (libkbench.so), the contract with the other pre-test:
 * `run(configJson)` is JSON in, JSON out; `pinToCpu(cpu)` pins the calling thread.
 */
object Bench {
    @Volatile var loadError: String? = null
    val loaded: Boolean = try {
        System.loadLibrary("kbench"); true
    } catch (t: Throwable) {
        loadError = t.toString(); false
    }

    @JvmStatic external fun run(configJson: String): String
    @JvmStatic external fun pinToCpu(cpu: Int): Boolean
}

/** B79 helpers (libkphone.so): native memory steps and the current CPU. */
object Native {
    val loaded: Boolean = try {
        System.loadLibrary("kphone"); true
    } catch (_: Throwable) {
        false
    }

    @JvmStatic external fun memAllocMiB(mib: Int): Boolean
    @JvmStatic external fun memFreeAll(): Int
    @JvmStatic external fun currentCpu(): Int
}

/**
 * B01 managed-code kernels (Kernels.java from the other pre-test), found by reflection so
 * the app still builds and runs without them. Same JSON contract as the native library.
 */
object Managed {
    private val runMethod = try {
        Class.forName("dev.kindling.pretests.Kernels").getMethod("run", String::class.java)
    } catch (_: Throwable) {
        null
    }
    val available: Boolean get() = runMethod != null

    fun run(configJson: String): String = runMethod?.invoke(null, configJson) as? String
        ?: """{"error":"managed kernels missing"}"""
}
