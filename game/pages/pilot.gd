## The Pilot page (T2.3b.2 and T2.3b.4, and the pilot of 7 October 2026): the stand-in area, a
## meadow with a river across it, drawn as the game will draw it at the game's camera in the late
## afternoon: the ground's tiles near, middle and far, each in versions picked by place and
## blending where one takes over from the next; the river's bed below the water's level, tinted by
## depth, with the surface's marks stepping in whole texture pixels, its glints and its shore line.
## Pick a place, then a zoom band, and each band reads its own tile. Implements PRE-23, PRE-26 and
## PRE-22.
extends RigPage

const LAND_SHADER := preload("res://look/land.gdshader")
const RIVER_SHADER := preload("res://look/land-river.gdshader")
const WATER_SHADER := preload("res://look/water.gdshader")
## Metres a screen pixel at band 0, the closest zoom (A5.3): a texture pixel 2 screen pixels wide,
## and twice that for each band after.
const BAND_ZERO := 1.0 / 128.0
## How far north of the river's middle line the meadow's view lies, in centimetres.
const MEADOW_NORTH := 4000

## The stand-in area, its water, and the band and place the view is at.
var area := KdArea.new()
var water := Water.new()
var band := 0
var place := "Meadow"
## Every line the page says, for the tests.
var shown := PackedStringArray()

var _ladders := {}
var _readout: Label
var _origin := Vector2i.ZERO


func _ready() -> void:
	super._ready()
	if problem.is_empty():
		problem = _build_area()
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


## Takes the view to a place: "Meadow", "Shore" or "River".
func look_at_place(name: String) -> void:
	place = name
	_show()


## Sets the zoom to a band's, 0 to 6: its texture pixel 2 screen pixels wide at the focus.
func set_band(number: int) -> void:
	band = clampi(number, 0, 6)
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


## Where a place is, in centimetres east and north of the area's centre.
func place_at(name: String) -> Vector2i:
	match name:
		"River":
			return Vector2i(0, 0)
		"Shore":
			return Vector2i(0, int(area.banks_at(0).x * 100.0))
		_:
			return Vector2i(0, MEADOW_NORTH)


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


func _show() -> void:
	var at := place_at(place)
	view_at(at.x, at.y, 0.0, BAND_ZERO * pow(2.0, float(band)))
	if _readout == null:
		return
	var words := "%s at band %d: %d texture pixels a metre" % [place, band, 64 >> band]
	if _ladders.has("ground"):
		var ladder: Dictionary = _ladders["ground"]
		var tile := tile_at("ground", band)
		var versions: int = (ladder["count"] as PackedInt32Array)[Surfaces.TILES.find(tile)]
		words += (
			"\nthe meadow reads its %s tile, %d version%s picked by place"
			% [tile, versions, "" if versions == 1 else "s"]
		)
	shown.append(words)
	_readout.text = words
	if not problem.is_empty():
		_readout.text += "\n" + problem
		_readout.add_theme_color_override("font_color", Palette.FAIL)


func _add_controls() -> void:
	var places := HBoxContainer.new()
	add_child(places)
	for name: String in ["Meadow", "Shore", "River"]:
		_button(places, name, look_at_place.bind(name))
	var bands := HBoxContainer.new()
	add_child(bands)
	for number in 7:
		_button(bands, "Band %d" % number, set_band.bind(number))
