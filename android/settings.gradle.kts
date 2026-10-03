// Kindling's Android shell (A2.5). Repositories in A2.8's order: Google's, then Google's mirror of Maven
// Central, which a fresh cloud session reaches more reliably, then Maven Central itself.
pluginManagement {
    repositories {
        google()
        maven("https://maven-central.storage-download.googleapis.com/maven2/")
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        maven("https://maven-central.storage-download.googleapis.com/maven2/")
        mavenCentral()
    }
}

rootProject.name = "kindling"
include(":app")
