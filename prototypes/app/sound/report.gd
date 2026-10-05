## What P14 reports and plays back (IMPLEMENTATION α0.7c): Measure's pass line, its words and its
## line for the chat (SND-01, PLT-04), the voices heard one by one (SND-03), and the reel's tour of
## the camp (SND-12). Pre-production code (research 00).
extends RefCounted

const RUNS := preload("res://look/runs.gd")

## Measure: seconds to settle, then seconds measured ("seconds=" on the command line shortens it),
## and the pass line: the audio thread's mean share of each block's time, and the most any block may
## take; a block over all of its time is a break.
const SETTLE := 5.0
const RUN := 60.0
const MEAN_SHARE := 0.25
const WORST_SHARE := 0.5
## The voices you hear one by one (SND-03's check): who, how they feel, and the caption.
const LINEUP := [
	["child", "calm", "A child"],
	["woman", "calm", "A woman"],
	["man", "calm", "A man"],
	["old man", "calm", "An old man"],
	["man", "angry", "A man, angry"],
	["woman", "grieving", "A woman, grieving"],
]
## The reel (SND-12): seconds from its start, the hour, whether everyone sounds at once, where you
## walk to, the caption, and whether the voices are heard one by one.
const REEL := [
	[0.0, "day", false, Vector2(140, 640), "Day. You walk up the path to the camp.", false],
	[22.0, "day", false, Vector2(184, 424), "The stream, and birds in the wood above.", false],
	[
		34.0,
		"day",
		false,
		Vector2(150, 200),
		"Above the cliff: two men argue; the camp is muffled.",
		false
	],
	[48.0, "day", false, Vector2(205, 392), "Down by the fire: knapping, scraping, talk.", false],
	[62.0, "dusk", false, Vector2(212, 352), "Dusk under the rock shelter, with its echo.", false],
	[90.0, "dusk", false, Vector2(84, 404), "At the tents.", false],
	[100.0, "dusk", false, Vector2(84, 404), "Voices one by one.", true],
	[
		120.0,
		"night",
		false,
		Vector2(205, 380),
		"Night by the fire: a wolf far off, the dogs.",
		false
	],
	[146.0, "storm", false, Vector2(212, 350), "A storm, from under the shelter.", false],
	[161.0, "storm", false, Vector2(140, 560), "Out in the storm.", false],
	[
		171.0,
		"dusk",
		true,
		Vector2(205, 395),
		"Everything at once: 32 sounds, the rest in hums.",
		false
	],
	[193.0, "", false, Vector2(205, 395), "", false],
]


## Whether the audio thread kept within its share, in words.
static func verdict(stats: Dictionary) -> String:
	var mean: float = stats.get("mean", 0.0)
	var worst: float = stats.get("worst", 0.0)
	var over: int = stats.get("over", 0)
	if int(stats.get("blocks", 0)) == 0:
		return "The probe saw no sound mixed."
	if over > 0:
		return "%d blocks took longer than they play: breaks." % over
	if mean <= MEAN_SHARE and worst <= WORST_SHARE:
		return (
			"Passes: the mix took %d%% of the time it plays for, never more than %d%%."
			% [roundi(mean * 100.0), roundi(worst * 100.0)]
		)
	return (
		"No breaks, but over its share: %d%% on average, %d%% at worst."
		% [roundi(mean * 100.0), roundi(worst * 100.0)]
	)


## The line for the chat: the sounds playing, the audio thread's share of each block's time (mean,
## 99 in 100 within, worst), the blocks over their time and late, the blocks' size and rate, the
## output's delay, and the heat.
static func line(
	head: String,
	stats: Dictionary,
	counts: Array[int],
	fullest: Dictionary,
	rate: float,
	latency: float,
	heat: Array
) -> String:
	var most := 0
	var total := 0
	for c in counts:
		most = maxi(most, c)
		total += c
	var runs := PackedStringArray(
		[
			(
				"%d sounds at most (%s), %.1f on average"
				% [most, _shares(fullest), float(total) / maxf(counts.size(), 1.0)]
			),
			(
				"audio thread %.1f%% mean, %d%% for 99 in 100, %.1f%% worst"
				% [
					float(stats.get("mean", 0.0)) * 100.0,
					roundi(float(stats.get("p99", 0.0)) * 100.0),
					float(stats.get("worst", 0.0)) * 100.0
				]
			),
			(
				"%d over, %d late, %d blocks of %d frames at %d Hz, %d thread changes"
				% [
					int(stats.get("over", 0)),
					int(stats.get("late", 0)),
					int(stats.get("blocks", 0)),
					int(stats.get("frames", 0)),
					roundi(rate),
					int(stats.get("threads", 0))
				]
			),
			"delay %d ms" % roundi(latency * 1000.0),
		]
	)
	return RUNS.code(head, runs, heat)


## The shares as the line gives them: place, voices, music, free and work.
static func _shares(fullest: Dictionary) -> String:
	var parts := PackedStringArray()
	for share: String in ["place", "voices", "music", "free", "work"]:
		parts.append("%s %d" % [share, int(fullest.get(share, 0))])
	return " ".join(parts)
