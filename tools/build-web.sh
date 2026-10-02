#!/usr/bin/env bash
# Builds the web alpha into dist/web (A2.6, A15.3).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"; . tools/env.sh; . android/version.properties
export KD_BUILD="$versionName $(git rev-parse --short HEAD)"
env -u KINDLING_SIGNING_PASSPHRASE cargo build -p kd-web --target wasm32-unknown-unknown --profile release-web --locked
rm -rf dist/web && mkdir -p dist/web/pkg
wasm-bindgen --target web --no-typescript --out-dir dist/web/pkg target/wasm32-unknown-unknown/release-web/kd_web.wasm
cp web/index.html web/glue.js dist/web/
SIZE=$(stat -c %s dist/web/pkg/kd_web_bg.wasm)
[ "$SIZE" -le $((12 * 1024 * 1024)) ] || { echo "Web: wasm is $SIZE bytes, over 12 MB (A15.3)"; exit 1; }
echo "Web: dist/web (kd_web_bg.wasm $((SIZE / 1024)) KB)"
