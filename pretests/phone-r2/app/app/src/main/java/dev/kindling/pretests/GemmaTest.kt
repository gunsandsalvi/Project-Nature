package dev.kindling.pretests

import android.content.Context
import android.os.SystemClock
import com.google.ai.edge.litertlm.Backend
import com.google.ai.edge.litertlm.Conversation
import com.google.ai.edge.litertlm.ConversationConfig
import com.google.ai.edge.litertlm.Engine
import com.google.ai.edge.litertlm.EngineConfig
import com.google.ai.edge.litertlm.Message
import com.google.ai.edge.litertlm.MessageCallback
import com.google.ai.edge.litertlm.SamplerConfig
import com.google.ai.edge.litertlm.ThinkingConfig
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.util.Collections
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import java.util.concurrent.atomic.AtomicReference

/**
 * B73, Gemma 4 E2B on the phone (PRE-37; RSK-15, RSK-22), following pretests/b73-writer/GEMMA.md.
 * - Downloads the public Tensor G6 build once over Wi-Fi, resuming if cut (ModelDownload), after checking
 *   there is room; skips politely if not.
 * - Starts it with Google's LiteRT-LM runtime on the GOOGLE_TENSOR backend, or on GPU if that fails, and
 *   records which worked.
 * - Runs the same 20 prompts as Gemini Nano (WriterTest.PROMPTS) with the same settings: temperature 0.3,
 *   top-k 20, seed 73, at most 256 new tokens, and top-p 0.95 as in the cloud stand-in run. It records the
 *   same measures, plus the app's memory while it writes (decision rule 5).
 * - Keeps the model file for later rounds; only the owner's "Delete the model" button removes it.
 * - Crash-safe: a marker names each part (download, start-up on one backend, one prompt). If the app dies,
 *   the next launch records which part, and the test carries on from the next one.
 * Language models describe, never decide (PRN-06): nothing written here feeds back into anything.
 */
