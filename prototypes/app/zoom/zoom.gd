## P8 The zoom (IMPLEMENTATION α0.5b), second round: one descent from space to a person over P7's
## world, a planet at every scale, never unrolled into a flat map (WLD-02). The ground is chunks in
## a tree of levels, each made on a worker thread from the seed alone (WLD-13) and morphing
## smoothly into the next as the camera moves (CDLOD, A8.1); one shader colours every level the
## same way, so only detail changes; a pass over the picture adds the air's scattering and the
## clouds, which the camera passes through on its way down (A8.4, A8.6). The picture is drawn at
## a pixel for every two of the screen's and shown in pixels whose size the variant sets (A4).
## Variants to compare (A/B): pixels, land, water, clouds, light and the camera's path, and the
## time of day. Measure descends from the globe to a person and back by itself, timing every stop.
## Pre-production code (research 00): the app's README names its items.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const BASE := 2  # the screen's pixels a drawn pixel
const INK := Color("ebe5da")
const FLAME := Color("f6a33c")
const NIGHT := Color("0b0a12")
const GAP := 10
const THREADS := 4
## The zoom's stops (PRE-03), each with the metres the picture shows across at the focus: a person
## about 10 m, the close camp 45 m, the camp 370 m, the valley 12 km, the region 120 km; the world
## map, a continent on the curving globe; the globe whole.
const STOPS := [
	["person", 10.0],
	["close camp", 45.0],
	["camp", 370.0],
	["valley", 12000.0],
	["region", 120000.0],
	["world map", 420000.0],
	["globe", 950000.0],
]
## The camera's field of view across the picture, degrees.
const FOV := 25.0
## The chunk tree (A8.1): 4 × 2 roots of 500 km, each splitting into four down to chunks 30.5 m
## across; every chunk N points a side; a chunk splits while the camera is nearer than SPLIT of
## its sides, and morphs into its parent over the last quarter of its parent's reach.
const ROOT := 500000.0
const DEEPEST := 14
const N := 33
const SPLIT := 2.4
## The most chunks being made at once, and the most kept made, drawn or not.
const MAKING := 6
const KEEP := 900
## The origin moves to the focus once the focus is this far from it (A8.2).
const REBASE := 2000.0
## The variants (A/B): each name, and what its letters mean, for the buttons and the note.
const VARIANTS := {
	"pixels": ["A fixed 4", "B stepped 2–6", "C blended 2–6"],
	"land": ["A art book", "B natural", "C natural, banded"],
	"water": ["A bands", "B deep and shallow", "C with waves"],
	"clouds": ["A volume", "B decks", "C flat"],
	"light": ["A art book", "B the air's"],
	"path": ["A down", "B flight"],
	"time": ["noon", "dusk", "night", "live"],
}
## Measure: seconds down from the globe to a person, the hold there, and back up.
const DESCENT_SECONDS := 20.0
const HOLD_SECONDS := 3.0

var gen: RefCounted
var world_m := Vector2(2.0e6, 1.0e6)
## The focus, metres east and north of the world's corner, and the zoom: 0 at the person, 1 at the
## globe, in even steps of the logarithm of the metres shown.
var focus := Vector2.ZERO
var zoom := 1.0
var choice := {"pixels": 2, "land": 1, "water": 1, "clouds": 0, "light": 1, "path": 0, "time": 0}

var _origin := Vector2.ZERO
var _art: SubViewport
var _view: TextureRect
var _cam: Camera3D
var _sky: MeshInstance3D
var _readout: Label
var _buttons := {}
var _measure_button: Button
var _scenario: RID
var _ground: ShaderMaterial
var _indices := PackedInt32Array()
## Each chunk made, by its key "depth:i:j": its mesh and instance, and the frame it was last drawn.
var _chunks := {}
var _asked := {}
var _tasks := {}
var _done := []
var _done_lock := Mutex.new()
var _drawn := {}
var _frame := 0
var _missing := 0
var _worker: Thread
var _touches := {}
var _tilt := 23.0
var _year := 0.15
var _day := 0.5
var _weather := 0.0
var _measuring := false
var _clock := 0.0
var _late := {}
var _samples := {}
var _area_asked := -1.0
var _area_time := -1.0
var _heat := []
var _read_clock := 0.0


