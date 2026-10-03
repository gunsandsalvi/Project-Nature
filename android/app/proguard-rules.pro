# The JNI names libkindling.so exports must survive R8 (A2.5).
-keep class dev.kindling.app.Native { *; }
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}
