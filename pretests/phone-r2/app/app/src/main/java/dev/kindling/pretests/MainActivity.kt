package dev.kindling.pretests

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Intent
import android.graphics.Typeface
import android.media.AudioManager
import android.media.ToneGenerator
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.util.Log
import android.util.TypedValue
import android.view.View
import android.view.WindowInsets
import android.view.WindowManager
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import android.window.OnBackInvokedCallback
import android.window.OnBackInvokedDispatcher
import androidx.core.content.FileProvider
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.util.concurrent.Executors
import java.util.concurrent.TimeoutException
import java.util.concurrent.atomic.AtomicReference

/**
 * Phone test app, round 2: one screen, one tap, then the owner rates a few texts and pastes back one code.
 * Runs, in order: storage and terrain (B04, B11), sound (B74), the writer AI with Gemini Nano, then with
 * Gemma 4 E2B (B73), then the owner's rating (B73 rule 6). As in round 1, a marker is written before each
 * test and its result as soon as it ends, so a crash is reported on the next launch and the rest can continue.
 * The tests run on their own thread; the screen thread only draws and takes taps.
 */
class MainActivity : Activity() {
    companion object {
        private var crashHandlerInstalled = false
        private const val CODE_LIMIT = 15_500 // aim: a code of about 15 KB at most
        // One store, probe and run per app process, whichever screen shows it, so a screen Android re-creates
        // never writes over the test thread's results.
        private var sharedStore: Store? = null
        @SuppressLint("StaticFieldLeak") // holds the application context only
        private var sharedProbe: Probe? = null
        /** The activity on screen, for the test thread's updates. */
        @SuppressLint("StaticFieldLeak")
        @Volatile private var active: MainActivity? = null
        @Volatile private var running = false
        @SuppressLint("StaticFieldLeak") // holds the application context only
        @Volatile private var gemma: GemmaTest? = null
    }

    private class Step(val id: String, val title: String, val estS: Int, val body: () -> Unit)

    private lateinit var store: Store
    private lateinit var probe: Probe
    private val ui = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor() // file writes and the code, off the screen thread

    /** Work for the helper thread; an error there is shown, never a crash. */
    private fun background(f: () -> Unit) = io.execute {
        try { f() } catch (t: Throwable) { say("Error: $t") }
    }
    private var backCallback: OnBackInvokedCallback? = null

