// Kindling's app module (A2.5, A15.3): built unsigned; tools/build-apk.sh signs it (A15.5).
import java.util.Properties

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val ndkDir: String = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first")
val jniOut = layout.buildDirectory.dir("rustJniLibs").get().asFile
val version = Properties().apply { rootProject.file("version.properties").inputStream().use { load(it) } }

android {
    namespace = "dev.kindling.app"
    compileSdk = 36
    ndkPath = ndkDir
    ndkVersion = "30.0.16248370"
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
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    packaging { jniLibs { useLegacyPackaging = false } }
    sourceSets["main"].jniLibs.srcDir(jniOut)
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }

// Build libkindling.so for arm64 before packaging (cargo is incremental on its own).
val cargoNdk by tasks.registering(Exec::class) {
    workingDir = rootProject.file("..")
    commandLine("cargo", "ndk", "-t", "arm64-v8a", "-P", "31", "-o", jniOut.absolutePath,
        "build", "--release", "-p", "kd-android", "--locked")
}
tasks.named("preBuild") { dependsOn(cargoNdk) }