func _ready() -> void:
	add_to_group("busy")
	var background := ColorRect.new()
	background.color = NIGHT
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE
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
	env.background_color = Color.BLACK
	env.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	var world_env := WorldEnvironment.new()
	world_env.environment = env
	_art.add_child(world_env)
	_cam = Camera3D.new()
	_cam.keep_aspect = Camera3D.KEEP_WIDTH
	_cam.fov = FOV
	_art.add_child(_cam)
	_view = TextureRect.new()
	_view.texture = _art.get_texture()
	_view.expand_mode = TextureRect.EXPAND_IGNORE_SIZE  # the picture is drawn larger than it shows
	_view.stretch_mode = TextureRect.STRETCH_SCALE
	_view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var show := ShaderMaterial.new()
	show.shader = load("res://zoom/shaders/pixels.gdshader")
	show.set_shader_parameter("drawn_near", _art.get_texture())
	show.set_shader_parameter("drawn_soft", _art.get_texture())
	_view.material = show
	add_child(_view)
	_build_controls()
	resized.connect(_layout)
	_layout()
	if not ClassDB.class_exists("WorldGen"):
		_readout.text = "This build has no C++ part, so nothing can run."
		remove_from_group("busy")
		return
	gen = ClassDB.instantiate("WorldGen")
	_worker = Thread.new()
	_worker.start(_make_world)


func _build_controls() -> void:
	var margin := MarginContainer.new()
	margin.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	margin.mouse_filter = Control.MOUSE_FILTER_IGNORE
	for side in ["left", "top", "right", "bottom"]:
		margin.add_theme_constant_override("margin_" + side, GAP)
	add_child(margin)
	var column := VBoxContainer.new()
	column.mouse_filter = Control.MOUSE_FILTER_IGNORE
	column.add_theme_constant_override("separation", 6)
	margin.add_child(column)
	_text(column, "P8 The zoom · round 2", FLAME, 20)
	_readout = _text(column, "Making a world…", INK, 13)
	var space := Control.new()
	space.size_flags_vertical = SIZE_EXPAND_FILL
	space.mouse_filter = Control.MOUSE_FILTER_IGNORE
	column.add_child(space)
	var grid := GridContainer.new()
	grid.columns = 3
	grid.add_theme_constant_override("h_separation", 6)
	grid.add_theme_constant_override("v_separation", 6)
	column.add_child(grid)
	for name: String in VARIANTS:
		var b := Button.new()
		b.custom_minimum_size = Vector2(0, 44)
		b.size_flags_horizontal = SIZE_EXPAND_FILL
		b.add_theme_font_size_override("font_size", 13)
		b.pressed.connect(_cycle.bind(name))
		grid.add_child(b)
		_buttons[name] = b
	_measure_button = Button.new()
	_measure_button.text = "Measure"
	_measure_button.custom_minimum_size = Vector2(0, 44)
	_measure_button.size_flags_horizontal = SIZE_EXPAND_FILL
	_measure_button.disabled = true
	_measure_button.pressed.connect(measure)
	grid.add_child(_measure_button)
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size = Vector2(0, 44)
	back.size_flags_horizontal = SIZE_EXPAND_FILL
	back.pressed.connect(func() -> void: closed.emit())
	grid.add_child(back)
	_label_buttons()


func _label_buttons() -> void:
	for name: String in VARIANTS:
		var b: Button = _buttons[name]
		b.text = "%s: %s" % [name.capitalize(), VARIANTS[name][choice[name]]]


func _cycle(name: String) -> void:
	choice[name] = (choice[name] + 1) % (VARIANTS[name] as Array).size()
	_label_buttons()


func _make_world() -> void:
	gen.make(THREADS)
	_world_made.call_deferred()


