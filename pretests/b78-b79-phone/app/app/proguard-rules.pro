# B01 managed kernels are called by reflection.
-keep class dev.kindling.pretests.Kernels { public static java.lang.String run(java.lang.String); }
# JavaScript bridge for the WebView drawing test.
-keepclassmembers class * { @android.webkit.JavascriptInterface <methods>; }
