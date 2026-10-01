package dev.kindling.pretests

import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.graphics.Typeface
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.util.Log
import android.util.TypedValue
import android.view.View
import android.view.ViewGroup
import android.view.WindowInsets
import android.view.WindowManager
import android.widget.Button
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import android.window.OnBackInvokedCallback
import android.window.OnBackInvokedDispatcher
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.util.concurrent.atomic.AtomicInteger

/**
 * B78/B79 round-1 phone test app: one screen, one tap, about 15 minutes, one result code.
 * Runs the steps in order; writes a marker before each step and the results after it, so a
 * crash is reported on the next launch and the rest can continue (RSK-23: one install, one paste).
 */
class MainActivity : Activity() {
    companion object {
        private const val KERNEL_BUDGET_S = 300.0 // B01/B02 phone side: about 5 minutes
        private const val REPEAT_S = 0.25 // the same-core repeat only checks the checksum (X11)
        private const val SUSTAINED_S = 600 // B79: 10 minutes of all-core load
        private const val FRAME_S = 20
        private const val WEB_S = 30 // optional drawing test (B66)
        private var crashHandlerInstalled = false
    }

    private class Step(val id: String, val title: String, val estS: Int, val body: () -> Unit)

    private lateinit var store: Store
    private lateinit var probe: Probe
    private lateinit var tests: Tests
    private val ui = Handler(Looper.getMainLooper())
    private val trimLevel = AtomicInteger(0)
    @Volatile private var running = false
    private var backCallback: OnBackInvokedCallback? = null

