# Tuning log

Every change to a tuned number in `data/`, with what it was tuned against and why (A3.9, `RES-16`).
Tuning runs never use the pace tests' seeds.

## 3 October 2026, α01a: ozone's red depth

- **Changed:** `data/palette/light.md`, `ozone` red 0.020 to 0.031.
- **Tuned against:** the light card's swatches at 18:30 (the sun 7° below the horizon at 21° N), where every look turned purple.
- **Why:** the red channel's band covers about 580 to 700 nm and so includes the Chappuis band's peak near 600 nm; 680 nm alone gave red less loss than green, which tinted twilight purple. Averaged over the band, red loses a little more than green, and twilight turns blue, as the eye sees it and A11.4 means it.
- **Seeds:** none; the palette has no randomness.
