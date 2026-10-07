## The Lab page (A5.4, α2.3a): every material the art lane has made, as the phone shows it, for your
## yes or no. A list of the materials, those offered for your yes or no first, then those the art
## lane holds back as still below the artwork's level, with why, then those you have decided; tap
## one for its sheet: each of its tiles and versions, every level at the bands it serves at true
## size, each texture pixel 2 × 2 screen pixels as at its own zoom (A5.3), the first level's corner
## enlarged, and its record's words. The textures are the build's own, each made into an image with
## its own levels and uploaded as the world does, and the page says how long that took and the
## memory they hold. Implements PRE-20 and PRE-22.
extends VBoxContainer

const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const HEAD := Palette.HEAD
const WARN := Palette.WARN
const FAIL := Palette.FAIL
## Screen pixels a texture pixel covers at its own zoom (A5.3), and enlarged.
const TRUE_SIZE := 2.0
const ENLARGED := 8.0
## The enlarged corner of the first level, in texture pixels.
const CORNER := 40
## A big surface's tiles serve these bands (A5.3); a material with one tile serves bands 0 to 6.
const BANDS := {"near": [0, 1], "middle": [2, 3], "far": [4, 5, 6], "one": [0, 1, 2, 3, 4, 5, 6]}
## A record's approval while it is offered for your yes or no, and while it is held back.
const OFFERED := "waiting"
const HELD := "not offered yet"

## What loading the catalogue found, as KdWorld.load_catalogue gives it.
var loaded: Dictionary = {}
## The textures by entry name ("art:meadow/middle"), each {"record", "levels": Array of
## ImageTexture}; the materials' names in order, each with its textures' names in order.
var textures: Dictionary = {}
var materials: Dictionary = {}
## How long making and uploading every texture took, in milliseconds, and their memory in bytes.
var load_ms := 0.0
var load_bytes := 0
## The material whose sheet is shown, or "" for the list; and every line the page shows, for the
## tests.
var showing := ""
var shown := PackedStringArray()

var _world := KdWorld.new()
var _look := KdLook.new()
var _list: VBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_list.add_theme_constant_override("separation", 6)
	scroll.add_child(_list)
	loaded = GameData.load_into(_world)
	_load_textures()
	# the cloud's picture of a material's sheet (tools/picture.sh game ... -- Lab <material>)
	var args := OS.get_cmdline_user_args()
	var at := args.find("Lab")
	var asked := args[at + 1] if at >= 0 and at + 1 < args.size() else ""
	if materials.has(asked):
		open(asked)
	else:
		list()


## Shows the list of materials.
func list() -> void:
	showing = ""
	_clear()
	_line(
		(
			"%d materials, %d textures, made and uploaded in %.0f ms, %.1f MB with their levels"
			% [materials.size(), textures.size(), load_ms, load_bytes / 1.0e6]
		),
		15,
		QUIET
	)
	for problem: String in loaded["problems"]:
		_line(problem, 14, FAIL)
	var groups := {
		"offered": "For your yes or no: tap one to see it at true size and enlarged.",
		"held": "Not offered yet: the art lane is still bringing these up to the artwork.",
		"decided": "Decided by you.",
	}
	for group: String in groups:
		var members: Array = materials.keys().filter(
			func(m: String) -> bool: return state_of(m) == group
		)
		if members.is_empty():
			continue
		_gap()
		_line(groups[group], 17, HEAD)
		for material: String in members:
			_button(material)


## Where a material stands: "offered" if any of its textures is offered for your yes or no; else
## "held" if any is held back; else "decided".
func state_of(material: String) -> String:
	var held := false
	for name: String in materials[material]:
		var approved := str(textures[name]["record"].get("approved", ""))
		if approved == OFFERED:
			return "offered"
		held = held or approved.begins_with(HELD)
	return "held" if held else "decided"


func _button(material: String) -> void:
	var first: Dictionary = textures[materials[material][0]]["record"]
	var button := Button.new()
	button.text = "%s: %s" % [material, first.get("about", "")]
	# a material offered with some of its tiles held back says which
	var held: Array = []
	for name: String in materials[material]:
		if str(textures[name]["record"].get("approved", "")).begins_with(HELD):
			held.append(_tile_of(material, name))
	if not held.is_empty() and state_of(material) == "offered":
		button.text += " (%s not offered yet)" % ", ".join(held)
	button.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	button.alignment = HORIZONTAL_ALIGNMENT_LEFT
	button.custom_minimum_size = Vector2(0, 48)
	button.pressed.connect(open.bind(material))
	_list.add_child(button)
	shown.append(button.text)


func _tile_of(material: String, name: String) -> String:
	var rest := name.trim_prefix("art:" + material).trim_prefix("/")
	return rest if rest != "" else "near tile"


## Shows a material's sheet: each texture of it, its levels at true size and enlarged, its words.
func open(material: String) -> void:
	showing = material
	_clear()
	var back := Button.new()
	back.text = "All materials"
	back.custom_minimum_size = Vector2(0, 48)
	back.pressed.connect(list)
	_list.add_child(back)
	_line(material, 22, HEAD)
	var names: Array = materials[material]
	var tiled := names.has("art:%s/middle" % material)
	for name: String in names:
		_texture(material, name, tiled)


