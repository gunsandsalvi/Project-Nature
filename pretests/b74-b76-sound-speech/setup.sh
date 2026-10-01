#!/usr/bin/env bash
# B74/B76 pre-test: fetch the tools and voices into the shared cache, never into the repo.
# Safe to re-run: every step is skipped when already done.
set -euo pipefail
CACHE=${CACHE:-/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache}
B76=$CACHE/b76
mkdir -p "$B76/voices" "$B76/pip-cache"

# B76 approach S1: the espeak-ng formant synthesiser, fed phonemes directly.
command -v espeak-ng >/dev/null || { apt-get update -qq && apt-get install -y -qq espeak-ng >/dev/null; }

# B76 approach S2: Piper neural voices run with onnxruntime, fed IPA phoneme ids directly.
if [ ! -x "$B76/venv/bin/python" ]; then
  python3 -m venv "$B76/venv"
fi
PIP_CACHE_DIR=$B76/pip-cache "$B76/venv/bin/pip" install -q onnxruntime numpy

HF=https://huggingface.co/rhasspy/piper-voices/resolve/main
for v in en/en_US/libritts_r/medium/en_US-libritts_r-medium cy/cy_GB/gwryw_gogleddol/medium/cy_GB-gwryw_gogleddol-medium; do
  name=$(basename "$v")
  for ext in onnx onnx.json; do
    f=$B76/voices/$name.$ext
    [ -s "$f" ] || curl -fsSL -o "$f.part" "$HF/$v.$ext" && { [ -s "$f" ] || mv "$f.part" "$f"; }
  done
done
ls -la "$B76/voices"
echo "setup done"
