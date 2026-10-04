# Research 14: the interface on the phone

**Question:** how should a deep simulation be read and controlled on one phone, one-handed in portrait and two-handed in landscape?
Can Godot lay it out, read the gestures and draw crisp pixel text on your phone (`PRE-32` to `PRE-35`, `PRE-40`, `PRE-45`, `PLT-02`, `VIS-14`)?

## What `PROJECT.md` asks

- **The world first:** panels only when asked; a few seconds after a touch, only the world, live moments and talk bubbles remain (`PRE-32`).
- **Both orientations:** each view is one column of panels.
  - In portrait it is full width, with its controls in the bottom third.
  - In landscape it sits beside the world (`PRE-34`, `PLT-02`).
- **Gestures** (`PRE-33`):
  - drag to move and twist to turn;
  - pinch, or double-tap and drag with one thumb, to zoom and set time's speed;
  - tap to select, long-press for powers;
  - a handle above the bottom edge for the views, "the edge itself is the phone's".
- **Cards** for people, things, crafts, places and peoples (`PRE-35`).
- **Talk bubbles** with a picture of the topic (`PRE-45`).
- **A few screens:** first launch, worlds, settings and credits; short help cards and no tutorial (`PRE-40`).

## How others do it

- **Prioritise, don't shrink:** "The goal of mobile UI adaptation is not to fit everything on screen at once. It is to show exactly what the player needs, exactly when they need it."
  Panels change with what the player is doing ([Ocean View Games](https://oceanviewgames.co.uk/blog/posts/mobile-ui-design-complex-games)).
- **Targets big enough for a thumb:**
  - Android's accessibility guidance: "at least 48x48dp, separated by 8dp of space or more", about 9 mm ([Android Accessibility Help](https://support.google.com/accessibility/android/answer/7101858));
  - Ocean View Games: "no interactive element smaller than 44x44 points, regardless of how it looks visually".
- **How people really hold phones:** Steven Hoober watched "1,333 observations of people using mobile devices" ([UXmatters](https://www.uxmatters.com/mt/archives/2013/02/how-do-users-really-hold-mobile-devices.php)):
  - 49% held them in one hand, 36% cradled them, and 15% used two hands;
  - one-handed, 67% used the right thumb;
  - his reach maps mark easy, stretch and regrip zones, and he warns of "how much of the screen a finger may obscure".

  This is why `PRE-34` puts portrait controls in the bottom third, and why every gesture must work with one thumb (`PRE-33`).
- **What the screen is in each orientation:** Android's window size classes put phones in portrait at compact width (under 600 dp).
  In landscape they are at medium width but compact height, under 480 dp ([Android: window size classes](https://developer.android.com/develop/ui/compose/layouts/adaptive/use-window-size-classes)).
  So in landscape a panel beside the world, not under it, is the natural layout: `PRE-34` already says so.
- **The edges belong to the phone:**
  - "Edge-to-edge is enforced on Android 15 (API level 35) and higher once your app targets SDK 35."
  - "System gesture insets represent the areas of the window where system gestures take priority over your app", and apps should "pad swipeable views away from the edges", naming "bottom sheets, swiping in games".
  - In landscape, the display cutout "may be on the vertical edge" ([Android: edge-to-edge](https://developer.android.com/develop/ui/views/layout/edge-to-edge)).

  This is why `PRE-33` places the views' handle above the bottom edge.

## Can Godot do it?

| Need | What Godot has | Verdict |
|---|---|---|
| Layouts that reflow | Containers: "all children Control nodes give up their own positioning ability"; box, margin, scroll and flow containers lay them out ([Godot docs: containers](https://docs.godotengine.org/en/stable/tutorials/ui/gui_containers.html)) | Yes |
| Both orientations | `DisplayServer.screen_set_orientation` with `SCREEN_SENSOR`, "Automatically rotate based on device orientation sensor" ([DisplayServer](https://docs.godotengine.org/en/stable/classes/class_displayserver.html)) | Yes |
| One design for both | For both orientations, "Set your project's base resolution to be a *square* (1:1 aspect ratio)"; the integer scale mode (Godot 4.2+) "rounds down to prevent pixel art distortion" ([Godot docs: multiple resolutions](https://docs.godotengine.org/en/stable/tutorials/rendering/multiple_resolutions.html)) | Yes |
| Crisp pixel text | Nearest filtering, subpixel positioning "Disabled", and "the font size must also be an integer multiple of the design size … and the Control node … must be scaled by an integer multiple as well" ([Godot docs: fonts](https://docs.godotengine.org/en/stable/tutorials/ui/gui_using_fonts.html)) | Yes, with care |
| Notches and gesture bars | `get_display_safe_area()` "not including areas with a notch", and `get_display_cutouts()`; edge-to-edge on Android since Godot 4.5, drawing "on the entire screen … with system bar overlays" ([DisplayServer](https://docs.godotengine.org/en/stable/classes/class_displayserver.html), [Godot 4.5](https://godotengine.org/releases/4.5/)) | Yes |
| Pinch and twist | `InputEventMagnifyGesture` exists, but on Android it needs the `input_devices/pointing/android/enable_pan_and_scale_gestures` setting ([class docs](https://docs.godotengine.org/en/stable/classes/class_inputeventmagnifygesture.html)) | We read raw touches ourselves instead |
| Sections that fold | `FoldableContainer`, new in 4.5, for accordion sections ([Godot 4.5](https://godotengine.org/releases/4.5/)) | Yes, for long cards |
| Screen readers | Godot 4.5 added "screen reader support to Control nodes" through AccessKit ([Godot 4.5](https://godotengine.org/releases/4.5/)) | A free extra for cards and the book |

**Verdict:** nothing here needs changes to Godot.
The care points are:
- integer scaling of the pixel font;
- our own gesture reader;
- the system's gesture areas at the edges.

## What we take

1. **The world fills the screen,** and panels show only what the moment needs, then fade (`PRE-32`).
2. **One column of panels:** full width with controls in the bottom third in portrait; beside the world in landscape (`PRE-34`).
   This follows Hoober's thumb zones and Android's size classes.
3. **Every control at least 48 dp,** about 9 mm, whatever the pixel scale, with 8 dp between them.
4. **Our own gesture reader on raw touches,** so all gestures share one rule set and a scripted test can tell them apart (`PRE-33`).
   The handle and any edge swipe stay outside the system's gesture insets.
5. **One Godot theme in the art bible's palette and a pixel font** at whole multiples of its design size, nearest filtering and no subpixel positioning.
   The interface is laid out on a square base so both orientations scale the same.
   Safe areas and cutouts come from `DisplayServer`.
6. **Cards that open to what matters now, with deeper sections folding out** (`PRE-35`).
   Screen-reader labels come at little cost with Godot 4.5.
7. **An interface prototype before production,** on your phone:
   - a stand-in world, one card, one book page and the time controls, in both orientations;
   - it checks thumb reach, gesture misreads in a scripted test, and that text stays crisp at the chosen scale.

## Sources

- Practice:
  - [Ocean View Games: mobile UI for complex games](https://oceanviewgames.co.uk/blog/posts/mobile-ui-design-complex-games)
  - [Android Accessibility Help: touch targets](https://support.google.com/accessibility/android/answer/7101858)
  - [UXmatters: Hoober, how users hold devices](https://www.uxmatters.com/mt/archives/2013/02/how-do-users-really-hold-mobile-devices.php)
  - [Android: window size classes](https://developer.android.com/develop/ui/compose/layouts/adaptive/use-window-size-classes)
  - [Android: edge-to-edge](https://developer.android.com/develop/ui/views/layout/edge-to-edge)
- Godot:
  - [Godot docs: containers](https://docs.godotengine.org/en/stable/tutorials/ui/gui_containers.html)
  - [Godot docs: multiple resolutions](https://docs.godotengine.org/en/stable/tutorials/rendering/multiple_resolutions.html)
  - [Godot docs: using fonts](https://docs.godotengine.org/en/stable/tutorials/ui/gui_using_fonts.html)
  - [Godot class: DisplayServer](https://docs.godotengine.org/en/stable/classes/class_displayserver.html)
  - [Godot class: InputEventMagnifyGesture](https://docs.godotengine.org/en/stable/classes/class_inputeventmagnifygesture.html)
  - [Godot 4.5 release](https://godotengine.org/releases/4.5/)
