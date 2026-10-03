# Research 10: the interface on the phone

**Question:** how should a deep simulation be read and controlled on one phone, one-handed in portrait and two-handed in landscape (`PRE-32` to `PRE-35`)?

## How others do it

- **Prioritise, don't shrink.**
  - Desktop sims show many panels at once, which a 6-inch screen cannot.
  - The answer is "not miniaturization but prioritization": show what the player needs when they need it.
  - Panels change with what the player is doing ([Ocean View Games](https://oceanviewgames.co.uk/blog/posts/mobile-ui-design-complex-games), [Game Developer](https://www.gamedeveloper.com/design/mobile-vs-desktop-ui-how-they-differ-in-design)).
- **Thumb reach.**
  In portrait the middle and bottom of the screen are easy to reach and the top is hard, so controls go low and read-only information goes high.
  Bottom sheets suit portrait ([Pocket Gamer](https://www.pocketgamer.biz/how-to-make-your-games-perfect-for-mobile), [UXPin](https://www.uxpin.com/studio/blog/what-is-mobile-ui/)).
- **Both orientations.**
  The Elder Scrolls: Blades built one interface that works in landscape and portrait ([UESP](https://content2.uesp.net/wiki/General:Building_the_Interface_of_The_Elder_Scrolls:_Blades_in_Landscape_and_Portrait)).
- **Godot's touch support:**
  - raw touches and drags;
  - pinch (magnify) and pan gestures, which need a project setting on Android;
  - themes that style every control at once ([Godot docs](https://docs.godotengine.org/en/4.2/classes/class_inputeventmagnifygesture.html)).

## What we take

1. **The world fills the screen**; panels appear only when asked and fade after a touch (`PRE-32`).
2. **Portrait:** bottom sheets for cards and views, controls in the bottom third, read-only lines at the top.
   **Landscape:** the same sheet as a side column (`PRE-34`).
3. **Our own gesture reader on raw touches**, as in the test app: drag, pinch, twist, double-tap-drag, long-press.
   Godot's built-in pinch is not used, so all gestures share one rule set and a scripted test can tell them apart (`PRE-33`).
4. **One Godot theme in the art guide's palette and a pixel font**, at whole multiples of the art pixel, so text is as crisp as the world (`PRE-01`).
5. **Cards are context-sensitive:**
   - tap a person and their card opens to what matters now (what they do and why);
   - deeper views slide in from it (`PRE-35`).
