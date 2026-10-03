# Tuning log

Every change to a tuned number in `data/`, with what it was tuned against and why (A3.9, `RES-16`).
Tuning runs never use the pace tests' seeds.

## 3 October 2026, α01a: ozone's red depth

- **Changed:** `data/palette/light.md`, `ozone` red 0.020 to 0.031.
- **Tuned against:** the light card's swatches at 18:30 (the sun 7° below the horizon at 21° N), where every look turned purple.
- **Why:** the red channel's band covers about 580 to 700 nm and so includes the Chappuis band's peak near 600 nm; 680 nm alone gave red less loss than green, which tinted twilight purple. Averaged over the band, red loses a little more than green, and twilight turns blue, as the eye sees it and A11.4 means it.
- **Seeds:** none; the palette has no randomness.

## 3 October 2026, α01a: twilight's ozone path

- **Changed:** `data/palette/light.md`, `twilight_ozone_per_deg` 10 to 3.
- **Tuned against:** the light card's golden at 18:30 (`tests/golden/light-card-1830.png`, the sun 7° below the horizon), where the block and every swatch were an electric blue.
- **Why:** 10 air masses a degree put sunlight through 70 air masses of ozone at −7° and 120 at −12°, making twilight's light six times as blue as green; sunlight grazing the ozone layer crosses at most about 40, and measured nautical twilight (about 15,000 to 25,000 K) is about twice as blue as green. 3 a degree gives 21 at −7° and 36 at −12°, a slate blue, and twilight's light at −6° comes nearer the 3 lux the entry names.
- **Seeds:** none; the palette has no randomness.

## 3 October 2026, α01d: the surfaces' relief

- **Changed:** `data/models/surfaces.md`, each surface's `relief_tilt`, first tried at 0.3 to 0.5: grass 0.2, dirt 0.15, rock 0.3, scree 0.25.
- **Tuned against:** the close camp's goldens at 16:30 (`tests/golden/valley-close-1630.png`, `valley-near-1630.png`) and the zoom strip's counts at both stops.
- **Why:** at 0.3 to 0.5 the texture crawled more as the camera zoomed, and mottled the shadows' edges widely; at these tilts a self-similar surface's bumps are about as steep at every size, and the ground reads as grass, soil and stone without shimmering.
- **Seeds:** none; the relief is a noise fixed to the world.

## 3 October 2026, α01d: the stones' and tufts' shade

- **Changed:** the contact shade's strength 0.6 and its cap 0.7, and the share of the sky a stone's middle sees, 0.6, and a blade's foot, 0.45: first `ground::cover`'s constants, then at α02a `cover_contact`, `cover_contact_max`, `stone_foot_sky` and `blade_foot_sky` in `data/tuning/render.md`, the values unchanged.
- **Tuned against:** the close camp's goldens at 16:30 and the person stop's view of the demo area's scree and grass.
- **Why:** the contact shade first widened the light's dither band too, which speckled lit grass, so it now only darkens; tufts vanished into the ground in shade until their feet saw less of the sky than their tips; stones vanished on scree of their own look until their sides did the same. The cap keeps a dense clump from blacking out the ground.
- **Seeds:** the demo area's own stones and tufts, seeded by its number; no pace test's seed.

## 3 October 2026, α02a: where streams begin

- **Changed:** `data/tuning/world.md`, `stream_min_km2` 1 to 5.
- **Tuned against:** the first region's water map (the note's `kd map preview --layer water`): the share of land cells holding a stream and the springs at their heads.
- **Why:** every 1 km cell drains its own 1.05 km², so at 1 every land cell held a stream (59% even counting only the land draining into a cell), and a spring rose at nearly every head; at 5 a stream drains about five cells, a quarter of the island's cells hold one, and the network reads like a 1:250,000 map's.
- **Seeds:** the first region's seed 1; no pace test's seed.
