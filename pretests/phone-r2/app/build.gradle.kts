buildscript {
    repositories { google() }
    dependencies {
        // A newer R8 than AGP 8.13's own: LiteRT-LM and the Kotlin library it brings are Kotlin 2.4, whose
        // metadata needs R8 9.1.29 or later (developer.android.com/build/kotlin-support). AGP 8.5.2+ can use it.
        classpath("com.android.tools:r8:9.1.56")
    }
}
plugins {
    id("com.android.application") version "8.13.2" apply false
    id("org.jetbrains.kotlin.android") version "2.3.21" apply false
}
