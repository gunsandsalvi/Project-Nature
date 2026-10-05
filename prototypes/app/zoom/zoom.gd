## P8 The zoom (IMPLEMENTATION α0.5b): one pinch from the globe to a person over P7's world, at the
## art book's pixel size, a quarter of the screen, drawn through Godot's RenderingServer: the world
## map from the world's cells, its rivers and shore as lines, bent onto the globe past the world map
## (PRE-29, WLD-02); the middle ground to about 16 km in coarse chunks; the near ground within about
## 300 m, a vertex every metre or two, with its trees (A8.1, A8.3). Each chunk is made on a worker
## thread from the seed alone (A7.5, WLD-13). Each ring draws round the finer ones, where their
## chunks are all made, giving way to them by dithering, and shrinks as the zoom leaves it; all
## round an origin that moves with the focus (A8.2). Measure pinches from the globe to a person over
## the start region, made afresh, and back, and times the frames at every stop and the making of a
## full area. Pre-production code (research 00): the app's README names its items.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const ART := 4
const INK := Color("ebe5da")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const GAP := 12
const THREADS := 4
## The zoom's stops (PRE-03), each with the metres an art pixel shows there, on the phone's 336 art
## pixels across: a person about 53 art pixels tall in 10 m, the close camp about 45 m, the camp
## about 370 m, the valley 12 km, the region 120 km, the world map the world's 2,000 km around; then
## the globe, the map bent onto it.
const STOPS := [
	["person", 0.03],
	["close camp", 0.13],
	["camp", 1.1],
	["valley", 37.0],
	["region", 370.0],
	["world map", 5500.0],
	["globe", 0.0],
]
## The share of the zoom's range up to the world map; the rest bends the map onto the globe.
const FLAT := 0.85
## The levels of detail (A8.1), finest first, as rings round the focus, as clipmaps do: a chunk's
## side in metres, its vertices a side, how far round the focus the ring reaches, the most metres an
## art pixel shows while it is drawn, its ring shrinking to nothing over the last halving of the
## scale before that, and how its chunks' trees are drawn, if they hold any. The near rings are the
## full areas within about 300 m (PRE-03), the first of them a full area across, their trees as
## models; the middle ones, the areas' coarse ground beyond.
const LEVELS := [
	{"name": "near", "side": 64.0, "n": 65, "reach": 128.0, "most": 1.0, "trees": "models"},
	{"name": "near out", "side": 128.0, "n": 65, "reach": 300.0, "most": 2.0, "trees": "models"},
	{"name": "middle", "side": 1600.0, "n": 41, "reach": 5000.0, "most": 70.0, "trees": ""},
	{"name": "middle out", "side": 3200.0, "n": 41, "reach": 16000.0, "most": 140.0, "trees": ""},
]
## The trees beyond the near rings as cards, two faces for a crown and two for a trunk, on the
## middle ground out to where the camp's tilted view ends, so the forest goes on past the full
## areas; beyond them, the cover's colour (A8.3). A ring of chunks with no ground of their own.
const CARDS := {
	"name": "cards", "side": 256.0, "n": 0, "reach": 900.0, "most": 2.0, "trees": "cards"
}
## How far a card sinks into the coarse ground it stands on, which may lie below the ground's true
## height between its vertices.
const CARD_SINK := 1.5
## The map and its lines, each with its shader.
const MAP_SHADERS := {"map": "map", "rivers": "lines", "coasts": "lines"}
## A tree may stand every TREE_GRID metres in a near chunk, as its cover allows (A9).
const TREE_GRID := 5.0
## The origin moves to the focus once the focus is this far from it (A8.2).
const REBASE := 2000.0
## The most chunks being made at once, on the worker threads.
const MAKING := 6
## The camera's tilt down from level, from the camp inward, rising to straight down at the valley.
const PITCH := 30.0
## The camera's field of view across the picture, in degrees, as the map bends onto the globe: from
## nearly parallel, so the flat map looks as it did, to a globe's (A8.4).
const GLOBE_FOV := Vector2(10.0, 30.0)
## Measure: seconds from the globe to a person, the hold there, and back.
const PINCH_SECONDS := 15.0
const HOLD_SECONDS := 3.0
## A full area, about 256 m across (WLD-12): Measure times it from the first near ring's first chunk
## asked until every chunk within this many metres of the focus is made.
const AREA_HALF := 128.0

var gen: RefCounted
## The focus in metres east and north of the world's corner; the world's own size in metres.
var focus := Vector2.ZERO
var world_m := Vector2.ZERO
## 0 at the person, FLAT at the world map, 1 at the globe.
var zoom := 1.0

