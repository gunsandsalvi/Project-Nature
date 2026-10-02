package dev.kindling.app

import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.os.Build
import android.os.Bundle
import android.util.Base64
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.window.OnBackInvokedDispatcher
import org.json.JSONArray
import java.io.ByteArrayOutputStream
import java.util.zip.GZIPOutputStream

/** Lifecycle, immersive full screen, Back and the self-check box (A2.5, A15.4). Rotation never restarts it (PLT-02). */
class MainActivity : Activity() {
    private lateinit var view: GameView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.setDecorFitsSystemWindows(false)
        val device = "${Build.MANUFACTURER} ${Build.MODEL} SDK ${Build.VERSION.SDK_INT}"
        val handle = Native.create(filesDir.path, cacheDir.path, device)
        view = GameView(this, handle, ::onRequests)
        setContentView(view)
        hideBars()
        if (Build.VERSION.SDK_INT >= 33) {
            onBackInvokedDispatcher.registerOnBackInvokedCallback(OnBackInvokedDispatcher.PRIORITY_DEFAULT) { goBack() }
        }
    }

    private fun hideBars() {
        window.insetsController?.let {
            it.hide(WindowInsets.Type.systemBars())
            it.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
        }
    }

    private fun goBack() {
        if (view.backCount > 0) view.back() else moveTaskToBack(true)
    }

    @Deprecated("Below SDK 33 only; newer versions use the back callback")
    override fun onBackPressed() = goBack()

    override fun onResume() {
        super.onResume()
        hideBars()
        view.resumeGame()
    }

    override fun onPause() {
        view.pauseGame()
        super.onPause()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideBars()
    }

    /** Requests from Rust, a JSON array such as [{"SelfCheck":{"json":"..."}}] (A2.2). */
    private fun onRequests(json: String) {
        val list = JSONArray(json)
        for (i in 0 until list.length()) {
            val r = list.getJSONObject(i)
            r.optJSONObject("SelfCheck")?.let { showSelfCheck(it.getString("json")) }
        }
    }

    /** The self-check's KDS1: code, gzip then base64 (A15.4). */
    private fun showSelfCheck(json: String) {
        val bytes = ByteArrayOutputStream()
        GZIPOutputStream(bytes).use { it.write(json.toByteArray(Charsets.UTF_8)) }
        val code = "KDS1:" + Base64.encodeToString(bytes.toByteArray(), Base64.NO_WRAP)
        AlertDialog.Builder(this)
            .setTitle("Kindling self-check")
            .setMessage("Something failed. Tap Copy and paste the code in your reply.\n\n$code")
            .setPositiveButton("Copy") { _, _ ->
                getSystemService(ClipboardManager::class.java).setPrimaryClip(ClipData.newPlainText("Kindling self-check", code))
            }
            .setNegativeButton("Close", null)
            .show()
    }
}
