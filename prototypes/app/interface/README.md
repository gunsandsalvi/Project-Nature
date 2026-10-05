# P12 The interface

The twelfth prototype (IMPLEMENTATION α0.7a, research 14) asks: do thumb reach, gestures and crisp pixel text work in
both orientations on your phone?

Items it is about: `PRE-32` (the world first), `PRE-33` (gestures), `PRE-34` (both orientations), `PRE-35` (cards),
`PLT-02` (portrait and landscape), with `TIM-04`, `TIM-11` (the time controls), `PRE-08` (live moments) and `PRE-05`
(the book of ages) as what it shows.

- **A stand-in world** (`interface.gd`): the art book's zoom stops at noon, from one person to the globe, painted
  upright and sideways (`world/`); a pinch, or a double tap dragged, moves between them and sets the speed each asks
  (`TIM-01`), a drag moves the picture and springs back, a twist turns the heading, and a long press opens your powers.
- **The interface the art book's plates show:** after any touch, the date, the real speed and the time controls
  (pause, play, the dial, the lock and skip), fading after a few seconds; a live moment; the handle for the views;
  Aru's card and the book of ages open at the age of hesoru (`panels.gd`).
- **Five grounds for the card and the book** (`styles.gd`), as you asked on 5 October: the art book's paper, Night,
  Hide, Slate and Glass. The Ground control at a panel's foot changes it.
- **Drawn in art pixels:** the whole screen is drawn into one picture, a whole number of screen pixels to an art pixel
  (3 on your phone's 1080-pixel setting, 4 on the full panel), so every letter and line is whole.
- **The pixel fonts** (`pixel_font.gd`, `hand_text.gd`): from `glyphs.json`, which `tools/pixel-font.py` writes from
  the art book's `font.js`, where they are designed: the plain font for reading, drawn only at whole multiples, and
  the handwriting for big titles.
- **One gesture reader** (`gestures.gd`) on raw touches, as A15 asks, so a scripted test can tell each gesture from
  every other.
- **Every control at least 48 dp and 8 dp apart,** and in portrait in the bottom third, where a thumb reaches.
- **The pictures** (`art/`, `world/`) are cut from the art book's plates by `make_art.py`.

Its tests, `test/interface_test.gd`, script every gesture on raw touches, and lay the screen out on your phone's
screen and on the full panel, both ways up and with each panel open, checking every control's size, spacing and
reach.
