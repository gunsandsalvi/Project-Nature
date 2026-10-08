## The Pilot page (T2.3b.2 and T2.3b.4, T2.3c.2, and the pilot of 7 October 2026): the stand-in
## area, a meadow with a river across it and a camp on its bank, drawn as the game will draw it at
## the game's camera in the late afternoon: the ground's tiles near, middle and far, each in
## versions picked by place and blending where one takes over from the next; the river's bed below
## the water's level, tinted by depth, with the surface's marks stepping in whole texture pixels,
## its glints and its shore line; and the camp's tent and club, put together from the kit's parts
## and set on the ground where the area's tuning says. Pick a place, then a zoom band, and each band
## reads its own tile; or open a piece's sheet at the top, the signed-off picture it came from, with
## what the engine draws of it below, at true size or enlarged to 8 screen pixels a texture pixel.
## Implements PRE-23, PRE-26, PRE-22, PRE-46 and PRE-42.
extends RigPage

const LAND_SHADER := preload("res://look/land.gdshader")
const RIVER_SHADER := preload("res://look/land-river.gdshader")
const WATER_SHADER := preload("res://look/water.gdshader")
const PART_SHADER := preload("res://look/part.gdshader")
## Metres a screen pixel at band 0, the closest zoom (A5.3): a texture pixel 2 screen pixels wide,
## and twice that for each band after; and a texture pixel enlarged to 8 screen pixels.
const BAND_ZERO := 1.0 / 128.0
const ENLARGED := 1.0 / 512.0
## How far north of the river's middle line the meadow's view lies, in centimetres.
const MEADOW_NORTH := 4000
## The places the page shows, in the order of its buttons.
const PLACES := ["Camp", "Meadow", "Shore", "River", "Tent", "Club"]
## What the engine shows beside each piece's sheet: the button's words and the place it is at.
const PIECES := {
	"meadow": {"label": "Meadow", "place": "Meadow"},
	"river": {"label": "River", "place": "River"},
	"river_bed": {"label": "Bed", "place": "Shore"},
	"club": {"label": "Club", "place": "Club"},
	"hide_tent_cone": {"label": "Tent", "place": "Tent"},
}
## The share of the screen's height a sheet takes at the top while it shows.
const SHEET_SHARE := 0.4

## The stand-in area, its water and its camp, and the band and place the view is at.
var area := KdArea.new()
var water := Water.new()
var kit := KdKit.new()
## The maps round the camp's things (A4.4): contact, openness and the sun's angle for the light.
var maps := ViewMaps.new()
var band := 0
var place := "Camp"
## The piece whose sheet shows at the top, or "", and whether the zoom is a texture pixel enlarged.
var sheet := ""
var enlarged := false
## Every line the page says, for the tests.
var shown := PackedStringArray()

var _ladders := {}
## The camp's things: for "Tent" and "Club", their recipe and where each stands in centimetres.
var _camp := {}
var _camp_back := 0
## The tent's forms by size (A6.3): the recipe drawn for each form, "full", "simple" and "small",
## the form drawn now, the tent's width in screen pixels under which the simple and the small forms
## take over, and how much past a switch the size must go to go back to a fuller form.
var _forms := {}
var _form := "full"
var _form_simple := 0.0
var _form_small := 0.0
var _form_margin := 0.0
var _readout: Label
var _origin := Vector2i.ZERO
var _sheet_panel: ScrollContainer
var _sheet_picture: TextureRect
var _sheet_files := {}


func _ready() -> void:
	super._ready()
	_sheet_files = GameData.sheets(GameData.build())
	_add_sheet_panel()
	if problem.is_empty():
		problem = _build_area()
	if problem.is_empty():
		problem = _build_camp()
	look.set_closest(ENLARGED)
	# the Look page's test board is not here
	var spacer := Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(spacer)
	_readout = _label(14, Palette.TEXT)
	_add_controls()
	_show()


