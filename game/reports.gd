## The scenes' reports as the cloud leaves them (A17, RES-06): where the app keeps the last ones,
## reading them, and the words the Reports page shows: each verdict with its rule, each measure's
## range over the runs, such as "in 18 of 20 worlds" (RES-13), each run, and the world a report
## brought with it. Implements RES-06 and RES-13.
class_name Reports
extends RefCounted

## Where the build puts the scenes' last reports, each <scene>.json, with one world each as
## <scene>-<run>.kindling (tools/gamedata.py).
const FOLDER := "res://data/reports"


## The reports in a folder, by scene name; one that cannot be read is left out.
static func read_all(folder: String = FOLDER) -> Array[Dictionary]:
	var out: Array[Dictionary] = []
	var names := Array(DirAccess.get_files_at(folder))
	names.sort()
	for file: String in names:
		if file.get_extension() == "json":
			var report := read(folder.path_join(file))
			if not report.is_empty():
				out.append(report)
	return out


## A report, or nothing when its file is missing or not a report.
static func read(path: String) -> Dictionary:
	var parsed: Variant = JSON.parse_string(FileAccess.get_file_as_string(path))
	if parsed is Dictionary and parsed.has("scene") and parsed.has("each"):
		return parsed
	return {}


## The verdict: "Pass: 20 of 20 runs met the rule, 16 needed", with a rerun and a provisional
## verdict named (RES-13).
static func verdict_words(r: Dictionary) -> String:
	var words := (
		"%s: %d of %d runs met the rule, %d needed"
		% ["Pass" if r["passed"] else "Fail", r["passes"], r["judged"], r["needed"]]
	)
	if r["reran"]:
		words += (
			"; it failed on its first %d, so it ran %d more on fresh seeds" % [r["runs"], r["runs"]]
		)
	if r["provisional"]:
		words += "; provisional, on fewer than 20 runs"
	return words


## How the scene was set: "20 worlds of 4 camps, seeds 1 to 20, each 20 game days".
static func setup_words(r: Dictionary) -> String:
	var runs: int = r["each"].size()
	var seed: int = r["seed"]
	var setup := (
		"%d worlds of %d camps, seeds %d to %d, each %s"
		% [runs, r["camps"], seed, seed + runs - 1, game_span_words(r["until"])]
	)
	var switches: Array = r["switches"]
	if not switches.is_empty():
		setup += (
			"; with %s %s"
			% ["one switch each:" if r["one_each"] else "the switches", ", ".join(switches)]
		)
	return setup


## The version it ran under, and the real time it took against its budget.
static func time_words(r: Dictionary) -> String:
	var words := (
		"It ran under %s and took %s of its budget of %s"
		% [r["build"], real_span_words(r["seconds"]), real_span_words(r["budget"])]
	)
	if r["over_budget"]:
		words += ", which ran out before every run was done"
	return words


## Its oddities, in all: "No oddities" or "3 oddities, in red below".
static func oddity_words(r: Dictionary) -> String:
	var n: int = r["oddities"]
	if n == 0:
		return "No oddities"
	return "%d %s, marked below" % [n, "oddity" if n == 1 else "oddities"]


## A measure's range over the runs: "16 to 18, 17 in the middle; inside its expected 5 to 30 in
## 20 of 20 worlds".
static func range_words(m: Dictionary) -> String:
	var words := (
		"%s to %s, %s in the middle"
		% [
			Worlds.count_words(m["lowest"]),
			Worlds.count_words(m["highest"]),
			Worlds.count_words(m["median"])
		]
	)
	if m["expected"] != null:
		var expected: Array = m["expected"]
		words += (
			"; inside its expected %s to %s in %d of %d worlds"
			% [
				Worlds.count_words(expected[0]),
				Worlds.count_words(expected[1]),
				m["inside"],
				m["runs"]
			]
		)
	return words


## Each run's value of a measure, in the runs' order; a run that gave none is left out.
static func values_of(r: Dictionary, measure: String) -> Array[int]:
	var out: Array[int] = []
	for run: Dictionary in r["each"]:
		if run["measures"].has(measure):
			out.append(int(run["measures"][measure]))
	return out


## A run, in a line: "Run 3, seed 3: greetings 1457, greetings_per_camp_day 18 ...".
static func run_words(run: Dictionary) -> String:
	var parts := PackedStringArray()
	for measure: String in run["measures"]:
		parts.append("%s %s" % [measure, Worlds.count_words(run["measures"][measure])])
	var words := "Run %d, seed %d: %s" % [int(run["index"]) + 1, run["seed"], ", ".join(parts)]
	if parts.is_empty():
		words = "Run %d, seed %d: no measures" % [int(run["index"]) + 1, run["seed"]]
	var switches: Array = run["switches"]
	if not switches.is_empty():
		words += "; switch %s" % ", ".join(switches)
	return words


## The world a report brought with it, as a .kindling file, and which run's it is: {"path",
## "index"}, or nothing.
static func world_of(folder: String, r: Dictionary) -> Dictionary:
	for run: Dictionary in r["each"]:
		var path := folder.path_join("%s-%d.kindling" % [r["scene"], int(run["index"]) + 1])
		if FileAccess.file_exists(path):
			return {"path": path, "index": int(run["index"])}
	return {}


## Game seconds as a span: "20 game days", "3 game hours".
static func game_span_words(seconds: int) -> String:
	if seconds % 86400 == 0:
		return "%d game day%s" % [seconds / 86400, "" if seconds == 86400 else "s"]
	return "%d game hour%s" % [seconds / 3600, "" if seconds == 3600 else "s"]


## Real seconds as a span: "under a second", "12 s", "6 min", "2 h".
static func real_span_words(seconds: int) -> String:
	return "under a second" if seconds < 1 else Worlds.ago_words(seconds)
