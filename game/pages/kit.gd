## The Kit page (A6.5): the model sheet. Every thing the recipes make and every part of the kit's
## families, put together by the assembler from the build's own parts and drawn as the engine draws
## them: the late afternoon's light, the parts' textures read through the one sampling function
## with their coordinates in metres, each thing at true size on the ground. Pick a thing or a part,
## then a zoom: up close, the close camp, the camp, or a texture pixel enlarged to 8 screen pixels.
## Implements PRE-46, PRE-42 and PRE-22.
extends RigPage

const PART_SHADER := preload("res://look/part.gdshader")
const FIELD_SHADER := preload("res://look/field.gdshader")
## Centimetres between the centres of the things and parts on the ground.
const SPACING := 600
## The zooms the buttons give, in metres a screen pixel: band 0's stop, the close camp's, the
## camp's, and a texture pixel enlarged to 8 screen pixels.
const ZOOMS := {
	"Up close": 1.0 / 128.0, "Close camp": 1.0 / 32.0, "Camp": 1.0 / 8.0, "Enlarged": 1.0 / 512.0
}

## The kit: its families, recipes and the textures its roles wear.
var kit := KdKit.new()
## The recipes' names, and the one whose thing, or the part, now on show; each one's place in
## centimetres east; every line the page shows, for the tests.
var models := PackedStringArray()
var showing := ""
var places := {}
var shown := PackedStringArray()
## What the page holds: "things" or "parts".
var mode := "things"

var _ids: Array[int] = []
var _families := {}
var _readout: Label
var _picks: HFlowContainer
var _zoom := 1.0 / 128.0
var _origin := Vector2i.ZERO


func _ready() -> void:
	super._ready()
	_load_kit()
	var ground := LookScene.ground(look, _scene, FIELD_SHADER)
	if problem.is_empty():
		problem = ground
	# the test board belongs to the Look page
	look.set_part("pattern", false)
	look.set_closest(1.0 / 512.0)
	var spacer := Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(spacer)
	_readout = _label(14, Palette.TEXT)
	_add_controls()
	show_things()


func _process(delta: float) -> void:
	super._process(delta)
	var state := look.state()
	var origin := Vector2i(int(state["origin_east"]), int(state["origin_north"]))
	if origin != _origin:
		_origin = origin
		kit.set_origin(origin.x, origin.y)


## Puts every recipe's thing on the ground in a row, each a seed's thing at true size, and shows
## the first.
func show_things() -> void:
	mode = "things"
	_clear()
	var east := 0
	for model: String in models:
		var made := kit.place(scenario(), model, 1, east, 0, 0, 0.0, {})
		_keep(model, east, made)
		east += SPACING
	_pick_buttons(models, show_thing)
	if not models.is_empty():
		show_thing(models[0])


## Puts every part of every family on the ground in a row, wearing the first texture its role is
## given by a recipe of its family, and shows the first.
func show_parts() -> void:
	mode = "parts"
	_clear()
	var east := 0
	var names := PackedStringArray()
	for family: String in _families:
		var wear := _wear_of(family)
		for part: String in kit.parts(family):
			var made := kit.place_part(scenario(), family, part, east, 0, 0, 0.0, wear)
			var name := "%s/%s" % [family, part]
			_keep(name, east, made)
			names.append(name)
			east += SPACING
	_pick_buttons(names, show_part)
	if not names.is_empty():
		show_part(names[0])


## Shows a thing: the view on it, and its facts.
func show_thing(model: String) -> void:
	showing = model
	view_at(int(places[model]), 0, 0.0, _zoom)
	var info := kit.model_info(model)
	var roles: Array[String] = []
	for role: String in info.get("roles", {}):
		var textures: PackedStringArray = info["roles"][role]
		roles.append("%s: %s" % [role, ", ".join(textures)])
	_say(
		(
			"%s: %s\n%d parts, %d copies, %d triangles; %s\nroles: %s"
			% [
				model,
				info.get("about", ""),
				(info.get("parts", PackedStringArray()) as PackedStringArray).size(),
				info.get("copies", 0),
				info.get("triangles", 0),
				info.get("approved", ""),
				"; ".join(roles),
			]
		)
	)


## Shows a part, "family/part": the view on it, and its facts.
func show_part(name: String) -> void:
	showing = name
	view_at(int(places[name]), 0, 0.0, _zoom)
	var info := kit.part_info(name.get_slice("/", 0), name.get_slice("/", 1))
	var roles: Array[String] = []
	for role: String in info.get("roles", {}):
		roles.append("%s %d" % [role, info["roles"][role]])
	var joints: PackedStringArray = info.get("joints", PackedStringArray())
	_say(
		(
			"%s\n%d triangles (%s); joints: %s; texture pixels stretch at most %.2f to 1"
			% [
				name,
				info.get("triangles", 0),
				", ".join(roles),
				", ".join(joints) if not joints.is_empty() else "none",
				info.get("stretch", 1.0),
			]
		)
	)


## Sets the zoom, in metres a screen pixel: 1/128 is band 0's stop; 1/512 shows a texture pixel as 8
## screen pixels.
func set_zoom(metres_per_pixel: float) -> void:
	_zoom = metres_per_pixel
	if showing != "":
		view_at(int(places[showing]), 0, 0.0, _zoom)


func _load_kit() -> void:
	var build := GameData.build()
	for family: String in GameData.models(build):
		var failed := kit.load_family(family, GameData.models(build)[family])
		if not failed.is_empty():
			problem = failed
		else:
			_families[family] = true
	kit.use_world(world)
	kit.set_shader(PART_SHADER.get_rid())
	for name: String in kit.texture_names():
		var record := world.entry("textures", name)
		if record.is_empty():
			problem = "the recipes name the texture %s, which the catalogue lacks" % name
			continue
		var failed := kit.set_texture(
			name, GameData.texture_path(name), GameData.tile_metres(record)
		)
		if not failed.is_empty():
			problem = failed
	models = kit.models()


## What each role of a family's parts wears when a part stands alone: the first texture any recipe
## of the family gives it.
func _wear_of(family: String) -> Dictionary:
	var wear := {}
	for model: String in models:
		var info := kit.model_info(model)
		if info.get("family", "") != family:
			continue
		for role: String in info.get("roles", {}):
			if not wear.has(role):
				wear[role] = (info["roles"][role] as PackedStringArray)[0]
	return wear


func _keep(name: String, east: int, made: Dictionary) -> void:
	places[name] = east
	if str(made["problem"]) != "":
		problem = str(made["problem"])
	else:
		_ids.append(int(made["id"]))


func _clear() -> void:
	for id in _ids:
		kit.remove(id)
	_ids.clear()
	places.clear()
	showing = ""


func _add_controls() -> void:
	_picks = HFlowContainer.new()
	add_child(_picks)
	var row := HBoxContainer.new()
	add_child(row)
	for label: String in ["Things", "Parts"]:
		_button(row, label, show_things if label == "Things" else show_parts)
	var zooms := HBoxContainer.new()
	add_child(zooms)
	for label: String in ZOOMS:
		_button(zooms, label, set_zoom.bind(ZOOMS[label]))


func _pick_buttons(names: PackedStringArray, show_one: Callable) -> void:
	for child in _picks.get_children():
		_picks.remove_child(child)
		child.free()
	for name: String in names:
		_button(_picks, name.trim_prefix("art:"), show_one.bind(name))


func _say(words: String) -> void:
	shown.append(words)
	_readout.text = words
	if not problem.is_empty():
		_readout.text += "\n" + problem
		_readout.add_theme_color_override("font_color", Palette.FAIL)
