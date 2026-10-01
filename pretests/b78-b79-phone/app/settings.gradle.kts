// B78/B79 round-1 phone test app (approach 1: Kotlin + Rust via JNI, Gradle runs cargo-ndk).
// Google's mirror of Maven Central comes first: Maven Central rate-limits the shared cloud egress (429).
pluginManagement {
    repositories {
        google()
        maven("https://maven-central.storage-download.googleapis.com/maven2/")
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositories {
        google()
        maven("https://maven-central.storage-download.googleapis.com/maven2/")
        mavenCentral()
    }
}
rootProject.name = "kindling-pretests"
include(":app")
