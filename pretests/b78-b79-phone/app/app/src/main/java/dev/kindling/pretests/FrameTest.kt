package dev.kindling.pretests

import android.app.Activity
import android.hardware.display.DisplayManager
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.view.Display
import android.view.Surface
import android.view.SurfaceHolder
import android.view.ViewGroup
import android.widget.FrameLayout
import org.json.JSONObject
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * B79 frame pacing: a simple OpenGL ES scene (a turning grid of 8,192 coloured triangles) for
 * `seconds`, after asking for the display's top refresh rate with Surface.setFrameRate.
 * Measures the interval between frames on the drawing thread. Call from a background thread.
 */
class FrameTest(private val activity: Activity, private val stage: FrameLayout) {

    fun run(seconds: Int): JSONObject {
        val display = activity.getSystemService(DisplayManager::class.java).getDisplay(Display.DEFAULT_DISPLAY)
        val maxHz = display.supportedModes.maxOf { it.refreshRate }
        val renderer = Scene(seconds)
        var requestError: String? = null
        var view: GLSurfaceView? = null
        activity.runOnUiThread {
            try {
                val v = GLSurfaceView(activity)
                v.setEGLContextClientVersion(3)
                v.setRenderer(renderer)
                v.holder.addCallback(object : SurfaceHolder.Callback {
                    override fun surfaceCreated(h: SurfaceHolder) {
                        try {
                            h.surface.setFrameRate(maxHz, Surface.FRAME_RATE_COMPATIBILITY_DEFAULT, Surface.CHANGE_FRAME_RATE_ALWAYS)
                        } catch (t: Throwable) {
                            requestError = t.toString().take(100)
                        }
                    }
                    override fun surfaceChanged(h: SurfaceHolder, format: Int, w: Int, height: Int) {}
                    override fun surfaceDestroyed(h: SurfaceHolder) {}
                })
                stage.addView(v, FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT))
                view = v
            } catch (t: Throwable) {
                renderer.error = t.toString().take(120)
                renderer.done.countDown()
            }
        }
        // Read the refresh rate the display actually runs at, halfway through.
        Thread.sleep((Scene.WARMUP_S + seconds / 2.0).times(1000).toLong())
        val hzDuring = display.refreshRate
        val finished = renderer.done.await(seconds + 20L, TimeUnit.SECONDS)
        val gone = CountDownLatch(1)
        activity.runOnUiThread {
            try { view?.let { it.onPause(); stage.removeView(it) } } catch (_: Throwable) {}
            gone.countDown()
        }
        gone.await(5, TimeUnit.SECONDS)

