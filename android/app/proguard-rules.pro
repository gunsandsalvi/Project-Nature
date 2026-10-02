# Keep the JNI surface (A2.5): tools/verify-apk.sh checks each native method survives R8.
-keep class dev.kindling.app.Native { *; }
-keepclasseswithmembernames,includedescriptorclasses class * { native <methods>; }
