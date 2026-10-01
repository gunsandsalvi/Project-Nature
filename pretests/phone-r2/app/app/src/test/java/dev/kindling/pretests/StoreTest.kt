package dev.kindling.pretests

import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

/** B78 robustness, round 2: results survive a crash, and the next launch finds what crashed. */
class StoreTest {
    private fun dir() = File(Files.createTempDirectory("store").toFile(), "r2")

    @Test fun crashDuringAStepIsFoundOnTheNextLaunch() {
        val dir = dir()
        val first = Store(dir)
        first.reset(JSONObject().put("t0", 1))
        first.markStart("st"); first.put("st", JSONObject().put("total_s", 20.1)); first.markDone("st", 20.0); first.markEnd()
        first.markStart("snd") // ... and the process dies here

        val next = Store(dir) // next launch
        assertEquals("snd", next.leftoverMarker())
        next.markEnd()
        next.recordCrash("snd")
        assertTrue(next.isDone("st"))
        assertTrue(next.isCrashed("snd"))
        assertFalse(next.isDone("snd"))
        assertEquals(20.1, next.results.getJSONObject("st").getDouble("total_s"), 0.0)
        assertNull(Store(dir).leftoverMarker())
        assertTrue(Store(dir).isCrashed("snd"))
    }

    @Test fun aCrashInsideAPartOfAStepLetsTheStepCarryOn() {
        val dir = dir()
        val s = Store(dir)
        s.reset(JSONObject().put("t0", 1))
        s.markStart("gm/init-GPU") // the app dies while the model starts on one backend
        val next = Store(dir)
        val marker = next.leftoverMarker()!!
        assertEquals("gm", marker.substringBefore('/'))
        assertEquals(1, next.recordPartCrash("gm", marker.substringAfter('/')))
        assertFalse(next.isCrashed("gm"))
        assertEquals(listOf("init-GPU"), Store(dir).partCrashes("gm"))
        assertEquals(2, Store(dir).recordPartCrash("gm", "run-r04-doc"))
    }

    @Test fun aHalfWrittenTempFileNeverReplacesResults() {
        val dir = dir()
        val s = Store(dir)
        s.reset(JSONObject().put("v", 2))
        File(dir, "results.json.tmp").writeText("{\"v\": 3, broken") // a write cut short
        assertEquals(2, Store(dir).results.getInt("v"))
    }

    @Test fun copiesAreIndependentOfLaterWrites() {
        val s = Store(dir())
        s.put("gm", JSONObject().put("status", "x"))
        val c = s.copyOf("gm")!!
        s.put("gm", JSONObject().put("status", "y"))
        assertEquals("x", c.getString("status"))
        assertTrue(s.has("gm"))
        assertFalse(s.flag("finished"))
    }
}
