package dev.kindling.pretests

import android.annotation.SuppressLint
import android.app.Activity
import android.webkit.JavascriptInterface
import android.webkit.RenderProcessGoneDetail
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.FrameLayout
import org.json.JSONObject
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * B66/B79, optional: the visual-style mockup (mockups/visual-style.html, copied into the app's
 * assets at build time) drawn with WebGL in a WebView, driven through its window.__view handle
 * in a slow turn and zoom sweep for `seconds`. Frame times come back through a JavaScript bridge.
 * Call from a background thread.
 */
class WebTest(private val activity: Activity, private val stage: FrameLayout) {
    companion object {
        const val ASSET = "visual-style.html"
        fun available(activity: Activity) = try { activity.assets.open(ASSET).close(); true } catch (_: Throwable) { false }
    }

    @Volatile private var result: String? = null
    private val done = CountDownLatch(1)

    inner class Bridge {
        @JavascriptInterface fun done(json: String) { result = json; done.countDown() }
    }

    @SuppressLint("SetJavaScriptEnabled")
    fun run(seconds: Int): JSONObject {
        var view: WebView? = null
        var gone: String? = null
        activity.runOnUiThread {
            try {
                val w = WebView(activity)
                w.settings.javaScriptEnabled = true
                w.addJavascriptInterface(Bridge(), "KBridge")
                w.webViewClient = object : WebViewClient() {
                    override fun onPageFinished(v: WebView, url: String) {
                        v.evaluateJavascript(sweep(seconds), null)
                    }
                    override fun onRenderProcessGone(v: WebView, detail: RenderProcessGoneDetail): Boolean {
                        gone = if (detail.didCrash()) "renderer crashed" else "renderer killed"
                        done.countDown()
                        return true // the app survives; the test records it
                    }
                }
                stage.addView(w, FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT))
                w.loadUrl("file:///android_asset/$ASSET")
                view = w
            } catch (t: Throwable) {
                result = JSONObject().put("err", t.toString().take(120)).toString()
                done.countDown()
            }
        }
        val finished = done.await(seconds + 40L, TimeUnit.SECONDS)
        val removed = CountDownLatch(1)
        activity.runOnUiThread {
            try { view?.let { stage.removeView(it); it.destroy() } } catch (_: Throwable) {}
            removed.countDown()
        }
        removed.await(5, TimeUnit.SECONDS)
        val r = try { JSONObject(result ?: "{}") } catch (t: Throwable) { JSONObject().put("err", "bad json") }
        if (!finished) r.put("err", "timeout")
        gone?.let { r.put("err", it) }
        return r
    }

    /** Waits for the page to be ready, then turns half a circle and sweeps the zoom twice, timing every frame. */
    private fun sweep(seconds: Int) = """
        (function () {
          const D = ${seconds * 1000}, wait0 = performance.now();
          function pct(a, q) { if (!a.length) return null; const s = a.slice().sort((x, y) => x - y);
            return Math.round(s[Math.min(s.length - 1, Math.ceil(q / 100 * s.length) - 1)] * 100) / 100; }
          function gl() { try { const c = document.createElement('canvas').getContext('webgl2');
            const d = c.getExtension('WEBGL_debug_renderer_info');
            return String(d ? c.getParameter(d.UNMASKED_RENDERER_WEBGL) : c.getParameter(c.RENDERER)).slice(0, 60); } catch (e) { return 'none'; } }
          function start() {
            const v = window.__view, iv = [], rt = [], y0 = v.S.yaw, t0 = performance.now();
            let last = 0;
            function frame(ts) {
              const e = performance.now() - t0, p = e / D;
              if (last) iv.push(ts - last);
              last = ts;
              const a = performance.now();
              v.set({ yaw: y0 + p * Math.PI, zoom: 0.08 + 0.5 * (0.5 - 0.5 * Math.cos(p * Math.PI * 4)), t: 2.4 + e / 1000 });
              rt.push(performance.now() - a);
              if (e < D) requestAnimationFrame(frame);
              else KBridge.done(JSON.stringify({ n: iv.length, p50: pct(iv, 50), p90: pct(iv, 90), p99: pct(iv, 99), max: pct(iv, 100),
                miss: iv.filter((x) => x > 1.5 * pct(iv, 50)).length, r50: pct(rt, 50), r99: pct(rt, 99),
                art: v.R.size ? [v.R.size.Wf, v.R.size.Hf] : null, css: [innerWidth, innerHeight], dpr: devicePixelRatio,
                wait: Math.round(t0 - wait0), gl: gl() }));
            }
            requestAnimationFrame(frame);
          }
          (function poll() {
            try { if (window.__view && window.__view.ready()) return start(); } catch (e) {}
            if (performance.now() - wait0 > 20000) return KBridge.done(JSON.stringify({ err: 'not ready after 20 s' }));
            setTimeout(poll, 250);
          })();
        })();
    """.trimIndent()
}