func _process(delta: float) -> void:
	super._process(delta)
	water.step(delta)
	var state := look.state()
	var origin := Vector2i(int(state["origin_east"]), int(state["origin_north"]))
	if origin != _origin:
		_origin = origin
		area.set_origin(origin.x, origin.y)
		kit.set_origin(origin.x, origin.y)
		maps.follow(origin.x, origin.y)
	# a pinch changes the tent's size on the screen as a button does
	_fit_form(float(state["metres_per_pixel"]))


## Takes the view to a place, one of PLACES, and puts any sheet away.
func look_at_place(name: String) -> void:
	place = name
	sheet = ""
	_sheet_panel.visible = false
	_show()


## Sets the zoom to a band's, 0 to 6: its texture pixel 2 screen pixels wide at the focus.
func set_band(number: int) -> void:
	band = clampi(number, 0, 6)
	enlarged = false
	_show()


## Sets the zoom to a texture pixel enlarged to 8 screen pixels, or back to the band's.
func set_enlarged(on: bool) -> void:
	enlarged = on
	_show()


## Shows a piece's sheet at the top and the place it is at below, at the zoom now set: one of
## PIECES' ids, whose sheet the build shipped.
func show_sheet(piece: String) -> void:
	if not PIECES.has(piece) or not _sheet_files.has(piece):
		return
	var texture := GameData.sheet_texture(str(_sheet_files[piece]))
	if texture == null:
		problem = "the sheet of %s cannot be read" % piece
		_show()
		return
	_sheet_picture.texture = texture
	_sheet_picture.custom_minimum_size = Vector2(texture.get_size()) / _stretch()
	_sheet_panel.scroll_vertical = 0
	_sheet_panel.visible = true
	sheet = piece
	place = str(PIECES[piece]["place"])
	_show()


## Puts the sheet away.
func hide_sheet() -> void:
	sheet = ""
	_sheet_panel.visible = false
	_show()


## Draws every surface as a test picture shows it: 0 the game's colours, 1 each material's number,
## 3 each tile and version of the ladder as a colour.
func set_picture(mode: int) -> void:
	RenderingServer.global_shader_parameter_set("kd_picture", mode)
	# a test picture is read back as drawn, so its colours are not toned
	var sky := _scene.get_node("Sky") as WorldEnvironment
	sky.environment.tonemap_mode = (
		Environment.TONE_MAPPER_AGX if mode == 0 else Environment.TONE_MAPPER_LINEAR
	)


## Tries a light without changing the tuning, for the cloud's runs that fit the light to the target
## card: the sun's colour (#rrggbb) and strength, and the sky's fill's, as shares of Godot's light.
func try_light(
	sun_colour: String, sun_energy: float, fill_colour: String, fill_energy: float
) -> void:
	var sun := _scene.get_node("Sun") as DirectionalLight3D
	sun.light_color = Color(sun_colour)
	sun.light_energy = sun_energy
	var sky := (_scene.get_node("Sky") as WorldEnvironment).environment
	sky.ambient_light_color = Color(fill_colour)
	sky.ambient_light_energy = fill_energy


## Holds the water's tick at a number, so the marks and glints are the same twice, or lets it run
## again.
func hold_water(number: int) -> void:
	water.hold_at(number)


func release_water() -> void:
	water.release()


## The tile of a surface that serves a band, "near", "middle" or "far", by where its tiles start
## (A5.3).
func tile_at(role: String, number: int) -> String:
	var first: PackedInt32Array = _ladders[role]["first"]
	if number >= first[2]:
		return "far"
	return "middle" if number >= first[1] else "near"


