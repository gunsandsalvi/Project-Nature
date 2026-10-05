## P14's stand-in camp (IMPLEMENTATION α0.7c, SND-01, SND-11): everything that sounds in the art
## book's close camp, placed on its picture in the picture's pixels: the fire and the people at work
## and talking under the rock shelter, two more at the tents, two above the cliff and a group in the
## meadow below; children; walkers on the path; dogs, birds in the wood and a wolf far off; the
## stream, the wind, rain and thunder. Each sounds in the hours it is set for, at its own pace.
## Pre-production code (research 00).
extends RefCounted

const Voices := preload("res://sound/voices.gd")

## Metres to a pixel of the picture, and the pixel at the world's middle.
const METRE := 0.3
const MIDDLE := Vector2(168.0, 374.0)
## The rock shelter, where sounds echo: its box in the picture.
const CAVE := Rect2(128.0, 284.0, 174.0, 82.0)
## The cliff's top and foot: a sound above the one and a listener below the other, or the other way
## round, hear each other muffled by the rock between (SND-08).
const CLIFF_TOP := 210.0
const CLIFF_FOOT := 290.0
const HOURS: Array[String] = ["day", "dusk", "night", "storm"]
## Where a maker is that sounds all round you, as wind and rain do.
const AROUND := Vector2(-1, -1)

## Who speaks: their base voice (0 the woman's, 1 the man's) and its shift for age and build.
const SPEAKERS := {
	"man": [1, {"pitch": 1.0, "formants": 1.0}],
	"big man": [1, {"pitch": 0.88, "formants": 0.94}],
	"woman": [0, {"pitch": 1.0, "formants": 1.0}],
	"girl": [0, {"pitch": 1.1, "formants": 1.05}],
	"old man": [1, {"pitch": 0.9, "formants": 0.97, "breath": 0.2, "tremor": 0.035}],
	"old woman": [0, {"pitch": 0.88, "formants": 0.97, "breath": 0.2, "tremor": 0.04}],
	"child": [0, {"pitch": 1.42, "formants": 1.16, "breath": 0.05}],
}
## How a feeling shifts the voice (SND-03, MND-19).
const FEELINGS := {
	"calm": {},
	"glad": {"tempo": 1.15, "range": 1.5, "loud": 1.2, "fall": 0.05, "lift": 1.1},
	"angry": {"tempo": 1.25, "range": 1.6, "loud": 1.6, "fall": 0.05, "lift": 1.15},
	"grieving": {"tempo": 0.75, "range": 0.6, "loud": 0.6, "fall": 0.3, "lift": 0.95},
	"hushed": {"tempo": 0.9, "range": 0.8, "loud": 0.7, "fall": 0.2, "lift": 0.97},
}

