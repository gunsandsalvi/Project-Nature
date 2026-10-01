package dev.kindling.pretests

import com.sun.net.httpserver.HttpExchange
import com.sun.net.httpserver.HttpServer
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import java.io.File
import java.net.InetSocketAddress
import java.nio.file.Files
import java.security.MessageDigest
import java.util.concurrent.atomic.AtomicInteger
import kotlin.random.Random

/**
 * B73, the Gemma download, against a local server shaped like Hugging Face's: the file's link answers with a
 * redirect to a storage host, which serves byte ranges. Checks resuming after a cut, a server that ignores
 * ranges, a damaged file, the owner's Skip, a missing file and no Wi-Fi.
 */
class DownloadTest {
    private val blob = Random(7).nextBytes(5_000_000 + 123)
    private val sha = MessageDigest.getInstance("SHA-256").digest(blob).joinToString("") { "%02x".format(it) }
    private lateinit var server: HttpServer
    private lateinit var base: String
    private val requests = AtomicInteger(0)
    private val ranged = AtomicInteger(0)
    @Volatile private var cutFirstAt = -1 // cut the first body after this many bytes
    @Volatile private var ignoreRange = false
    @Volatile private var redirects = true

    @Before fun start() {
        server = HttpServer.create(InetSocketAddress("127.0.0.1", 0), 0)
        server.createContext("/repo/resolve/main/model.bin") { ex ->
            if (redirects) {
                ex.responseHeaders.add("Location", "/cdn/abc?signed=1")
                ex.sendResponseHeaders(302, -1)
                ex.close()
            } else serve(ex)
        }
        server.createContext("/cdn/abc") { ex -> serve(ex) }
        server.start()
        base = "http://127.0.0.1:${server.address.port}"
    }

    @After fun stop() = server.stop(0)

    private fun serve(ex: HttpExchange) {
        val n = requests.incrementAndGet()
        val range = ex.requestHeaders.getFirst("Range")
        var from = 0
        if (range != null && !ignoreRange) {
            from = range.removePrefix("bytes=").substringBefore('-').toInt()
            ranged.incrementAndGet()
            ex.responseHeaders.add("Content-Range", "bytes $from-${blob.size - 1}/${blob.size}")
            ex.sendResponseHeaders(206, (blob.size - from).toLong())
        } else {
            ex.sendResponseHeaders(200, blob.size.toLong())
        }
        try {
            ex.responseBody.use { out ->
                val end = if (n == 1 && cutFirstAt > 0) cutFirstAt else blob.size
                var i = from
                while (i < end) {
                    val m = minOf(64 * 1024, end - i)
                    out.write(blob, i, m)
                    i += m
                }
                if (end < blob.size) throw java.io.IOException("cut on purpose")
            }
        } catch (_: java.io.IOException) {
            // A cut connection: the client sees the body end early.
        } finally {
            ex.close()
        }
    }

    private fun dir() = Files.createTempDirectory("dl").toFile()

    private fun download(dest: File, hash: String = sha, wifi: () -> Boolean = { true }, stop: () -> String? = { null }) =
        ModelDownload("$base/repo/resolve/main/model.bin", dest, blob.size.toLong(), hash,
            canUseNetwork = wifi, stop = stop, sleepMs = {}, networkWaitMs = 50, readTimeoutMs = 5_000)

    @Test fun downloadsThroughTheRedirectAndChecksTheFile() {
        val dest = File(dir(), "models/model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertTrue(dest.readBytes().contentEquals(blob))
        assertTrue(d.ready())
        assertFalse(d.part.exists())
        assertEquals("127.0.0.1", d.log.getString("host"))
        // A second run finds it ready, without the network.
        val again = download(dest, wifi = { false })
        assertEquals("ready", again.run())
    }

    @Test fun resumesAfterTheConnectionIsCut() {
        cutFirstAt = 1_500_000
        val dest = File(dir(), "model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertTrue(dest.readBytes().contentEquals(blob))
        assertEquals(1, d.log.getInt("resumes"))
        assertTrue(ranged.get() >= 1)
        assertEquals(blob.size.toLong(), d.log.getLong("got"))
    }

    @Test fun resumesAPartialFileFromAnEarlierRun() {
        val dest = File(dir(), "model.bin")
        File(dest.path + ".part").writeBytes(blob.copyOf(2_000_000))
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(2_000_000L, d.log.getLong("have0"))
        assertEquals((blob.size - 2_000_000).toLong(), d.log.getLong("got"))
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun startsAgainWhenTheServerIgnoresTheRange() {
        ignoreRange = true
        val dest = File(dir(), "model.bin")
        File(dest.path + ".part").writeBytes(blob.copyOf(1_000_000))
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(1, d.log.getInt("restarts"))
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun aDamagedFileIsDeletedNotUsed() {
        val dest = File(dir(), "model.bin")
        val d = download(dest, hash = "0".repeat(64))
        assertEquals("bad-hash", d.run())
        assertFalse(dest.exists())
        assertFalse(d.part.exists())
        assertFalse(d.ready())
    }

    @Test fun skipKeepsWhatCameForNextTime() {
        cutFirstAt = 1_000_000
        val dest = File(dir(), "model.bin")
        var calls = 0
        val d = download(dest, stop = { if (++calls > 3) "you tapped Skip" else null })
        assertTrue(d.run().startsWith("stopped: you tapped Skip"))
        assertFalse(dest.exists())
        assertTrue(d.part.length() > 0)
        // Next time it carries on and finishes.
        assertEquals("done", download(dest).run())
        assertTrue(dest.readBytes().contentEquals(blob))
    }

    @Test fun aMissingFileFailsAtOnceAndNoWifiWaitsThenGivesUp() {
        val missing = ModelDownload("$base/repo/resolve/main/nothing.bin", File(dir(), "x.bin"), 10, sha, sleepMs = {})
        assertEquals("failed: HTTP 404", missing.run())
        val noWifi = download(File(dir(), "model.bin"), wifi = { false })
        assertEquals("no-network", noWifi.run())
        assertEquals(0, requests.get())
    }

    @Test fun deleteRemovesTheFileAndAnyPart() {
        val dest = File(dir(), "model.bin")
        val d = download(dest)
        assertEquals("done", d.run())
        assertEquals(blob.size.toLong(), d.deleteAll())
        assertFalse(dest.exists() || d.part.exists() || d.okFile.exists())
    }
}
