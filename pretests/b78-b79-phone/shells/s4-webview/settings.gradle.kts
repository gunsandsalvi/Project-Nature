// B78 shell 4: WebView page + Rust library via a JavaScript bridge and JNI.
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
rootProject.name = "shell4"
include(":app")