var _origin := Vector2.ZERO
var _art: SubViewport
var _view: TextureRect
var _cam: Camera3D
var _readout: Label
var _measure_button: Button
var _scenario: RID
var _materials := {}
var _rids: Array[RID] = []
var _tree_mesh := RID()
var _card_mesh := RID()
## Each chunk made, by its key: its level, corner in metres, mesh and instance, and its trees'
## multimesh and instance.
var _chunks := {}
## Each ring's plan round the focus: where the focus was when it was made, the chunks the ring
## keeps, and those not yet made, nearest first, each with its square.
var _plans := {}
var _asked := {}
var _tasks := {}
var _done := []
var _done_lock := Mutex.new()
var _indices := {}
var _worker: Thread
var _touches := {}
var _measuring := false
var _clock := 0.0
var _late := {}
var _samples := {}
var _area_asked := -1.0
var _area_time := -1.0
var _heat := []
var _read_clock := 0.0


func _ready() -> void:
	# the cloud's pictures wait while the screen is still making what it draws
	add_to_group("busy")
	var background := ColorRect.new()
	background.color = NIGHT
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE  # the screen's own gestures take every touch
	add_child(background)
	_art = SubViewport.new()
	_art.msaa_3d = Viewport.MSAA_DISABLED
	_art.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	_art.positional_shadow_atlas_size = 0
	_art.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_art.own_world_3d = true
	RenderingServer.viewport_set_measure_render_time(_art.get_viewport_rid(), true)
	add_child(_art)
	var env := Environment.new()
	env.background_mode = Environment.BG_COLOR
	env.background_color = NIGHT
	var world_env := WorldEnvironment.new()
	world_env.environment = env
	_art.add_child(world_env)
	_cam = Camera3D.new()
	_cam.keep_aspect = Camera3D.KEEP_WIDTH
	_art.add_child(_cam)
	_view = TextureRect.new()
	_view.texture = _art.get_texture()
	_view.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_view.stretch_mode = TextureRect.STRETCH_SCALE
	_view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_view)
	var margin := MarginContainer.new()
	margin.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	margin.mouse_filter = Control.MOUSE_FILTER_IGNORE
	for side in ["left", "top", "right", "bottom"]:
		margin.add_theme_constant_override("margin_" + side, GAP)
	add_child(margin)
	var column := VBoxContainer.new()
	column.mouse_filter = Control.MOUSE_FILTER_IGNORE
	column.add_theme_constant_override("separation", GAP)
	margin.add_child(column)
	_text(column, "P8 The zoom", FLAME, 22)
	_readout = _text(column, "Making a world…", INK, 14)
	var space := Control.new()
	space.size_flags_vertical = SIZE_EXPAND_FILL
	space.mouse_filter = Control.MOUSE_FILTER_IGNORE
	column.add_child(space)
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", GAP)
	column.add_child(row)
	_measure_button = Button.new()
	_measure_button.text = "Measure"
	_measure_button.custom_minimum_size.y = 48
	_measure_button.size_flags_horizontal = SIZE_EXPAND_FILL
	_measure_button.disabled = true
	_measure_button.pressed.connect(measure)
	row.add_child(_measure_button)
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size.y = 48
	back.size_flags_horizontal = SIZE_EXPAND_FILL
	back.pressed.connect(func() -> void: closed.emit())
	row.add_child(back)
	resized.connect(_layout)
	_layout()
	if not ClassDB.class_exists("WorldGen"):
		_readout.text = "This build has no C++ part, so nothing can run."
		remove_from_group("busy")
		return
	gen = ClassDB.instantiate("WorldGen")
	_worker = Thread.new()
	_worker.start(_make_world)


func _make_world() -> void:
	gen.make(THREADS)
	_world_made.call_deferred()