## The world's cells as textures, the materials, the sky's pass and the focus on the start region.
func _world_made() -> void:
	_worker.wait_to_finish()
	_scenario = _art.find_world_3d().scenario
	world_m = gen.world_size()
	_tilt = gen.tilt(0)
	gen.prepare_ground(0)
	var cells: Vector2i = gen.world_cells()
	var heights := Image.create_from_data(
		cells.x,
		cells.y,
		false,
		Image.FORMAT_RF,
		(gen.heights(0) as PackedFloat32Array).to_byte_array()
	)
	heights.convert(Image.FORMAT_RH)
	heights.generate_mipmaps()
	var climate := Image.create_from_data(
		cells.x,
		cells.y,
		false,
		Image.FORMAT_RGBAF,
		(gen.climate_texture(0) as PackedFloat32Array).to_byte_array()
	)
	climate.convert(Image.FORMAT_RGBAH)
	climate.generate_mipmaps()
	var cover := Image.create_from_data(
		cells.x, cells.y, false, Image.FORMAT_RGBA8, gen.cover_texture(0)
	)
	cover.generate_mipmaps()
	var water := Image.create_from_data(
		cells.x, cells.y, false, Image.FORMAT_RGBA8, gen.water_texture(0)
	)
	var water_soft := water.duplicate() as Image
	water_soft.generate_mipmaps()
	var noise_size := 64
	var noise: PackedByteArray = gen.cloud_noise(noise_size)
	var layers: Array[Image] = []
	var slab := noise_size * noise_size * 4
	for z in noise_size:
		layers.append(
			Image.create_from_data(
				noise_size,
				noise_size,
				false,
				Image.FORMAT_RGBA8,
				noise.slice(z * slab, (z + 1) * slab)
			)
		)
	var cloud := ImageTexture3D.new()
	cloud.create(Image.FORMAT_RGBA8, noise_size, noise_size, noise_size, false, layers)
	var rs := RenderingServer
	rs.global_shader_parameter_set("zoom_height", ImageTexture.create_from_image(heights))
	rs.global_shader_parameter_set("zoom_climate", ImageTexture.create_from_image(climate))
	rs.global_shader_parameter_set("zoom_cover", ImageTexture.create_from_image(cover))
	rs.global_shader_parameter_set("zoom_water", ImageTexture.create_from_image(water))
	rs.global_shader_parameter_set("zoom_water_soft", ImageTexture.create_from_image(water_soft))
	rs.global_shader_parameter_set("zoom_cloud_noise", cloud)
	rs.global_shader_parameter_set("zoom_world", world_m)
	_ground = ShaderMaterial.new()
	_ground.shader = load("res://zoom/shaders/ground.gdshader")
	_indices = chunk_indices(N)
	# the sky's pass: a quad over the whole picture, drawn after the ground
	_sky = MeshInstance3D.new()
	var quad := QuadMesh.new()
	quad.size = Vector2(1.0, 1.0)
	_sky.mesh = quad
	var sky := ShaderMaterial.new()
	sky.shader = load("res://zoom/shaders/sky.gdshader")
	sky.render_priority = -100
	_sky.material_override = sky
	_sky.extra_cull_margin = 16384.0
	_sky.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	_cam.add_child(_sky)
	_sky.position = Vector3(0.0, 0.0, -2.0)
	focus = gen.start(0)
	_origin = focus
	var args := OS.get_cmdline_user_args()
	for arg: String in args:
		if arg.begins_with("stop="):
			zoom = zoom_of(arg.substr(5))
		for name: String in VARIANTS:
			if arg.begins_with(name + "="):
				var letter := arg.substr(name.length() + 1)
				for k in (VARIANTS[name] as Array).size():
					if (VARIANTS[name][k] as String).begins_with(letter):
						choice[name] = k
	_label_buttons()
	_measure_button.disabled = false
	_readout.text = "Pinch from the globe to a person; drag to move. Tap a variant to change it."
	if "measure" in args:
		measure.call_deferred()


func _layout() -> void:
	var k := _screen_scale()
	var screen := Vector2(DisplayServer.window_get_size())
	if screen.x <= 0.0:
		screen = size * k
	_art.size = Vector2i(ceili(screen.x / BASE), ceili(screen.y / BASE))
	_view.size = Vector2(_art.size) * BASE / k
	(_view.material as ShaderMaterial).set_shader_parameter(
		"screen_size", Vector2(_art.size) * BASE
	)


