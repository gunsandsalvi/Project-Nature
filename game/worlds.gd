## The worlds kept on the phone (A3.7): where they are, which one the Crowd page opens, a test's
## world marked as one, and the words for their sizes, counts and the free space. Implements TIM-08,
## PLT-10, PLT-05 and RES-10.
class_name Worlds
extends RefCounted

## Where every world is kept, a folder each.
const ROOT := "user://worlds"
## The world the Crowd page opens until you choose one: the crowd's, which α1.4a kept here.
const FIRST := "crowd"


## The worlds under a root, as the simulation's class keeps them.
static func at(root: String = ROOT) -> KdWorlds:
	var worlds := KdWorlds.new()
	worlds.set_root(ProjectSettings.globalize_path(root))
	return worlds


## The id of the world the Crowd page opens.
static func current_id(worlds: KdWorlds) -> String:
	var id := worlds.current()
	return id if id != "" else FIRST


## A world's name as the pages show it: its own, or its folder's, "Crowd" for α1.4a's.
static func name_of(world: Dictionary) -> String:
	var own: String = world.get("name", "")
	return own if own != "" else String(world["id"]).capitalize()


## A count with its thousands set apart: 10,000.
static func count_words(n: int) -> String:
	var digits := str(absi(n))
	var out := ""
	while digits.length() > 3:
		out = "," + digits.substr(digits.length() - 3) + out
		digits = digits.substr(0, digits.length() - 3)
	return ("-" if n < 0 else "") + digits + out


## A test's world marked as one, with the switches it ran with (RES-10), or nothing for a world of
## play. The game's own build has no switches, so such a world carries on here without them.
static func test_words(world: Dictionary) -> String:
	if not world.get("test", false):
		return ""
	var switches: PackedStringArray = world.get("switches", PackedStringArray())
	if switches.is_empty():
		return "A test's world, run with no switches"
	return (
		"A test's world, run with %s; this build has no switches, so it carries on without them"
		% ", ".join(switches)
	)


## A size as you would say it: "512 bytes", "240 KB", "1.2 MB", "3.4 GB".
static func size_words(bytes: int) -> String:
	if bytes < 1024:
		return "%d bytes" % bytes
	if bytes < 1024 * 1024:
		return "%d KB" % roundi(bytes / 1024.0)
	if bytes < 1024 * 1024 * 1024:
		return "%.1f MB" % (bytes / 1048576.0)
	return "%.1f GB" % (bytes / 1073741824.0)


## A span of real time as you would say it: "12 s", "3 min", "2 h", "4 days".
static func ago_words(seconds: int) -> String:
	if seconds < 120:
		return "%d s" % seconds
	if seconds < 7200:
		return "%d min" % (seconds / 60)
	if seconds < 172800:
		return "%d h" % (seconds / 3600)
	return "%d days" % (seconds / 86400)


## Removes a folder and everything in it, such as the benchmark's worlds once it ends, or a test's;
## nothing if it is not there.
static func remove_tree(path: String) -> void:
	if not DirAccess.dir_exists_absolute(path):
		return
	for sub in DirAccess.get_directories_at(path):
		remove_tree(path.path_join(sub))
	for file in DirAccess.get_files_at(path):
		DirAccess.remove_absolute(path.path_join(file))
	DirAccess.remove_absolute(path)


## The warning when the phone is nearly full, or nothing: the game asks which worlds to delete and
## never deletes one itself (PLT-10).
static func space_warning(free_mb: int, warn_below_mb: int) -> String:
	if free_mb < 0 or free_mb >= warn_below_mb:
		return ""
	return (
		"The phone is nearly full: %s left. Open Worlds to choose which worlds to delete."
		% size_words(free_mb * 1048576)
	)
