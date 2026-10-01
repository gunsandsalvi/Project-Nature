// B73 Part 4 compile check. Build: see ../NOTES.md ("How to re-run"). Outputs go to $CACHE, never the repo.
plugins {
    id("com.android.application") version "8.13.2"
    id("org.jetbrains.kotlin.android") version "2.3.21"
}

val cacheDir: String = System.getenv("CACHE") ?: error("set CACHE")
layout.buildDirectory.set(file("$cacheDir/b73/phone-check-build"))

android {
    namespace = "dev.kindling.pretests"
    compileSdk = 36
    defaultConfig {
        applicationId = "dev.kindling.pretests.b73check"
        minSdk = 31
        targetSdk = 36
        versionCode = 1
        versionName = "b73"
    }
    sourceSets["main"].java.srcDir("../phone")
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    buildTypes { release { isMinifyEnabled = true; proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro") } }
    lint { abortOnError = false; checkReleaseBuilds = false }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }

dependencies {
    // The one line the phone test app needs (B73, PRE-37):
    implementation("com.google.mlkit:genai-prompt:1.0.0-beta4")
}
