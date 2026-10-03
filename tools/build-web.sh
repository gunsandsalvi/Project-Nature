#!/usr/bin/env bash
# Builds the web alpha into dist/web (A2.6, A15.3): the wasm at most 12 MB, with the page and its glue.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
. tools/env.sh
. android/version.properties
export KD_BUILD="$versionName · $versionCode · $(git rev-parse --short HEAD)"
T0=$(date +%s)
env -u KINDLING_SIGNING_PASSPHRASE cargo build -p kd-web --target wasm32-unknown-unknown --profile release-web --locked
rm -rf dist/web
mkdir -p dist/web/pkg
env -u KINDLING_SIGNING_PASSPHRASE wasm-bindgen --target web --no-typescript --out-dir dist/web/pkg \
  target/wasm32-unknown-unknown/release-web/kd_web.wasm
cp web/index.html web/glue.js dist/web/
SIZE=$(stat -c %s dist/web/pkg/kd_web_bg.wasm)
[ "$SIZE" -le $((12 * 1024 * 1024)) ] || { echo "Web: the wasm is $((SIZE / 1024)) KB, over 12 MB (A15.3)" >&2; exit 1; }
echo "Web: dist/web (kd_web_bg.wasm $((SIZE / 1024)) KB, $(($(date +%s) - T0)) s)"
