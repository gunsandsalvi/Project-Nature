package dev.kindling.app

/**
 * The Rust app's entry points in libkindling.so (A2.5). `h` is the app's handle from [create]; every call but
 * [create] and [destroy] runs on the GL thread, through GameView's event queue.
 */
object Native {
    init {
        System.loadLibrary("kindling")
    }

    @JvmStatic external fun create(filesDir: String, cacheDir: String, device: String): Long
    @JvmStatic external fun destroy(h: Long)
    @JvmStatic external fun onResume(h: Long)
    @JvmStatic external fun onPause(h: Long)
    @JvmStatic external fun back(h: Long)
    @JvmStatic external fun glCreated(h: Long)
    @JvmStatic external fun glResized(h: Long, width: Int, height: Int)
    @JvmStatic external fun glDraw(h: Long, frameNanos: Long): Int
    @JvmStatic external fun touch(h: Long, action: Int, index: Int, ids: IntArray, xs: FloatArray, ys: FloatArray, timeNanos: Long)
    @JvmStatic external fun insets(h: Long, left: Int, top: Int, right: Int, bottom: Int)
    @JvmStatic external fun takeRequests(h: Long): String?
}