    private lateinit var scroll: ScrollView
    private lateinit var info: TextView
    private lateinit var runButton: Button
    private lateinit var continueButton: Button
    private lateinit var stepText: TextView
    private lateinit var progressText: TextView
    private lateinit var bar: ProgressBar
    private lateinit var notice: TextView
    private lateinit var skipButton: Button
    private lateinit var rateBox: LinearLayout
    private lateinit var rateHead: TextView
    private lateinit var rateTitle: TextView
    private lateinit var rateText: TextView
    private lateinit var factsButton: Button
    private lateinit var factsText: TextView
    private lateinit var rateBack: Button
    private lateinit var codeLabel: TextView
    private lateinit var codeView: TextView
    private lateinit var copyButton: Button
    private lateinit var shareNote: TextView
    private lateinit var shareButton: Button
    private lateinit var deleteButton: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        store = sharedStore ?: Store(File(filesDir, "r2")).also { sharedStore = it }
        installCrashHandler()
        probe = sharedProbe ?: Probe(applicationContext).also { sharedProbe = it }
        buildUi()
        active = this
        try {
            if (!running) checkLastLaunch()
        } catch (t: Throwable) {
            say("Could not read the last results: $t")
        }
        if (running && !store.flag("auto")) {
            setRunning(true)
            stepText.text = "The tests are running..."
        } else if (store.flag("auto") && !store.flag("finished")) {
            startRating()
        }
        refresh()
    }

    override fun onDestroy() {
        if (active === this) active = null
        super.onDestroy()
    }

    // ------------------------------------------------------------------ robustness

    /** Uncaught exceptions on any thread are written down before the app dies, and reported next launch. */
    private fun installCrashHandler() {
        if (crashHandlerInstalled) return
        crashHandlerInstalled = true
        val previous = Thread.getDefaultUncaughtExceptionHandler()
        val file = store.uncaughtFile
        Thread.setDefaultUncaughtExceptionHandler { thread, e ->
            try { file.writeText("${thread.name}: ${Log.getStackTraceString(e).take(1500)}") } catch (_: Throwable) {}
            previous?.uncaughtException(thread, e)
        }
    }

    private fun title(step: String) = when (step) {
        "dev" -> "Phone details"
        "st" -> "Storage and terrain"
        "snd" -> "Sound"
        "nano" -> "Writer AI: Gemini Nano"
        "gm" -> "Writer AI: Gemma"
        else -> step
    }

    private fun checkLastLaunch() {
        val marker = store.leftoverMarker()
        if (marker != null) {
            store.markEnd()
            val step = marker.substringBefore('/')
            val part = marker.substringAfter('/', "")
            if (part.isNotEmpty()) {
                // Inside the Gemma test: it carries on from the next part (backend, prompt) next time.
                val n = store.recordPartCrash(step, part)
                if (n > GemmaTest.MAX_CRASHES) store.recordCrash(step)
                say("Last time, the app closed during \"${title(step)}\" ($part). Tap Continue to carry on.")
            } else {
                store.recordCrash(step)
                say("Last time, the app closed during \"${title(step)}\". Tap Continue to run the remaining tests.")
            }
        }
        if (store.uncaughtFile.exists()) {
            store.put("uncaught", store.uncaughtFile.readText().take(300))
            store.uncaughtFile.delete()
        }
    }

    // ------------------------------------------------------------------ screen

    private fun dp(x: Int) = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, x.toFloat(), resources.displayMetrics).toInt()

    private fun text(s: String, sp: Float = 16f, bold: Boolean = false) = TextView(this).apply {
        text = s
        setTextSize(TypedValue.COMPLEX_UNIT_SP, sp)
        if (bold) setTypeface(typeface, Typeface.BOLD)
        setPadding(0, dp(6), 0, dp(6))
    }

    private fun button(label: String, visible: Boolean = true, onTap: () -> Unit) = Button(this).apply {
        text = label
        isAllCaps = false
        visibility = if (visible) View.VISIBLE else View.GONE
        setOnClickListener { safely(onTap) }
    }

    private fun buildUi() {
        val col = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
        col.addView(text("Kindling phone test, round 2", 22f, true))
        col.addView(text(
            "Before you start:\n" +
                "• Update AICore, Private Compute Services and Google Play services in the Play Store.\n" +
                "• Be on Wi-Fi: the Gemma model (3.3 GB) downloads once.\n" +
                "• Have at least 4 GB free.\n" +
                "• Plug in the charger, or have the battery above 60%.\n" +
                "• Set the media volume to about half: one test plays clicks and tones.\n" +
                "• Then leave the app open and the phone alone until it asks you to rate some texts. " +
                "That takes about 15 to 30 minutes, longer on slow Wi-Fi. The screen stays on.",
        ))
        info = text("", 15f).apply { setTextColor(0xFFB3261E.toInt()); visibility = View.GONE }
        col.addView(info)
        runButton = button("Run all tests (about 15 to 30 minutes)") { askAndRun() }
        col.addView(runButton)
        continueButton = button("Continue with the remaining tests", visible = false) { startRun(true) }
        col.addView(continueButton)
        stepText = text("", 16f, true)
        col.addView(stepText)
        progressText = text("", 15f)
        col.addView(progressText)
        bar = ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal).apply { max = 1000; visibility = View.GONE }
        col.addView(bar)
        notice = text("", 20f, true).apply { visibility = View.GONE }
        col.addView(notice)
        skipButton = button("Skip Gemma (keeps what has downloaded)", visible = false) { gemma?.skip = true; skipButton.isEnabled = false }
        col.addView(skipButton)

        rateBox = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL; visibility = View.GONE }
        rateBox.addView(text("Your turn: rate each text. Judge how well it reads as an entry in the game's " +
            "history, and whether its voice comes through. The facts are checked separately.", 15f))
        rateHead = text("", 14f)
        rateBox.addView(rateHead)
        rateTitle = text("", 17f, true)
        rateBox.addView(rateTitle)
        rateText = text("", 17f).apply { setTextIsSelectable(true) }
        rateBox.addView(rateText)
        factsText = text("", 14f).apply { setTextColor(0xFF555555.toInt()); visibility = View.GONE }
        factsButton = button("Show the facts it was written from") {
            val show = factsText.visibility != View.VISIBLE
            factsText.visibility = if (show) View.VISIBLE else View.GONE
            factsButton.text = if (show) "Hide the facts" else "Show the facts it was written from"
        }
        rateBox.addView(factsButton)
        rateBox.addView(factsText)
        val row = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }
        for ((label, value) in listOf("Poor" to 0, "Acceptable" to 1, "Good" to 2)) {
            row.addView(button(label) { rate(value) }, LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f))
        }
        rateBox.addView(row)
        rateBack = button("Back to the previous text", visible = false) { if (rateIndex > 0) { rateIndex--; showRateItem() } }
        rateBox.addView(rateBack)
        col.addView(rateBox)

        codeLabel = text("Result code. Copy it and paste it back in the chat:", 16f, true).apply { visibility = View.GONE }
        col.addView(codeLabel)
        codeView = text("", 11f).apply { typeface = Typeface.MONOSPACE; setTextIsSelectable(true); visibility = View.GONE }
        col.addView(codeView)
        copyButton = button("Copy result code", visible = false) { copyCode() }
        col.addView(copyButton)
        shareNote = text("The code above could not hold everything. Please also tap \"Share results file\" " +
            "and send the file in the chat.", 15f).apply { visibility = View.GONE }
        col.addView(shareNote)
        shareButton = button("Share results file", visible = false) { shareFile() }
        col.addView(shareButton)
        deleteButton = button("Delete the model (3.3 GB)", visible = false) { askDeleteModel() }
        col.addView(deleteButton)
        col.addView(text(
            "The Gemma model stays on the phone for later rounds unless you delete it here.\n\n" +
                "App ${BuildConfig.VERSION_NAME}. Results are also saved in the app's own files.",
            13f,
        ))
        scroll = ScrollView(this).apply { addView(col) }
        scroll.setOnApplyWindowInsetsListener { v, insets ->
            val b = insets.getInsets(WindowInsets.Type.systemBars() or WindowInsets.Type.displayCutout())
            v.setPadding(b.left + dp(16), b.top + dp(12), b.right + dp(16), b.bottom + dp(12))
            insets
        }
        setContentView(scroll)
    }

    /** Every tap is guarded: an exception is shown, never a crash. */
    private fun safely(f: () -> Unit) {
        try { f() } catch (t: Throwable) { say("Error: $t") }
    }

    /** Runs on the screen thread of whichever activity is showing (the test thread calls these). */
    private fun onUi(f: MainActivity.() -> Unit) {
        val a = active ?: this
        a.ui.post { if (!a.isDestroyed) try { a.f() } catch (t: Throwable) { Log.w("kindling", t) } }
    }

    private fun say(s: String) = onUi {
        info.text = s
        info.visibility = if (s.isEmpty()) View.GONE else View.VISIBLE
    }

    private fun stepLine(s: String, overall: Double) = onUi {
        stepText.text = s
        progressText.text = ""
        bar.visibility = View.VISIBLE
        bar.progress = (overall.coerceIn(0.0, 1.0) * 1000).toInt()
    }

    /** A detail line under the step; a fraction in 0..1 moves the bar (the step's own progress). */
    private fun progress(s: String, fraction: Double) = onUi {
        progressText.text = s
        if (fraction in 0.0..1.0) {
            bar.visibility = View.VISIBLE
            bar.progress = (fraction * 1000).toInt()
        }
    }

    private fun showNotice(s: String?) = onUi {
        notice.text = s ?: ""
        notice.visibility = if (s == null) View.GONE else View.VISIBLE
        if (s != null) scroll.post { scroll.smoothScrollTo(0, notice.top) }
    }

    private fun showSkip(on: Boolean) = onUi {
        skipButton.visibility = if (on) View.VISIBLE else View.GONE
        skipButton.isEnabled = true
    }

    private fun refresh() {
        try { refreshInner() } catch (t: Throwable) { say("Display error: $t") }
    }

    private fun refreshInner() {
        val finished = store.flag("finished")
        val started = store.has("t0")
        val rating = rateBox.visibility == View.VISIBLE
        runButton.isEnabled = !running
        continueButton.visibility = if (!running && started && !store.flag("auto")) View.VISIBLE else View.GONE
        if (!running) { bar.visibility = View.GONE; skipButton.visibility = View.GONE }
        if (!running && finished) {
            stepText.text = "All done. Thank you."
            progressText.text = ""
        }
        // The code shows once everything is done, or when a run stopped part-way (a code so far is better than none).
        if (!running && !rating && started) showCode(finished)
        else for (v in listOf(codeLabel, codeView, copyButton, shareNote, shareButton)) v.visibility = View.GONE
        background {
            val present = GemmaTest.modelFile(this).let { it.exists() || File(it.path + ".part").exists() } ||
                GemmaTest.cacheDir(this).exists()
            onUi { deleteButton.visibility = if (present && !running && started) View.VISIBLE else View.GONE }
        }
    }

    /** Builds the code off the screen thread, then shows it. */
    private fun showCode(finished: Boolean) {
        background {
            val c = try {
                Logic.resultCode(store.snapshot().first, CODE_LIMIT)
            } catch (t: Throwable) {
                Logic.Code("error: $t", 0, "")
            }
            onUi {
                codeView.text = c.code
                codeLabel.text = if (finished) "Result code (${c.code.length} characters). Copy it and paste it back in the chat:"
                else "Result code so far (${c.code.length} characters; the tests are not finished):"
                for (v in listOf(codeLabel, codeView, copyButton)) v.visibility = View.VISIBLE
                val big = c.trim > 0
                shareNote.visibility = if (big) View.VISIBLE else View.GONE
                shareButton.visibility = if (big) View.VISIBLE else View.GONE
            }
        }
    }

    private fun copyCode() {
        getSystemService(ClipboardManager::class.java).setPrimaryClip(ClipData.newPlainText("Kindling result code", codeView.text))
        Toast.makeText(this, "Copied. Paste it back in the chat.", Toast.LENGTH_SHORT).show()
    }

    /** "Share results file": everything (results and detail) as one file, through the share sheet. */
    private fun shareFile() {
        background {
            try {
                val (results, full) = store.snapshot()
                val dir = File(store.dir, "share").apply { mkdirs() }
                val f = File(dir, "kindling-r2-results.txt")
                f.writeText(JSONObject().put("results", results).put("detail", full).toString())
                val uri = FileProvider.getUriForFile(this, "dev.kindling.pretests.files", f)
                val send = Intent(Intent.ACTION_SEND).apply {
                    type = "text/plain"
                    putExtra(Intent.EXTRA_STREAM, uri)
                    putExtra(Intent.EXTRA_SUBJECT, "Kindling phone test, round 2: results")
                    clipData = ClipData.newRawUri("results", uri)
                    addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                }
                onUi { safely { startActivity(Intent.createChooser(send, "Send the results file")) } }
            } catch (t: Throwable) {
                say("Could not share the file: $t")
            }
        }
    }

    private fun askDeleteModel() {
        AlertDialog.Builder(this).setMessage("Delete the Gemma model (3.3 GB)? A later test round would download it again.")
            .setPositiveButton("Delete") { _, _ ->
                background {
                    val freed = try { GemmaTest.deleteModel(this) } catch (_: Throwable) { -1L }
                    onUi {
                        Toast.makeText(this, if (freed >= 0) "Deleted (${Logic.gb(freed)} freed)." else "Could not delete it.", Toast.LENGTH_SHORT).show()
                        refresh()
                    }
                }
            }
            .setNegativeButton("Keep it", null).show()
    }

    private fun setRunning(on: Boolean) {
        running = on
        if (on) window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        else window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        if (Build.VERSION.SDK_INT >= 33) {
            if (on && backCallback == null) {
                val cb = OnBackInvokedCallback { Toast.makeText(this, "The tests are running. Please wait.", Toast.LENGTH_SHORT).show() }
                onBackInvokedDispatcher.registerOnBackInvokedCallback(OnBackInvokedDispatcher.PRIORITY_DEFAULT, cb)
                backCallback = cb
            } else if (!on) {
                backCallback?.let { onBackInvokedDispatcher.unregisterOnBackInvokedCallback(it) }
                backCallback = null
            }
        }
        refresh()
    }

    // ------------------------------------------------------------------ the run

    private fun askAndRun() {
        if (running) return
        if (store.has("t0")) {
            AlertDialog.Builder(this).setMessage("Start all tests again? The previous results will be replaced. (The Gemma model is kept.)")
                .setPositiveButton("Start again") { _, _ -> safely { startRun(false) } }
                .setNegativeButton("Cancel", null).show()
        } else startRun(false)
    }

    private fun startRun(continuing: Boolean) {
        if (running) return
        say("")
        rateBox.visibility = View.GONE
        setRunning(true)
        Thread {
            try {
                if (!continuing) store.reset(JSONObject().put("v", 2).put("app", BuildConfig.VERSION_NAME)
                    .put("t0", System.currentTimeMillis() / 1000))
                warnings()
                runSteps()
            } catch (t: Throwable) {
                store.error("run", t)
            }
            onUi {
                notice.visibility = View.GONE
                skipButton.visibility = View.GONE
                startRating()
            }
        }.apply { name = "kindling-tests" }.start()
    }

    /** What the owner may still fix (before-you-start list); the tests start anyway. */
    private fun warnings() {
        val warn = ArrayList<String>()
        val level = probe.level()
        if (probe.plugged() <= 0 && level in 0..59) warn.add("the battery is at $level% and not charging: please plug in the charger")
        if (!probe.onWifi()) warn.add("the phone is not on Wi-Fi, which the Gemma download needs")
        if (!store.isDone("gm") && !GemmaTest.download(this).ready()) {
            val free = probe.freeBytes()
            if (free in 0 until 4_000_000_000L) warn.add("only ${Logic.gb(free)} is free: Gemma needs 4 GB, or it is left out")
        }
        if (warn.isNotEmpty()) say("Note: ${warn.joinToString("; ")}. The tests start anyway.")
    }

    private val steps: List<Step> by lazy {
        listOf(
            Step("dev", "Phone details", 2) { store.put("dev", probe.device()) },
            Step("st", "Storage and terrain", 45) { storageStep() },
            Step("snd", "Sound", 45) { soundStep() },
            Step("nano", "Writer AI: Gemini Nano", 280) { nanoStep() },
            Step("gm", "Writer AI: Gemma", 330) { gemmaStep() },
        )
    }

    private fun runSteps() {
        val total = steps.sumOf { it.estS }.toDouble()
        var doneEst = 0.0
        for ((i, s) in steps.withIndex()) {
            if (store.isDone(s.id) || store.isCrashed(s.id)) { doneEst += s.estS; continue }
            val download = !store.isDone("gm") && !GemmaTest.download(this).ready()
            stepLine("Test ${i + 1} of ${steps.size}: ${s.title}. ${Logic.minutes(total - doneEst).replaceFirstChar { it.uppercase() }} left" +
                (if (download) ", plus Gemma's one-time download." else "."), doneEst / total)
            store.markStart(s.id)
            val t0 = SystemClock.elapsedRealtime()
            try {
                s.body()
            } catch (t: Throwable) {
                store.error(s.id, t)
            }
            store.markDone(s.id, (SystemClock.elapsedRealtime() - t0) / 1000.0)
            store.markEnd()
            doneEst += s.estS
        }
        store.put("auto", true)
    }

    /** Runs a blocking call on its own thread; gives up waiting (and moves on) after `ms`. */
    private fun <T> withTimeout(name: String, ms: Long, body: () -> T): T {
        val box = AtomicReference<Any?>()
        val err = AtomicReference<Throwable?>()
        val t = Thread({ try { box.set(body()) } catch (x: Throwable) { err.set(x) } }, name)
        t.isDaemon = true
        t.start()
        t.join(ms)
        if (t.isAlive) throw TimeoutException("$name still running after ${ms / 1000} s; moved on")
        err.get()?.let { throw it }
        @Suppress("UNCHECKED_CAST")
        return box.get() as T
    }

    /** B04/B11: pretests/b04-b11-storage-terrain/INTEGRATION.md, with {"dir": filesDir}; about 20 s. */
    private fun storageStep() {
        progress("Saving and loading a test world, then making terrain (about 20 seconds to a minute).", -1.0)
        if (!Storage.loaded) {
            store.put("st", JSONObject().put("error", "library not loaded: ${Storage.loadError}".take(200)))
            return
        }
        val config = JSONObject().put("dir", filesDir.absolutePath).toString()
        val out = withTimeout("storage", 10 * 60_000L) { Storage.run(config) }
        store.put("st", JSONObject(out))
    }

    /** B74: pretests/b74-b76-sound-speech/INTEGRATION.md, its default config; about 35 s of sound. */
    private fun soundStep() {
        showNotice("Sound test: the phone will play clicks and tones for about 35 seconds.")
        try {
            progress("", -1.0)
            SystemClock.sleep(3000) // time to read the notice before the sound starts
            val vol = probe.mediaVolume()
            if (!Sound.loaded) {
                store.put("snd", JSONObject().put("error", "library not loaded: ${Sound.loadError}".take(200)).put("vol", vol))
                return
            }
            val out = withTimeout("sound", 150_000L) { Sound.run(Sound.DEFAULT_CONFIG) }
            store.put("snd", JSONObject(out).put("vol", vol))
        } finally {
            showNotice(null)
        }
    }

    /** B73, Gemini Nano: pretests/b73-writer/INTEGRATION.md; its own limits (at most about 4.5 minutes) apply. */
    private fun nanoStep() {
        progress("Checking whether Gemini Nano is ready...", -1.0)
        WriterTest.onProgress = { m ->
            progress("Gemini Nano, " + m.replace("FULL:", "main model:").replace("FAST:", "its fast variant:") + ".", -1.0)
        }
        try {
            // WriterTest stops itself within its 270 s budget; the extra wait only guards against a hang.
            val r = withTimeout("gemini-nano", 400_000L) { WriterTest.run(applicationContext) }
            store.put("nano", r)
        } finally {
            WriterTest.onProgress = null
        }
    }

    /** B73, Gemma 4 E2B: pretests/b73-writer/GEMMA.md (GemmaTest.kt). */
    private fun gemmaStep() {
        val g = GemmaTest(applicationContext, store, probe, { s, f -> progress(s, f) }, { on -> showSkip(on) })
        gemma = g
        try {
            g.run()
        } finally {
            gemma = null
            showSkip(false)
        }
    }

    // ------------------------------------------------------------------ rating (B73 rule 6)

    private var plan: List<Logic.RateItem> = emptyList()
    private var rateIndex = 0
    private val ratings = LinkedHashMap<String, Int>() // "n|r04-doc" -> 0 poor, 1 acceptable, 2 good

    private fun key(it: Logic.RateItem) = "${it.model}|${it.id}"

    private fun startRating() {
        background {
            val nano = store.copyOf("nano")
            val gm = store.copyOf("gm")
            val saved = store.copyArray("rate")
            val p = Logic.ratingPlan(nano, gm)
            onUi {
                plan = p
                ratings.clear()
                if (saved != null) for (i in 0 until saved.length()) saved.optJSONObject(i)?.let {
                    ratings["${it.optString("m")}|${it.optString("i")}"] = it.optInt("r")
                }
                if (plan.isEmpty()) {
                    finishRun("Neither writer AI produced texts to rate.")
                    return@onUi
                }
                setRunning(true)
                rateIndex = plan.indexOfFirst { key(it) !in ratings }.let { if (it < 0) plan.size else it }
                if (rateIndex >= plan.size) {
                    finishRun(null)
                    return@onUi
                }
                beep()
                stepText.text = "The automatic tests are done."
                progressText.text = ""
                bar.visibility = View.GONE
                rateBox.visibility = View.VISIBLE
                showRateItem()
            }
        }
    }

    private fun modelName(m: String) = if (m == "n") "Gemini Nano" else "Gemma"
    private fun voiceName(id: String) = if (id.endsWith("doc")) "documentary voice (a calm field report)"
    else "their own tradition (as their storytellers tell it)"

    private fun showRateItem() {
        val it = plan[rateIndex]
        val before = ratings[key(it)]?.let { r -> " (you chose ${listOf("Poor", "Acceptable", "Good")[r]})" } ?: ""
        rateHead.text = "Text ${rateIndex + 1} of ${plan.size}, written by ${modelName(it.model)}$before"
        rateTitle.text = "${Logic.RECORD_TITLES[it.id.substringBefore('-')] ?: it.id}, in the ${voiceName(it.id)}"
        rateText.text = it.text
        factsText.text = WriterTest.PROMPTS.firstOrNull { p -> p.id == it.id }?.let { p -> Logic.dataBlock(p.text) } ?: ""
        factsText.visibility = View.GONE
        factsButton.text = "Show the facts it was written from"
        rateBack.visibility = if (rateIndex > 0) View.VISIBLE else View.GONE
        scroll.post { scroll.smoothScrollTo(0, rateBox.top) }
    }

    private fun rate(value: Int) {
        if (rateIndex !in plan.indices) return
        ratings[key(plan[rateIndex])] = value
        val arr = JSONArray()
        for (p in plan) ratings[key(p)]?.let { r -> arr.put(JSONObject().put("m", p.model).put("i", p.id).put("r", r)) }
        background { store.put("rate", arr) }
        rateIndex++
        if (rateIndex >= plan.size) finishRun(null) else showRateItem()
    }

    private fun finishRun(note: String?) {
        rateBox.visibility = View.GONE
        if (note != null) say(note)
        background {
            store.put("finished", true)
            onUi { setRunning(false) }
        }
    }

    /** A short tone when the phone needs the owner (the rating), so they can come back to it. */
    private fun beep() {
        try {
            val tg = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 80)
            tg.startTone(ToneGenerator.TONE_PROP_BEEP2, 400)
            ui.postDelayed({ runCatching { tg.release() } }, 1000)
        } catch (_: Throwable) {}
    }
}
