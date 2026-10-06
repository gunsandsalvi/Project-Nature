#!/usr/bin/env bash
# Hands Codex a written request to write a Blender script and run it headless, outside every build: GPT's first pass
# at a family of the kit's parts, which the art lane then fixes (A6.1, IMPLEMENTATION.md, the art lane).
# Implements PRE-46, see A6.1.
#
# Usage: tools/art/gpt-blender.sh <request.txt> <scratch folder>
#   The request holds Purpose:, Script: (the script's file name, such as camp.py), Output: (the Blender file it
#   saves, such as camp.blend), Input files: (files of the repository, by their paths from its root, copied in for
#   the script to use, such as tools/art/kit.py, or none), Input script: (a script to start from, copied in under
#   the Script's name, or none) and, if wanted, Input pictures: (guide pictures of the repository shown to Codex with
#   the request, such as art/targets/people-variety.webp) lines, then Prompt:, whose text runs to the end of the file.
#   Works in <scratch folder>/<request>/, outside git: Codex writes the script there and runs it with Blender
#   headless until it saves the output; the prompt, Codex's own log and its last message (<request>.answer) stay
#   beside them. Adds one line to <scratch folder>/runs.md, which names no model, for the batch's report.
# While /tmp/kindling-gpt-paused exists, requests are held, not run (gpt-common.sh). Nothing here buys credits: a run
# that meets the plan's limit fails, and the art lane waits.
set -u
[ $# -eq 2 ] || { echo "usage: tools/art/gpt-blender.sh <request.txt> <scratch folder>"; exit 2; }
req="$1"
root=$(cd "$(dirname "$0")/../.." && pwd)
. "$root/tools/art/gpt-common.sh"
base=$(basename "$req" .txt)
if gpt_paused; then echo "held $base (paused)"; exit 0; fi
[ -f "$req" ] || { echo "no request $req"; exit 2; }
script=$(gpt_field Script "$req")
output=$(gpt_field Output "$req")
[ -n "$script" ] && [ -n "$output" ] || { echo "no Script: or Output: in $req"; exit 2; }
case "$script$output" in */*) echo "Script: and Output: are file names, not paths"; exit 2 ;; esac
inputs=$(gpt_field "Input files" "$req")
for f in $inputs; do
  [ "$f" = none ] || [ -f "$root/$f" ] || { echo "no input file $f"; exit 2; }
done
from=$(gpt_field "Input script" "$req")
[ -z "$from" ] || [ "$from" = none ] || [ -f "$from" ] || { echo "no input script $from"; exit 2; }
pictures=$(gpt_field "Input pictures" "$req")
shown=()
for p in $pictures; do
  [ "$p" = none ] && continue
  [ -f "$root/$p" ] || { echo "no input picture $p"; exit 2; }
  shown+=(-i "$root/$p")
done
mkdir -p "$2"
scratch=$(cd "$2" && pwd)
work="$scratch/$base"
mkdir -p "$work"
gpt_prompt "$req" "$work/$base.prompt" || { echo "no Prompt: in $req"; exit 2; }
for f in $inputs; do
  [ "$f" = none ] || cp "$root/$f" "$work/"
done
used="none"
if [ -n "$from" ] && [ "$from" != none ]; then
  cp "$from" "$work/$script"
  used="$from"
fi
rm -f "$work/$output"
how="Read the file $base.prompt in the current folder, all of it, and do what it asks. Write the Blender Python \
script $script in the current folder and run it headless from this folder with: blender -b --factory-startup \
--python-exit-code 1 --python $script -- $output. Blender 4.0.2 is at /usr/bin/blender; its Python has no numpy, so use \
only bpy, bmesh, mathutils and Python's standard library. Fix the script and run it again until it runs without an \
error and saves $output. Do not render, do not use the internet, and do not create or change files outside the current \
folder. When it is done, reply with the files you made and anything the prompt asked that you could not do."
[ ${#shown[@]} -eq 0 ] || how="$how The attached pictures are the guides the prompt names, in the order it names them."
start=$(date +%s)
# "--" ends the options, and with them the list of pictures, which would otherwise swallow the instruction.
codex exec --skip-git-repo-check -s workspace-write -C "$work" -o "$work/$base.answer" "${shown[@]}" -- "$how" \
  >"$work/$base.log" 2>&1
status=$?
secs=$(($(date +%s) - start))
result="failed"
[ -f "$work/$output" ] && [ -f "$work/$script" ] && result="$base/$output"
gpt_log "$scratch/runs.md" "${req#"$root"/}" "$used" "$result" "$secs" "$(gpt_tool), writing a Blender script"
echo "exit $status $result"
