## P8 The zoom (IMPLEMENTATION α0.5b), second round: one descent from space to a person over P7's
## world, a planet at every scale, never unrolled into a flat map (WLD-02). The ground is chunks in
## a tree of levels (chunks.gd), each made on a worker thread from the seed alone (WLD-13) and
## morphing smoothly into the next as the camera moves (CDLOD, A8.1); one shader colours every
## level the same way, so only detail changes; a pass over the picture adds the air's scattering
## and the clouds, which the camera passes through on its way down (A8.4, A8.6). The picture is
## drawn at the size of the pixels shown, which the variant sets (A4).
## Variants to compare (A/B): pixels, land, water, clouds, light and the camera's path, and the
## time of day. Measure descends from the globe to a person and back by itself, timing every stop.
## Once the world is made it starts in stages, the weather, the ground, then the sky, each noted in
## a file first, so if the phone stops the app, the zoom says where next time and starts in its
## light mode: no clouds, and the land and sea without their finest detail.
## Pre-production code (research 00): the app's README names its items.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const CHUNKS := preload("res://zoom/chunks.gd")
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
## The origin moves to the focus once the focus is this far from it (A8.2).
const REBASE := 2000.0
## The weather's picture of the whole world: 2 km a pixel, drawn every few frames.
const WEATHER_SIZE := Vector2i(1024, 512)
const WEATHER_EVERY := 4
## Where the zoom notes what it is starting, so a stop on the phone can be told next time.
const STAGE_FILE := "user://zoom_stage.txt"
const MAIN := preload("res://main.gd")
## The variants (A/B): each name, and what its letters mean, shown when one is chosen.
const VARIANTS := {
	"pixels": ["A fixed 4", "B stepped 2 to 6", "C smooth 2 to 6, blended"],
	"land": ["A the art book's map", "B vivid and lit", "C vivid, in clean steps"],
	"water": ["A the art book's bands", "B deep and shallow, currents", "C B with waves"],
	"clouds": ["A volumetric", "B volumetric, in clean steps", "C the art book's flat"],
	"light": ["A a glow at the rim", "B the air's haze too"],
	"path": ["A straight down", "B a flight"],
	"time": ["A morning", "B noon", "C dusk", "D night", "E the live hour"],
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

## Where the descent ends: by a river near the start region.
var _home := Vector2.ZERO
var _weather_view: SubViewport
## The light mode, after the phone stopped the app last time; what it was starting then; the
## start's stage (0 the world, 1 the weather, 2 the ground, 3 the sky, 4 running) and the frames
## it has waited in it.
var _light := false
var _stopped_at := ""
var _startup := 0
var _waited := 0
## Frames since the last note, and whether Measure waits for the start to finish.
var _settled := 0
var _measure_waiting := false
var _cloud_noise: ImageTexture3D
var _margin: MarginContainer
## The screen's pixels a drawn pixel takes.
var _texel := 0.0
var _art: SubViewport
var _view: TextureRect
var _cam: Camera3D
var _sky: MeshInstance3D
var _readout: Label
var _buttons := {}
var _measure_button: Button
var _scenario: RID
var _ground: ShaderMaterial
## The ground's chunk tree (chunks.gd), once the world is made.
var _tree: RefCounted
var _frame := 0
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
	_stopped_at = read_stage()
	_light = _stopped_at not in ["", "running", "closed"]
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
	show.set_shader_parameter("drawn", _art.get_texture())
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
	_stage("making the world")
	_worker = Thread.new()
	_worker.start(_make_world)


## The title, a line of what is happening, and the buttons, each with a short name and its letter,
## inside the safe area (A15): what a letter means shows on the line above when it is chosen.
func _build_controls() -> void:
	_margin = MarginContainer.new()
	_margin.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	_margin.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_margin)
	var column := VBoxContainer.new()
	column.mouse_filter = Control.MOUSE_FILTER_IGNORE
	column.add_theme_constant_override("separation", 6)
	_margin.add_child(column)
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
		b.clip_text = true
		b.add_theme_font_size_override("font_size", 14)
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
		b.text = "%s %s" % [name.capitalize(), (VARIANTS[name][choice[name]] as String).left(1)]
	if _light:
		(_buttons["clouds"] as Button).text = "Clouds off"


## A variant's next letter, and what it means on the line above. In the light mode the clouds'
## button turns the full zoom back on.
func _cycle(name: String) -> void:
	if name == "clouds" and _light:
		_light = false
		_readout.text = "The full zoom again: clouds %s." % VARIANTS.clouds[choice.clouds]
		if _startup >= 4:
			_use_shaders()
		_label_buttons()
		return
	choice[name] = (choice[name] + 1) % (VARIANTS[name] as Array).size()
	_label_buttons()
	_readout.text = "%s %s" % [name.capitalize(), VARIANTS[name][choice[name]]]
	if name == "clouds" and _startup >= 4:
		_use_shaders()