    private lateinit var info: TextView
    private lateinit var runButton: Button
    private lateinit var continueButton: Button
    private lateinit var progressText: TextView
    private lateinit var bar: ProgressBar
    private lateinit var stage: FrameLayout
    private lateinit var codeLabel: TextView
    private lateinit var codeView: TextView
    private lateinit var copyButton: Button
    private lateinit var memButton: Button
    private lateinit var scroll: ScrollView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        installCrashHandler()
        store = Store(filesDir)
        probe = Probe(this)
        tests = Tests(probe, store)
        buildUi()
        try {
            checkLastLaunch()
        } catch (t: Throwable) {
            say("Could not read the last results: $t")
        }
        refresh()
    }

    // ------------------------------------------------------------------ robustness

    /** Uncaught exceptions on any thread are written down before the app dies, and reported next launch. */
    private fun installCrashHandler() {
        if (crashHandlerInstalled) return
        crashHandlerInstalled = true
        val previous = Thread.getDefaultUncaughtExceptionHandler()
        val file = File(filesDir, "uncaught.txt")
        Thread.setDefaultUncaughtExceptionHandler { thread, e ->
            try { file.writeText("${thread.name}: ${Log.getStackTraceString(e).take(1500)}") } catch (_: Throwable) {}
            previous?.uncaughtException(thread, e)
        }
    }

    private fun checkLastLaunch() {
        val marker = store.leftoverMarker()
        if (marker != null) {
            store.markEnd()
            if (marker == "mem") {
                store.put("mem", tests.memoryFromProgress())
                say("The memory test ended with Android closing the app. That is one of its expected endings; its result is now in the code below.")
            } else {
                store.recordCrash(marker)
                say("Last time, the app closed during the test \"$marker\". Tap Continue to run the remaining tests.")
            }
        }
        if (store.uncaughtFile.exists()) {
            store.put("uncaught", store.uncaughtFile.readText().take(300))
            store.uncaughtFile.delete()
        }
    }

    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
        trimLevel.accumulateAndGet(level) { a, b -> maxOf(a, b) }
    }

    // ------------------------------------------------------------------ screen

    private fun dp(x: Int) = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, x.toFloat(), resources.displayMetrics).toInt()

    private fun text(s: String, sp: Float = 16f, bold: Boolean = false) = TextView(this).apply {
        text = s
        setTextSize(TypedValue.COMPLEX_UNIT_SP, sp)
        if (bold) setTypeface(typeface, Typeface.BOLD)
        setPadding(0, dp(6), 0, dp(6))
    }

    private fun buildUi() {
        val col = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
        col.addView(text("Kindling phone test, round 1", 22f, true))
        col.addView(text(
            "Before you start:\n" +
                "• Unplug the charger.\n" +
                "• Battery above 50%.\n" +
                "• Close other apps.\n" +
                "• Lay the phone flat on a table.\n" +
                "• Don't touch it until the tests finish (about 15 minutes). The screen stays on.",
        ))
        info = text("", 15f).apply { setTextColor(0xFFB3261E.toInt()); visibility = View.GONE }
        col.addView(info)
        runButton = Button(this).apply { text = "Run all tests (about 15 minutes)"; setOnClickListener { safely { askAndRun() } } }
        col.addView(runButton)
        continueButton = Button(this).apply {
            text = "Continue with the remaining tests"; visibility = View.GONE; setOnClickListener { safely { startRun(true) } }
        }
        col.addView(continueButton)
        progressText = text("", 15f)
        col.addView(progressText)
        bar = ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal).apply { max = 1000; visibility = View.GONE }
        col.addView(bar)
        stage = FrameLayout(this).apply { visibility = View.GONE }
        col.addView(stage, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, (resources.displayMetrics.heightPixels * 0.8).toInt()))
        codeLabel = text("Result code. Copy it and paste it back in the chat:", 16f, true).apply { visibility = View.GONE }
        col.addView(codeLabel)
        codeView = text("", 11f).apply { typeface = Typeface.MONOSPACE; setTextIsSelectable(true); visibility = View.GONE }
        col.addView(codeView)
        copyButton = Button(this).apply { text = "Copy result code"; visibility = View.GONE; setOnClickListener { safely { copyCode() } } }
        col.addView(copyButton)
        col.addView(text("Optional, after the main tests:", 15f, true))
        memButton = Button(this).apply { text = "Memory test (may close the app)"; setOnClickListener { safely { startMemoryTest() } } }
        col.addView(memButton)
        col.addView(text(
            "If the memory test closes the app, open it again: the result code then includes how far it got.\n\n" +
                "App ${BuildConfig.VERSION_NAME}, benchmark library: ${BuildConfig.KBENCH}. Results are also saved in the app's own files.",
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

    /** Every tap is guarded: a Kotlin exception is shown, never a crash. */
    private fun safely(f: () -> Unit) {
        try { f() } catch (t: Throwable) { say("Error: $t") }
    }

    private fun say(s: String) = ui.post {
        info.text = s
        info.visibility = if (s.isEmpty()) View.GONE else View.VISIBLE
    }

    private fun progress(s: String, fraction: Double) = ui.post {
        progressText.text = s
        bar.visibility = if (running) View.VISIBLE else View.GONE
        bar.progress = (fraction.coerceIn(0.0, 1.0) * 1000).toInt()
    }

    /** Shows or hides the area where the drawing tests run, scrolled into view. */
    private fun showStage(on: Boolean) = ui.post {
        stage.visibility = if (on) View.VISIBLE else View.GONE
        scroll.post { scroll.scrollTo(0, if (on) stage.top else 0) }
    }

    private fun refresh() {
        try { refreshInner() } catch (t: Throwable) { say("Display error: $t") }
    }

    private fun refreshInner() {
        val r = store.results
        val finished = r.optBoolean("finished")
        val started = r.has("started")
        runButton.isEnabled = !running
        memButton.isEnabled = !running
        continueButton.visibility = if (!running && started && !finished) View.VISIBLE else View.GONE
        if (!running) bar.visibility = View.GONE
        val showCode = !running && (finished || r.has("mem") || r.has("crash"))
        if (showCode) {
            val code = try { Logic.resultCode(r) } catch (t: Throwable) { "error: $t" }
            codeView.text = code
            codeLabel.text = if (finished) "Result code (${code.length} characters). Copy it and paste it back in the chat:"
            else "Result code so far (${code.length} characters):"
        }
        for (v in listOf(codeLabel, codeView, copyButton)) v.visibility = if (showCode) View.VISIBLE else View.GONE
        if (!running && finished) progressText.text = "All tests done."
    }

    private fun copyCode() {
        getSystemService(ClipboardManager::class.java).setPrimaryClip(ClipData.newPlainText("Kindling result code", codeView.text))
        Toast.makeText(this, "Copied. Paste it back in the chat.", Toast.LENGTH_SHORT).show()
    }

    private fun setRunning(on: Boolean) {
        running = on
        if (on) window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        else window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        if (Build.VERSION.SDK_INT >= 33) {
            if (on && backCallback == null) {
                val cb = OnBackInvokedCallback { Toast.makeText(this, "Tests are running. Please wait.", Toast.LENGTH_SHORT).show() }
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
        if (store.results.has("started")) {
            AlertDialog.Builder(this).setMessage("Start all tests again? The previous results will be replaced.")
                .setPositiveButton("Start again") { _, _ -> safely { startRun(false) } }
                .setNegativeButton("Cancel", null).show()
        } else startRun(false)
    }

    private fun startRun(continuing: Boolean) {
        if (running) return
        val warn = ArrayList<String>()
        if (probe.plugged() > 0) warn.add("the charger is connected")
        if (probe.level() in 0..49) warn.add("the battery is below 50%")
        say(if (warn.isEmpty()) "" else "Note: ${warn.joinToString(" and ")}. The tests run anyway; results are less clean.")
        if (!continuing) {
            val keepMem = store.results.optJSONObject("mem")
            store.reset(JSONObject().put("v", 1).put("app", BuildConfig.VERSION_NAME).put("lib", BuildConfig.KBENCH)
                .put("started", System.currentTimeMillis() / 1000))
            if (keepMem != null) store.put("mem", keepMem)
        }
        setRunning(true)
        Thread {
            try {
                runSteps()
            } catch (t: Throwable) {
                store.error("run", t)
            }
            ui.post {
                stage.visibility = View.GONE
                setRunning(false)
            }
        }.apply { name = "kindling-tests" }.start()
    }

    /** The kernel run list (B01/B02 phone side): the library's plan, then the managed-code kernels. */
    private fun kernelPlan(): List<Pair<JSONObject, Boolean>> {
        val out = ArrayList<Pair<JSONObject, Boolean>>()
        if (Bench.loaded) try {
            Logic.planFrom(JSONObject(Bench.run("""{"list":true}"""))).forEach { out.add(it to false) }
        } catch (_: Throwable) {}
        if (Managed.available) try {
            Logic.planFrom(JSONObject(Managed.run("""{"list":true}"""))).forEach { out.add(it to true) }
        } catch (_: Throwable) {}
        return out
    }

    private var clusters: List<Logic.Cluster> = emptyList()
    private var kUsed = 0.0 // kernel-phase seconds used so far
    private var kFixedSum = 0.0 // per config: time beyond the timed seconds (setup, warm-up, repeats)
    private var kFixedN = 0
    private var totalEst = 1.0
    private var doneEst = 0.0

    private fun findClusters(plan: List<Pair<JSONObject, Boolean>>): List<Logic.Cluster> {
        val (cl, _) = probe.clusters()
        if (cl != null) return cl
        // Cluster files unreadable: group the cores by measured speed instead.
        val first = plan.firstOrNull { !it.second }?.first ?: return listOf(Logic.Cluster((0 until probe.ncpu).toList(), 0))
        val speed = DoubleArray(probe.ncpu) { cpu -> tests.runOne(first, cpu, 1, 0.3, false).opsPerSec.let { if (it.isNaN()) 0.0 else it } }
        store.put("speedProbe", JSONArray(speed.map { Logic.num(it) }))
        return Logic.clustersBySpeed(speed)
    }

    private fun buildSteps(plan: List<Pair<JSONObject, Boolean>>): List<Step> {
        val steps = ArrayList<Step>()
        steps.add(Step("dev", "Phone details", 2) { store.put("dev", probe.device()) })
        steps.add(Step("lib", "Library check", 3) { libStep(plan) })
        steps.add(Step("idle", "Resting baseline", 10) { store.put("idle", tests.idle(10)) })
        steps.add(Step("fp", "Smoothness (OpenGL)", FRAME_S + 3) {
            showStage(true)
            try { store.put("fp", FrameTest(this, stage).run(FRAME_S)) } finally { showStage(false) }
        })
        if (WebTest.available(this)) steps.add(Step("wv", "Drawing test (WebView)", WEB_S + 8) {
            showStage(true)
            try { store.put("wv", WebTest(this, stage).run(WEB_S)) } finally { showStage(false) }
        })
        val perConfig = (KERNEL_BUDGET_S / maxOf(1, plan.size)).toInt()
        plan.forEachIndexed { i, (cfg, managed) ->
            steps.add(Step("k$i", Logic.label(cfg), perConfig) { kernelStep(i, cfg, managed, plan) })
        }
        steps.add(Step("sus", "Sustained load", SUSTAINED_S) { sustainedStep(plan) })
        return steps
    }

    private fun runSteps() {
        val plan = kernelPlan()
        clusters = findClusters(plan)
        val steps = buildSteps(plan)
        totalEst = steps.sumOf { it.estS }.toDouble()
        doneEst = 0.0
        val times = store.results.optJSONObject("time")
        kUsed = plan.indices.sumOf { times?.optDouble("k$it", 0.0) ?: 0.0 }
        kFixedSum = 0.0
        kFixedN = 0
        for ((i, s) in steps.withIndex()) {
            if (store.isDone(s.id) || store.isCrashed(s.id)) { doneEst += s.estS; continue }
            val left = ((totalEst - doneEst) / 60).toInt() + 1
            progress("Test ${i + 1} of ${steps.size}: ${s.title}. About $left min left.", doneEst / totalEst)
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
        store.put("finished", true)
        progress("All tests done.", 1.0)
    }

    private fun libStep(plan: List<Pair<JSONObject, Boolean>>) {
        val o = JSONObject().put("loaded", Bench.loaded).put("kphone", Native.loaded).put("managed", Managed.available)
            .put("plan", plan.size).put("clusters", JSONArray(clusters.map { c -> Logic.jsonInts(c.cpus) }))
        Bench.loadError?.let { o.put("loadErr", it.take(150)) }
        if (Bench.loaded) try {
            val st = JSONObject(Bench.run("""{"selftest":true}"""))
            o.put("selftest", st.opt("selftest") ?: JSONObject.NULL)
            st.optJSONArray("failures")?.let { if (it.length() > 0) o.put("fails", it) }
        } catch (t: Throwable) {
            o.put("selftestErr", t.toString().take(100))
        }
        store.put("lib", o)
    }

    /** One kernel config: on each cluster, a timed run and a short repeat on the same core; then all cores. */
    private fun kernelStep(i: Int, cfg: JSONObject, managed: Boolean, plan: List<Pair<JSONObject, Boolean>>) {
        val r = store.results
        val k = r.optJSONObject("k") ?: JSONObject()
            .put("cl", JSONArray(clusters.map { c -> JSONArray().put(Logic.representative(c)).put(c.maxKHz / 1000) }))
            .put("lab", JSONArray()).put("s", JSONArray()).put("ops", JSONArray()).put("mw", JSONArray())
            .put("ck", JSONArray()).put("rep", "").put("x", "").put("pinMiss", 0)
        // Seconds per timed run, so the whole kernel phase ends near its budget.
        val fixed = if (kFixedN == 0) 2.5 else kFixedSum / kFixedN
        val seconds = Logic.secondsPerRun(KERNEL_BUDGET_S - kUsed, plan.size - i, clusters.size, fixed)
        val t0 = SystemClock.elapsedRealtime()

        val ops = JSONArray(); val mw = JSONArray(); val runs = JSONArray()
        var ck: String? = null
        var repOk = true; var same = true; var err = false; var pinMiss = 0
        fun note(tag: String, run: Tests.Run) {
            runs.put(JSONObject().put("on", tag).put("ops", Logic.num(run.opsPerSec, 4)).put("ck", run.checksum)
                .put("cons", run.consistent).put("seen", Logic.jsonInts(run.cpusSeen)).put("mA", Logic.num(run.meanUa / 1000))
                .put("mV", run.mv).put("wall", Logic.dp(run.wallS, 2)).put("err", run.error ?: JSONObject.NULL))
        }
        for ((ci, c) in clusters.withIndex()) {
            val cpu = Logic.representative(c)
            val a = tests.runOne(cfg, cpu, 1, seconds, managed)
            val b = tests.runOne(cfg, cpu, 1, REPEAT_S, managed)
            note("c$ci", a); note("c${ci}r", b)
            ops.put(Logic.num(a.opsPerSec)); mw.put(Logic.num(a.mw, 2))
            if (!a.ok || !b.ok) { err = true; continue }
            if (a.checksum != b.checksum || !a.consistent || !b.consistent) repOk = false
            if (ck == null) ck = a.checksum else if (a.checksum != ck) same = false
            if (!managed && (a.cpusSeen + b.cpusSeen).any { it != cpu }) pinMiss++
        }
        val all = tests.runOne(cfg, null, probe.ncpu, seconds, managed)
        note("all", all)
        ops.put(Logic.num(all.opsPerSec)); mw.put(Logic.num(all.mw, 2))
        if (all.ok) { if (ck == null) ck = all.checksum else if (all.checksum != ck) same = false; if (!all.consistent) same = false } else err = true

        val wall = (SystemClock.elapsedRealtime() - t0) / 1000.0
        kUsed += wall
        kFixedSum += (wall - (clusters.size + 1) * seconds).coerceAtLeast(0.0)
        kFixedN++
        k.getJSONArray("lab").put(Logic.label(cfg))
        k.getJSONArray("s").put(Logic.dp(seconds, 2))
        k.getJSONArray("ops").put(ops)
        k.getJSONArray("mw").put(mw)
        k.getJSONArray("ck").put(ck?.take(8) ?: JSONObject.NULL)
        // rep: same-core repeats matched (X11); x: every run of this config gave the same checksum.
        k.put("rep", k.optString("rep") + if (err && ck == null) "-" else if (repOk) "1" else "0")
        k.put("x", k.optString("x") + if (err && ck == null) "-" else if (same) "1" else "0")
        k.put("pinMiss", k.optInt("pinMiss") + pinMiss)
        if (err) k.put("errs", k.optInt("errs") + 1)
        val detail = store.full.optJSONObject("k") ?: JSONObject().also { store.full.put("k", it) }
        detail.put("k$i", JSONObject().put("cfg", cfg).put("managed", managed).put("runs", runs))
        store.put("k", k)
    }

    private fun sustainedStep(plan: List<Pair<JSONObject, Boolean>>) {
        val native = plan.filter { !it.second }.map { it.first }
        val cfg = native.firstOrNull { it.optString("kernel") == "rust:heat" && it.optString("format") == "f32" }
            ?: native.firstOrNull() ?: throw IllegalStateException("no native kernel for the sustained load")
        val (summary, detail) = tests.sustained(cfg, SUSTAINED_S, clusters) { s ->
            if (s % 5 == 0) progress("Sustained load: ${s / 60} min ${s % 60} s of 10 min. Please don't touch the phone.",
                (doneEst + s) / totalEst)
        }
        store.put("sus", summary, detail)
    }

    // ------------------------------------------------------------------ memory test (separate button)

    private fun startMemoryTest() {
        if (running) return
        AlertDialog.Builder(this)
            .setMessage("This fills the phone's memory step by step, up to 10 GiB. Android may close the app; if it does, just open it again.")
            .setPositiveButton("Start") { _, _ -> safely { runMemoryTest() } }
            .setNegativeButton("Cancel", null).show()
    }

    private fun runMemoryTest() {
        say("")
        trimLevel.set(0)
        setRunning(true)
        Thread {
            store.markStart("mem")
            try {
                val r = tests.memory(trimLevel) { mib -> progress("Memory test: %.2f GiB of 10".format(mib / 1024.0), mib / 10240.0) }
                store.put("mem", r)
            } catch (t: Throwable) {
                store.error("mem", t)
            }
            store.markEnd()
            ui.post { setRunning(false) }
        }.apply { name = "kindling-memory" }.start()
    }
}
