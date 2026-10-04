# Research 02: the phone

**Question:** what can your Pixel 11 Pro XL really give a game, and what about it can go wrong (`PLT-01`, `PLT-04`, `VIS-14`, `TIM-07`, `TIM-12`)?

## The hardware

- **Screen:** 6.8-inch LTPO OLED, 1344 × 2992 pixels, 1 to 120 Hz ([Droid Life](https://www.droid-life.com/2026/08/03/heres-pixel-11-series-specs-and-prices-again/)).
  At 4 × 4 screen pixels per art pixel, the art is 336 × 748 in portrait (`PRE-22`).
- **Memory, storage and battery:** 16 GB of memory and 512 GB of storage on yours (`PLT-01`); a 5,115 mAh battery, slightly smaller than the Pixel 10 Pro XL's ([Beebom](https://gadgets.beebom.com/reviews/google-pixel-11-pro-xl-review)).
- **Processor (Tensor G6):** seven cores ([9to5Google](https://9to5google.com/2026/05/04/google-pixel-11-specs-cameras-tensor-g6-leak/), [Notebookcheck](https://www.notebookcheck.net/Google-Pixel-11-Pro-XL-Geekbench-7-result-offers-early-look-at-Tensor-G6-performance.1364515.0.html)):
  - one fast C1-Ultra at 4.11 GHz;
  - four C1-Pro at 3.38 GHz;
  - two C1-Pro at 2.65 GHz.
  - Against last year's Tensor G5: about 18% faster on one core and 21% on all.
    Against the Snapdragon 8 Elite Gen 5: 37% and 52% behind ([Android Authority](https://www.androidauthority.com/tensor-g6-benchmarks-tests-3699714/)).
- **Graphics chip:** an Imagination PowerVR C-Series CXTP-48-1536 at about 1.29 GHz.
  - It is tile-based, so its limit is memory traffic rather than raw arithmetic.
  - It runs Vulkan 1.4, with Imagination's driver 25.3 ([Bevy issue 25788](https://github.com/bevyengine/bevy/issues/25788)).
  - Its peak is only 7 to 10% above the Pixel 10's.
    Rival chips score from 60% higher (Apple) to more than double (Adreno 840) ([Android Authority](https://www.androidauthority.com/tensor-g6-benchmarks-tests-3699714/)).
- **Efficiency:** both Tensor G5 and G6 trail Snapdragon and Apple in work done per watt, on the processor and the graphics chip alike ([Notebookcheck](https://notebookcheck.net/Pixel-11-s-Tensor-G6-and-Pixel-10-s-Tensor-G5-still-fall-behind-in-performance-per-watt-testing-shows.1381970.0.html)).

## How it behaves under load

- **Heat:**
  - Its graphics speed falls below last year's chip after a few minutes of stress, keeping about 79% of its peak; rivals keep 50 to 75% ([Android Authority](https://www.androidauthority.com/tensor-g6-benchmarks-tests-3699714/)).
  - In Genshin Impact it passed 40 °C after five minutes, with stretches of low frame rates ([Android Authority](https://www.androidauthority.com/pixel-11-gaming-test-3708496/)).
- **Games:** it cannot hold 60 frames a second at high settings in Asphalt Legends or Genshin Impact, and gains less than 5% over the Pixel 10 Pro XL ([Android Authority](https://www.androidauthority.com/pixel-11-gaming-test-3708496/)).
  Call of Duty Mobile at medium holds above 60.
- **The power draw of a heavy game:** on the Pixel 10 Pro XL, Genshin Impact drew 7.2 W ([Android Authority](https://www.androidauthority.com/pixel-10-pro-xl-gaming-benchmarks-3609172/)).
  `PLT-04` allows an hour's play to use 25 to 30% of the battery.
  That is about 5 to 6 W for the whole phone, screen included, so Kindling must stay well under a heavy game's draw.
- **The last generation's processor** fell to 47% of its peak under a long processor-only test ([9to5Google](https://9to5google.com/2025/08/27/google-pixel-10-tensor-g5-chip-benchmarks-heat/)).
  So the simulation's speed targets must be read at the speed the phone holds after minutes of load, as `PLT-04`'s "held speed" already says.

## Its graphics drivers: the biggest risk

- **The Pixel 10's PowerVR chip** shipped with an old driver (24.3) that did not support Android 16.
  Imagination already had a newer one, and Google promised fixes in monthly updates ([Android Authority](https://www.androidauthority.com/pixel-10-genshin-impact-gpu-problem-3604104/), [TechRadar](https://www.techradar.com/phones/google-pixel-phones/google-promises-pixel-10-owners-that-more-gpu-updates-are-on-the-way)).
  Genshin Impact dropped PowerVR support for a while.
- **On the Pixel 11:**
  - Genshin Impact showed bright purple textures until a fix was agreed with its maker ([Notebookcheck](https://www.notebookcheck.net/Pixel-11-s-new-PowerVR-GPU-leaves-Genshin-Impact-unplayable-Google-confirms-a-fix-is-in-the-works.1378023.0.html)).
  - Bevy crashed before its first frame, because the PowerVR shader compiler fails on compute shaders that sample images ([Bevy issue 25788](https://github.com/bevyengine/bevy/issues/25788)).
- **Since the September 2026 update,** the Pixel 11 runs OpenGL ES through ANGLE, a translation layer on Vulkan.
  It stutters in OpenGL ES games while asset loads rebuild its caches.
  Vulkan games are barely affected ([Android Authority](https://www.androidauthority.com/pixel-11-angle-gpu-test-gaming-problem-3712446/)).

**What this means for Kindling:**
- **Use Vulkan.**
  Godot's Compatibility renderer (OpenGL ES) is no safe fallback on this phone, so the Mobile renderer is our path (research 01).
- **Avoid compute shaders that read textures,** and prefer full-screen fragment passes for effects.
- **Exercise every rendering feature in the first phone builds:** shadows, MultiMesh, transparency, every shader trick.
  A driver bug found late is the most expensive kind.
- **Keep the build's driver details on the self-check screen,** so a report says which driver was in use.

## Android's rules that shape the game

- **Frame rate and battery:**
  - Godot's frame cap renders 60 frames a second but leaves the screen refreshing at 120.
    Battery-minded games ask Android to run the screen at 60 ([Godot forum](https://forum.godotengine.org/t/android-godot-why-fps-limit-display-refresh-rate-on-120-hz-devices-terraria-mobile-legends-example/132320)).
  - We aim at a steady 60, through a few lines of plug-in code.
- **Heat:** Android's thermal headroom forecast (ADPF) warns before throttling, so the game can slow time on its own before the phone does (`PRN-11`) ([Android Developers](https://developer.android.com/games/engines/unreal/unreal-adpf)).
- **Background:** Godot stops when the app leaves the screen, and Android may close it there, so the world is saved on pausing (`TIM-05`, `PLT-07`).
- **Overnight (`TIM-12`):** the app stays in front with the screen black and Godot's drawing switched off, which uses little power and cannot burn in an OLED screen.
  The phone's writer also only works while the app is in front ([ML Kit](https://developers.google.com/ml-kit/genai)).

## Budgets to measure first

These are starting estimates, to be replaced by the prototypes' measurements on your phone:
- **Frame:** 16.7 ms at 60 frames a second, with the graphics chip under about 8 ms in the busiest scene, so heat leaves room.
- **Simulation:** up to the four 3.38 GHz cores at the speed they hold after 10 minutes (`PLT-01`); the fast core and the two slower ones stay for Godot, sound and the system.
- **Power:** about 3 W for the game while playing (`PLT-01`), to be measured with the phone's own counters.
- **Memory:** within about 8 GB (`PLT-01`), leaving the rest to Android and the writer.

## What we take

1. **Vulkan and Godot's Mobile renderer,** with no compute shaders that read textures.
2. **Every first phone build exercises the full set of rendering features,** and its self-check shows the driver.
3. **Budgets are set from measurements on your phone** at held speed, not from benchmarks of other phones.
4. **Kindling slows time before the phone overheats** (ADPF), and runs the screen at 60 Hz.
5. **Overnight mode keeps the app in front on a black screen.**

## Sources

- Specs:
  - [Droid Life: Pixel 11 specs](https://www.droid-life.com/2026/08/03/heres-pixel-11-series-specs-and-prices-again/)
  - [9to5Google: Tensor G6](https://9to5google.com/2026/05/04/google-pixel-11-specs-cameras-tensor-g6-leak/)
  - [Notebookcheck: Geekbench](https://www.notebookcheck.net/Google-Pixel-11-Pro-XL-Geekbench-7-result-offers-early-look-at-Tensor-G6-performance.1364515.0.html)
  - [Beebom: review](https://gadgets.beebom.com/reviews/google-pixel-11-pro-xl-review)
- Tests:
  - [Android Authority: Tensor G6 tests](https://www.androidauthority.com/tensor-g6-benchmarks-tests-3699714/)
  - [Android Authority: Pixel 11 gaming](https://www.androidauthority.com/pixel-11-gaming-test-3708496/)
  - [Android Authority: ANGLE](https://www.androidauthority.com/pixel-11-angle-gpu-test-gaming-problem-3712446/)
  - [Android Authority: Pixel 10 Pro XL gaming](https://www.androidauthority.com/pixel-10-pro-xl-gaming-benchmarks-3609172/)
  - [Notebookcheck: performance per watt](https://notebookcheck.net/Pixel-11-s-Tensor-G6-and-Pixel-10-s-Tensor-G5-still-fall-behind-in-performance-per-watt-testing-shows.1381970.0.html)
  - [9to5Google: Tensor G5 heat](https://9to5google.com/2025/08/27/google-pixel-10-tensor-g5-chip-benchmarks-heat/)
- Drivers:
  - [Bevy issue 25788](https://github.com/bevyengine/bevy/issues/25788)
  - [Notebookcheck: Genshin on Pixel 11](https://www.notebookcheck.net/Pixel-11-s-new-PowerVR-GPU-leaves-Genshin-Impact-unplayable-Google-confirms-a-fix-is-in-the-works.1378023.0.html)
  - [Android Authority: Pixel 10 GPU](https://www.androidauthority.com/pixel-10-genshin-impact-gpu-problem-3604104/)
  - [TechRadar: Pixel 10 GPU updates](https://www.techradar.com/phones/google-pixel-phones/google-promises-pixel-10-owners-that-more-gpu-updates-are-on-the-way)
- Android:
  - [ML Kit GenAI](https://developers.google.com/ml-kit/genai)
  - [ADPF](https://developer.android.com/games/engines/unreal/unreal-adpf)
  - [Godot forum: 120 Hz](https://forum.godotengine.org/t/android-godot-why-fps-limit-display-refresh-rate-on-120-hz-devices-terraria-mobile-legends-example/132320)
