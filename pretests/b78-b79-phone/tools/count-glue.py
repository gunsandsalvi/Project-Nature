#!/usr/bin/env python3
"""B78: lines of glue code per shell: non-blank lines that aren't only a comment.
Glue = everything needed to get the shared core onto the phone screen, outside the core itself."""
import json, os, re, sys

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "shells")
GLUE = {
    "s1-kotlin-rust": ["settings.gradle.kts", "build.gradle.kts", "gradle.properties", "app/build.gradle.kts",
                       "app/src/main/AndroidManifest.xml", "app/src/main/java/dev/kindling/shell1/MainActivity.kt",
                       "rust/Cargo.toml", "rust/.cargo/config.toml", "rust/src/lib.rs"],
    "s2-kotlin-cpp": ["settings.gradle.kts", "build.gradle.kts", "gradle.properties", "app/build.gradle.kts",
                      "app/src/main/AndroidManifest.xml", "app/src/main/java/dev/kindling/shell2/MainActivity.kt",
                      "app/src/main/cpp/CMakeLists.txt", "app/src/main/cpp/jni.cpp"],
    "s3-rust-native": ["Cargo.toml", ".cargo/config.toml", "AndroidManifest.xml", "build.sh", "src/lib.rs"],
    "s4-webview": ["settings.gradle.kts", "build.gradle.kts", "gradle.properties", "app/build.gradle.kts",
                   "app/src/main/AndroidManifest.xml", "app/src/main/java/dev/kindling/shell4/MainActivity.kt",
                   "app/proguard-rules.pro", "app/src/main/assets/index.html",
                   "rust/Cargo.toml", "rust/.cargo/config.toml", "rust/src/lib.rs"],
}
# The cloud (headless Linux) side of the shared core, counted apart.
CLOUD = {"rust": ["core/Cargo.toml", "core/src/bin/headless.rs"],
         "cpp": ["s2-kotlin-cpp/core/CMakeLists.txt", "s2-kotlin-cpp/core/headless.cpp"]}
COMMENT = re.compile(r"^\s*(//|#(?!include|define|pragma|!)|<!--|/\*|\*)")

def count(path):
    with open(path) as f:
        return sum(1 for line in f if line.strip() and not COMMENT.match(line))

out = {s: {"files": len(fs), "lines": sum(count(os.path.join(HERE, s, f)) for f in fs)} for s, fs in GLUE.items()}
out["cloud-side"] = {k: sum(count(os.path.join(HERE, f)) for f in fs) for k, fs in CLOUD.items()}
json.dump(out, sys.stdout, indent=1)
print()
