# B73 check only: nothing calls WriterTest in this throwaway app, so keep it (as the real app's screen would)
# and let R8 process it together with ML Kit, to prove the minified release build works.
-keep class dev.kindling.pretests.WriterTest { *; }
