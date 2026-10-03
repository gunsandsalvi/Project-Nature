package dev.kindling.app

import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.util.Base64
import android.widget.TextView
import java.io.ByteArrayOutputStream
import java.util.zip.GZIPOutputStream

/**
 * The code dialog (A15.4): a report from the app, gzipped and written in base64 after its prefix (`KDS1:`), shown
 * with Copy so you can paste it into your reply. It works before the game has any screens of its own.
 */
object CodeDialog {
    fun code(prefix: String, json: String): String {
        val bytes = ByteArrayOutputStream()
        GZIPOutputStream(bytes).use { it.write(json.toByteArray(Charsets.UTF_8)) }
        return prefix + Base64.encodeToString(bytes.toByteArray(), Base64.NO_WRAP)
    }

    fun show(activity: Activity, title: String, prefix: String, json: String) {
        val code = code(prefix, json)
        val text = TextView(activity).apply {
            this.text = code
            setTextIsSelectable(true)
            setPadding(48, 24, 48, 0)
        }
        AlertDialog.Builder(activity)
            .setTitle(title)
            .setMessage("Something didn't work. Tap Copy and paste the code into your reply.")
            .setView(text)
            .setPositiveButton("Copy") { _, _ ->
                val clip = activity.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                clip.setPrimaryClip(ClipData.newPlainText("Kindling code", code))
            }
            .setNegativeButton("Close", null)
            .show()
    }
}