## Screen pixels to one of the interface's units.
func _screen_scale() -> float:
	var window := DisplayServer.window_get_size()
	return window.x / size.x if size.x > 0.0 and window.x > 0 else 1.0


func _process(delta: float) -> void:
	if _scenario == RID():
		return
	_frame += 1
	if _measuring:
		_step_measure(delta)
	_collect()
	var width := metres_across(zoom)
	_weather += delta * clampf(width / 2000.0, 1.0, 400.0)
	if choice.time == 3:
		_day = fposmod(_day + delta / 300.0, 1.0)
	_place(width)
	_select()
	_show_pixels(width)
	if is_in_group("busy") and _tasks.is_empty() and _asked.is_empty() and _missing == 0:
		remove_from_group("busy")


## The metres the picture shows across at the focus, at a zoom.
static func metres_across(z: float) -> float:
	return exp(lerpf(log(STOPS[0][1]), log(STOPS[6][1]), clampf(z, 0.0, 1.0)))


## The zoom at a stop.
static func zoom_of(stop: String) -> float:
	for s: Array in STOPS:
		if s[0] == stop:
			return log(s[1] / STOPS[0][1]) / log(STOPS[6][1] / STOPS[0][1])
	return 1.0


## The stop nearest a zoom, for Measure's counts.
static func stop_at(z: float) -> String:
	var width := metres_across(z)
	var best: String = STOPS[0][0]
	var gap := INF
	for stop: Array in STOPS:
		var d := absf(log(width / float(stop[1])))
		if d < gap:
			gap = d
			best = stop[0]
	return best


## The camera's tilt down from level, degrees, by the path's variant: A looks down from the
## valley out, tilting to the art book's 30° at the camp; B, a flight, stays tilted higher up,
## showing the horizon and the clouds from the side, and looks straight down only at the globe.
static func pitch_at(width: float, path: int) -> float:
	var camp: float = STOPS[2][1]
	var top: float = STOPS[3][1] if path == 0 else STOPS[5][1]
	var t := clampf(log(width / camp) / log(top / camp), 0.0, 1.0)
	return lerpf(30.0, 90.0, t * t * (3.0 - 2.0 * t))


## The pixel shown, in the screen's pixels, by the pixels' variant: A the art book's 4 always; B
## from 2 at the person to 6 at the globe in whole steps; C the same, smoothly.
static func pixel_at(width: float, variant: int) -> float:
	if variant == 0:
		return 4.0
	var t := clampf(log(width / STOPS[0][1]) / log(STOPS[6][1] / STOPS[0][1]), 0.0, 1.0)
	var p := lerpf(2.0, 6.0, t)
	return float(roundi(p)) if variant == 1 else p


## The camera, the shared values every shader reads, and the sun (A8.4).
func _place(width: float) -> void:
	if _local(focus).length() > REBASE:
		_origin = focus
		for key: String in _chunks:
			_set_chunk_place(key)
	var at := _local(focus)
	var radius := world_m.x / TAU
	var h: PackedFloat32Array = gen.ground(0, focus.x, focus.y, 1, 1.0)
	var ground := maxf(h[0], 0.0) if h.size() > 0 and width < 200000.0 else 0.0
	var look := at + Vector3(0.0, ground, 0.0)
	var pitch := pitch_at(width, choice.path)
	var basis := Basis.from_euler(Vector3(deg_to_rad(-pitch), 0.0, 0.0))
	var distance := width * 0.5 / tan(deg_to_rad(FOV) * 0.5)
	_cam.transform = Transform3D(basis, look + basis.z * distance)
	_cam.near = maxf(distance * 0.01, 0.05)
	_cam.far = distance + radius * 4.0
	var rs := RenderingServer
	rs.global_shader_parameter_set("zoom_focus", focus)
	rs.global_shader_parameter_set("zoom_focus_at", at)
	rs.global_shader_parameter_set(
		"zoom_fine", Vector2(fposmod(focus.x, 4096.0), fposmod(focus.y, 4096.0))
	)
	# near the ground, east-west at its true scale round the focus; none of it from the region out
	var strength := clampf(log(120000.0 / width) / log(120000.0 / 12000.0), 0.0, 1.0)
	rs.global_shader_parameter_set("zoom_true", Vector2(strength, maxf(width * 4.0, 5000.0)))
	rs.global_shader_parameter_set("zoom_sun", sun_direction())
	rs.global_shader_parameter_set("zoom_clock", Vector4(_weather, _year, _day, 1.0))
	rs.global_shader_parameter_set(
		"zoom_style", Vector4(choice.land, choice.water, choice.clouds, choice.light)
	)


