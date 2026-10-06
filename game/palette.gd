## The app's colours, written once: every page takes its ground and text, its quieter lines,
## headings, warnings, and what passed or failed from here, until the game's own look replaces
## them (M2).
class_name Palette
extends RefCounted

## The pages' ground, and text on it.
const GROUND := Color("#1f1a24")
const TEXT := Color("#efe6d8")
## Lines that say less: details, plain facts, a chart's axes.
const QUIET := Color("#a89f95")
## Headings, apart from the yellow that warns.
const HEAD := Color("#9db7d8")
const WARN := Color("#e8c25a")
## What passed, and what failed.
const GOOD := Color("#8fd18a")
const FAIL := Color("#ef7b6b")
