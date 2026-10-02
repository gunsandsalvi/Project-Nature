# Kindling α00b: The checks in full

## What is new
- Nothing new to see: the same golden cube. This alpha builds the safety net under every later one.
- Before any work joins the main version, the checks now also confirm that each part of the game only uses the parts below it, that banned clocks, files and platform maths really are banned, that every ID and section the three project files cite exists, that a change to the project file names the items it changed, and that every feature has its place in the plan and, once built, a test.
- The APK is now signed with your release key, made from your passphrase. From the next alpha on, each one updates Kindling in place.

## What to try
1. Tap the APK button. This once Android refuses the update, because the key changed: uninstall Kindling (long-press its icon, Uninstall), then tap the APK button again and install. Nothing is lost: there are no worlds yet.
2. Open Kindling: the same golden cube turns as before; nothing else has changed for you.
3. In your free hobbyist account on the Android Developer Console, register the package `dev.kindling.app` with this SHA-256 certificate fingerprint:
   `D7:A2:CB:DD:0A:69:17:5E:49:BC:6F:ED:82:EC:6F:F0:DC:69:54:3E:83:49:9B:49:D8:BE:B5:85:4D:A3:48:63`
   If the console refuses it, reply with its message.
4. From the next alpha on, the APK button offers to update Kindling rather than install it anew.

## What is rough
- One uninstall and reinstall, this once.
- I could not sign in to your developer account, so whether the console takes this kind of key (EC) is confirmed from Google's guide only, which asks just for the fingerprint. If it refuses, the next alpha switches to the fallback key type.
- Still only the cube.

## Your α00 results
Recorded from your reply: the phone check page passed in full (WebAssembly, loading files, modules, WebGL2 and storage all work), and the APK installed and the cube works on the phone.

## IDs delivered
In part: `PRN-14`, `PRC-07`, `PRC-10`, `PRC-12`, `PLT-06`.

## Links
- APK: [kindling.apk](https://github.com/gunsandsalvi/Project-Nature/raw/a00b/dist/kindling.apk) (497 KB, version a00b, code 1002, release key)
- Web: [Kindling alpha](https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH)
- Phone check: [Kindling phone check](https://claude.ai/artifact/RZuafpPi6Hdmu9o9nu5dHi)