## Toward the sun in the picture, from the date, the time of day and the world's tilt: the sun's
## height over the focus by the hour angle and its declination (TIM-18).
func sun_direction() -> Vector3:
	var lat := PI * (focus.y / world_m.y - 0.5)
	var decl := deg_to_rad(_tilt) * sin(TAU * _year)
	var hour := TAU * (_day - 0.5)
	match choice.time:
		0:
			hour = 0.0
		1:
			# dusk: the sun 5° over the horizon in the west, the art book's
			var c := (sin(deg_to_rad(5.0)) - sin(lat) * sin(decl)) / (cos(lat) * cos(decl))
			hour = acos(clampf(c, -1.0, 1.0))
		2:
			hour = PI
	var east := -cos(decl) * sin(hour)
	var north := cos(lat) * sin(decl) - sin(lat) * cos(decl) * cos(hour)
	var up := sin(lat) * sin(decl) + cos(lat) * cos(decl) * cos(hour)
	return Vector3(east, up, -north).normalized()


## The pixel shown, and how: point or blended (A4).
func _show_pixels(width: float) -> void:
	var pixel := pixel_at(width, choice.pixels)
	var m := _view.material as ShaderMaterial
	m.set_shader_parameter("pixel", pixel)
	m.set_shader_parameter("blend", choice.pixels == 2)
	RenderingServer.global_shader_parameter_set("zoom_pixel", pixel / BASE)


## The chunk tree (A8.1): from the roots down, a chunk splits into its four while the camera is
## nearer than SPLIT of its sides and all four are made; those not yet made are asked for, nearest
## first, and their parent drawn meanwhile, so nothing is ever missing from the picture.
func _select() -> void:
	var cam := _cam.global_position
	var at := _local(focus)
	var ground := Vector2(focus.x + cam.x - at.x, focus.y - (cam.z - at.z))
	var want := []
	var shown := {}
	_missing = 0
	for j in 2:
		for i in 4:
			_visit(0, i, j, ground, cam.y, want, shown)
	for key: String in _drawn:
		if not shown.has(key) and _chunks.has(key):
			RenderingServer.instance_set_visible(_chunks[key].instance, false)
	for key: String in shown:
		if not _drawn.has(key):
			RenderingServer.instance_set_visible(_chunks[key].instance, true)
		_chunks[key].used = _frame
	_drawn = shown
	want.sort_custom(func(a: Array, b: Array) -> bool: return a[0] < b[0])
	for w: Array in want:
		if _tasks.size() >= MAKING:
			break
		if not _asked.has(w[1]):
			_ask(w[1], w[2], w[3], w[4])
	_forget_old()
	if _measuring and _area_asked >= 0.0 and _area_time < 0.0 and _missing == 0:
		_area_time = _clock - _area_asked


func _visit(
	depth: int, i: int, j: int, ground: Vector2, height: float, want: Array, shown: Dictionary
) -> void:
	var side := ROOT / float(1 << depth)
	var key := "%d:%d:%d" % [depth, i, j]
	if not _chunks.has(key):
		_missing += 1
		want.append([float(depth), key, depth, i, j])
		return
	var east := float(i) * side
	var north := float(j) * side
	var dx := _east_gap(ground.x, east, side)
	var dy := maxf(maxf(north - ground.y, ground.y - (north + side)), 0.0)
	var dist := sqrt(dx * dx + dy * dy + height * height)
	if depth < DEEPEST and dist < SPLIT * side:
		var ready := true
		for c in 4:
			var ci := i * 2 + c % 2
			var cj := j * 2 + c / 2
			var child := "%d:%d:%d" % [depth + 1, ci, cj]
			if not _chunks.has(child):
				ready = false
				_missing += 1
				if not _asked.has(child):
					want.append([dist, child, depth + 1, ci, cj])
		if ready:
			for c in 4:
				_visit(depth + 1, i * 2 + c % 2, j * 2 + c / 2, ground, height, want, shown)
			return
	shown[key] = true


