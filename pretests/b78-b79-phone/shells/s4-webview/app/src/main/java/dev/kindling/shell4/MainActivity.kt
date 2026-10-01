package dev.kindling.shell4

import android.annotation.SuppressLint
import android.app.Activity
import android.os.Bundle
import android.webkit.JavascriptInterface
import android.webkit.WebView

// B78 shell 4: one screen drawn by a local web page; the page calls native code through a bridge.
class MainActivity : Activity() {
    @SuppressLint("SetJavaScriptEnabled")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val web = WebView(this)
        web.settings.javaScriptEnabled = true
        web.addJavascriptInterface(Bridge(), "Native")
        web.loadUrl("file:///android_asset/index.html")
        setContentView(web)
    }

    class Bridge {
        @JavascriptInterface
        fun mix(seed: String, rounds: Int): String = java.lang.Long.toHexString(Core.mix(seed.toLong(), rounds))
    }
}

object Core {
    init { System.loadLibrary("shell4") }
    @JvmStatic external fun mix(seed: Long, rounds: Int): Long
}
