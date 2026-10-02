// Kindling's Android shell: Kotlin Activity + Rust library via JNI, Gradle runs cargo-ndk (A2.5; from B78 shell 1).
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
rootProject.name = "kindling"
include(":app")