## The world's map and its lines, the materials, and the focus on the start region.
func _world_made() -> void:
	_worker.wait_to_finish()
	_scenario = _art.find_world_3d().scenario
	world_m = gen.world_size()
	var cells: Vector2i = gen.world_cells()
	var map := Image.create_from_data(cells.x, cells.y, false, Image.FORMAT_RGB8, gen.map_at(0, 1))
	map.flip_y()  # rows from the south, as the heights'
	var heights := Image.create_from_data(
		cells.x,
		cells.y,
		false,
		Image.FORMAT_RF,
		(gen.heights(0) as PackedFloat32Array).to_byte_array()
	)
	heights.convert(Image.FORMAT_RH)  # half floats, which every phone's graphics can filter
	var height_tex := ImageTexture.create_from_image(heights)
	for level: Dictionary in LEVELS + [CARDS]:
		_materials[level.name] = _material("ground")
	for name: String in MAP_SHADERS:
		_materials[name] = _material(MAP_SHADERS[name])
		_materials[name].set_shader_parameter("height_tex", height_tex)
	_materials.map.set_shader_parameter("map_tex", ImageTexture.create_from_image(map))
	_materials.rivers.set_shader_parameter("tint", Vector3(0.38, 0.52, 0.83))
	_materials.coasts.set_shader_parameter("tint", Vector3(0.92, 0.94, 0.89))
	_add_whole(_map_mesh(256, 128), _materials.map)
	_add_whole(_lines(gen.rivers(0, 1000.0)), _materials.rivers)
	_add_whole(_lines(gen.coasts(0)), _materials.coasts)
	_tree_mesh = _make_tree()
	_card_mesh = _make_card()
	_forget_chunks()
	focus = gen.start(0)
	_origin = focus
	var args := OS.get_cmdline_user_args()
	for stop: Array in STOPS:
		if "stop=" + stop[0] in args:
			zoom = zoom_of(stop[0])
	_measure_button.disabled = false
	_readout.text = "Pinch from the globe to a person; drag to move. Measure pinches by itself."
	if "measure" in args:
		measure.call_deferred()


func _material(shader: String) -> ShaderMaterial:
	var m := ShaderMaterial.new()
	m.shader = load("res://zoom/shaders/%s.gdshader" % shader)
	m.set_shader_parameter("world_m", world_m)
	return m


func _layout() -> void:
	var k := _screen_scale()
	var screen := Vector2(DisplayServer.window_get_size())
	if screen.x <= 0.0:
		screen = size * k
	_art.size = Vector2i(ceili(screen.x / ART), ceili(screen.y / ART))
	_view.size = Vector2(_art.size) * ART / k


## Screen pixels to one of the interface's units.
func _screen_scale() -> float:
	var window := DisplayServer.window_get_size()
	return window.x / size.x if size.x > 0.0 and window.x > 0 else 1.0


func _process(delta: float) -> void:
	if _scenario == RID():
		return
	if _measuring:
		_step_measure(delta)
	_collect()
	var mpp := metres_a_pixel(zoom)
	for level: Dictionary in LEVELS + [CARDS]:
		_update_level(level, mpp)
	_place(mpp)
	if is_in_group("busy") and _tasks.is_empty() and _asked.is_empty():
		remove_from_group("busy")


## The metres an art pixel shows at a zoom: from the person's to the world map's in even steps of
## its logarithm, held at the world map's while the map bends onto the globe.
static func metres_a_pixel(z: float) -> float:
	var t := clampf(z / FLAT, 0.0, 1.0)
	return exp(lerpf(log(STOPS[0][1]), log(STOPS[5][1]), t))


## The zoom at a stop.
static func zoom_of(stop: String) -> float:
	if stop == STOPS[6][0]:
		return 1.0
	for s: Array in STOPS.slice(0, 6):
		if s[0] == stop:
			return FLAT * log(s[1] / STOPS[0][1]) / log(STOPS[5][1] / STOPS[0][1])
	return 1.0


## How far the map is bent onto the globe at a zoom: 0 to the world map, 1 at the globe.
static func bend_at(z: float) -> float:
	return smoothstep(FLAT, 1.0, z)


## The camera's tilt down from level (A8.4): PITCH from the camp inward, straight down from the
## valley out, in between in even steps of the scale's logarithm.
static func pitch_at(mpp: float) -> float:
	var camp: float = STOPS[2][1]
	var valley: float = STOPS[3][1]
	return lerpf(PITCH, 90.0, clampf(log(mpp / camp) / log(valley / camp), 0.0, 1.0))


## How much of a ring is drawn at a scale: all of it to half its most metres an art pixel, shrinking
## to none at its most, in even steps of the scale's logarithm.
static func ring_at(level: Dictionary, mpp: float) -> float:
	return clampf(log(float(level.most) / mpp) / log(2.0), 0.0, 1.0)


## The stop nearest a zoom, for Measure's counts.
static func stop_at(z: float) -> String:
	if z > (FLAT + 1.0) / 2.0:
		return STOPS[6][0]
	var mpp := metres_a_pixel(z)
	var best: String = STOPS[0][0]
	var gap := INF
	for stop: Array in STOPS.slice(0, 6):
		var d := absf(log(mpp / float(stop[1])))
		if d < gap:
			gap = d
			best = stop[0]
	return best


