# Round-2 test app: keep rules for the minified release build. Nothing can be tried on the phone first,
# so these keep whole libraries rather than the least that might do.

# B73, Gemma: LiteRT-LM's native code finds its Kotlin classes, methods and fields by name
# (SamplerConfig.getTopK, the message callback's onMessage, InputData$Text, ...). Its package ships no keep rules.
-keep class com.google.ai.edge.litertlm.** { *; }
-keepclassmembers class com.google.ai.edge.litertlm.** { *; }

# B73, Gemini Nano: pretests/b73-writer/INTEGRATION.md says to keep ML Kit whole if R8 ever misbehaves.
-keep class com.google.mlkit.** { *; }

# Our own JNI entry points (Storage, Sound): class and method names must match the native symbols.
-keep class dev.kindling.pretests.Storage { *; }
-keep class dev.kindling.pretests.Sound { *; }
-keepclasseswithmembernames,includedescriptorclasses class * { native <methods>; }

# Kotlin reflection, pulled in by LiteRT-LM, warns about optional classes that are not on Android.
-dontwarn kotlin.reflect.jvm.internal.**
-dontwarn org.jetbrains.annotations.**