## Screen pixels in one of the interface's units: the interface is drawn at 540 wide and stretched
## to the screen.
func screen_pixels() -> float:
	var units := get_viewport().get_visible_rect().size.x if is_inside_tree() else 0.0
	var pixels := float(DisplayServer.window_get_size().x)
	return pixels / units if units > 0.0 and pixels > 0.0 else 1.0


## The bands a texture's levels serve, from its name: a big surface's near, middle and far tiles and
## their versions, or a material with one tile.
func bands_of(material: String, name: String, tiled: bool) -> Array:
	var rest := name.trim_prefix("art:" + material).trim_prefix("/")
	var tile := rest.get_slice("/", 0)
	if tile in ["middle", "far"]:
		return BANDS[tile]
	return BANDS["near"] if tiled else BANDS["one"]


func _texture(material: String, name: String, tiled: bool) -> void:
	var record: Dictionary = textures[name]["record"]
	var levels: Array = textures[name]["levels"]
	var bands := bands_of(material, name, tiled)
	_gap()
	_line(
		(
			"%s: bands %s, %d texture pixels a metre at the first"
			% [_tile_of(material, name), _listed(bands), record.get("texels_a_metre", 0)]
		),
		17,
		HEAD
	)
	var approved := str(record.get("approved", ""))
	if approved.begins_with(HELD):
		_line(approved.substr(0, 1).to_upper() + approved.substr(1), 15, WARN)
	var unit := 1.0 / screen_pixels()
	var row := HFlowContainer.new()
	row.add_theme_constant_override("h_separation", 8)
	row.add_theme_constant_override("v_separation", 8)
	_list.add_child(row)
	var first := int(record.get("first_band", 0))
	for band: int in bands:
		var level := band - first
		if level < 0 or level >= levels.size():
			continue
		row.add_child(_picture(levels[level], TRUE_SIZE * unit, Rect2()))
	if not levels.is_empty():
		var corner := Rect2(0, 0, CORNER, CORNER)
		_list.add_child(_picture(levels[0], ENLARGED * unit, corner))
	for key: String in ["route", "made", "regrid_loss", "truth", "approved"]:
		_line("%s: %s" % [key, record.get(key, "")], 13, QUIET)


## A level as a picture, each texture pixel `units` of the interface across, nearest-pixel; or its
## corner, if a region is given.
func _picture(texture: ImageTexture, units: float, region: Rect2) -> TextureRect:
	var shown_texture: Texture2D = texture
	var size := Vector2(texture.get_width(), texture.get_height())
	if region.has_area():
		var atlas := AtlasTexture.new()
		atlas.atlas = texture
		atlas.region = region
		shown_texture = atlas
		size = region.size
	var rect := TextureRect.new()
	# its own size, never stretched by the column it is in
	rect.size_flags_horizontal = Control.SIZE_SHRINK_BEGIN
	rect.texture = shown_texture
	rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	rect.stretch_mode = TextureRect.STRETCH_SCALE
	rect.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	rect.custom_minimum_size = size * units
	return rect


## Every texture the catalogue's records name, made and uploaded from its .kdtex, timed.
func _load_textures() -> void:
	var names: Array[String] = []
	for kind: Dictionary in loaded.get("kinds", []):
		if kind["folder"] == "textures":
			for entry: Dictionary in kind["entries"]:
				names.append(str(entry["name"]))
	names.sort_custom(func(a: String, b: String) -> bool: return _order(a) < _order(b))
	var started := Time.get_ticks_usec()
	for name: String in names:
		var read := _look.texture_levels(GameData.texture_path(name))
		var levels: Array[ImageTexture] = []
		for image: Image in read["levels"]:
			load_bytes += image.get_data_size()
			levels.append(ImageTexture.create_from_image(image))
		if str(read["problem"]) != "":
			(loaded["problems"] as PackedStringArray).append(str(read["problem"]))
		textures[name] = {"record": _world.entry("textures", name), "levels": levels}
		var material := name.trim_prefix("art:").get_slice("/", 0)
		if not materials.has(material):
			materials[material] = []
		materials[material].append(name)
	load_ms = (Time.get_ticks_usec() - started) / 1000.0


## Where a texture comes in its material's sheet: the near tile, the middle and the far, each with
## its versions after it, then any other, such as a wrap atlas.
static func _order(name: String) -> String:
	var parts := name.trim_prefix("art:").split("/")
	var tile := parts[1] if parts.size() > 1 else "near"
	var rank: String = (
		{"near": "0", "v2": "0", "v3": "0", "v4": "0", "middle": "1", "far": "2"}.get(tile, "3")
	)
	return "%s %s %s" % [parts[0], rank, name]


func _listed(bands: Array) -> String:
	if bands.size() == 1:
		return str(bands[0])
	return "%d to %d" % [bands[0], bands[-1]] if bands.size() > 2 else "%d and %d" % bands


func _clear() -> void:
	for child in _list.get_children():
		_list.remove_child(child)
		child.free()
	shown.clear()


func _line(text: String, font_size: int, colour: Color) -> void:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	_list.add_child(label)
	shown.append(text)


func _gap() -> void:
	var gap := Control.new()
	gap.custom_minimum_size = Vector2(0, 10)
	_list.add_child(gap)
