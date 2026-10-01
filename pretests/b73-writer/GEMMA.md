# B73: testing Gemma on the phone next round

Gemma is the open model the app would download and run itself (`PRE-37`), through Google's LiteRT-LM runtime.

## The news: Gemma 4 needs nothing from you

Checked on Hugging Face on 1 October 2026 (file listings only; nothing gated was downloaded):

| Model | Licence | Gate | Phone file and size |
|---|---|---|---|
| **Gemma 4 E2B** (`litert-community/gemma-4-E2B-it-litert-lm`) | Apache 2.0 | **none** | Tensor G6 build `gemma-4-E2B-it_Google_Tensor_G6.litertlm`, 3.3 GB; GPU build 2.0 GB; CPU build 2.6 GB |
| **Gemma 4 E4B** (`litert-community/gemma-4-E4B-it-litert-lm`) | Apache 2.0 | **none** | GPU build 3.0 GB; CPU build 3.7 GB (no Tensor G6 build) |
| Gemma 3 1B (`litert-community/Gemma3-1B-IT`) | Gemma terms | yes (approved at once) | Tensor G6 build 2.0 GB; int4 CPU or GPU build 0.58 GB |
| Gemma 3 1B for Tensor NPU (`litert-community/Gemma3-1B-IT-Tensor-NPU`) | Gemma terms | none | 0.88 GB, but only for Tensor G3 and G4, not the G6 |
| Gemma 3n E2B / E4B (`google/gemma-3n-E*-it-litert-lm`) | Gemma terms | yes (approved by hand) | 3.7 GB / 4.9 GB |

So the next round can test **Gemma 4 E2B**, the best fit, with no account, licence step or token. The app downloads the file once over Wi-Fi from its public link and keeps it. It needs about 3.3 GB of free storage.

## What you must do

- **For Gemma 4 (recommended): nothing.** Just have Wi-Fi and about 4 GB free on the phone when you run the next test app.
- **Only if you also want Gemma 3 1B or Gemma 3n** (older and smaller, not needed for the decision):
  1. Make a free Hugging Face account at huggingface.co.
  2. Open the model's page (for example `huggingface.co/litert-community/Gemma3-1B-IT`) and accept Google's Gemma licence. Gemma 3 1B is approved at once; Gemma 3n is reviewed by hand and can take days.
  3. Either download the file on the phone's browser while logged in, and pick it in the test app, or create a **read** token (Settings, Access Tokens, "Read") and give it to the lead as a session secret. Never paste it into a chat or a file in the repository.

## For the lead

- Runtime: `implementation("com.google.ai.edge.litertlm:litertlm-android:0.17.1")` (Google Maven). It adds a 46 MB native library, and pulls Kotlin reflect 2.4 and coroutines 1.11.
- API: `Engine(EngineConfig(modelPath, backend))`, then `initialize()`, `createConversation(...)` and `sendMessageAsync(prompt)` as a stream. Backends: `CPU`, `GPU`, `NPU`, and `GOOGLE_TENSOR` (try it with the Tensor G6 file).
- Unlike Gemini Nano, Gemma runs inside the app, so its memory counts against the 10 GiB (`PLT-01`). Record the app's memory while it writes (decision rule 5 in NOTES.md).
- Reuse the same 20 prompts (`prompts/v2/all-prompts.json`) and the same settings: temperature 0.3, top-k 20, at most 256 new tokens.
- The same Gemma 4 E2B model already ran in the cloud as a stand-in (see NOTES.md), so the phone run can be compared with it.
