#!/usr/bin/env bash
# Copies the shared scene into the Godot project and (re)imports it. Every texture keeps its pixels, with mipmaps so
# it stays steady when drawn small.
set -euo pipefail
cd "$(dirname "$0")/godot"
GODOT="${GODOT:-$HOME/.cache/bakeoff/Godot_v4.7.2-stable_linux.x86_64}"
rm -rf assets && cp -r ../assets assets
for png in assets/textures/*.png; do
  cat > "$png.import" <<IMP
[remap]

importer="texture"
type="CompressedTexture2D"

[deps]

source_file="res://$png"

[params]

compress/mode=0
mipmaps/generate=true
mipmaps/limit=-1
process/fix_alpha_border=false
detect_3d/compress_to=0
IMP
done
timeout 300 "$GODOT" --headless --path . --import >/dev/null 2>&1
echo "Godot project synced and imported"