class GemmaTest(
    private val ctx: Context,
    private val store: Store,
    private val probe: Probe,
    /** Progress for the screen: a line of text, and the step's fraction done (negative: unknown). */
    private val show: (String, Double) -> Unit,
    /** Shows or hides the owner's "Skip Gemma" button (only during the download). */
    private val showSkip: (Boolean) -> Unit,
) {
    companion object {
        const val REPO = "litert-community/gemma-4-E2B-it-litert-lm"
        const val REVISION = "b3ca0d2f076785a8f4b2219ddbd2bdb99954eae1" // the repo's main on 1 October 2026
        const val FILE = "gemma-4-E2B-it_Google_Tensor_G6.litertlm"
        const val SIZE = 3_313_938_293L
        const val SHA256 = "f86c7c19c736e9307267946edef58b0a45494a9127036d54d90dd6b7617c95b7"
        const val URL = "https://huggingface.co/$REPO/resolve/$REVISION/$FILE"
        /** GOOGLE_TENSOR first, then GPU, as asked for this round. */
        val BACKENDS = listOf("GOOGLE_TENSOR", "GPU")
        const val BUDGET_MS = 270_000L // all prompts, as WriterTest's default budget for Gemini Nano
        const val PROMPT_MS = 60_000L // each prompt cut off at 60 s, as for Gemini Nano
        const val MIN_LEFT_MS = 15_000L // never start a prompt with less than 15 s left, as for Gemini Nano
        const val INIT_MS = 240_000L // start-up limit for one backend
        const val DOWNLOAD_MS = 45 * 60_000L
        const val MAX_CRASHES = 3
        val SAMPLER = SamplerConfig(topK = 20, topP = 0.95, temperature = 0.3, seed = 73)
        const val MAX_NEW_TOKENS = 256

        fun modelFile(ctx: Context) = File(File(ctx.filesDir, "models"), FILE)
        /** LiteRT-LM's own cache (compiled programs and the like), beside the model, so deleting removes both. */
        fun cacheDir(ctx: Context) = File(File(ctx.filesDir, "models"), "litertlm-cache")
        fun download(ctx: Context) = ModelDownload(URL, modelFile(ctx), SIZE, SHA256)

        /** The owner's "Delete the model" button: the model, any partial download and the runtime's cache. */
        fun deleteModel(ctx: Context): Long {
            val cache = cacheDir(ctx)
            val cached = cache.walkTopDown().filter { it.isFile }.sumOf { it.length() }
            cache.deleteRecursively()
            return download(ctx).deleteAll() + cached
        }

        fun label(b: String) = when (b) {
            "GOOGLE_TENSOR" -> "the Tensor chip's AI unit"
            "GPU" -> "the graphics chip"
            else -> b
        }

        private val LOG_WORDS = listOf("litert", "tflite", "tensor", "dispatch", "npu", "tpu", "gpu", "opencl",
            "engine", "backend", "model", "delegate", "accelerator", "error", "fail")
    }

    /** Set by the owner's Skip button. */
    @Volatile var skip = false

    private val gm: JSONObject = store.copyOf("gm") ?: JSONObject()
    @Volatile private var mem: Mem? = null
    private fun now() = SystemClock.elapsedRealtime()
    private fun save() = store.put("gm", JSONObject(gm.toString()), JSONObject().put("memSeries", mem?.series() ?: JSONArray()))
    private fun arr(key: String) = gm.optJSONArray(key) ?: JSONArray().also { gm.put(key, it) }
    private fun end(status: String) {
        gm.put("status", status)
        save()
    }

    fun run() {
        if (gm.has("status")) return // this test already ended (done, skipped or failed)
        gm.put("file", FILE)
        applyCrashes()
        val crashes = store.partCrashes("gm").size
        if (crashes > MAX_CRASHES) return end("failed: the app closed $crashes times during this test")
        save()
        val dl = ModelDownload(URL, modelFile(ctx), SIZE, SHA256,
            canUseNetwork = { probe.onWifi() }, stop = { stopReason() }, progress = ::downloadProgress)
        if (!dl.ready() && !download(dl)) return
        val m = Mem().also { it.start() }
        mem = m
        var engine: Engine? = null
        var stuck = false
        try {
            engine = start()
            if (engine != null) stuck = write(engine, m)
        } finally {
            gm.put("mem", m.stop())
            val e = engine
            if (e != null && !stuck) runCatching { e.close() }.onFailure { gm.put("closeErr", it.toString().take(120)) }
            runCatching { gm.put("cacheMB", cacheDir(ctx).walkTopDown().filter { it.isFile }.sumOf { it.length() } shr 20) }
        }
        end(when {
            engine == null && gm.optBoolean("hung") -> "failed: start-up never finished"
            engine == null -> "failed: no backend could run the model"
            stuck -> "done (a text never stopped)"
            else -> "done"
        })
    }

    // ------------------------------------------------------------------ crashes from earlier launches

    /** Turns crashes recorded at launch ("init-GPU", "run-r04-doc") into entries, once each. */
    private fun applyCrashes() {
        for (part in store.partCrashes("gm")) {
            when {
                part.startsWith("init-") -> {
                    val b = part.removePrefix("init-")
                    val tries = arr("tries")
                    if ((0 until tries.length()).none { tries.optJSONObject(it)?.optString("b") == b })
                        tries.put(JSONObject().put("b", b).put("ok", false).put("e", "the app closed during start-up"))
                }
                part.startsWith("run-") -> {
                    val id = part.removePrefix("run-")
                    val runs = arr("runs")
                    if ((0 until runs.length()).none { runs.optJSONObject(it)?.optString("id") == id })
                        runs.put(JSONObject().put("id", id).put("error_code", "APP_CLOSED"))
                }
            }
        }
    }

    // ------------------------------------------------------------------ download

    private var downloadStart = 0L

    private fun stopReason(): String? = when {
        skip -> "you tapped Skip"
        downloadStart > 0 && now() - downloadStart > DOWNLOAD_MS -> "over ${DOWNLOAD_MS / 60_000} minutes"
        else -> null
    }

    private fun downloadProgress(phase: String, done: Long, total: Long, bps: Double) {
        val f = if (total > 0) done.toDouble() / total else -1.0
        when (phase) {
            "wifi" -> show("Gemma needs Wi-Fi for its one-time download. Waiting for Wi-Fi (${done} s of ${total} s)...", -1.0)
            "check" -> show("Gemma: checking the downloaded file (${(f * 100).toInt()}%).", f)
            else -> {
                val left = if (bps > 1) Logic.minutes((total - done) / bps) + " left" else "starting"
                show("Downloading Gemma (once, 3.3 GB): ${Logic.gb(done)} so far, ${"%.0f".format(bps / 1e6)} MB/s, $left. " +
                    "You can tap Skip to leave Gemma out.", f)
            }
        }
    }

    /** True when the checked model file is in place; otherwise records why not. */
    private fun download(dl: ModelDownload): Boolean {
        val free = probe.freeBytes()
        val have = if (dl.part.isFile) dl.part.length() else 0L
        val need = Logic.needFreeBytes(SIZE, have)
        gm.put("free0", Logic.dp(free / 1e9, 2)).put("need", Logic.dp(need / 1e9, 2))
        if (free in 0 until need) {
            end("skipped: ${Logic.gb(free)} free, ${Logic.gb(need)} needed")
            return false
        }
        show("Gemma: connecting to download the model...", -1.0)
        downloadStart = now()
        showSkip(true)
        store.markStart("gm/dl")
        val status = try {
            dl.run()
        } catch (t: Throwable) {
            "failed: ${t.toString().take(120)}"
        } finally {
            showSkip(false)
            store.markStart("gm")
        }
        gm.put("dl", dl.log)
        return when {
            status == "done" || status == "ready" -> { save(); true }
            status.startsWith("stopped: you tapped Skip") -> { end("skipped: you tapped Skip during the download (what came is kept for next time)"); false }
            status.startsWith("stopped: over") -> { end("skipped: the download took ${status.removePrefix("stopped: ")} (what came is kept for next time)"); false }
            status.startsWith("stopped") -> { end("skipped: download ${status} (what came is kept for next time)"); false }
            status == "no-network" -> { end("skipped: not on Wi-Fi"); false }
            status == "bad-hash" -> { end("failed: the downloaded file was damaged, so it was deleted"); false }
            else -> { end("failed: download: ${status.removePrefix("failed: ")}"); false }
        }
    }

    // ------------------------------------------------------------------ start-up

    /** Starts the engine on the first backend that works (or the one that worked before a crash). */
    private fun start(): Engine? {
        val path = modelFile(ctx).path
        while (true) {
            val tries = arr("tries")
            val b = Logic.nextBackend(BACKENDS, tries) ?: return null
            val worked = (0 until tries.length()).any { tries.optJSONObject(it)?.let { t -> t.optString("b") == b && t.optBoolean("ok") } == true }
            show("Writer AI, Gemma: starting the model on ${label(b)}${if (worked) " again" else ""} (this can take a minute)...", -1.0)
            if (!gm.has("memBase")) gm.put("memBase", JSONArray(probe.procMem().toList()))
            store.markStart("gm/init-$b")
            val t0 = now()
            val (engine, err) = initEngine(path, b)
            store.markStart("gm")
            val ms = now() - t0
            if (worked) {
                if (engine == null) gm.put("restartErr", (err ?: "?").take(200))
                return engine
            }
            val t = JSONObject().put("b", b).put("ok", engine != null).put("ms", ms)
            if (err != null) t.put("e", err.take(300)).put("log", logTail())
            tries.put(t)
            if (engine != null) gm.put("be", b).put("init_ms", ms)
            save()
            if (engine != null) return engine
            if (gm.optBoolean("hung")) return null // a start-up still running: don't load the model twice
        }
    }

    private fun initEngine(path: String, b: String): Pair<Engine?, String?> {
        val backend = when (b) {
            "GOOGLE_TENSOR" -> Backend.GOOGLE_TENSOR()
            "GPU" -> Backend.GPU()
            else -> Backend.CPU()
        }
        // The runtime's cache goes in a folder of its own beside the model (Google's sample app lets it default to
        // the model's folder); its size is recorded, and "Delete the model" removes it too.
        val cache = cacheDir(ctx).apply { mkdirs() }
        val config = EngineConfig(modelPath = path, backend = backend, cacheDir = cache.path)
        val result = AtomicReference<Engine?>()
        val error = AtomicReference<Throwable?>()
        val gaveUp = AtomicBoolean(false)
        val lock = Any()
        val t = Thread({
            try {
                val e = Engine(config)
                e.initialize()
                synchronized(lock) { if (gaveUp.get()) runCatching { e.close() } else result.set(e) }
            } catch (x: Throwable) {
                error.set(x)
            }
        }, "gemma-start")
        t.isDaemon = true
        t.start()
        t.join(INIT_MS)
        synchronized(lock) {
            result.get()?.let { return it to null }
            if (t.isAlive) {
                // Still starting: let it finish (and close itself) in the background; never load the model twice.
                gaveUp.set(true)
                gm.put("hung", true)
                return null to "start-up still running after ${INIT_MS / 1000} s"
            }
        }
        return result.get() to error.get()?.toString()
    }

    /** The app's own recent warnings and errors (Android lets an app read its own log), for start-up failures. */
    private fun logTail(): JSONArray {
        val out = JSONArray()
        try {
            val p = ProcessBuilder("logcat", "-d", "-t", "500", "--pid=${android.os.Process.myPid()}", "*:W")
                .redirectErrorStream(true).start()
            val lines = Collections.synchronizedList(ArrayList<String>())
            val reader = Thread { runCatching { p.inputStream.bufferedReader().forEachLine { lines.add(it) } } }
            reader.isDaemon = true
            reader.start()
            reader.join(4000)
            p.destroy()
            val prefix = Regex("""^\S+\s+\S+\s+\d+\s+\d+\s+([VDIWEF])\s+""")
            val keep = synchronized(lines) { lines.toList() }
                .filter { l -> LOG_WORDS.any { l.contains(it, ignoreCase = true) } }
                .map { prefix.replace(it, "$1 ").take(220) }
                .distinct().takeLast(12)
            keep.forEach { out.put(it) }
        } catch (t: Throwable) {
            out.put("no log: ${t.toString().take(100)}")
        }
        return out
    }

    // ------------------------------------------------------------------ writing

    /**
     * Conversation settings, from ours down to the engine's own, in case a backend refuses some of them
     * (Google's own sample app passes no sampler settings to the phone's AI unit). The level used is recorded.
     */
    private fun conversationConfig(level: Int): ConversationConfig = when (level) {
        0 -> ConversationConfig(samplerConfig = SAMPLER, maxOutputToken = MAX_NEW_TOKENS, thinkingConfig = ThinkingConfig(enableThinking = false))
        1 -> ConversationConfig(samplerConfig = SAMPLER, maxOutputToken = MAX_NEW_TOKENS)
        2 -> ConversationConfig(maxOutputToken = MAX_NEW_TOKENS)
        else -> ConversationConfig()
    }

    private var level = 0
    private var anyText = false

    private fun newConversation(engine: Engine): Conversation {
        while (true) {
            try {
                return engine.createConversation(conversationConfig(level))
            } catch (t: Throwable) {
                if (level >= 3) throw t
                arr("cfgErr").put("level $level: ${t.toString().take(160)}")
                level++
                gm.put("cfg", level)
            }
        }
    }

    /** Runs the prompts not done yet; true if a text never stopped (then nothing is closed, to stay safe). */
    private fun write(engine: Engine, mem: Mem): Boolean {
        level = gm.optInt("cfg", 0)
        val runs = arr("runs")
        val done = HashSet<String>()
        for (i in 0 until runs.length()) runs.optJSONObject(i)?.let {
            done.add(it.optString("id"))
            if (!it.has("error_code")) anyText = true
        }
        if (!gm.has("before")) gm.put("before", probe.vitals())
        val deadline = now() + BUDGET_MS
        val prompts = WriterTest.PROMPTS
        mem.phase = "write"
        var stuck = false
        for (p in prompts) {
            if (p.id in done) continue
            val left = deadline - now()
            if (left < MIN_LEFT_MS) {
                gm.put("stopped", "time budget")
                break
            }
            show("Writer AI, Gemma on ${label(gm.optString("be"))}: text ${runs.length() + 1} of ${prompts.size}.",
                runs.length().toDouble() / prompts.size)
            store.markStart("gm/run-${p.id}")
            var r = runOne(engine, p, minOf(PROMPT_MS, left), mem)
            // The first texts failing on our settings: try the engine's own, as for creating a conversation.
            while (!anyText && level < 3 && r.optString("error_code") in setOf("ERROR", "EXCEPTION") &&
                deadline - now() >= MIN_LEFT_MS) {
                arr("cfgErr").put("level $level, ${p.id}: ${r.optString("error").take(160)}")
                level++
                gm.put("cfg", level)
                r = runOne(engine, p, minOf(PROMPT_MS, deadline - now()), mem)
            }
            store.markStart("gm")
            r.put("battery_c", probe.tempC()).put("thermal", probe.thermalStatus())
            runs.put(r)
            if (!r.has("error_code")) anyText = true
            save()
            if (r.optBoolean("stuck")) {
                stuck = true
                gm.put("stopped", "a text never stopped")
                break
            }
        }
        gm.put("cfg", level).put("summary", Logic.summary(runs)).put("after", probe.vitals())
        return stuck
    }

    private fun runOne(engine: Engine, p: WriterTest.Prompt, timeoutMs: Long, mem: Mem): JSONObject {
        val r = JSONObject().put("id", p.id).put("voice", p.voice).put("dark", p.dark)
        mem.newRun()
        var conv: Conversation? = null
        var stuck = false
        try {
            val c0 = now()
            conv = newConversation(engine)
            r.put("cv_ms", now() - c0)
            val first = AtomicLong(-1L)
            val text = StringBuffer()
            val chunks = AtomicInteger(0)
            val done = CountDownLatch(1)
            val error = AtomicReference<Throwable?>()
            val start = now()
            conv.sendMessageAsync(p.text, object : MessageCallback {
                override fun onMessage(message: Message) {
                    val s = message.toString()
                    if (s.isNotBlank()) first.compareAndSet(-1L, now())
                    text.append(s)
                    chunks.incrementAndGet()
                }

                override fun onDone() = done.countDown()

                override fun onError(throwable: Throwable) {
                    error.set(throwable)
                    done.countDown()
                }
            })
            val finished = done.await(timeoutMs, TimeUnit.MILLISECONDS)
            val end = now() // a text cut off at the limit counts up to the limit, as for Gemini Nano
            if (!finished) {
                runCatching { conv.cancelProcess() }
                stuck = !done.await(5_000, TimeUnit.MILLISECONDS)
            }
            val t = text.toString().trim()
            Logic.measures(r, start, first.get(), end, t)
            r.put("finish", if (!finished) "timeout" else if (error.get() != null) "error" else "done")
                .put("text", t).put("chunks", chunks.get())
            if (!stuck) runCatching { r.put("tok", conv.getTokenCount()) }
            when {
                !finished -> r.put("error_code", "TIMEOUT")
                error.get() != null -> r.put("error_code", "ERROR").put("error", error.get().toString().take(200))
            }
        } catch (t: Throwable) {
            r.put("error_code", "EXCEPTION").put("error", t.toString().take(200))
        } finally {
            if (stuck) r.put("stuck", true) else runCatching { conv?.close() }
        }
        r.put("mem_mb", mem.runPeak())
        return r
    }

    // ------------------------------------------------------------------ memory (decision rule 5)

    /**
     * Samples this process's memory while Gemma is loaded: resident memory (RSS) twice a second, split into
     * anonymous and file-backed pages (the model file is mapped, so much of it is file-backed and reclaimable),
     * plus PSS, graphics memory and the phone's free memory every 4 s. Keeps the peaks per phase and per text.
     */
    private inner class Mem {
        @Volatile var phase = "init"
        private val running = AtomicBoolean(true)
        private val peaks = HashMap<String, LongArray>() // phase -> rss, anon, file, pss, gfx
        private val memSeries = JSONArray() // every 4 s: seconds, phase, rss, anon, file, pss, graphics, phone's free
        @Volatile private var runMax = 0L
        private var availMin = Long.MAX_VALUE
        private var thread: Thread? = null
        private val t0 = now()

        @Synchronized fun series(): JSONArray = JSONArray(memSeries.toString())

        fun start() {
            thread = Thread({
                var i = 0
                while (running.get()) {
                    try { sample(i % 8 == 0) } catch (_: Throwable) {}
                    i++
                    SystemClock.sleep(500)
                }
            }, "gemma-memory").also { it.isDaemon = true; it.start() }
        }

        @Synchronized private fun sample(full: Boolean) {
            val m = probe.procMem() // MiB: rss, anon, file, peak
            val p = peaks.getOrPut(phase) { longArrayOf(-1, -1, -1, -1, -1) }
            for (k in 0..2) p[k] = maxOf(p[k], m[k])
            runMax = maxOf(runMax, m[0])
            if (full) {
                val (pss, gfx) = probe.pssAndGraphics()
                p[3] = maxOf(p[3], pss)
                p[4] = maxOf(p[4], gfx)
                val avail = probe.memInfo().availMem shr 20
                availMin = minOf(availMin, avail)
                if (memSeries.length() < 400) memSeries.put(JSONArray(listOf((now() - t0) / 1000, phase, m[0], m[1], m[2], pss, gfx, avail)))
            }
        }

        fun newRun() { runMax = 0L }
        fun runPeak(): Long = runMax

        @Synchronized fun stop(): JSONObject {
            running.set(false)
            val o = JSONObject().put("units", "MiB: rss, anon, file, pss, graphics")
            for ((k, v) in peaks) o.put(k, JSONArray(v.toList()))
            if (availMin != Long.MAX_VALUE) o.put("availMin", availMin)
            o.put("hwm", probe.procMem()[3])
            return o
        }
    }
}
