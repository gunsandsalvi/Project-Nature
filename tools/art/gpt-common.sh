# What the art lane's GPT tools share (gpt-run.sh for pictures, gpt-blender.sh for Blender scripts), sourced by
# both, never run: the pause file, a request's fields and prompt, and the line each run adds to runs.md (A5.4).
# Implements PRE-46, see A5.4 and A6.1.
#
# While the pause file exists, requests are held, not run: /tmp/kindling-gpt-paused, or the file KINDLING_GPT_PAUSE
# names, so the tools' tests never hold the lane's real runs.
GPT_PAUSE="${KINDLING_GPT_PAUSE:-/tmp/kindling-gpt-paused}"

# Whether runs are held.
gpt_paused() { [ -e "$GPT_PAUSE" ]; }

# A request's first value for a field: gpt_field <field> <request.txt>, such as gpt_field Orientation req.txt.
gpt_field() { sed -n "s/^$1:[[:space:]]*//p" "$2" | head -1; }

# A request's prompt, the text after its Prompt: line to the end of the file, written to a file: gpt_prompt
# <request.txt> <out>; fails if there is none.
gpt_prompt() {
  awk 'found{print} /^Prompt:/{found=1}' "$1" >"$2"
  [ -s "$2" ]
}

# The tool that ran, by its version, naming no model.
gpt_tool() { codex --version 2>/dev/null | sed 's/^codex-cli /Codex /'; }

# One line of a scratch folder's runs.md: gpt_log <runs.md> <request> <input> <result> <seconds> <how>.
gpt_log() {
  printf '| %s | %s | %s | %s | %s s | %s |\n' "$(date -u '+%Y-%m-%d %H:%M')" "$2" "$3" "$4" "$5" "$6" >>"$1"
}