## The camera and the shaders' shared places for this frame: the focus where the moving origin puts
## it, the camera orthographic, pitched for the close stops and straight down from the valley out,
## then perspective as the map bends onto the globe (A8.4); and where each ring draws.
func _place(mpp: float) -> void:
	if _local(focus).length() > REBASE:
		_origin = focus
		for key: String in _chunks:
			_set_chunk_place(_chunks[key])
	var bend := bend_at(zoom)
	var ground := 0.0
	if mpp <= float(LEVELS[-1].most):
		var h: PackedFloat32Array = gen.ground(0, focus.x, focus.y, 1, 1.0)
		ground = maxf(h[0], 0.0) if h.size() > 0 else 0.0
	var at := _local(focus)
	var look := at + Vector3(0.0, ground, 0.0)
	var radius := world_m.x / TAU
	var width := lerpf(float(_art.size.x) * mpp, 2.6 * radius, bend)
	var basis := Basis.from_euler(Vector3(deg_to_rad(-pitch_at(mpp)), 0.0, 0.0))
	if bend <= 0.0:
		var back := maxf(5000.0, width * 3.0)
		_cam.projection = Camera3D.PROJECTION_ORTHOGONAL
		_cam.size = width
		_cam.near = 1.0
		_cam.far = back * 2.0 + width * 2.0
		_cam.transform = Transform3D(basis, look + basis.z * back)
	else:
		var fov := lerpf(GLOBE_FOV.x, GLOBE_FOV.y, bend)
		var distance := width * 0.5 / tan(deg_to_rad(fov) * 0.5)
		_cam.projection = Camera3D.PROJECTION_PERSPECTIVE
		_cam.fov = fov
		_cam.near = distance * 0.5
		_cam.far = distance + 2.5 * radius
		_cam.transform = Transform3D(basis, look + basis.z * distance)
	for m: ShaderMaterial in _materials.values():
		m.set_shader_parameter("focus_m", focus)
		m.set_shader_parameter("focus_at", at)
		m.set_shader_parameter("bend", bend)
	# each ring draws beyond the finer rings' reach and within its own, which shrinks as the zoom
	# leaves it; the map beyond them all; the cards beyond the trees drawn as models
	var inner := 0.0
	var models := 0.0
	for level: Dictionary in LEVELS:
		var shown := _covered(level) * ring_at(level, mpp)
		_materials[level.name].set_shader_parameter("inner", inner)
		_materials[level.name].set_shader_parameter("outer", shown)
		inner = maxf(inner, shown)
		if level.trees == "models":
			models = maxf(models, shown)
	for name: String in MAP_SHADERS:
		_materials[name].set_shader_parameter("inner", inner)
	_materials[CARDS.name].set_shader_parameter("inner", models)
	_materials[CARDS.name].set_shader_parameter("outer", _covered(CARDS) * ring_at(CARDS, mpp))
	if _measuring and _area_asked >= 0.0 and _area_time < 0.0:
		if _covered(LEVELS[0]) >= AREA_HALF:
			_area_time = _clock - _area_asked


## How far round the focus a ring's chunks are all made, up to its reach; none while it is not
## planned.
func _covered(level: Dictionary) -> float:
	var plan: Dictionary = _plans[level.name]
	if plan.at == Vector2.INF:
		return 0.0
	var least: float = level.reach
	var f: Vector2 = plan.at + _wrapped(focus - plan.at)
	for m: Array in plan.missing:
		least = minf(least, _nearest(m[4], f) - 1.0)
	return maxf(least, 0.0)


## The distance from a point to a square's nearest point.
static func _nearest(square: Rect2, p: Vector2) -> float:
	var q := Vector2(
		clampf(p.x, square.position.x, square.end.x), clampf(p.y, square.position.y, square.end.y)
	)
	return q.distance_to(p)


func _key(level: Dictionary, i: int, j: int) -> String:
	var side: float = level.side
	var nx := roundi(world_m.x / side)
	var ny := roundi(world_m.y / side)
	return "%s:%d:%d" % [level.name, posmod(i, nx), posmod(j, ny)]


## A ring's chunks round the focus (A8.1): those it needs, nearest first, asked of the worker
## threads, and those beyond its reach, or all of them once it is far from drawn, freed. It is
## planned again only when the focus has moved half a chunk.
func _update_level(level: Dictionary, mpp: float) -> void:
	var plan: Dictionary = _plans[level.name]
	if mpp > float(level.most) * 2.0:
		if plan.at != Vector2.INF:
			_free_level(level.name)
			_plans[level.name] = _no_plan()
		return
	if mpp > float(level.most):
		return  # kept a while, so a pinch back and forth over its edge makes nothing again
	if _wrapped(focus - plan.at).length() > float(level.side) * 0.5:
		plan = _plan(level)
		_plans[level.name] = plan
		if level.name == LEVELS[0].name and _measuring and _area_asked < 0.0:
			_area_asked = _clock
	var still := []
	for m: Array in plan.missing:
		if _chunks.has(m[1]):
			continue
		still.append(m)
		if _tasks.size() < MAKING and not _asked.has(m[1]):
			_ask(level, m[1], m[2], m[3])
	plan.missing = still


