package dev.kindling.pretests

/**
 * B04/B11 storage and terrain (libkstorage.so), the contract in pretests/b04-b11-storage-terrain/INTEGRATION.md:
 * `run(configJson)` is JSON in, JSON out; it blocks for about 20 s, so it is called from the test thread.
 * As with the sound library (Sound.kt), a library that fails to load is recorded, never fatal.
 */
object Storage {
    @Volatile var loadError: String? = null
    val loaded: Boolean = try {
        System.loadLibrary("kstorage"); true
    } catch (t: Throwable) {
        loadError = t.toString(); false
    }

    @JvmStatic external fun run(configJson: String): String
}
