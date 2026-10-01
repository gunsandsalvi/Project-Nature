"""B73: write the 20 prompts to prompts/v2/all-prompts.json and embed the same prompts in phone/WriterTest.kt,
so the cloud stand-in and the phone get exactly the same text (PRE-37, PRE-19)."""
import json
import pathlib
import re

from writer import PROMPTS, all_prompts

HERE = pathlib.Path(__file__).resolve().parent


def main():
    prompts = all_prompts()
    (HERE / "prompts" / "v2" / "all-prompts.json").write_text(json.dumps(prompts, indent=1, ensure_ascii=False) + "\n")
    rows = []
    for p in prompts:
        t = p["prompt"]
        assert '"""' not in t and "$" not in t, p["id"]  # Kotlin raw strings can't hold these
        rows.append(f'        Prompt("{p["id"]}", "{p["rec"]}", "{p["voice"]}", {str(p["dark"]).lower()}, """{t}"""),')
    block = (f'    const val PROMPTS_VERSION = "{PROMPTS}"\n'
             + "    val PROMPTS: List<Prompt> = listOf(\n" + "\n".join(rows) + "\n    )\n")
    kt = HERE / "phone" / "WriterTest.kt"
    src = kt.read_text()
    src = re.sub(r"(// BEGIN GENERATED PROMPTS[^\n]*\n).*?(    // END GENERATED PROMPTS)",
                 lambda m: m.group(1) + block + m.group(2), src, flags=re.S)
    kt.write_text(src)
    print(f"{len(prompts)} prompts; longest {max(len(p['prompt']) for p in prompts)} chars")


if __name__ == "__main__":
    main()
