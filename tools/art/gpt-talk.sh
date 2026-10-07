#!/usr/bin/env bash
# Carries on one design conversation with GPT through Codex, a turn at a time, outside every build: the builder and
# GPT discuss a catalogue group's pieces as designers, and GPT draws them with its image tool (IMPLEMENTATION.md, the
# catalogue). One conversation for each group, in its own folder, which is GPT's working folder at every turn.
# Implements PRE-46, see A5.4.
#
# Usage: tools/art/gpt-talk.sh <folder> <message.txt> [<picture>...]
#   The first turn starts the conversation (codex exec) and keeps its id in <folder>/session; every later turn resumes
#   it (codex exec resume) from the same folder, so GPT finds its own drawings there again. The pictures are shown with
#   the message. Each turn N keeps, outside git, turn-NN.message (the message as passed), the pictures shown
#   (seen/turn-NN-<k>-<name>), turn-NN.jsonl (Codex's events) and turn-NN.answer (GPT's reply), and adds one line to
#   <folder>/runs.md, which names no model.
# While /tmp/kindling-gpt-paused exists, turns are held, not run (gpt-common.sh). Nothing here buys credits: a turn
# that meets the plan's limit fails, and the work waits.
set -u
[ $# -ge 2 ] || { echo "usage: tools/art/gpt-talk.sh <folder> <message.txt> [<picture>...]"; exit 2; }
folder_in="$1"
message="$2"
shift 2
root=$(cd "$(dirname "$0")/../.." && pwd)
. "$root/tools/art/gpt-common.sh"
if gpt_paused; then echo "held $(basename "$folder_in") (paused)"; exit 0; fi
[ -s "$message" ] || { echo "no message $message"; exit 2; }
for p in "$@"; do [ -f "$p" ] || { echo "no picture $p"; exit 2; }; done
mkdir -p "$folder_in/seen"
folder=$(cd "$folder_in" && pwd)
n=$(($(find "$folder" -maxdepth 1 -name 'turn-*.message' | wc -l) + 1))
turn=$(printf 'turn-%02d' "$n")
cp "$message" "$folder/$turn.message"
shown=()
k=0
for p in "$@"; do
  k=$((k + 1))
  copy="$folder/seen/$turn-$k-$(basename "$p")"
  cp "$p" "$copy"
  shown+=(-i "$copy")
done
how="Read the file $turn.message in the current folder, all of it, and answer it as it asks. This folder is yours for \
the whole conversation: your earlier drawings and notes stay here. When it asks you to draw, use your built-in image \
generation tool and save each picture in this folder under the name it gives; never draw with code. Reply in plain \
words, and end by listing the files you saved."
start=$(date +%s)
if [ -f "$folder/session" ]; then
  id=$(cat "$folder/session")
  # the id first, since a picture option may take several values; "--" ends them before the instruction
  (cd "$folder" && codex exec resume "$id" --skip-git-repo-check -c sandbox_mode='"workspace-write"' --json \
    -o "$folder/$turn.answer" "${shown[@]}" -- "$how" </dev/null) >"$folder/$turn.jsonl" 2>"$folder/$turn.log"
else
  codex exec --skip-git-repo-check -s workspace-write -C "$folder" --json -o "$folder/$turn.answer" "${shown[@]}" \
    -- "$how" </dev/null >"$folder/$turn.jsonl" 2>"$folder/$turn.log"
  id=$(sed -n 's/.*"thread_id":"\([^"]*\)".*/\1/p' "$folder/$turn.jsonl" | head -1)
  [ -n "$id" ] && echo "$id" >"$folder/session"
fi
status=$?
secs=$(($(date +%s) - start))
result="failed"
[ -s "$folder/$turn.answer" ] && result="$turn.answer"
gpt_log "$folder/runs.md" "$turn.message" "$*" "$result" "$secs" "$(gpt_tool), a conversation"
echo "exit $status $result"
