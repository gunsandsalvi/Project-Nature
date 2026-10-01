// B78 shell 2: Kotlin Activity + C++ library via JNI (Gradle runs CMake).
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
rootProject.name = "shell2"
include(":app")