## The sounds' makers. Each: what it is; its kind; its share; where it is (or the path it walks);
## the hours it sounds ("busy": only with everyone at once); the seconds between its sounds; how
## loud it is at `unit` metres; and, for talk, who speaks and how they feel. Loops sound all
## through their hours.
const SOURCES: Array[Dictionary] = [
	{
		"name": "fire",
		"kind": "fire",
		"share": Voices.Share.WORK,
		"at": Vector2(212, 340),
		"hours": HOURS,
		"volume": -4.0,
		"unit": 3.0
	},
	{
		"name": "stream",
		"kind": "river",
		"share": Voices.Share.PLACE,
		"at": Vector2(40, 418),
		"hours": HOURS,
		"volume": -8.0,
		"unit": 6.0,
		"seed": 1
	},
	{
		"name": "stream",
		"kind": "river",
		"share": Voices.Share.PLACE,
		"at": Vector2(176, 440),
		"hours": HOURS,
		"volume": -8.0,
		"unit": 6.0,
		"seed": 2
	},
	{
		"name": "stream",
		"kind": "river",
		"share": Voices.Share.PLACE,
		"at": Vector2(300, 455),
		"hours": HOURS,
		"volume": -8.0,
		"unit": 6.0,
		"seed": 3
	},
	{
		"name": "wind",
		"kind": "wind",
		"share": Voices.Share.PLACE,
		"at": AROUND,
		"hours": HOURS,
		"volume": -15.0,
		"unit": 1.0
	},
	{
		"name": "rain",
		"kind": "rain",
		"share": Voices.Share.PLACE,
		"at": AROUND,
		"hours": ["storm"],
		"volume": -17.0,
		"unit": 1.0
	},
	{
		"name": "thunder",
		"kind": "thunder",
		"share": Voices.Share.FREE,
		"at": AROUND,
		"hours": ["storm"],
		"every": Vector2(9, 24),
		"volume": 2.0,
		"unit": 300.0
	},
	{
		"name": "a thrush",
		"kind": "bird",
		"share": Voices.Share.PLACE,
		"at": Vector2(40, 24),
		"hours": ["day"],
		"every": Vector2(3, 9),
		"volume": -6.0,
		"unit": 4.0,
		"seed": 11
	},
	{
		"name": "a wren",
		"kind": "bird",
		"share": Voices.Share.PLACE,
		"at": Vector2(180, 52),
		"hours": ["day"],
		"every": Vector2(4, 11),
		"volume": -8.0,
		"unit": 4.0,
		"seed": 12
	},
	{
		"name": "a finch",
		"kind": "bird",
		"share": Voices.Share.PLACE,
		"at": Vector2(300, 30),
		"hours": ["day", "dusk"],
		"every": Vector2(5, 13),
		"volume": -8.0,
		"unit": 4.0,
		"seed": 13
	},
	{
		"name": "a knapper by the fire",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(190, 350),
		"hours": ["day", "dusk"],
		"every": Vector2(0.5, 1.4),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.95, 0.25, 0.0]
	},
	{
		"name": "a knapper at the tents",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(88, 396),
		"hours": ["day"],
		"every": Vector2(0.6, 1.8),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.9, 0.45, 0.0]
	},
	{
		"name": "a hide scraper",
		"kind": "scrape",
		"share": Voices.Share.WORK,
		"at": Vector2(262, 352),
		"hours": ["day", "dusk"],
		"every": Vector2(0.7, 1.6),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.3, 0.5, 0.2]
	},
	{
		"name": "a scraper at the tents",
		"kind": "scrape",
		"share": Voices.Share.WORK,
		"at": Vector2(40, 392),
		"hours": ["day"],
		"every": Vector2(0.8, 2.0),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.25, 0.4, 0.3]
	},
	{
		"name": "a wood chopper",
		"kind": "chop",
		"share": Voices.Share.WORK,
		"at": Vector2(300, 80),
		"hours": ["day"],
		"every": Vector2(1.2, 2.4),
		"volume": -2.0,
		"unit": 4.0,
		"stuff": [0.7, 0.6, 0.1]
	},
	{
		"name": "a walker",
		"kind": "step",
		"share": Voices.Share.WORK,
		"path": [Vector2(184, 412), Vector2(150, 520), Vector2(122, 640), Vector2(100, 745)],
		"hours": ["day", "dusk"],
		"every": Vector2(0.5, 0.6),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.3, 0.5, 0.0],
		"speed": 4.0
	},
	{
		"name": "a walker",
		"kind": "step",
		"share": Voices.Share.WORK,
		"path": [Vector2(100, 745), Vector2(122, 640), Vector2(150, 520), Vector2(184, 412)],
		"hours": ["day"],
		"every": Vector2(0.55, 0.65),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.35, 0.65, 0.0],
		"speed": 3.6
	},
	{
		"name": "a dog",
		"kind": "bark",
		"share": Voices.Share.WORK,
		"at": Vector2(30, 405),
		"hours": ["day", "dusk", "night"],
		"every": Vector2(12, 40),
		"volume": -2.0,
		"unit": 4.0,
		"stuff": [0.5, 0.4, 0.0],
		"answers": "wolf"
	},
	{
		"name": "a big dog",
		"kind": "bark",
		"share": Voices.Share.WORK,
		"at": Vector2(150, 330),
		"hours": ["day", "dusk", "night"],
		"every": Vector2(15, 50),
		"volume": -2.0,
		"unit": 4.0,
		"stuff": [0.5, 0.75, 0.0],
		"answers": "wolf"
	},
	{
		"name": "wolf",
		"kind": "howl",
		"share": Voices.Share.WORK,
		"at": Vector2(-300, -900),
		"hours": ["dusk", "night"],
		"every": Vector2(14, 34),
		"volume": 8.0,
		"unit": 40.0,
		"stuff": [0.5, 0.6, 0.0]
	},
	{
		"name": "a man by the fire",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(198, 336),
		"hours": HOURS,
		"every": Vector2(1.0, 5.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "man",
		"feeling": "calm"
	},
	{
		"name": "a woman by the fire",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(232, 340),
		"hours": HOURS,
		"every": Vector2(1.0, 5.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "woman",
		"feeling": "calm"
	},
	{
		"name": "an old woman by the fire",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(248, 352),
		"hours": ["dusk", "night", "storm"],
		"every": Vector2(3.0, 9.0),
		"volume": -5.0,
		"unit": 2.0,
		"who": "old woman",
		"feeling": "calm"
	},
	{
		"name": "a woman at the tents",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(70, 383),
		"hours": ["day", "dusk"],
		"every": Vector2(1.5, 6.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "girl",
		"feeling": "calm"
	},
	{
		"name": "an old man at the tents",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(100, 388),
		"hours": ["day", "dusk"],
		"every": Vector2(2.0, 7.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "old man",
		"feeling": "calm"
	},
	{
		"name": "a man above the cliff",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(131, 182),
		"hours": ["day"],
		"every": Vector2(1.0, 4.0),
		"volume": -2.0,
		"unit": 2.0,
		"who": "big man",
		"feeling": "angry"
	},
	{
		"name": "a man above the cliff",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(151, 183),
		"hours": ["day"],
		"every": Vector2(1.0, 4.0),
		"volume": -2.0,
		"unit": 2.0,
		"who": "man",
		"feeling": "angry"
	},
	{
		"name": "a man in the meadow",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(232, 515),
		"hours": ["day", "dusk"],
		"every": Vector2(1.5, 6.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "man",
		"feeling": "calm"
	},
	{
		"name": "a woman in the meadow",
		"kind": "talk",
		"share": Voices.Share.VOICES,
		"at": Vector2(246, 528),
		"hours": ["day", "dusk"],
		"every": Vector2(1.5, 6.0),
		"volume": -4.0,
		"unit": 2.0,
		"who": "woman",
		"feeling": "grieving"
	},
	{
		"name": "children at play",
		"kind": "child",
		"share": Voices.Share.VOICES,
		"circle": [Vector2(212, 376), 20.0],
		"hours": ["dusk"],
		"every": Vector2(1.0, 3.5),
		"volume": -2.0,
		"unit": 2.0,
		"who": "child",
		"feeling": "glad",
		"speed": 0.5
	},
	{
		"name": "children at play",
		"kind": "child",
		"share": Voices.Share.VOICES,
		"circle": [Vector2(250, 540), 26.0],
		"hours": ["day"],
		"every": Vector2(1.0, 3.5),
		"volume": -2.0,
		"unit": 2.0,
		"who": "child",
		"feeling": "glad",
		"speed": 0.4
	},
	{
		"name": "children's feet",
		"kind": "step",
		"share": Voices.Share.WORK,
		"circle": [Vector2(212, 376), 20.0],
		"hours": ["dusk"],
		"every": Vector2(0.28, 0.36),
		"volume": -14.0,
		"unit": 2.0,
		"stuff": [0.3, 0.15, 0.0],
		"speed": 0.5
	},
	{
		"name": "a knapper",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(160, 360),
		"hours": ["busy"],
		"every": Vector2(0.5, 1.2),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.95, 0.25, 0.0]
	},
	{
		"name": "a knapper",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(276, 340),
		"hours": ["busy"],
		"every": Vector2(0.5, 1.2),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.9, 0.45, 0.0]
	},
	{
		"name": "a knapper",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(120, 398),
		"hours": ["busy"],
		"every": Vector2(0.5, 1.2),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.95, 0.25, 0.0]
	},
	{
		"name": "a knapper",
		"kind": "strike",
		"share": Voices.Share.WORK,
		"at": Vector2(228, 506),
		"hours": ["busy"],
		"every": Vector2(0.5, 1.2),
		"volume": -6.0,
		"unit": 2.0,
		"stuff": [0.9, 0.45, 0.0]
	},
	{
		"name": "a scraper",
		"kind": "scrape",
		"share": Voices.Share.WORK,
		"at": Vector2(176, 330),
		"hours": ["busy"],
		"every": Vector2(0.6, 1.4),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.3, 0.5, 0.2]
	},
	{
		"name": "a scraper",
		"kind": "scrape",
		"share": Voices.Share.WORK,
		"at": Vector2(60, 372),
		"hours": ["busy"],
		"every": Vector2(0.6, 1.4),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.25, 0.4, 0.3]
	},
	{
		"name": "a scraper",
		"kind": "scrape",
		"share": Voices.Share.WORK,
		"at": Vector2(262, 530),
		"hours": ["busy"],
		"every": Vector2(0.6, 1.4),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.3, 0.5, 0.2]
	},
	{
		"name": "a wood chopper",
		"kind": "chop",
		"share": Voices.Share.WORK,
		"at": Vector2(20, 300),
		"hours": ["busy"],
		"every": Vector2(1.0, 2.0),
		"volume": -2.0,
		"unit": 4.0,
		"stuff": [0.7, 0.6, 0.1]
	},
	{
		"name": "a walker",
		"kind": "step",
		"share": Voices.Share.WORK,
		"path": [Vector2(40, 470), Vector2(170, 480), Vector2(320, 500)],
		"hours": ["busy"],
		"every": Vector2(0.5, 0.6),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.3, 0.5, 0.0],
		"speed": 4.2
	},
	{
		"name": "a walker",
		"kind": "step",
		"share": Voices.Share.WORK,
		"path": [Vector2(300, 380), Vector2(240, 400), Vector2(150, 405), Vector2(60, 410)],
		"hours": ["busy"],
		"every": Vector2(0.5, 0.6),
		"volume": -10.0,
		"unit": 2.0,
		"stuff": [0.35, 0.65, 0.0],
		"speed": 3.8
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(253, 399),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.3, 0.0],
		"seed": 21
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(245, 403),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.8, 0.0],
		"seed": 22
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(233, 406),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.5, 0.0],
		"seed": 23
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(219, 408),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.9, 0.0],
		"seed": 24
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(205, 408),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.4, 0.0],
		"seed": 25
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(191, 406),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.7, 0.0],
		"seed": 26
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(179, 403),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.6, 0.0],
		"seed": 27
	},
	{
		"name": "a drummer",
		"kind": "drum",
		"share": Voices.Share.MUSIC,
		"at": Vector2(171, 399),
		"hours": ["busy"],
		"volume": -9.0,
		"unit": 3.0,
		"stuff": [0.6, 0.35, 0.0],
		"seed": 28
	},
]