## The chunks a ring keeps until the focus moves half a chunk: those within its reach of anywhere
## the focus may then be, and the ones not yet made, nearest first; those it no longer keeps freed.
func _plan(level: Dictionary) -> Dictionary:
	var side: float = level.side
	var reach: float = level.reach
	var r := ceili(reach / side) + 1
	var ci := floori(focus.x / side)
	var cj := floori(focus.y / side)
	var keep := {}
	var missing := []
	for j in range(cj - r, cj + r + 1):
		for i in range(ci - r, ci + r + 1):
			var square := Rect2(i * side, j * side, side, side)
			var near := _nearest(square, focus)
			if near > reach + side * 0.5:
				continue
			var key := _key(level, i, j)
			keep[key] = true
			if not _chunks.has(key):
				missing.append([near, key, i, j, square])
	missing.sort_custom(func(a: Array, b: Array) -> bool: return a[0] < b[0])
	for key: String in _chunks.keys():
		if key.begins_with(level.name + ":") and not keep.has(key):
			_free_chunk(_chunks[key])
			_chunks.erase(key)
	return {"at": focus, "keep": keep, "missing": missing}


static func _no_plan() -> Dictionary:
	return {"at": Vector2.INF, "keep": {}, "missing": []}


func _ask(level: Dictionary, key: String, i: int, j: int) -> void:
	var side: float = level.side
	var n: int = level.n
	var east := posmod(i, roundi(world_m.x / side)) * side
	var north := posmod(j, roundi(world_m.y / side)) * side
	_asked[key] = true
	var spacing := side / float(maxi(n - 1, 1))
	var id := WorkerThreadPool.add_task(
		_make_chunk.bind(key, level.name, east, north, n, spacing, side, level.trees)
	)
	_tasks[id] = key


## On a worker thread: a chunk's ground, if its ring has any, and its trees, if it draws them.
func _make_chunk(
	key: String,
	level: String,
	east: float,
	north: float,
	n: int,
	spacing: float,
	side: float,
	trees_as: String
) -> void:
	var arrays: Array = gen.ground_mesh(0, east, north, n, spacing) if n > 0 else []
	var trees := PackedVector3Array()
	if trees_as != "":
		trees = gen.trees(0, east, north, side, TREE_GRID)
	_done_lock.lock()
	_done.append([key, level, east, north, n, spacing, arrays, trees])
	_done_lock.unlock()


## The chunks the worker threads have made, given their meshes and placed, unless their level no
## longer keeps them.
func _collect() -> void:
	for id: int in _tasks.keys():
		if WorkerThreadPool.is_task_completed(id):
			WorkerThreadPool.wait_for_task_completion(id)
			_tasks.erase(id)
	_done_lock.lock()
	var done := _done
	_done = []
	_done_lock.unlock()
	for d: Array in done:
		var key: String = d[0]
		_asked.erase(key)
		var plan: Dictionary = _plans[d[1]]
		var ground: Array = d[6]
		if not plan.keep.has(key) or _chunks.has(key) or (d[4] > 0 and ground.size() < 3):
			continue
		var chunk := {"level": d[1], "east": d[2], "north": d[3]}
		var material := (_materials[d[1]] as ShaderMaterial).get_rid()
		if d[4] > 0:
			var arrays := []
			arrays.resize(Mesh.ARRAY_MAX)
			arrays[Mesh.ARRAY_VERTEX] = ground[0]
			arrays[Mesh.ARRAY_NORMAL] = ground[1]
			arrays[Mesh.ARRAY_COLOR] = ground[2]
			arrays[Mesh.ARRAY_INDEX] = _grid_indices(d[4])
			chunk.mesh = RenderingServer.mesh_create()
			RenderingServer.mesh_add_surface_from_arrays(
				chunk.mesh, RenderingServer.PRIMITIVE_TRIANGLES, arrays
			)
			chunk.instance = RenderingServer.instance_create2(chunk.mesh, _scenario)
			RenderingServer.instance_geometry_set_material_override(chunk.instance, material)
		var trees: PackedVector3Array = d[7]
		if trees.size() > 0:
			var cards: bool = d[1] == CARDS.name
			chunk.trees = RenderingServer.multimesh_create()
			RenderingServer.multimesh_allocate_data(
				chunk.trees, trees.size(), RenderingServer.MULTIMESH_TRANSFORM_3D
			)
			RenderingServer.multimesh_set_mesh(chunk.trees, _card_mesh if cards else _tree_mesh)
			RenderingServer.multimesh_set_buffer(
				chunk.trees, _tree_buffer(trees, CARD_SINK if cards else 0.0)
			)
			chunk.tree_instance = RenderingServer.instance_create2(chunk.trees, _scenario)
			RenderingServer.instance_geometry_set_material_override(chunk.tree_instance, material)
		_chunks[key] = chunk
		_set_chunk_place(chunk)


