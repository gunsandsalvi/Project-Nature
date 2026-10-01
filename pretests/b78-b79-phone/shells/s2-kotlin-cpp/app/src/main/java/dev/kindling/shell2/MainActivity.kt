package dev.kindling.shell2

import android.app.Activity
import android.os.Bundle
import android.widget.TextView

// B78 shell 2: one screen, one native call.
class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val text = TextView(this)
        text.text = "mix(42, 1000000) = " + java.lang.Long.toHexString(Core.mix(42, 1_000_000))
        setContentView(text)
    }
}

object Core {
    init { System.loadLibrary("shell2") }
    @JvmStatic external fun mix(seed: Long, rounds: Int): Long
}