func _make_world() -> void:
	gen.make(THREADS)
	_world_made.call_deferred()


## The world's cells as textures, the ground's material, and the focus by a river near the start
## region; then the start goes on in stages (_start).
func _world_made() -> void:
	_worker.wait_to_finish()
	_stage("preparing the world's pictures")
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
	var water_cells: PackedByteArray = gen.water_texture(0)
	var water := Image.create_from_data(cells.x, cells.y, false, Image.FORMAT_RGBA8, water_cells)
	var water_soft := water.duplicate() as Image
	water_soft.generate_mipmaps()
	var rs := RenderingServer
	rs.global_shader_parameter_set("zoom_height", ImageTexture.create_from_image(heights))
	rs.global_shader_parameter_set("zoom_climate", ImageTexture.create_from_image(climate))
	rs.global_shader_parameter_set("zoom_cover", ImageTexture.create_from_image(cover))
	rs.global_shader_parameter_set("zoom_water", ImageTexture.create_from_image(water))
	rs.global_shader_parameter_set("zoom_water_soft", ImageTexture.create_from_image(water_soft))
	rs.global_shader_parameter_set("zoom_world", world_m)
	_ground = ShaderMaterial.new()
	_tree = CHUNKS.new(gen, world_m, _scenario, _ground)
	_home = river_near(gen.start(0), water_cells, cells, world_m)
	focus = _home
	_tree.rebase(focus)
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
	if _light:
		_readout.text = (
			(
				"Last time the phone stopped the zoom while %s, so it starts in its light mode: no"
				% _stopped_at
			)
			+ " clouds, and the land and sea without their finest detail. Tap Clouds off to try"
			+ " the full zoom again."
		)
	_measure_waiting = "measure" in args
	_startup = 1
	_waited = 0


## The start in stages, each noted before it begins (_stage): the weather's picture, then the
## ground, then the sky; and once all has run a while, that it runs.
func _start() -> void:
	_waited += 1
	match _startup:
		1:
			if _waited == 1 and not _light:
				_stage("drawing the weather")
				_make_weather()
			elif _waited >= 4:
				_startup = 2
				_waited = 0
		2:
			if _waited == 1:
				_stage("drawing the ground" + (" in the light mode" if _light else ""))
				_ground.shader = load(_ground_shader())
			elif _waited >= 4 and not _tree.drawn.is_empty():
				_startup = 3
				_waited = 0
		3:
			if _waited == 1:
				_make_sky()
			elif _waited >= 4:
				_startup = 4
				_waited = 0
				if _measure_waiting:
					_measure_waiting = false
					measure()


## The weather's picture of the whole world (A8.6), made once clouds are first shown, and drawn
## again every few frames as the weather moves.
func _make_weather() -> void:
	if _weather_view != null:
		return
	_weather_view = SubViewport.new()
	_weather_view.size = WEATHER_SIZE
	_weather_view.disable_3d = true
	_weather_view.transparent_bg = false
	_weather_view.render_target_update_mode = SubViewport.UPDATE_ONCE
	var sheet := ColorRect.new()
	sheet.size = Vector2(WEATHER_SIZE)
	var weather := ShaderMaterial.new()
	weather.shader = load("res://zoom/shaders/weather.gdshader")
	sheet.material = weather
	_weather_view.add_child(sheet)
	add_child(_weather_view)
	RenderingServer.global_shader_parameter_set("zoom_weather", _weather_view.get_texture())


## The clouds' noise and its smaller copies, its mipmaps, as a 3D texture, made once the volumetric
## clouds are first shown.
func _make_cloud_noise() -> void:
	if _cloud_noise != null:
		return
	var noise_size := 64
	var noise: PackedByteArray = gen.cloud_noise(noise_size)
	var layers: Array[Image] = []
	var start := 0
	var n := noise_size
	while n >= 1:
		var slab := n * n * 4
		for z in n:
			layers.append(
				Image.create_from_data(
					n,
					n,
					false,
					Image.FORMAT_RGBA8,
					noise.slice(start + z * slab, start + (z + 1) * slab)
				)
			)
		start += n * slab
		n /= 2
	_cloud_noise = ImageTexture3D.new()
	_cloud_noise.create(Image.FORMAT_RGBA8, noise_size, noise_size, noise_size, true, layers)
	RenderingServer.global_shader_parameter_set("zoom_cloud_noise", _cloud_noise)