        val n = renderer.count
        val ms = DoubleArray(n) { renderer.intervals[it] / 1e6 }
        val sorted = ms.sortedArray()
        val periodMs = 1000.0 / hzDuring
        val r = JSONObject()
            .put("req", Logic.dp(maxHz.toDouble(), 1)).put("hz", Logic.dp(hzDuring.toDouble(), 1))
            .put("n", n).put("fps", Logic.dp(if (n > 0) n / (ms.sum() / 1000) else Double.NaN, 1))
            .put("p50", Logic.dp(Logic.percentile(sorted, 50.0), 2)).put("p90", Logic.dp(Logic.percentile(sorted, 90.0), 2))
            .put("p99", Logic.dp(Logic.percentile(sorted, 99.0), 2)).put("p999", Logic.dp(Logic.percentile(sorted, 99.9), 2))
            .put("max", Logic.dp(sorted.lastOrNull() ?: Double.NaN, 2))
            .put("miss", ms.count { it > 1.5 * periodMs })
            .put("missPct", Logic.dp(if (n > 0) 100.0 * ms.count { it > 1.5 * periodMs } / n else Double.NaN, 2))
            .put("gl", renderer.glInfo.take(60)).put("done", finished)
        renderer.error?.let { r.put("err", it) }
        requestError?.let { r.put("reqErr", it) }
        return r
    }

    private class Scene(private val seconds: Int) : GLSurfaceView.Renderer {
        companion object { const val WARMUP_S = 1.0 }

        val done = CountDownLatch(1)
        val intervals = LongArray(seconds * 250 + 100)
        @Volatile var count = 0
        @Volatile var error: String? = null
        @Volatile var glInfo = ""
        private var start = 0L
        private var last = 0L
        private var finished = false
        private var program = 0
        private var buffer = 0
        private var angle = -1
        private var vertices = 0

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            try {
                glInfo = "${GLES30.glGetString(GLES30.GL_RENDERER)} | ${GLES30.glGetString(GLES30.GL_VERSION)}"
                program = link(VS, FS)
                angle = GLES30.glGetUniformLocation(program, "uAngle")
                val g = 64
                val data = FloatArray(g * g * 6 * 5)
                var i = 0
                for (y in 0 until g) for (x in 0 until g) {
                    val x0 = -1f + 2f * x / g; val x1 = x0 + 1.8f / g
                    val y0 = -1f + 2f * y / g; val y1 = y0 + 1.8f / g
                    val r = x / g.toFloat(); val gr = y / g.toFloat(); val b = 1f - r
                    for ((px, py) in listOf(x0 to y0, x1 to y0, x1 to y1, x0 to y0, x1 to y1, x0 to y1)) {
                        data[i++] = px; data[i++] = py; data[i++] = r; data[i++] = gr; data[i++] = b
                    }
                }
                vertices = g * g * 6
                val fb = ByteBuffer.allocateDirect(data.size * 4).order(ByteOrder.nativeOrder()).asFloatBuffer()
                fb.put(data).position(0)
                val ids = IntArray(1)
                GLES30.glGenBuffers(1, ids, 0)
                buffer = ids[0]
                GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, buffer)
                GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, data.size * 4, fb, GLES30.GL_STATIC_DRAW)
            } catch (t: Throwable) {
                error = t.toString().take(120)
                program = 0
            }
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            GLES30.glViewport(0, 0, width, height)
        }

        override fun onDrawFrame(gl: GL10?) {
            try {
                val now = System.nanoTime()
                if (start == 0L) start = now
                val t = (now - start) / 1e9
                if (!finished) {
                    if (t >= WARMUP_S && last != 0L && count < intervals.size) intervals[count++] = now - last
                    if (t >= WARMUP_S + seconds) { finished = true; done.countDown() }
                }
                last = now
                GLES30.glClearColor(0.05f, 0.05f, 0.08f, 1f)
                GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
                if (program != 0) {
                    GLES30.glUseProgram(program)
                    GLES30.glUniform1f(angle, (t * 0.6).toFloat())
                    GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, buffer)
                    GLES30.glEnableVertexAttribArray(0)
                    GLES30.glEnableVertexAttribArray(1)
                    GLES30.glVertexAttribPointer(0, 2, GLES30.GL_FLOAT, false, 20, 0)
                    GLES30.glVertexAttribPointer(1, 3, GLES30.GL_FLOAT, false, 20, 8)
                    GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, vertices)
                }
            } catch (t: Throwable) {
                if (error == null) error = t.toString().take(120)
                if (!finished) { finished = true; done.countDown() }
            }
        }

        private fun compile(type: Int, src: String): Int {
            val s = GLES30.glCreateShader(type)
            GLES30.glShaderSource(s, src)
            GLES30.glCompileShader(s)
            val ok = IntArray(1)
            GLES30.glGetShaderiv(s, GLES30.GL_COMPILE_STATUS, ok, 0)
            if (ok[0] == 0) throw IllegalStateException("shader: " + GLES30.glGetShaderInfoLog(s))
            return s
        }

        private fun link(vs: String, fs: String): Int {
            val p = GLES30.glCreateProgram()
            GLES30.glAttachShader(p, compile(GLES30.GL_VERTEX_SHADER, vs))
            GLES30.glAttachShader(p, compile(GLES30.GL_FRAGMENT_SHADER, fs))
            GLES30.glLinkProgram(p)
            val ok = IntArray(1)
            GLES30.glGetProgramiv(p, GLES30.GL_LINK_STATUS, ok, 0)
            if (ok[0] == 0) throw IllegalStateException("link: " + GLES30.glGetProgramInfoLog(p))
            return p
        }

        private val VS = """#version 300 es
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec3 aCol;
            uniform float uAngle;
            out vec3 vCol;
            void main() {
                float c = cos(uAngle), s = sin(uAngle);
                gl_Position = vec4(0.9 * vec2(c * aPos.x - s * aPos.y, s * aPos.x + c * aPos.y), 0.0, 1.0);
                vCol = aCol;
            }
        """.trimIndent()

        private val FS = """#version 300 es
            precision mediump float;
            in vec3 vCol;
            out vec4 color;
            void main() { color = vec4(vCol, 1.0); }
        """.trimIndent()
    }
}
