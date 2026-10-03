package dev.kindling.app

import android.annotation.SuppressLint
import android.content.Context
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import android.view.Surface
import android.view.SurfaceHolder
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * The GL surface (A2.5): OpenGL ES 3.0, RGBA 8888 with no depth, stencil or multisampling (the passes draw into
 * the renderer's own targets), the context kept while paused, 120 Hz asked for. Touches and every other call reach
 * the app through the GL thread's queue, in order, so no Rust state is shared between threads.
 */
@SuppressLint("ViewConstructor")
class GameView(context: Context, private val handle: Long, private val onRequests: (String) -> Unit) :
    GLSurfaceView(context) {

    /** How many cards or views Back would close, from the last frame. */
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
                h.surface.setFrameRate(120f, Surface.FRAME_RATE_COMPATIBILITY_DEFAULT, Surface.CHANGE_FRAME_RATE_ALWAYS)
            }

            override fun surfaceChanged(h: SurfaceHolder, format: Int, width: Int, height: Int) {}

            override fun surfaceDestroyed(h: SurfaceHolder) {}
        })
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(e: MotionEvent): Boolean {
        val n = e.pointerCount
        val ids = IntArray(n) { e.getPointerId(it) }
        val xs = FloatArray(n) { e.getX(it) }
        val ys = FloatArray(n) { e.getY(it) }
        val action = e.actionMasked
        val index = e.actionIndex
        val nanos = e.eventTime * 1_000_000L
        queueEvent { Native.touch(handle, action, index, ids, xs, ys, nanos) }
        return true
    }

    fun queueInsets(left: Int, top: Int, right: Int, bottom: Int) = queueEvent { Native.insets(handle, left, top, right, bottom) }

    fun queueBack() = queueEvent { Native.back(handle) }

    /** Time runs again; the queued call reaches the app before the next frame (A2.5). */
    fun resumeGame() {
        queueEvent { Native.onResume(handle) }
        onResume()
    }

    /** Queued first, so the GL thread runs it before `onPause` returns (A2.5). */
    fun pauseGame() {
        queueEvent { Native.onPause(handle) }
        onPause()
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow() // ends the GL thread
        Native.destroy(handle)
    }
}