## The sky's pass: a quad over the whole picture, drawn after the ground.
func _make_sky() -> void:
	_sky = MeshInstance3D.new()
	var quad := QuadMesh.new()
	quad.size = Vector2(1.0, 1.0)
	# it covers the picture whatever the camera's near plane, so it must never be culled by it
	quad.custom_aabb = AABB(Vector3(-1.0e8, -1.0e8, -1.0e8), Vector3(2.0e8, 2.0e8, 2.0e8))
	_sky.mesh = quad
	var sky := ShaderMaterial.new()
	sky.render_priority = -100
	_sky.material_override = sky
	_sky.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	_cam.add_child(_sky)
	_sky.position = Vector3(0.0, 0.0, -2.0)
	_use_shaders()


## The ground's and the sky's shaders for the mode and the clouds' variant: the volumetric sky only
## when its clouds are shown, the light ones in the light mode; the weather's picture and the
## clouds' noise made as they are first needed; each change noted first, as the start's are.
func _use_shaders() -> void:
	var volume: bool = not _light and choice.clouds < 2
	if not _light:
		_make_weather()
	if volume:
		_make_cloud_noise()
	var sky := "res://zoom/shaders/sky.gdshader"
	if _light:
		sky = "res://zoom/shaders/sky_air.gdshader"
		_stage("drawing the sky in the light mode")
	elif volume:
		sky = "res://zoom/shaders/sky_volume.gdshader"
		_stage("drawing the volumetric clouds")
	else:
		_stage("drawing the flat clouds")
	_ground.shader = load(_ground_shader())
	(_sky.material_override as ShaderMaterial).shader = load(sky)


func _ground_shader() -> String:
	if _light:
		return "res://zoom/shaders/ground_light.gdshader"
	return "res://zoom/shaders/ground.gdshader"


## What the zoom last noted it was starting, or "" if it has noted nothing yet.
static func read_stage() -> String:
	if not FileAccess.file_exists(STAGE_FILE):
		return ""
	var f := FileAccess.open(STAGE_FILE, FileAccess.READ)
	return f.get_as_text().strip_edges() if f != null else ""


## Notes what the zoom is starting, before it starts it; closing the file hands it to the system,
## so it is there even if the phone stops the app at once.
func _stage(text: String) -> void:
	var f := FileAccess.open(STAGE_FILE, FileAccess.WRITE)
	if f != null:
		f.store_string(text)
		f.close()
	_settled = 0


func _layout() -> void:
	_fit(_texel if _texel > 0.0 else 2.0)
	var inset := MAIN.insets(size)
	_margin.add_theme_constant_override("margin_left", GAP + inset.x)
	_margin.add_theme_constant_override("margin_top", GAP + inset.y)
	_margin.add_theme_constant_override("margin_right", GAP + inset.z)
	_margin.add_theme_constant_override("margin_bottom", GAP + inset.w)


## The picture drawn with a pixel for every `texel` of the screen's, and shown over the screen.
func _fit(texel: float) -> void:
	var k := _screen_scale()
	var screen := Vector2(DisplayServer.window_get_size())
	if screen.x <= 0.0:
		screen = size * k
	_texel = texel
	var drawn := Vector2i(ceili(screen.x / texel), ceili(screen.y / texel))
	if _art.size != drawn:
		_art.size = drawn
	_view.size = Vector2(drawn) * texel / k
	var m := _view.material as ShaderMaterial
	m.set_shader_parameter("drawn_size", Vector2(drawn))
	m.set_shader_parameter("texel", texel)


## Screen pixels to one of the interface's units.
func _screen_scale() -> float:
	var window := DisplayServer.window_get_size()
	return window.x / size.x if size.x > 0.0 and window.x > 0 else 1.0


func _process(delta: float) -> void:
	if _scenario == RID():
		return
	_frame += 1
	if _startup < 4:
		_start()
	else:
		_settled += 1
		if _settled == 90:
			_stage("running")
	if _measuring:
		_step_measure(delta)
	_tree.collect()
	var width := metres_across(zoom)
	_weather += delta * clampf(width / 2000.0, 1.0, 400.0)
	if choice.time == 4:
		_day = fposmod(_day + delta / 300.0, 1.0)
	if _weather_view != null and _frame % WEATHER_EVERY == 0:
		_weather_view.render_target_update_mode = SubViewport.UPDATE_ONCE
	_place(width)
	if _startup >= 2:
		_tree.select(_cam, focus)
		if _measuring and _area_asked >= 0.0 and _area_time < 0.0 and _tree.missing == 0:
			_area_time = _clock - _area_asked
	_show_pixels(width)
	if (
		is_in_group("busy")
		and _tree.tasks.is_empty()
		and _tree.asked.is_empty()
		and _tree.missing == 0
	):
		remove_from_group("busy")


