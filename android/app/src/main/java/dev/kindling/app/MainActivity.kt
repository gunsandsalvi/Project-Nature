package dev.kindling.app

import android.app.Activity
import android.os.Build
import android.os.Bundle
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.window.OnBackInvokedDispatcher
import org.json.JSONArray

/**
 * The one activity (A2.5): full screen with the system bars hidden until swiped in, the screen kept on while the
 * game draws, Back handled by the app first, and the app's requests carried out as they arrive.
 */
class MainActivity : Activity() {
    private var handle = 0L
    private lateinit var view: GameView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.setDecorFitsSystemWindows(false)
        val device = "${Build.MANUFACTURER} ${Build.MODEL} SDK ${Build.VERSION.SDK_INT}"
        handle = Native.create(filesDir.path, cacheDir.path, device)
        view = GameView(this, handle) { json -> carryOut(json) }
        setContentView(view)
        view.setOnApplyWindowInsetsListener { _, insets ->
            val i = insets.getInsetsIgnoringVisibility(
                WindowInsets.Type.systemBars() or WindowInsets.Type.displayCutout() or WindowInsets.Type.mandatorySystemGestures()
            )
            view.queueInsets(i.left, i.top, i.right, i.bottom)
            insets
        }
        if (Build.VERSION.SDK_INT >= 33) {
            onBackInvokedDispatcher.registerOnBackInvokedCallback(OnBackInvokedDispatcher.PRIORITY_DEFAULT) { back() }
        }
        hideSystemBars()
    }

    override fun onResume() {
        super.onResume()
        hideSystemBars()
        view.resumeGame()
    }

    override fun onPause() {
        view.pauseGame()
        super.onPause()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemBars()
    }

    @Deprecated("Back on Android 12, before OnBackInvokedCallback")
    override fun onBackPressed() = back()

    /** The app closes its top card or view; with none open, the game goes to the background. */
    private fun back() {
        if (view.backCount > 0) view.queueBack() else moveTaskToBack(true)
    }

    private fun hideSystemBars() {
        window.insetsController?.let {
            it.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            it.hide(WindowInsets.Type.systemBars())
        }
    }

    /** The requests the app posted (A2.2), a JSON array of objects named by their kind. */
    private fun carryOut(json: String) {
        val list = JSONArray(json)
        for (i in 0 until list.length()) {
            val r = list.getJSONObject(i)
            r.optJSONObject("ShowCode")?.let {
                CodeDialog.show(this, it.getString("title"), it.getString("prefix"), it.getString("json"))
            }
        }
    }
}
