"""B73 Part 3: run the 20 prompts through a small open model on the CPU (llama.cpp), as a stand-in
for the phone's models. Proves the pipeline and the checker; its quality says little about the phone.

Usage (CPU-heavy, run under the shared lock):
  flock $CACHE/cpu.lock python run_standin.py --model $CACHE/b73/models/<file>.gguf --name qwen2.5-1.5b
Settings match the phone run (PhoneWriterTest.kt): temperature 0.3, top-k 20, at most 256 new tokens.
"""
import argparse
import json
import pathlib
import platform
import time

from llama_cpp import Llama

from writer import all_prompts

HERE = pathlib.Path(__file__).resolve().parent


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True)
    ap.add_argument("--name", required=True)
    ap.add_argument("--threads", type=int, default=4)
    ap.add_argument("--only", default="", help="comma-separated prompt ids, for a quick test")
    ap.add_argument("--prompts", default="v2", help="prompt version (folder in prompts/)")
    args = ap.parse_args()

    t0 = time.time()
    llm = Llama(model_path=args.model, n_ctx=2048, n_threads=args.threads, seed=73, verbose=False)
    load_s = time.time() - t0
    prompts = all_prompts(args.prompts)
    if args.only:
        prompts = [p for p in prompts if p["id"] in args.only.split(",")]
    runs = []
    for p in prompts:
        start = time.time()
        first = None
        chunks = []
        ntok = 0
        for ev in llm.create_chat_completion(
            messages=[{"role": "user", "content": p["prompt"]}],
            temperature=0.3, top_k=20, top_p=0.95, max_tokens=256, seed=73, stream=True,
        ):
            delta = ev["choices"][0]["delta"].get("content")
            if delta:
                ntok += 1
                if first is None and delta.strip():
                    first = time.time()
                chunks.append(delta)
        end = time.time()
        text = "".join(chunks).strip()
        words = len(text.split())
        gen = end - (first or end)
        runs.append({
            "id": p["id"], "rec": p["rec"], "voice": p["voice"], "dark": p["dark"], "text": text,
            "ttfw_s": round((first or end) - start, 2), "total_s": round(end - start, 2),
            "tokens": ntok, "words": words,
            "words_per_s": round(words / gen, 2) if gen > 0 else None,
            "prompt_tokens": len(llm.tokenize(p["prompt"].encode())),
        })
        r = runs[-1]
        print(f'{p["id"]}: {r["words"]} words, first word {r["ttfw_s"]} s, {r["words_per_s"]} words/s', flush=True)
    out = {
        "model": args.name, "prompts": args.prompts, "model_file": pathlib.Path(args.model).name, "threads": args.threads,
        "machine": f"{platform.machine()}, {args.threads} threads, CPU only", "load_s": round(load_s, 1),
        "settings": {"temperature": 0.3, "top_k": 20, "top_p": 0.95, "max_tokens": 256, "seed": 73},
        "runs": runs,
    }
    path = HERE / "results" / f"standin-{args.name}-{args.prompts}.json"
    path.parent.mkdir(exist_ok=True)
    path.write_text(json.dumps(out, indent=1, ensure_ascii=False) + "\n")
    print("wrote", path)


if __name__ == "__main__":
    main()
