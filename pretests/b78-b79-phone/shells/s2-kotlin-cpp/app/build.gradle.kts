plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "dev.kindling.shell2"
    compileSdk = 36
    ndkPath = System.getenv("ANDROID_NDK_HOME") ?: error("source tools/env.sh first")
    ndkVersion = "30.0.16248370"
    defaultConfig {
        applicationId = "dev.kindling.shell2"
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
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"))
            signingConfig = signingConfigs.getByName("pretest")
        }
    }
    externalNativeBuild { cmake { path = file("src/main/cpp/CMakeLists.txt"); version = "3.31.6" } }
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }
