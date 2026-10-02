# Kindling α00: Skeleton on the phone

## What is new
- A golden cube turns slowly on a dark violet background, on the phone and in the browser. It is the game's skeleton: the same Rust code draws it on both.
- Under it: game time and dates, keyed chance, and maths that give the same results on the phone, in the browser and in the cloud.
- Scripts that build, sign, check and deliver every later alpha.

## What to try
1. Open the phone check link: the top line should read `WebAssembly works`. Reply "works", or "blocked" with the last line (`KDP1 ...`).
2. Tap the APK button at the top of this page. Allow installs from the browser if asked, install, and open "Kindling" (an orange flame icon).
3. You should see a golden cube turning slowly on a dark violet background, filling the screen with no bars.
4. Drag a finger sideways: the cube turns with it and keeps spinning a moment after you let go.
5. Turn the phone to landscape and back: the cube keeps turning without a restart, still filling the screen.
6. Press Back: the app goes to the background. Reopen it from recent apps: the cube is still turning.
7. Open the web link: the same cube, with a line at the bottom starting `WebAssembly works · WebGL2 works · core OK`.
8. If a box titled "Kindling self-check" appears in either, tap Copy and paste the code in your reply.

## What is rough
- No world yet: just the cube. In portrait the cube runs past the screen's sides.
- The APK is signed with a public throwaway key. The signing passphrase secret is not set yet; once it is (see the plan's "Before α00"), the next build switches to your release key and needs one uninstall and reinstall. Nothing is lost before α07a, the first alpha that keeps worlds.
- Once, please make `main` the default branch on GitHub: Settings, General, Default branch. New sessions then start from it.

## IDs delivered
In part: `TIM-16`, `TIM-14`, `TIM-18`, `RES-05`, `PRC-09`, `PRC-10`, `PRC-11`, `SCP-03`, `SCP-15`, `PLT-01`, `PLT-02`, `PLT-03`, `PLT-06`.

## Links
- APK: [kindling.apk](https://github.com/gunsandsalvi/Project-Nature/raw/a00/dist/kindling.apk) (497 KB, version a00, code 1000)
- Web: [Kindling alpha](https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH)
- Phone check: [Kindling phone check](https://claude.ai/artifact/RZuafpPi6Hdmu9o9nu5dHi)