func _set_chunk_place(chunk: Dictionary) -> void:
	var place := Transform3D(Basis(), _local(Vector2(chunk.east, chunk.north)))
	for k: String in ["instance", "tree_instance"]:
		if chunk.has(k):
			RenderingServer.instance_set_transform(chunk[k], place)


func _free_chunk(chunk: Dictionary) -> void:
	for k: String in ["tree_instance", "trees", "instance", "mesh"]:
		if chunk.has(k):
			RenderingServer.free_rid(chunk[k])


func _free_level(name: String) -> void:
	for key: String in _chunks.keys():
		if key.begins_with(name + ":"):
			_free_chunk(_chunks[key])
			_chunks.erase(key)


## Every chunk freed and every plan cleared, so the land round the focus is made afresh.
func _forget_chunks() -> void:
	for level: Dictionary in LEVELS + [CARDS]:
		_free_level(level.name)
		_plans[level.name] = _no_plan()


## A place in the world's metres, in the picture's from the moving origin: x east, z south.
func _local(m: Vector2) -> Vector3:
	var d := _wrapped(m - _origin)
	return Vector3(d.x, 0.0, -d.y)


## A step between two places in the world's metres, the shorter way round each of its seams.
func _wrapped(d: Vector2) -> Vector2:
	if not d.is_finite():
		return d
	return d - world_m * (d / world_m + Vector2(0.5, 0.5)).floor()


## The triangles of an n × n grid of vertices, shared by every chunk of that size.
func _grid_indices(n: int) -> PackedInt32Array:
	if _indices.has(n):
		return _indices[n]
	var out := PackedInt32Array()
	out.resize((n - 1) * (n - 1) * 6)
	var k := 0
	for j in n - 1:
		for i in n - 1:
			var a := j * n + i
			for v: int in [a, a + n, a + 1, a + 1, a + n, a + n + 1]:
				out[k] = v
				k += 1
	_indices[n] = out
	return out


## The trees' transforms for a multimesh, 12 floats each: each tree, given as metres east of its
## chunk's corner, the ground's height and metres north, standing there, sunk by `sink` metres, and
## sized by its place.
static func _tree_buffer(trees: PackedVector3Array, sink: float) -> PackedFloat32Array:
	var out := PackedFloat32Array()
	out.resize(trees.size() * 12)
	for t in trees.size():
		var p: Vector3 = trees[t]
		var s := 0.8 + 0.4 * fposmod(p.x * 0.37 + p.z * 0.61, 1.0)
		var row := t * 12
		for v: float in [s, 0.0, 0.0, p.x, 0.0, s, 0.0, p.y - sink, 0.0, 0.0, s, -p.z]:
			out[row] = v
			row += 1
	return out


## A tree as the camp zoom needs it, the cost of P2's: a crown of 8 faces on a trunk of two crossed
## faces, 12 triangles, in its cover's colours.
func _make_tree() -> RID:
	var leaf := Color8(65, 93, 77)
	var bark := Color8(92, 74, 58)
	var v := PackedVector3Array()
	var c := PackedColorArray()
	var nrm := PackedVector3Array()
	for side: Vector3 in [Vector3(0.25, 0.0, 0.0), Vector3(0.0, 0.0, 0.25)]:
		var up := Vector3(0.0, 2.4, 0.0)
		var face := side.cross(Vector3.UP).normalized()
		for p: Vector3 in [-side, side, side + up, -side, side + up, -side + up]:
			v.append(p)
			c.append(bark)
			nrm.append(face)
	var top := Vector3(0.0, 7.4, 0.0)
	var low := Vector3(0.0, 2.6, 0.0)
	var ring: Array[Vector3] = []
	for k in 4:
		var a := TAU * k / 4.0
		ring.append(Vector3(cos(a) * 3.0, 4.4, sin(a) * 3.0))
	for k in 4:
		var p0: Vector3 = ring[k]
		var p1: Vector3 = ring[(k + 1) % 4]
		for tri: Array in [[top, p1, p0], [low, p0, p1]]:
			var face: Vector3 = ((tri[1] - tri[0]) as Vector3).cross(tri[2] - tri[0]).normalized()
			for p: Vector3 in tri:
				v.append(p)
				c.append(leaf)
				nrm.append(face)
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = v
	arrays[Mesh.ARRAY_NORMAL] = nrm
	arrays[Mesh.ARRAY_COLOR] = c
	var mesh := RenderingServer.mesh_create()
	RenderingServer.mesh_add_surface_from_arrays(mesh, RenderingServer.PRIMITIVE_TRIANGLES, arrays)
	_rids.append(mesh)
	return mesh