## Where the descent ends (PRE-03): the nearest place to `start` beside a river that drains at least
## 300 km², within 25 km, so the close stops show water as the art book's camp does; the start
## itself if there is none. `water` is the world's water cells (WorldGen.water_texture).
static func river_near(
	start: Vector2, water: PackedByteArray, cells: Vector2i, world: Vector2
) -> Vector2:
	var cell := world.x / float(cells.x)
	var cx := int(start.x / cell)
	var cy := int(start.y / cell)
	var least := int(16.0 * log(300.0) / log(2.0))
	var best := start
	var gap := INF
	for dy in range(-25, 26):
		for dx in range(-25, 26):
			var x := posmod(cx + dx, cells.x)
			var y := cy + dy
			if y < 0 or y >= cells.y:
				continue
			var k := (y * cells.x + x) * 4
			if water[k] > 7 or water[k + 1] < least:
				continue
			var d := float(dx * dx + dy * dy)
			if d < gap:
				gap = d
				best = Vector2((float(cx + dx) + 0.5) * cell, (float(y) + 0.5) * cell)
	best.x = fposmod(best.x, world.x)
	return best


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


## The screen's pixels a drawn pixel takes, by the pixels' variant (A4): A and B draw one for each
## pixel shown, crisp; C draws two each way for each, never finer than two of the screen's, and
## shows their average, so small things blend into the pixels as they grow. In steps of a
## twentieth, so the drawn picture is not made anew at every frame of a pinch.
static func texel_at(pixel: float, variant: int) -> float:
	if variant < 2:
		return pixel
	return snappedf(maxf(pixel * 0.5, 2.0), 0.05)


## The camera, the shared values every shader reads, and the sun (A8.4).
func _place(width: float) -> void:
	if _tree.local(focus).length() > REBASE:
		_tree.rebase(focus)
	var at: Vector3 = _tree.local(focus)
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
	var lat := PI * (focus.y / world_m.y - 0.5)
	_tree.stretch = strength * (1.0 / maxf(cos(lat), 0.33) - 1.0)
	_tree.reach = maxf(width * 4.0, 5000.0)
	rs.global_shader_parameter_set("zoom_true", Vector2(_tree.stretch, _tree.reach))
	rs.global_shader_parameter_set("zoom_sun", sun_direction())
	var declination := deg_to_rad(_tilt) * sin(TAU * _year)
	rs.global_shader_parameter_set("zoom_clock", Vector4(_weather, _year, _day, declination))
	rs.global_shader_parameter_set(
		"zoom_style", Vector4(choice.land, choice.water, choice.clouds, choice.light)
	)


## Toward the sun in the picture, from the date, the time of day and the world's tilt: the sun's
## height over the focus by the hour angle and its declination (TIM-18). Morning puts the sun 35°
## up in the east, so the land's relief and the clouds' heaps show; dusk 5° up in the west, the art
## book's; night behind the planet; live, the day's own hour, moving.
func sun_direction() -> Vector3:
	var lat := PI * (focus.y / world_m.y - 0.5)
	var decl := deg_to_rad(_tilt) * sin(TAU * _year)
	var hour := TAU * (_day - 0.5)
	match choice.time:
		0:
			hour = -_hour_at_height(lat, decl, 35.0)
		1:
			hour = 0.0
		2:
			hour = _hour_at_height(lat, decl, 5.0)
		3:
			hour = PI
	var east := -cos(decl) * sin(hour)
	var north := cos(lat) * sin(decl) - sin(lat) * cos(decl) * cos(hour)
	var up := sin(lat) * sin(decl) + cos(lat) * cos(decl) * cos(hour)
	return Vector3(east, up, -north).normalized()


## The hour angle, radians after noon, at which the sun stands `degrees` over the horizon.
static func _hour_at_height(lat: float, decl: float, degrees: float) -> float:
	var c := (sin(deg_to_rad(degrees)) - sin(lat) * sin(decl)) / (cos(lat) * cos(decl))
	return acos(clampf(c, -1.0, 1.0))


## The pixel shown, and the picture drawn for it (A4).
func _show_pixels(width: float) -> void:
	var pixel := pixel_at(width, choice.pixels)
	var texel := texel_at(pixel, choice.pixels)
	if texel != _texel:
		_fit(texel)
	(_view.material as ShaderMaterial).set_shader_parameter("pixel", pixel)
	RenderingServer.global_shader_parameter_set("zoom_pixel", pixel / texel)


## Measure (PLT-04, PRE-03): from the globe to a person over the start region in DESCENT_SECONDS,
## a hold, and back, at 60 frames a second; each frame counted at its nearest stop.
func measure() -> void:
	if _scenario == RID() or _measuring or _startup < 4:
		return
	focus = _home
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
	runs.append("variants " + letters + (" light mode" if _light else ""))
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
	_stage("closed")
	if _worker != null and _worker.is_started():
		_worker.wait_to_finish()
	if _tree != null:
		_tree.free_all()
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
