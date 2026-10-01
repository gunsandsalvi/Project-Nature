// B78/B79 round-1 test app. Build: `. ../tools/env.sh && gradle assembleRelease` (or tools/build-app.sh).
// Native libraries: libkbench.so (B01/B02 benchmark library: the real crate when it builds,
// else the stub in native/stub-kbench) and libkphone.so (B79 helpers in native/kphone).
plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val ndkDir: String = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first")
val cacheDir: String = System.getenv("CACHE") ?: error("source tools/env.sh first")
val nativeDir = rootProject.file("native")
val jniOut = layout.buildDirectory.dir("rustJniLibs").get().asFile
val realKbench = rootProject.file("../../b01-b02-numbers-random/kbench")
val managedKernels = rootProject.file("../../b01-b02-numbers-random/jvm")
// Optional drawing test (B66/B79): the visual-style mockup, copied into the app's assets.
val mockup = rootProject.file("../../../mockups/visual-style.html")
val mockupAssets = layout.buildDirectory.dir("mockupAssets").get().asFile
// -Pkbench=stub forces the stand-in; default: the real crate if its folder exists.
val useRealKbench = (findProperty("kbench") ?: "real") == "real" && File(realKbench, "Cargo.toml").exists()

android {
    namespace = "dev.kindling.pretests"
    compileSdk = 36
    ndkPath = ndkDir
    ndkVersion = "30.0.16248370"
    defaultConfig {
        applicationId = "dev.kindling.pretests"
        minSdk = 31
        targetSdk = 36
        versionCode = 1
        versionName = "r1"
        ndk { abiFilters += "arm64-v8a" }
        buildConfigField("String", "KBENCH", if (useRealKbench) "\"real\"" else "\"stub\"")
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
    sourceSets["main"].jniLibs.srcDir(jniOut)
    sourceSets["main"].assets.srcDir(mockupAssets)
    // B01: managed-code kernels from the other pre-test, if present (called by reflection).
    if (File(managedKernels, "Kernels.java").exists()) sourceSets["main"].java.srcDir(managedKernels)
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
    // JVM unit tests of the app's plain logic (org.json is part of Android, but stubbed in unit tests).
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.json:json:20240303")
}

// Native libraries for arm64, 16 KB aligned (cargo is incremental on its own).
val cargoKphone by tasks.registering(Exec::class) {
    workingDir = nativeDir
    commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
        "build", "--release", "-p", "kphone", "--features", "android")
}
val cargoKbench by tasks.registering(Exec::class) {
    if (useRealKbench) {
        // Build the other pre-test's crate without writing into its folder.
        workingDir = realKbench
        environment("CARGO_TARGET_DIR", "$cacheDir/b78-target/kbench-real")
        environment("CARGO_TARGET_AARCH64_LINUX_ANDROID_RUSTFLAGS", "-C link-arg=-Wl,-z,max-page-size=16384")
        commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
            "build", "--release", "--lib", "--features", "android", "--locked")
    } else {
        workingDir = nativeDir
        commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
            "build", "--release", "-p", "kbench", "--features", "android")
    }
}
// B01/X11: the checksums the phone should give, from the other pre-test (if present).
val predicted = rootProject.file("../../b01-b02-numbers-random/results/determinism.csv")
val copyMockup by tasks.registering(Copy::class) {
    from(mockup)
    from(predicted)
    into(mockupAssets)
}
tasks.named("preBuild") { dependsOn(cargoKphone, cargoKbench, copyMockup) }