## A tree as a card, for the middle distance: the model's crown as the tilted camera sees it, a
## diamond of two faces, on a trunk of two, all facing the camera, which always looks north.
func _make_card() -> RID:
	var leaf := Color8(65, 93, 77)
	var bark := Color8(92, 74, 58)
	var v := PackedVector3Array(
		[
			Vector3(-0.25, 0.0, 0.0),
			Vector3(0.25, 2.6, 0.0),
			Vector3(0.25, 0.0, 0.0),
			Vector3(-0.25, 0.0, 0.0),
			Vector3(-0.25, 2.6, 0.0),
			Vector3(0.25, 2.6, 0.0),
			Vector3(-3.0, 4.4, 0.0),
			Vector3(0.0, 7.4, 0.0),
			Vector3(3.0, 4.4, 0.0),
			Vector3(-3.0, 4.4, 0.0),
			Vector3(3.0, 4.4, 0.0),
			Vector3(0.0, 2.6, 0.0),
		]
	)
	var c := PackedColorArray()
	var nrm := PackedVector3Array()
	for k in v.size():
		c.append(bark if k < 6 else leaf)
		nrm.append(Vector3(0.0, 0.5, 1.0).normalized())
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = v
	arrays[Mesh.ARRAY_NORMAL] = nrm
	arrays[Mesh.ARRAY_COLOR] = c
	var mesh := RenderingServer.mesh_create()
	RenderingServer.mesh_add_surface_from_arrays(mesh, RenderingServer.PRIMITIVE_TRIANGLES, arrays)
	_rids.append(mesh)
	return mesh


## The map's mesh: a grid spanning the world, east and west from -0.5 to 0.5 of it round the focus,
## south to north from 0 to 1, which its shader places, flat or bent onto the globe.
func _map_mesh(nx: int, ny: int) -> RID:
	var v := PackedVector3Array()
	for j in ny + 1:
		for i in nx + 1:
			v.append(Vector3(float(i) / nx - 0.5, 0.0, float(j) / ny))
	var idx := PackedInt32Array()
	for j in ny:
		for i in nx:
			var a := j * (nx + 1) + i
			idx.append_array([a, a + 1, a + nx + 1, a + 1, a + nx + 2, a + nx + 1])
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = v
	arrays[Mesh.ARRAY_INDEX] = idx
	var mesh := RenderingServer.mesh_create()
	RenderingServer.mesh_add_surface_from_arrays(mesh, RenderingServer.PRIMITIVE_TRIANGLES, arrays)
	return mesh


## Line segments from pairs of places in the world's metres, which the lines' shader places, each
## end with its segment's middle, so both ends cross the world's seam together.
func _lines(pairs: PackedVector2Array) -> RID:
	var v := PackedVector3Array()
	var middles := PackedVector2Array()
	v.resize(pairs.size())
	middles.resize(pairs.size())
	for k in pairs.size():
		v[k] = Vector3(pairs[k].x, 0.0, pairs[k].y)
		middles[k] = (pairs[k - k % 2] + pairs[k - k % 2 + 1]) * 0.5
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = v
	arrays[Mesh.ARRAY_TEX_UV] = middles
	var mesh := RenderingServer.mesh_create()
	if v.size() > 0:
		RenderingServer.mesh_add_surface_from_arrays(mesh, RenderingServer.PRIMITIVE_LINES, arrays)
	return mesh


## A mesh drawn over the whole world, its shader placing it, so never culled.
func _add_whole(mesh: RID, material: ShaderMaterial) -> void:
	var huge := AABB(Vector3(-4.0e6, -4.0e6, -4.0e6), Vector3(8.0e6, 8.0e6, 8.0e6))
	RenderingServer.mesh_set_custom_aabb(mesh, huge)
	var instance := RenderingServer.instance_create2(mesh, _scenario)
	RenderingServer.instance_geometry_set_material_override(instance, material.get_rid())
	_rids.append(instance)
	_rids.append(mesh)


## Measure (PLT-04, PRE-03): from the globe to a person over the start region, its land made
## afresh, in PINCH_SECONDS, a hold, and back, at 60 frames a second; each frame counted at its
## nearest stop.
func measure() -> void:
	if _scenario == RID() or _measuring:
		return
	_forget_chunks()
	focus = gen.start(0)
	zoom = 1.0
	_measuring = true
	_clock = 0.0
	_late = {}
	_samples = {}
	_area_asked = -1.0
	_area_time = -1.0
	_heat = [RUNS.thermal()]
	_read_clock = 0.0
	Engine.max_fps = 60
	_measure_button.disabled = true
	_readout.text = "Measuring: from the globe to a person and back, about 35 seconds…"


