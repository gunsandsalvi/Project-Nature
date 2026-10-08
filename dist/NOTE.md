# Kindling α2.8a: Light, shadows and terrain

**Candidate release, not yet delivered:** the signed 30801 attempt exceeded the 50 MiB limit. A lossless size fix is saved but still needs a rebuilt APK, final review and the full delivery check. The APK link below still serves the older 30301 build. The instructions below are for the forthcoming 30801.

Build 30801 brings the first two 2D engine steps together: the fixed camera and Fixtures page, followed by the Terrain page's light, shadows, slopes, shelter and water.
The pictures are labelled developer fixtures; they are ready for engine review, not final art approval.

## What is new

- **Fixtures:** a fixed 37-degree camera, crisp world pixels, pan and pinch, selection, walk/work previews and the animal's four/eight-facing comparison. Controls stay at the screen's native resolution when you turn the phone.
- **Terrain:** flat ground, a slope, cliffs, a shelter, water and a separate cave interior. Sun, sky and fire contribute separately, so sun shade does not dim the fire's emission.
- **Height and cover:** shadows start at the receiving surface's height. Roof and floor remain separate; selected cover fades and occupied shelters open for inspection.
- **Water:** a visible bed, depth colour, projected reflections and wading use the same surface heights.
- **Inspection:** sun direction, noon/dusk, declared weather profiles, receiver/normal/layer views and an optional design sheet.

## What to try

1. Install **30801** over your last build. Let the first-start self-check finish; send its code if it fails.
2. Open **Terrain**, starting with **flat**. Compare **Noon / dusk**, **Sun direction** and **Fire**. Judge the flat shadow style first: contact, direction, long shadows and fire in shade.
3. Then choose **slope**, **cliff**, **shelter** and **water**. Check feet against the ground, shadows across tile edges, and figures in front of and behind cover.
4. Try **Select person** and **Reveal** beneath the tree and roof. Use **Cave entrance / exit** to enter the separate interior and return to the same exterior focus.
5. Pan, pinch and turn the phone. Open **Fixtures** to try **Walk / work**, **Turn**, **4 / 8 facings** and the six piece inspections.

Say whether the flat style passes, then whether the terrain result passes. Name the scene, hour and control setting for anything wrong.

## What is rough

- Tree, shelter, boulder and ground previews still include sheet crops; people and animals are diagrams. The old dome-shaped shelter crop is temporary: the intended tent is the 16.4 cone with a 4.2 m ring and 3.1 m tips, which will arrive with its reviewed art. Final neutral art, aligned material/normal pages, repaired zoom art and seasonal shapes have separate reviews.
- Terrain, hours and weather are declared test fixtures. They are not generated geography, a physical sky or a living camp. The winter setting changes the light; it does not prove winter tree shapes.
- Several inspection selectors still say “colour”; their labels need clarification.
- Water still reads as a green test rectangle and the ground is visibly noisy. The separate reflection and bed targets work, but their final art treatment needs review.
- The cave is a separate restricted interior. Geological slices and stacked overhangs remain later work.
- Phone frame time, power and temperature have not been measured for this 2D build. Cloud captures do not establish a phone performance pass. The two-phone comparison remains a later step.
- Older 3D pages remain during migration; their results are historical evidence, not a pass for this 2D engine.

## IDs delivered

Restricted engine work for `PRE-01`, `PRE-02`, `PRE-03`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-27`, `PRE-28`, `PRE-30`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PLT-02`, `PLT-04`, `WLD-13`, `TIM-17` and `RES-05`.
These fixtures establish the drawing route; final art, complete world behaviour and phone acceptance are not claimed.

## Links

- [Eight-second review clip](pictures/a28a-review.mp4): sun, shelter, water and cave entry/exit.

- [Flat lighting comparison](pictures/a28a-flat-lighting.png) and [cliff lighting comparison](pictures/a28a-cliff-lighting.png): unlit fixture versus lit fixture, not approved art baselines.
- [Shelter](pictures/a28a-shelter.png), [water](pictures/a28a-water.png), [receiver debug](pictures/a28a-receivers.png) and [cave](pictures/a28a-cave.png).

- [Download the APK](https://github.com/gunsandsalvi/Project-Nature/raw/main/dist/kindling.apk)
- [APK checksum](https://github.com/gunsandsalvi/Project-Nature/blob/main/dist/kindling.apk.sha256)
- [This note](https://github.com/gunsandsalvi/Project-Nature/blob/main/dist/NOTE.md)
- [The remaining plan](https://github.com/gunsandsalvi/Project-Nature/blob/main/IMPLEMENTATION.md)
