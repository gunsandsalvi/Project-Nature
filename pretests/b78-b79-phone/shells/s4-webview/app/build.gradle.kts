plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val ndkDir: String = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first")
val rustDir = rootProject.file("rust")
val jniOut = layout.buildDirectory.dir("rustJniLibs").get().asFile

android {
    namespace = "dev.kindling.shell4"
    compileSdk = 36
    ndkPath = ndkDir
    ndkVersion = "30.0.16248370"
    defaultConfig {
        applicationId = "dev.kindling.shell4"
        minSdk = 31
        targetSdk = 36
        versionCode = 1
        versionName = "1"
        ndk { abiFilters += "arm64-v8a" }
    }
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
    }
    sourceSets["main"].jniLibs.srcDir(jniOut)
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }

// Build the Rust library for arm64 before packaging (cargo is incremental on its own).
val cargoNdk by tasks.registering(Exec::class) {
    workingDir = rustDir
    commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath, "build", "--release")
}
tasks.named("preBuild") { dependsOn(cargoNdk) }