func _step_measure(delta: float) -> void:
	_clock += delta
	_read_clock += delta
	if _read_clock >= RUNS.READ_EVERY:
		_read_clock = 0.0
		_heat.append(RUNS.thermal())
	var half := PINCH_SECONDS
	if _clock < half:
		zoom = 1.0 - _clock / half
	elif _clock < half + HOLD_SECONDS:
		zoom = 0.0
	elif _clock < 2.0 * half + HOLD_SECONDS:
		zoom = (_clock - half - HOLD_SECONDS) / half
	else:
		_finish_measure()
		return
	if _clock > 1.0:
		var stop := stop_at(zoom)
		if not _samples.has(stop):
			_samples[stop] = []  # an Array, which grows in place, where a packed array is copied
			_late[stop] = 0
		(_samples[stop] as Array).append(
			RenderingServer.viewport_get_measured_render_time_gpu(_art.get_viewport_rid())
		)
		if delta > 1.15 / 60.0:
			_late[stop] += 1


func _finish_measure() -> void:
	_measuring = false
	Engine.max_fps = 0
	_measure_button.disabled = false
	_heat.append(RUNS.thermal())
	RUNS.finish(self, "P8", results(_samples, _late, _area_time), _heat, 0)


## Measure's results for the chat: each stop's graphics time and frames on time, in the stops'
## order, and the full area's time.
static func results(samples: Dictionary, late: Dictionary, area: float) -> PackedStringArray:
	var out := PackedStringArray()
	for stop: Array in STOPS:
		var name: String = stop[0]
		if samples.has(name):
			out.append(RUNS.summary(name, PackedFloat32Array(samples[name]), late.get(name, 0)))
	out.append("full area %s" % ("%.2f s" % area if area >= 0.0 else "not made"))
	return out


func _gui_input(e: InputEvent) -> void:
	if _measuring or _scenario == RID():
		return
	if e is InputEventMouse and e.device == InputEvent.DEVICE_ID_EMULATION:
		return  # the phone's mouse events made from the first finger, already read as touches
	if e is InputEventScreenTouch:
		var t := e as InputEventScreenTouch
		if t.pressed:
			_touches[t.index] = t.position
		else:
			_touches.erase(t.index)
	elif e is InputEventScreenDrag:
		var d := e as InputEventScreenDrag
		var old: Vector2 = _touches.get(d.index, d.position)
		if _touches.size() == 1:
			_pan(d.position - old)
		elif _touches.size() >= 2:
			var other := Vector2.ZERO
			for k: int in _touches:
				if k != d.index:
					other = _touches[k]
					break
			var d0 := (old - other).length()
			var d1 := (d.position - other).length()
			if d0 > 4.0 and d1 > 4.0:
				_pinch(d0 / d1)
		_touches[d.index] = d.position
	elif e is InputEventMouseMotion and (e as InputEventMouseMotion).button_mask:
		_pan((e as InputEventMouseMotion).relative)
	elif e is InputEventMouseButton and (e as InputEventMouseButton).pressed:
		var b := e as InputEventMouseButton
		if b.button_index == MOUSE_BUTTON_WHEEL_UP:
			_pinch(0.9)
		elif b.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			_pinch(1.1)


## A pinch by a factor of the metres an art pixel shows, as an even step of the zoom.
func _pinch(factor: float) -> void:
	var per := FLAT / log(STOPS[5][1] / STOPS[0][1])
	zoom = clampf(zoom + log(factor) * per, 0.0, 1.0)


## A drag moves the ground under the finger: across the picture as it is, up the picture by the
## ground's length the tilted camera shows there. The focus stays between the poles.
func _pan(by: Vector2) -> void:
	var mpp := metres_a_pixel(zoom)
	var metres := mpp * _screen_scale() / ART
	focus.x = fposmod(focus.x - by.x * metres, world_m.x)
	focus.y = clampf(focus.y + by.y * metres / sin(deg_to_rad(pitch_at(mpp))), 0.0, world_m.y)


func _exit_tree() -> void:
	if _worker != null and _worker.is_started():
		_worker.wait_to_finish()
	for id: int in _tasks.keys():
		WorkerThreadPool.wait_for_task_completion(id)
	for key: String in _chunks:
		_free_chunk(_chunks[key])
	for rid: RID in _rids:
		RenderingServer.free_rid(rid)
	Engine.max_fps = 0


func _text(box: VBoxContainer, text: String, colour: Color, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_color_override("font_color", colour)
	label.add_theme_font_size_override("font_size", font_size)
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	box.add_child(label)
	return label
