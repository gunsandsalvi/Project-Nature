package dev.kindling.pretests

/**
 * B74 sound test (libksound.so, from pretests/b74-b76-sound-speech): JSON in, JSON out,
 * like Bench.run. It blocks for about 35 s and plays a camp test pattern through the
 * speaker, so call it from a background thread with the media volume at about half.
 */
object Sound {
    @Volatile var loadError: String? = null
    val loaded: Boolean = try {
        System.loadLibrary("ksound"); true
    } catch (t: Throwable) {
        loadError = t.toString(); false
    }

    /** Default plan: 8, 32 and 128 voices, 8 s each, at moderate volume. */
    const val DEFAULT_CONFIG = """{"voices":[8,32,128],"seconds_each":8,"gain":0.5}"""

    @JvmStatic external fun run(configJson: String): String
}
