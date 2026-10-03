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
