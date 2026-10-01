package dev.kindling.pretests

import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.nio.file.Files

/** B78/B79 robustness: results survive a crash, and the next launch finds the step that crashed. */
class StoreTest {
    @Test fun crashDuringAStepIsFoundOnTheNextLaunch() {
        val dir = Files.createTempDirectory("store").toFile()
        val first = Store(dir)
        first.reset(JSONObject().put("started", 1))
        first.markStart("dev"); first.put("dev", JSONObject().put("model", "x")); first.markDone("dev", 1.0); first.markEnd()
        first.markStart("k3") // ... and the process dies here

        val next = Store(dir) // next launch
        assertEquals("k3", next.leftoverMarker())
        next.markEnd()
        next.recordCrash("k3")
        assertTrue(next.isDone("dev"))
        assertTrue(next.isCrashed("k3"))
        assertFalse(next.isDone("k3"))
        assertEquals("x", next.results.getJSONObject("dev").getString("model"))
        assertNull(Store(dir).leftoverMarker())
        assertTrue(Store(dir).isCrashed("k3"))
    }

    @Test fun aHalfWrittenTempFileNeverReplacesResults() {
        val dir = Files.createTempDirectory("store").toFile()
        val s = Store(dir)
        s.reset(JSONObject().put("v", 1))
        java.io.File(dir, "results.json.tmp").writeText("{\"v\": 2, broken") // a write cut short
        assertEquals(1, Store(dir).results.getInt("v"))
    }
}
