#!/usr/bin/env bash
# Runs one picture request through Codex's image tool on the owner's ChatGPT plan, outside every build (A5.4).
# Implements PRE-46, see A5.4.
#
# Usage: tools/art/gpt-run.sh <request.txt> <scratch folder>
#   The request holds Purpose:, Orientation:, Input picture: (an absolute path, or none) and Transparent: (yes or no)
#   lines, then Prompt:, whose text runs to the end of the file.
#   Writes <scratch folder>/<request>.png, with the prompt and Codex's own log beside it, all outside git, and adds one
#   line to <scratch folder>/runs.md, which names no model, for the batch's report.
# While /tmp/kindling-gpt-paused exists, requests are held, not run. Nothing here buys credits: a run that meets the
# plan's limit fails, and the art lane waits.
set -u
[ $# -eq 2 ] || { echo "usage: tools/art/gpt-run.sh <request.txt> <scratch folder>"; exit 2; }
req="$1"
out="$2"
root=$(cd "$(dirname "$0")/../.." && pwd)
base=$(basename "$req" .txt)
if [ -e /tmp/kindling-gpt-paused ]; then echo "held $base (paused)"; exit 0; fi
[ -f "$req" ] || { echo "no request $req"; exit 2; }
mkdir -p "$out"
log="$out/runs.md"
orient=$(sed -n 's/^Orientation:[[:space:]]*//p' "$req" | head -1 | tr 'A-Z' 'a-z')
input=$(sed -n 's/^Input picture:[[:space:]]*//p' "$req" | head -1)
awk 'found{print} /^Prompt:/{found=1}' "$req" >"$out/$base.prompt"
[ -s "$out/$base.prompt" ] || { echo "no Prompt: in $req"; exit 2; }
size="Portrait, 1024 x 1536"
[ "${orient#landscape}" != "$orient" ] && size="Landscape, 1536 x 1024"
[ "${orient#square}" != "$orient" ] && size="Square, 1024 x 1024"
args=(exec --skip-git-repo-check -s workspace-write -C "$out")
how="Give it, as its prompt, the full text of the file $base.prompt in the current folder, exactly as written, without rewriting, shortening or adding anything."
if grep -qi '^Transparent:[[:space:]]*yes' "$req"; then
  how="$how Ask the image tool for a transparent background and keep the PNG's alpha channel when saving."
fi
used="none"
if [ -n "$input" ] && [ "$input" != "none" ]; then
  [ -f "$input" ] || { echo "no input picture $input"; exit 2; }
  args+=(-i "$input")
  how="$how Use the attached picture as the input picture to repaint or follow, as the prompt says."
  used="$input"
fi
rm -f "$out/$base.png"
start=$(date +%s)
# "--" ends the list of input pictures, which would otherwise swallow the instruction.
codex "${args[@]}" -- "Use your built-in image generation tool exactly once. $how $size. Save the result in the current folder as $base.png. Do nothing else: no edits, no post-processing, no scripts other than reading the prompt file and copying the image. Reply with the file name." >"$out/$base.log" 2>&1
status=$?
secs=$(($(date +%s) - start))
result="failed"
[ -f "$out/$base.png" ] && result="$base.png"
tool="$(codex --version 2>/dev/null | sed 's/^codex-cli /Codex /'), its image tool"
printf '| %s | %s | %s | %s | %s s | %s |\n' "$(date -u '+%Y-%m-%d %H:%M')" "${req#"$root"/}" "$used" "$result" \
  "$secs" "$tool" >>"$log"
echo "exit $status $result"
