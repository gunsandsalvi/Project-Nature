// Phone test app, round 2. Build: `. ../tools/env.sh && gradle assembleRelease` (see ../NOTES.md, "How to re-run").
// It runs, in order: storage and terrain (B04, B11), sound (B74), the writer AI with Gemini Nano and with
// Gemma 4 E2B (B73), and the owner's rating of a few texts (B73).
// Native libraries, all arm64-v8a and 16 KB aligned:
// - libkstorage.so: B04/B11, built from pretests/b04-b11-storage-terrain/phone (its INTEGRATION.md);
// - libksound.so: B74, built from pretests/b74-b76-sound-speech (its INTEGRATION.md);
// - liblitertlm_jni.so: Google's prebuilt LiteRT-LM runtime for Gemma (from the Maven package).
// The other pre-tests' crates are built from their own folders, with every output in the shared cache.
plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val ndkDir: String = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first")
val cacheDir: String = System.getenv("CACHE") ?: error("source tools/env.sh first")
val jniOut = layout.buildDirectory.dir("rustJniLibs").get().asFile
val pretests = rootProject.file("../..")
val storageCrate = File(pretests, "b04-b11-storage-terrain/phone")
val soundCrate = File(pretests, "b74-b76-sound-speech")
val writerPrompts = File(pretests, "b73-writer/prompts/v2/all-prompts.json")
val align16k = "-C link-arg=-Wl,-z,max-page-size=16384"

android {
    namespace = "dev.kindling.pretests"
    compileSdk = 36
    ndkPath = ndkDir
    ndkVersion = "30.0.16248370"
    defaultConfig {
        applicationId = "dev.kindling.pretests"
        minSdk = 31
        targetSdk = 36
        versionCode = 2
        versionName = "r2"
        ndk { abiFilters += "arm64-v8a" }
    }
    buildFeatures { buildConfig = true }
    signingConfigs {
        create("pretest") {
            storeFile = file(System.getenv("KINDLING_KS"))
            storePassword = System.getenv("KINDLING_KS_PASS")
            keyAlias = "pretests"
            keyPassword = System.getenv("KINDLING_KS_PASS")
            enableV2Signing = true
            enableV3Signing = true
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
            signingConfig = signingConfigs.getByName("pretest")
        }
        debug { signingConfig = signingConfigs.getByName("pretest") }
    }
    // Native libraries stay uncompressed in the APK, 16 KB aligned, and load straight from it.
    packaging { jniLibs { useLegacyPackaging = false } }
    sourceSets["main"].jniLibs.srcDir(jniOut)
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    lint {
        abortOnError = false
        checkReleaseBuilds = false
        textReport = true
    }
    testOptions { unitTests.isReturnDefaultValues = true }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }

dependencies {
    // B73, Gemini Nano through ML Kit's GenAI Prompt API (pretests/b73-writer/INTEGRATION.md).
    implementation("com.google.mlkit:genai-prompt:1.0.0-beta4")
    // B73, Gemma 4 E2B run inside the app by Google's LiteRT-LM runtime (pretests/b73-writer/GEMMA.md).
    implementation("com.google.ai.edge.litertlm:litertlm-android:0.17.1")
    // FileProvider for "Share results file"; the same version ML Kit already brings in.
    implementation("androidx.core:core:1.9.0")
    // JVM unit tests of the app's plain logic (org.json is part of Android, but stubbed in unit tests).
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.json:json:20240303")
}

// The unit tests read other pre-tests' files: the writer test's prompt file (the app's prompts must equal it),
// its cloud stand-in texts (real model texts, to size the result code), and the storage test's cloud result.
tasks.withType<Test>().configureEach {
    systemProperty("b73.prompts", writerPrompts.absolutePath)
    systemProperty("b73.results", File(pretests, "b73-writer/results").absolutePath)
    systemProperty("b04.cloud", File(pretests, "b04-b11-storage-terrain/results/phone-cloud.json").absolutePath)
}

// Native libraries for arm64, 16 KB aligned. Cargo is incremental, and --locked leaves the crates' lock files alone.
val cargoKstorage by tasks.registering(Exec::class) {
    workingDir = storageCrate
    environment("CARGO_TARGET_DIR", "$cacheDir/r2-target/kstorage")
    environment("CARGO_TARGET_AARCH64_LINUX_ANDROID_RUSTFLAGS", align16k)
    commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
        "build", "--release", "--features", "android", "--locked")
}
val cargoKsound by tasks.registering(Exec::class) {
    workingDir = soundCrate
    environment("CARGO_TARGET_DIR", "$cacheDir/r2-target/ksound")
    environment("CARGO_TARGET_AARCH64_LINUX_ANDROID_RUSTFLAGS", align16k)
    commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
        "build", "--release", "--lib", "--features", "android", "--locked")
}
tasks.named("preBuild") { dependsOn(cargoKstorage, cargoKsound) }
