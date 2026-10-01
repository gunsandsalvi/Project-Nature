package dev.kindling.pretests

import org.json.JSONArray
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File

/**
 * B73: Gemini Nano and Gemma get exactly the 20 prompts of pretests/b73-writer/prompts/v2/all-prompts.json.
 * Both read WriterTest.PROMPTS (embedded in WriterTest.kt by the writer test's make_prompts.py); this checks
 * them against the file, text for text.
 */
class PromptsTest {
    @Test fun appPromptsAreTheWriterTestsPromptFile() {
        val file = File(System.getProperty("b73.prompts") ?: error("b73.prompts not set"))
        val want = JSONArray(file.readText())
        val got = WriterTest.PROMPTS
        assertEquals(20, want.length())
        assertEquals(want.length(), got.size)
        assertEquals("v2", WriterTest.PROMPTS_VERSION)
        for (i in 0 until want.length()) {
            val w = want.getJSONObject(i)
            val g = got[i]
            assertEquals(w.getString("id"), g.id)
            assertEquals(w.getString("rec"), g.rec)
            assertEquals(w.getString("voice"), g.voice)
            assertEquals(w.getBoolean("dark"), g.dark)
            assertEquals("prompt ${g.id}", w.getString("prompt"), g.text)
        }
    }

    @Test fun theRatedRecordsAreTheFeudKillingTheDreamAndTheSign() {
        val byId = WriterTest.PROMPTS.associateBy { it.id }
        assertEquals(6, Logic.RATE_IDS.size)
        for (id in Logic.RATE_IDS) assertTrue(id, id in byId)
        assertTrue(byId.getValue("r04-doc").text.contains("Event: feud killing."))
        assertTrue(byId.getValue("r06-tra").text.contains("Event: dream that changed a mind."))
        assertTrue(byId.getValue("r08-doc").text.contains("\"sky-fire\""))
        // The facts shown to the owner on request are the DATA block alone.
        val facts = Logic.dataBlock(byId.getValue("r04-tra").text)
        assertTrue(facts.startsWith("Event: feud killing."))
        assertTrue(!facts.contains("Rules:"))
    }

    @Test fun gemmaDownloadsThePublicGeneralBuild() {
        assertEquals("https://huggingface.co/litert-community/gemma-4-E2B-it-litert-lm/resolve/" +
            "b3ca0d2f076785a8f4b2219ddbd2bdb99954eae1/gemma-4-E2B-it.litertlm", GemmaTest.URL)
        assertEquals(2_588_147_712L, GemmaTest.SIZE)
        assertEquals("181938105e0eefd105961417e8da75903eacda102c4fce9ce90f50b97139a63c", GemmaTest.SHA256)
        assertEquals(listOf("GPU", "CPU"), GemmaTest.BACKENDS)
    }
}