## Where a place is, in centimetres east and north of the area's centre: the camp's middle between
## its tent and the river, the tent and the club where they stand.
func place_at(name: String) -> Vector2i:
	match name:
		"River":
			return Vector2i(0, 0)
		"Shore":
			return Vector2i(0, _north_bank())
		"Camp":
			if _camp.has("Tent"):
				return Vector2i(int(_camp["Tent"]["east"]), _north_bank() + _camp_back / 2)
		"Tent", "Club":
			if _camp.has(name):
				return Vector2i(int(_camp[name]["east"]), int(_camp[name]["north"]))
	return Vector2i(0, MEADOW_NORTH)


## The camp's thing at a place, "Tent" or "Club": {"id", "model", "east", "north", "up"} in
## centimetres, or empty.
func camp_thing(name: String) -> Dictionary:
	return _camp.get(name, {})


func _north_bank() -> int:
	return int(area.banks_at(0).x * 100.0)


func _build_area() -> String:
	var said := area.use_world(world)
	if not said.is_empty():
		return said
	var names := Surfaces.texture_names(loaded)
	var named := area.surfaces()
	for role: String in named:
		var ladder := Surfaces.ladder(world, names, str(named[role]).trim_prefix("art:"))
		if ladder.has("problem"):
			return str(ladder["problem"])
		said = area.set_surface(role, ladder["paths"], ladder["count"], ladder["first"])
		if not said.is_empty():
			return said
		_ladders[role] = ladder
	said = water.apply(world, float(area.info()["level"]))
	if not said.is_empty():
		return said
	return area.build(
		scenario(), LAND_SHADER.get_rid(), RIVER_SHADER.get_rid(), WATER_SHADER.get_rid()
	)


## Puts the camp's tent and club on the meadow by the north bank, where tuning/area says: the tent
## so far north of the bank, its door to the south, the club a distance from it on a bearing.
func _build_camp() -> String:
	var tuning := world.entry("tuning/area", "base:area")
	if tuning.is_empty():
		return "the catalogue has no tuning/area"
	var made := KitScene.load_into(kit, world, PART_SHADER.get_rid())
	var problems: PackedStringArray = made["problems"]
	if not problems.is_empty():
		return problems[0]
	# the tuning's lengths are millimetres
	_camp_back = roundi(float(tuning["camp_back"]) / 10.0)
	var away := float(tuning["club_away"]) / 10.0
	var bearing := deg_to_rad(float(tuning["club_bearing"]))
	var seed_of := int(tuning["camp_seed"])
	var tent_north := _north_bank() + _camp_back
	var said := _stand(
		"Tent", str(tuning["tent"]), 0, tent_north, float(tuning["tent_turn"]), seed_of
	)
	if not said.is_empty():
		return said
	_forms = {
		"full": str(tuning["tent"]),
		"simple": str(tuning["tent_simple"]),
		"small": str(tuning["tent_small"]),
	}
	_form = "full"
	_form_simple = float(tuning["form_simple"])
	_form_small = float(tuning["form_small"])
	_form_margin = float(tuning["form_margin"]) / 1.0e6
	said = _stand(
		"Club",
		str(tuning["club"]),
		roundi(away * sin(bearing)),
		tent_north + roundi(away * cos(bearing)),
		float(tuning["club_turn"]),
		seed_of
	)
	if not said.is_empty():
		return said
	# the ground's patches: its masses of growth and the clearing worn round the tent (A4.6)
	said = maps.build_patches(
		world, float(_camp["Tent"]["east"]) / 100.0, float(_camp["Tent"]["north"]) / 100.0
	)
	if not said.is_empty():
		return said
	# the ground's openness, contact and shadows round the camp's things
	said = maps.build(kit, [_camp["Tent"]["id"], _camp["Club"]["id"]], world)
	var state := look.state()
	maps.follow(int(state["origin_east"]), int(state["origin_north"]))
	return said


