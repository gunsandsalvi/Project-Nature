import java.util.Properties
import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

// versionCode = stage × 1000 + alpha × 10 + split (A15.3), kept beside the Gradle files so scripts can read it.
val version = Properties().apply { rootProject.file("version.properties").inputStream().use { load(it) } }
val ndkDir: String = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first: ANDROID_NDK_HOME is not set")
val jniOut = layout.buildDirectory.dir("rustJniLibs")

android {
    namespace = "dev.kindling.app"
    compileSdk = 36
    ndkVersion = "30.0.16248370"
    ndkPath = ndkDir

    defaultConfig {
        applicationId = "dev.kindling.app"
        minSdk = 31
        targetSdk = 36
        versionCode = version.getProperty("versionCode").toInt()
        versionName = version.getProperty("versionName")
        ndk { abiFilters += "arm64-v8a" }
    }

    buildTypes {
        release {
            // Signed afterwards by tools/build-apk.sh with the release key (A15.5), so no signing here.
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    // Native libraries stored uncompressed and 16 KB aligned in the APK (A2.5).
    packaging { jniLibs { useLegacyPackaging = false } }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    sourceSets["main"].jniLibs.srcDir(jniOut)
}

kotlin { compilerOptions { jvmTarget.set(JvmTarget.JVM_17) } }

// libkindling.so from the Rust workspace, built before anything else (A2.5).
val cargoNdk by tasks.registering(Exec::class) {
    workingDir = rootProject.file("..")
    commandLine(
        "cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.get().asFile.path,
        "build", "--release", "-p", "kd-android", "--locked",
    )
}
tasks.named("preBuild") { dependsOn(cargoNdk) }