## The gap east or west from a place to a span of the world, the shorter way round.
func _east_gap(x: float, east: float, side: float) -> float:
	var mid := east + side * 0.5 - x
	mid -= world_m.x * floorf(mid / world_m.x + 0.5)
	return maxf(absf(mid) - side * 0.5, 0.0)


func _ask(key: String, depth: int, i: int, j: int) -> void:
	var side := ROOT / float(1 << depth)
	_asked[key] = true
	var id := WorkerThreadPool.add_task(
		_make_chunk.bind(key, depth, float(i) * side, float(j) * side, side / float(N - 1))
	)
	_tasks[id] = key


## On a worker thread: a chunk's ground.
func _make_chunk(key: String, depth: int, east: float, north: float, spacing: float) -> void:
	var arrays: Array = gen.chunk(0, east, north, N, spacing)
	_done_lock.lock()
	_done.append([key, depth, east, north, spacing, arrays])
	_done_lock.unlock()


## The chunks the worker threads have made, given their meshes, hidden until the tree draws them.
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
		var made: Array = d[5]
		if made.size() < 4 or _chunks.has(key):
			continue
		var arrays := []
		arrays.resize(Mesh.ARRAY_MAX)
		arrays[Mesh.ARRAY_VERTEX] = made[0]
		arrays[Mesh.ARRAY_NORMAL] = made[1]
		arrays[Mesh.ARRAY_CUSTOM0] = made[2]
		arrays[Mesh.ARRAY_CUSTOM1] = made[3]
		arrays[Mesh.ARRAY_INDEX] = _indices
		var rs := RenderingServer
		var format := (
			(rs.ARRAY_CUSTOM_RGBA_FLOAT << rs.ARRAY_FORMAT_CUSTOM0_SHIFT)
			| (rs.ARRAY_CUSTOM_RGBA_FLOAT << rs.ARRAY_FORMAT_CUSTOM1_SHIFT)
		)
		var mesh := rs.mesh_create()
		rs.mesh_add_surface_from_arrays(mesh, rs.PRIMITIVE_TRIANGLES, arrays, [], {}, format)
		# placed by its shader on the sphere, so never culled by where its flat points lie
		var huge := AABB(Vector3(-4.0e6, -4.0e6, -4.0e6), Vector3(8.0e6, 8.0e6, 8.0e6))
		rs.mesh_set_custom_aabb(mesh, huge)
		var instance := rs.instance_create2(mesh, _scenario)
		rs.instance_geometry_set_material_override(instance, _ground.get_rid())
		var depth: int = d[1]
		var side := ROOT / float(1 << depth)
		rs.instance_geometry_set_shader_parameter(instance, "spacing", d[4])
		var reach := SPLIT * side * 2.0
		var morph := Vector2(reach * 0.7, reach) if depth > 0 else Vector2(1e12, 2e12)
		rs.instance_geometry_set_shader_parameter(instance, "morph", morph)
		rs.instance_set_visible(instance, false)
		_chunks[key] = {
			"mesh": mesh, "instance": instance, "east": d[2], "north": d[3], "used": _frame
		}
		_set_chunk_place(key)


## A chunk's place in the picture: its corner from the moving origin, east the shorter way round,
## north as it is, since the poles are the globe's edges.
func _set_chunk_place(key: String) -> void:
	var c: Dictionary = _chunks[key]
	var dx: float = c.east - _origin.x
	dx -= world_m.x * floorf(dx / world_m.x + 0.5)
	var dy: float = c.north - _origin.y
	RenderingServer.instance_set_transform(c.instance, Transform3D(Basis(), Vector3(dx, 0.0, -dy)))


