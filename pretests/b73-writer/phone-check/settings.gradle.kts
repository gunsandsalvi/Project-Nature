// B73: throwaway module that only proves phone/WriterTest.kt compiles and packages (Part 4).
// Same plugin versions and repositories as the phone test app (pretests/b78-b79-phone/app).
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
rootProject.name = "b73-phone-check"
