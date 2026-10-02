package dev.kindling.app

import android.content.Context
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import android.view.Surface
import android.view.SurfaceHolder
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * The game's surface (A2.5): OpenGL ES 3.0, RGBA 8888, no depth or stencil, continuous rendering, 120 Hz asked for
 * (B79). The app lives on the GL thread; touches and lifecycle calls reach it through queueEvent, in order.
 */
class GameView(context: Context, private val handle: Long, private val onRequests: (String) -> Unit) :
    GLSurfaceView(context) {

    /** How many cards or views Back would close, from the last frame (A2.5). */
    @Volatile var backCount = 0

    init {
        setEGLContextClientVersion(3)
        setEGLConfigChooser(8, 8, 8, 8, 0, 0)
        preserveEGLContextOnPause = true
        setRenderer(object : Renderer {
            override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) = Native.glCreated(handle)
            override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) = Native.glResized(handle, width, height)
            override fun onDrawFrame(gl: GL10?) {
                backCount = Native.glDraw(handle, System.nanoTime())
                Native.takeRequests(handle)?.let { json -> post { onRequests(json) } }
            }
        })
        renderMode = RENDERMODE_CONTINUOUSLY
        holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(h: SurfaceHolder) {
                try {
                    h.surface.setFrameRate(120f, Surface.FRAME_RATE_COMPATIBILITY_DEFAULT, Surface.CHANGE_FRAME_RATE_ALWAYS)
                } catch (_: Throwable) {
                    // An older display may refuse; the default rate is fine.
                }
            }
            override fun surfaceChanged(h: SurfaceHolder, format: Int, w: Int, height: Int) {}
            override fun surfaceDestroyed(h: SurfaceHolder) {}
        })
    }

    override fun onTouchEvent(e: MotionEvent): Boolean {
        val n = e.pointerCount
        val ids = IntArray(n) { e.getPointerId(it) }
        val xs = FloatArray(n) { e.getX(it) }
        val ys = FloatArray(n) { e.getY(it) }
        val action = e.actionMasked
        val index = e.actionIndex
        val t = e.eventTime * 1_000_000L
        queueEvent { Native.touch(handle, action, index, ids, xs, ys, t) }
        return true
    }

    fun resumeGame() {
        queueEvent { Native.onResume(handle) }
        onResume()
    }

    fun pauseGame() {
        queueEvent { Native.onPause(handle) }
        onPause()
    }

    fun back() = queueEvent { Native.back(handle) }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow() // ends the GL thread
        Native.destroy(handle)
    }
}
