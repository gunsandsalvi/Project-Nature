# Kindling α00: Skeleton on the phone

## What is new

- Kindling was rebuilt from nothing, as you asked. This first step is only the frame everything else will stand on.
- The app opens full screen on the phone and shows a test card. It is signed with your one release key, so every later version installs over it.
- The same card runs in the browser at the web link.
- The card is drawn at art resolution: one art pixel is 4 × 4 screen pixels, the size the game's pixel art will use.

## What to try

1. Uninstall the old Kindling first, since the version numbers start again. Then tap **Download and install** at the top of this page, allow the browser once if it asks, and open Kindling.
2. You should see a dark screen with a small, fine checker near the top-left corner, eight grey steps from black to white under it, and a thin orange bar sliding from left to right, over and over.
3. Look closely at the checker: it should look even, with no stripes or bands, and every tiny square the same size. Your screen's scale (2.625) can only be checked on the phone itself.
4. Turn the phone: the card is laid out again at once, with the same pixel size, and nothing restarts.
5. Back puts the app in the background; open it again and the bar carries on.
6. Open the web link on the phone: the same card in the browser.

## What is rough

- There is no world yet, only the test card. Numbers, time and chance come next (α00b); colour from light and the ground come in α01.
- If a box with a code in it appears (titled "Kindling self-check" or "Kindling stopped drawing"), something went wrong: tap Copy and paste the code into your reply.
- Once, if not done yet: register the package `dev.kindling.app` and the release key's fingerprint in your free developer account. The fingerprint is `d7a2cbdd0a69175e49bc6fed82ec6ff0dc69543e83499b49d8beb5854da34863`.

## IDs delivered

`PRC-11` (part: an APK and a web page each alpha, this note, the self-check's first part), `SCP-15` (part), `PLT-01` (part: an arm64 build on the phone), `PLT-02` (part: turning never restarts the app), `PLT-03` (part: no network permission), `PLT-06` (part: signed with the one release key), `PRE-22` (part: an art pixel is 4 × 4 screen pixels), `PRC-09` (part: work joins `main` by pull request), `PRC-10` (part: the first check script).

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Web: https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
