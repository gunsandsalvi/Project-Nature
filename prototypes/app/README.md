# The prototype app

Pre-production's one app on your phone (α0.1a, research 00): it opens on the self-check, and its menu lists the
prototypes the plan brings, each named with its step; each phone prototype joins the menu as it is built.
Like every prototype it is thrown away at the end of pre-production, once the vertical slice stands.

What it is about:
- `PLT-01` one phone: the self-check shows the version, the phone's model, the screen and the graphics driver, and
  copies them in one line for the chat;
- `PLT-02` portrait and landscape: one column of panels, the menu below the self-check in portrait and beside it in
  landscape;
- `PLT-03` offline: no network permission; `PLT-06` installing: package `dev.kindling.app`, over the old app.

Its test, `test/main_test.gd`, lays the screen out in both orientations with facts as long as a phone's.
`tools/build.sh` builds it, and `tools/godot-picture.gd` draws its pictures for the note.
The icon is the old app's 16 × 16 campfire, drawn at whole-pixel sizes.