## Sets a recipe's thing on the ground at a place, in centimetres, and keeps where it stands.
func _stand(
	name: String, model: String, east: int, north: int, turn: float, seed_of: int
) -> String:
	var up := roundi(area.ground_height(east, north) * 100.0)
	var made := kit.place(scenario(), model, seed_of, east, north, up, turn, {})
	if str(made["problem"]) != "":
		return str(made["problem"])
	# the thing's widest side, in metres, from the bounds of what the assembler put together
	var low: Vector3 = made["lowest"]
	var high: Vector3 = made["highest"]
	_camp[name] = {
		"id": int(made["id"]),
		"model": model,
		"east": east,
		"north": north,
		"up": up,
		"turn": turn,
		"seed": seed_of,
		"width": maxf(high.x - low.x, high.z - low.z),
	}
	return ""


## The form the tent's width on the screen calls for, from the one drawn now (A6.3): a smaller
## form from the switch down, and a fuller one only the margin past it, so a form never flickers
## at a switch.
func form_for(pixels: float, now: String) -> String:
	var back := 1.0 + _form_margin
	var form := now
	if form == "full" and pixels < _form_simple:
		form = "simple"
	elif form == "simple" and pixels >= _form_simple * back:
		form = "full"
	if form == "simple" and pixels < _form_small:
		form = "small"
	elif form == "small" and pixels >= _form_small * back:
		form = "simple"
		if pixels >= _form_simple * back:
			form = "full"
	return form


## The form of the tent drawn now: "full", "simple" or "small".
func tent_form() -> String:
	return _form


## Draws the tent in the form its size on the screen calls for, at a zoom in metres a screen pixel.
func _fit_form(metres_per_pixel: float) -> void:
	if not _camp.has("Tent") or _forms.is_empty() or metres_per_pixel <= 0.0:
		return
	var pixels := float(_camp["Tent"]["width"]) / metres_per_pixel
	var form := form_for(pixels, _form)
	if form == _form:
		return
	var tent: Dictionary = _camp["Tent"]
	kit.remove(int(tent["id"]))
	var made := kit.place(
		scenario(),
		str(_forms[form]),
		int(tent["seed"]),
		int(tent["east"]),
		int(tent["north"]),
		int(tent["up"]),
		float(tent["turn"]),
		{}
	)
	if str(made["problem"]) != "":
		push_error(str(made["problem"]))
		return
	tent["id"] = int(made["id"])
	_form = form


## The view: the place at the zoom, and, while a sheet covers the top of the screen, put in the
## middle of what it leaves free.
func _show() -> void:
	var at := place_at(place)
	var metres_per_pixel := _metres_per_pixel()
	if sheet != "":
		at = _lowered(at, metres_per_pixel)
	view_at(at.x, at.y, 0.0, metres_per_pixel)
	_fit_form(metres_per_pixel)
	if _readout == null:
		return
	var words := _words()
	shown.append(words)
	_readout.text = words
	if not problem.is_empty():
		_readout.text += "\n" + problem
		_readout.add_theme_color_override("font_color", Palette.FAIL)


func _metres_per_pixel() -> float:
	return ENLARGED if enlarged else BAND_ZERO * pow(2.0, float(band))


## What the readout says: the place and zoom, the tile the meadow reads, and a piece's sheet.
func _words() -> String:
	var words := "%s " % place
	if enlarged:
		words += "enlarged: a texture pixel 8 screen pixels wide"
	else:
		words += "at band %d: %d texture pixels a metre" % [band, 64 >> band]
	if _ladders.has("ground"):
		var ladder: Dictionary = _ladders["ground"]
		var tile := tile_at("ground", band)
		var versions: int = (ladder["count"] as PackedInt32Array)[Surfaces.TILES.find(tile)]
		words += (
			"\nthe meadow reads its %s tile, %d version%s picked by place"
			% [tile, versions, "" if versions == 1 else "s"]
		)
	if sheet != "":
		words += "\nthe sheet of %s above; below, what the engine draws" % PIECES[sheet]["label"]
		var approved := _approved(sheet)
		if approved != "":
			words += ": " + approved
	return words