## The chunks not drawn for longest, freed once more than KEEP are made.
func _forget_old() -> void:
	if _chunks.size() <= KEEP:
		return
	var old := []
	for key: String in _chunks:
		if not _drawn.has(key) and not key.begins_with("0:"):
			old.append([_chunks[key].used, key])
	old.sort()
	for k in mini(old.size(), _chunks.size() - KEEP):
		var key: String = old[k][1]
		RenderingServer.free_rid(_chunks[key].instance)
		RenderingServer.free_rid(_chunks[key].mesh)
		_chunks.erase(key)


## A place in the world's metres, in the picture's from the moving origin: x east the shorter way
## round, z south.
func _local(m: Vector2) -> Vector3:
	var d := m - _origin
	d.x -= world_m.x * floorf(d.x / world_m.x + 0.5)
	return Vector3(d.x, 0.0, -d.y)


## The triangles of a chunk of n × n points and the skirts hanging from its four edges.
static func chunk_indices(n: int) -> PackedInt32Array:
	var out := PackedInt32Array()
	for j in n - 1:
		for i in n - 1:
			var a := j * n + i
			out.append_array([a, a + n, a + 1, a + 1, a + n, a + n + 1])
	for e in 4:
		var skirt := n * n + e * n
		for k in n - 1:
			var g0 := edge_point(n, e, k)
			var g1 := edge_point(n, e, k + 1)
			out.append_array([g0, g1, skirt + k, g1, skirt + k + 1, skirt + k])
	return out


## The k-th point along a chunk's edge e: 0 south, 1 north, 2 west, 3 east.
static func edge_point(n: int, e: int, k: int) -> int:
	match e:
		0:
			return k
		1:
			return (n - 1) * n + k
		2:
			return k * n
	return k * n + n - 1


## Measure (PLT-04, PRE-03): from the globe to a person over the start region in DESCENT_SECONDS,
## a hold, and back, at 60 frames a second; each frame counted at its nearest stop.
func measure() -> void:
	if _scenario == RID() or _measuring:
		return
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
	_readout.text = "Measuring: from the globe to a person and back, about 45 seconds…"


func _step_measure(delta: float) -> void:
	_clock += delta
	_read_clock += delta
	if _read_clock >= RUNS.READ_EVERY:
		_read_clock = 0.0
		_heat.append(RUNS.thermal())
	var down := DESCENT_SECONDS
	if _clock < down:
		zoom = 1.0 - _clock / down
	elif _clock < down + HOLD_SECONDS:
		zoom = 0.0
	elif _clock < 2.0 * down + HOLD_SECONDS:
		zoom = (_clock - down - HOLD_SECONDS) / down
	else:
		_finish_measure()
		return
	if _area_asked < 0.0 and metres_across(zoom) <= STOPS[2][1]:
		_area_asked = _clock
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
	var letters := ""
	for name: String in VARIANTS:
		letters += (VARIANTS[name][choice[name]] as String).left(1)
	var runs := results(_samples, _late, _area_time)
	runs.append("variants " + letters)
	RUNS.finish(self, "P8", runs, _heat, 0)


## Measure's results for the chat: each stop's graphics time and frames on time, in the stops'
## order, and the seconds the ground's detail took to be in once the camp was reached.
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


## A pinch by a factor of the metres shown, as an even step of the zoom.
func _pinch(factor: float) -> void:
	zoom = clampf(zoom + log(factor) / log(STOPS[6][1] / STOPS[0][1]), 0.0, 1.0)


## A drag moves the ground under the finger; the focus stays between the poles.
func _pan(by: Vector2) -> void:
	var width := metres_across(zoom)
	var metres := width / maxf(size.x, 1.0)
	var pitch := deg_to_rad(pitch_at(width, choice.path))
	focus.x = fposmod(focus.x - by.x * metres, world_m.x)
	focus.y = clampf(focus.y + by.y * metres / sin(pitch), 0.0, world_m.y)


func _exit_tree() -> void:
	if _worker != null and _worker.is_started():
		_worker.wait_to_finish()
	for id: int in _tasks.keys():
		WorkerThreadPool.wait_for_task_completion(id)
	for key: String in _chunks:
		RenderingServer.free_rid(_chunks[key].instance)
		RenderingServer.free_rid(_chunks[key].mesh)
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