## A picture's pixel as a place in the world, in metres: x to the right, z towards you.
static func world(at: Vector2) -> Vector3:
	return Vector3((at.x - MIDDLE.x) * METRE, 0.0, (at.y - MIDDLE.y) * METRE)


## Whether the cliff stands between two places in the picture (SND-08's muffling, which the game's
## simulation finds by a line through the land).
static func muffled(a: Vector2, b: Vector2) -> bool:
	return (a.y < CLIFF_TOP and b.y > CLIFF_FOOT) or (b.y < CLIFF_TOP and a.y > CLIFF_FOOT)


## Where a maker is at a time: still, along its path there and back, or round its circle.
static func where(source: Dictionary, now: float) -> Vector2:
	if source.has("path"):
		var path: Array = source["path"]
		var lengths: Array[float] = []
		var total := 0.0
		for i in path.size() - 1:
			lengths.append((path[i + 1] as Vector2).distance_to(path[i]))
			total += lengths[-1]
		# there and back, at its speed in pixels a second
		var along := fmod(now * float(source.get("speed", 4.0)), total * 2.0)
		if along > total:
			along = total * 2.0 - along
		for i in lengths.size():
			if along <= lengths[i]:
				return (path[i] as Vector2).lerp(path[i + 1], along / maxf(lengths[i], 0.001))
			along -= lengths[i]
		return path[-1]
	if source.has("circle"):
		var centre: Vector2 = source["circle"][0]
		var radius: float = source["circle"][1]
		var turn := now * float(source.get("speed", 0.5))
		return centre + Vector2(cos(turn), sin(turn) * 0.6) * radius
	return source["at"]


## Whether a maker sounds all round you.
static func around(source: Dictionary) -> bool:
	return source.get("at", Vector2.ZERO) == AROUND


## Whether a maker sounds in an hour.
static func sounds_in(source: Dictionary, hour: String) -> bool:
	return hour in (source["hours"] as Array)