## What a piece's record says of your yes or no.
func _approved(piece: String) -> String:
	var record := {}
	match piece:
		"meadow", "river_bed", "river":
			var role := {"meadow": "ground", "river_bed": "bed", "river": "marks"}[piece] as String
			record = world.entry("textures", str(area.surfaces().get(role, "")))
		"club", "hide_tent_cone":
			var thing := "Club" if piece == "club" else "Tent"
			if _camp.has(thing):
				record = kit.model_info(str(_camp[thing]["model"]))
	return str(record.get("approved", ""))


## Where to look so that a place appears in the middle of what the sheet and the controls leave
## free of the screen, as centimetres east and north: the view put on the place itself, the ground
## under that middle read off the camera, and the focus as far past the place the other way.
func _lowered(at: Vector2i, metres_per_pixel: float) -> Vector2i:
	view_at(at.x, at.y, 0.0, metres_per_pixel)
	LookScene.place_camera(_camera, look)
	# the camera's screen is the interface's: the visible rectangle in canvas units
	var visible := get_viewport().get_visible_rect().size
	var middle := Vector2(visible.x * 0.5, visible.y * _free_middle())
	var from := _camera.project_ray_origin(middle)
	var along := _camera.project_ray_normal(middle)
	if absf(along.y) < 1e-6:
		return at
	var state := look.state()
	var ground := area.ground_height(at.x, at.y)
	var hit := from + along * ((ground - from.y) / along.y)
	# the world is about the rig's origin: x east, z south, in metres
	var east := float(state["origin_east"]) + hit.x * 100.0
	var north := float(state["origin_north"]) - hit.z * 100.0
	return Vector2i(at.x * 2 - roundi(east), at.y * 2 - roundi(north))


## The height of the middle of what a sheet and the controls leave free, as a share of the
## screen's.
func _free_middle() -> float:
	var height := get_viewport().get_visible_rect().size.y
	var top := global_position.y + SHEET_SHARE * height
	var bottom := _readout.global_position.y if _readout != null else height
	return clampf((top + bottom) / 2.0 / height, 0.2, 0.8)


func _add_sheet_panel() -> void:
	_sheet_panel = ScrollContainer.new()
	_sheet_panel.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_sheet_panel.size_flags_vertical = Control.SIZE_SHRINK_BEGIN
	_sheet_panel.custom_minimum_size = Vector2(
		0.0, SHEET_SHARE * get_viewport().get_visible_rect().size.y
	)
	var ground := StyleBoxFlat.new()
	ground.bg_color = Palette.GROUND
	_sheet_panel.add_theme_stylebox_override("panel", ground)
	_sheet_panel.visible = false
	add_child(_sheet_panel)
	# a sheet is read at its own pixels, each one of the screen's
	_sheet_picture = TextureRect.new()
	_sheet_picture.size_flags_horizontal = Control.SIZE_SHRINK_BEGIN
	_sheet_picture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_sheet_picture.stretch_mode = TextureRect.STRETCH_SCALE
	_sheet_picture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_sheet_panel.add_child(_sheet_picture)


func _add_controls() -> void:
	var places := HFlowContainer.new()
	add_child(places)
	for name: String in PLACES:
		_button(places, name, look_at_place.bind(name))
	var bands := HBoxContainer.new()
	add_child(bands)
	for number in 7:
		_button(bands, "Band %d" % number, set_band.bind(number))
	var sheets := HFlowContainer.new()
	add_child(sheets)
	for piece: String in PIECES:
		if _sheet_files.has(piece):
			_button(sheets, "Sheet: %s" % PIECES[piece]["label"], show_sheet.bind(piece))
	_button(sheets, "No sheet", hide_sheet)
	var zooms := HBoxContainer.new()
	add_child(zooms)
	_button(zooms, "True size", set_enlarged.bind(false))
	_button(zooms, "Enlarged", set_enlarged.bind(true))
