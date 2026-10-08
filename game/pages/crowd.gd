## The crowd (PLT-01, TIM-01, WLD-13, PLT-07): the demonstration's 400 camps of 25 markers walking,
## meeting and greeting on the simulation's own thread, drawn from its newest snapshot at the
## screen's game time, with the Time page's speeds and the counters. Drag to move, pinch or scroll
## to zoom, tap a camp to call it home. It opens the world chosen on the Worlds page, kept on the
## phone: saved every 30 seconds and whenever the app leaves the screen, it opens again where it
## was, and warns when the phone is nearly full. Implements PLT-01, TIM-01, WLD-13, PLT-07, TIM-08
## and PLT-10.
extends VBoxContainer

## The crowd's square is drawn in areas, ACROSS by ACROSS, each its own MultiMesh with its own
## bounding box, so what is off the screen is never drawn (A3.8).
const ACROSS := 4
## A walker is at least this many pixels across, so the crowd shows when zoomed out, and at
## least this many metres when close.
const MARKER_PIXELS := 3.0
const MARKER_METRES := 1.0
## The screen's height in metres: at first, closest and farthest.
const FIRST_VIEW := 3000.0
const CLOSEST := 40.0
const FARTHEST := 30000.0
## A touch that moves less than this many pixels is a tap, and reaches a camp this many away.
const TAP_SLOP := 12.0
const TAP_REACH := 40.0
## The benchmark's camera tour (A18.1): the view's centre circles the crowd's middle this many
## metres out, once in TOUR_CIRCLE real seconds; it zooms from closest to farthest and back in
## TOUR_ZOOM, and turns a full circle in TOUR_TURN.
const TOUR_RADIUS := 1500.0
const TOUR_CIRCLE := 60.0
const TOUR_ZOOM := 40.0
const TOUR_TURN := 90.0
const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const DAY_GROUND := Color("#4f5b3c")
const NIGHT_GROUND := Color("#1e2433")

## The folder the worlds are kept in, and the one this world is kept in, the current world's
## unless a test sets its own before the page opens.
var root := Worlds.ROOT
var folder := ""
## What opening the world found (KdWorld.open_crowd).
var opened := {}
var world: KdWorld
var crowd: KdCrowd
var device: KdDevice
var bar: SpeedBar
## The areas' and the camps' MultiMeshes, as the crowd draws into them.
var areas: Array[MultiMesh] = []
var camps: MultiMesh
## What the colours mean.
var legend: RichTextLabel
## The view's centre, in world centimetres: the origin everything is drawn from (A8.2).
var focus_east := 0
var focus_north := 0
## The phone's last heat forecast, as a share of its first throttling level, or -1 before one; the
## benchmark reads it here, so the phone is not asked more often than it allows.
var forecast := -1.0

var _compact: Label
var _pause_button: Button
var _drawer: ScrollContainer
var _clock: Label
var _counters: Label
var _pin: CheckButton
var _view_box: SubViewportContainer
var _view: SubViewport
var _camera: Camera3D
var _ground: StandardMaterial3D
var _touches := {}
var _pinch := 0.0
var _press := Vector2.ZERO
var _dragged := false
var _called := ""
var _heat_every := 2.0
var _heat_wait := 0.0
var _greetings := 0
var _last_greeting := -1
var _problems := PackedStringArray()
var _name := ""
var _test := ""


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 8)
	_clock = _label(16, TEXT)
	_counters = _label(15, QUIET)
	world = KdWorld.new()
	device = KdDevice.new()
	var loaded := GameData.load_into(world)
	_problems = loaded.get("problems", PackedStringArray())
	_add_legend(loaded)
	_build_view()
	bar = SpeedBar.new()
	add_child(bar)
	bar.setup(world, _problems)
	_pin = CheckButton.new()
	_pin.text = "Pin the world to the middle cores"
	_pin.toggled.connect(_set_pinned)
	add_child(_pin)
	_compose()
	if not _problems.is_empty() or world.entry("tuning/crowd", "demo:crowd").is_empty():
		_counters.text = "The crowd did not load: %s" % "; ".join(_problems)
		set_process(false)
		return
	var heat := world.entry("tuning/heat", "base:heat")
	_heat_every = float(heat.get("reading", 2))
	var worlds := Worlds.at(root)
	if folder == "":
		var id := Worlds.current_id(worlds)
		for saved: Dictionary in worlds.list():
			if saved.id == id and saved.get("kind") == "camp_alpha":
				id = "marker-fixture"
		folder = root.path_join(id)
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	# the crowd's world, the same on every phone, from the simulation's own seed
	opened = world.open_crowd(
		ProjectSettings.globalize_path(folder), KdWorld.crowd_seed(), 0, version
	)
	for listed: Dictionary in worlds.list():
		if listed["id"] == folder.get_file():
			_name = Worlds.name_of(listed)
			_test = Worlds.test_words(listed)
	if opened.has("problem"):
		_counters.text = "The crowd's world did not open: %s" % opened["problem"]
		set_process(false)
		return
	if opened["made"]:
		# the page opens on the morning, when the markers wake
		world.begin_at(KdWorld.morning())
	var square := world.crowd_square()
	focus_east = int(square["west"]) + int(square["side"]) / 2
	focus_north = int(square["south"]) + int(square["side"]) / 2
	crowd = KdCrowd.new()
	crowd.set_world(world)
	var rids := []
	for area: MultiMesh in areas:
		rids.append(area.get_rid())
	crowd.set_areas(rids, ACROSS)
	crowd.set_camps(camps.get_rid())
	bar.choose_speed(0)
	_draw_crowd()


func _notification(what: int) -> void:
	# the world is saved as the app leaves the screen, well before Android may freeze it (A3.7)
	if what == NOTIFICATION_APPLICATION_PAUSED and world != null:
		world.save_now()


func _exit_tree() -> void:
	if world != null:
		world.save_now()


func _process(delta: float) -> void:
	world.frame()
	_draw_crowd()
	_read_heat(delta)
	var greetings := world.drain_greetings()
	if greetings.size() >= 3:
		_greetings += greetings.size() / 3
		_last_greeting = greetings[greetings.size() - 3]
	_show()


## Metres a pixel of the view covers now.
func metres_per_pixel() -> float:
	return _camera.size / maxf(1.0, float(_view.size.y))


func _draw_crowd() -> void:
	var size := maxf(MARKER_METRES, MARKER_PIXELS * metres_per_pixel())
	crowd.draw(world.screen_time(), focus_east, focus_north, size)
	_ground.albedo_color = NIGHT_GROUND if world.night_at(world.screen_time()) else DAY_GROUND


func _read_heat(delta: float) -> void:
	_heat_wait -= delta
	if _heat_wait > 0.0:
		return
	_heat_wait = _heat_every
	var thermal := device.thermal()
	# time slows a margin below the phone's own light throttling level, where it gives one (A3.9)
	if thermal.has("light"):
		world.set_heat_light(float(thermal["light"]))
	if thermal.get("available", false) and not is_nan(float(thermal["forecast_10s"])):
		forecast = float(thermal["forecast_10s"])
		world.heat_reading(forecast)


func _set_pinned(on: bool) -> void:
	var cores := device.middle_cores() if on else PackedInt32Array()
	world.set_pinned(cores)
	if on and cores.is_empty():
		_pin.text = "Pin the world to the middle cores (this one has none)"


func _show() -> void:
	_clock.text = "%s, %s" % [world.date_text(), world.time_text()]
	var c := world.counters()
	var lines := PackedStringArray()
	if _name != "":
		lines.append(_name)
	if _test != "":
		lines.append(_test)
	if opened.get("update", "none") == "small":
		lines.append("Saved by an earlier version, this world carries on under this one")
	var warning := Worlds.space_warning(c.get("free_mb", -1), c.get("warn_below_mb", 0))
	if warning != "":
		lines.append(warning)
	if c.get("save_failed", false):
		(
			lines
			. append(
				(
					"The world stopped: the phone could not save it. Free some space, then open it again"
					+ " from Worlds; it opens where it was last saved"
				)
			)
		)
	if world.catching_up():
		lines.append(
			"Catching up to where the world was, %s" % KdWorld.moment_text(opened["was_at"])
		)
	lines.append(
		(
			"%s walkers on one core, %s events a second, a batch in %.1f ms"
			% [
				Worlds.count_words(c["walkers"]),
				Worlds.count_words(roundi(c["events_per_second"])),
				c["batch_ms"]
			]
		)
	)
	var asked := "Asked: %s" % SpeedBar.speed_words(c["speed"], false)
	if c["speed"] > c["limit"]:
		asked += ", more than this phone can"
	lines.append(asked)
	if c["share"] < 1.0:
		lines.append(
			"Time slowed to keep the phone cool: %d%% of its work" % roundi(c["share"] * 100.0)
		)
	var last := ""
	if _last_greeting >= 0:
		last = ", the last at %s" % _clock_of(_last_greeting)
	lines.append("Greetings: %s%s" % [Worlds.count_words(_greetings), last])
	if _called != "":
		lines.append(_called)
	_counters.text = "\n".join(lines)
	bar.show_speed()
	if _compact != null:
		_compact.text = (
			"%s walkers · %s greetings"
			% [Worlds.count_words(c.walkers), Worlds.count_words(_greetings)]
		)
		if warning != "" or c.get("save_failed", false):
			_compact.text += "\n" + (warning if warning != "" else "Saving failed · open Details")
		_pause_button.text = "Play" if world.is_paused() else "Pause"


## A game second's hour of the day, "06:05".
static func _clock_of(second: int) -> String:
	var of_day := posmod(second, 86400)
	return "%02d:%02d" % [of_day / 3600, (of_day / 60) % 60]


func _build_view() -> void:
	_view_box = SubViewportContainer.new()
	_view_box.stretch = true
	_view_box.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_view_box.gui_input.connect(_on_view_input)
	add_child(_view_box)
	_view = SubViewport.new()
	_view.own_world_3d = true
	_view_box.add_child(_view)
	_camera = Camera3D.new()
	_camera.projection = Camera3D.PROJECTION_ORTHOGONAL
	_camera.keep_aspect = Camera3D.KEEP_HEIGHT
	_camera.size = FIRST_VIEW
	_camera.near = 1.0
	_camera.far = 4000.0
	_camera.position = Vector3(0.0, 2000.0, 0.0)
	_camera.rotation_degrees = Vector3(-90.0, 0.0, 0.0)
	_view.add_child(_camera)
	var ground := MeshInstance3D.new()
	var plane := PlaneMesh.new()
	plane.size = Vector2(4.0 * FARTHEST, 4.0 * FARTHEST)
	ground.mesh = plane
	_ground = _flat(DAY_GROUND)
	ground.material_override = _ground
	_view.add_child(ground)
	# one square for every walker and camp, coloured by each instance
	var mark := PlaneMesh.new()
	mark.size = Vector2.ONE
	var colours := _flat(Color.WHITE)
	colours.vertex_color_use_as_albedo = true
	mark.material = colours
	for i in ACROSS * ACROSS:
		areas.append(_multimesh(mark))
	camps = _multimesh(mark)


func _multimesh(mesh: Mesh) -> MultiMesh:
	var multimesh := MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D
	multimesh.use_colors = true
	multimesh.mesh = mesh
	var node := MultiMeshInstance3D.new()
	node.multimesh = multimesh
	_view.add_child(node)
	return multimesh


func _flat(colour: Color) -> StandardMaterial3D:
	var material := StandardMaterial3D.new()
	material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	material.albedo_color = colour
	return material


## What the colours mean, each kind of marker in its colour from the catalogue, and how to move.
func _add_legend(loaded: Dictionary) -> void:
	var words := PackedStringArray()
	for kind: Dictionary in loaded.get("kinds", []):
		if kind["folder"] != "marker":
			continue
		for entry: Dictionary in kind["entries"]:
			var entry_name: String = entry["name"]
			var colour: String = world.entry("marker", entry_name).get("colour", "#ffffff")
			words.append("[bgcolor=%s]   [/bgcolor] %s" % [colour, entry_name.get_slice(":", 1)])
	words.append("flashing: greeting")
	words.append("dim: asleep")
	words.append("large: camp, tap to call it home")
	legend = RichTextLabel.new()
	legend.bbcode_enabled = true
	legend.fit_content = true
	legend.scroll_active = false
	legend.add_theme_font_size_override("normal_font_size", 15)
	legend.add_theme_color_override("default_color", QUIET)
	legend.text = "[center]%s\nDrag to move, pinch to zoom[/center]" % ",  ".join(words)
	add_child(legend)


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label


## Drag to move the view, pinch or scroll to zoom; a touch also arrives as a mouse, which is
## left out.
func _on_view_input(event: InputEvent) -> void:
	if event is InputEventScreenTouch:
		if event.pressed:
			_touches[event.index] = event.position
			if _touches.size() == 1:
				_press = event.position
				_dragged = false
		else:
			_touches.erase(event.index)
			if _touches.is_empty() and not _dragged:
				tap(event.position)
		_pinch = _spread()
	elif event is InputEventScreenDrag:
		_touches[event.index] = event.position
		_dragged = _dragged or event.position.distance_to(_press) > TAP_SLOP
		if _touches.size() >= 2:
			var spread := _spread()
			if _pinch > 0.0 and spread > 0.0:
				zoom_by(_pinch / spread)
			_pinch = spread
		else:
			pan_by(event.relative)
	elif event is InputEventMouseMotion and event.device != InputEvent.DEVICE_ID_EMULATION:
		if event.button_mask & MOUSE_BUTTON_MASK_LEFT:
			pan_by(event.relative)
	elif event is InputEventMouseButton and event.device != InputEvent.DEVICE_ID_EMULATION:
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				_press = event.position
				_dragged = false
			elif event.position.distance_to(_press) <= TAP_SLOP:
				tap(event.position)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_UP:
			zoom_by(1.0 / 1.15)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			zoom_by(1.15)


## A tap at a point of the view: the camp nearest it, within a finger's width, is called home.
func tap(at: Vector2) -> void:
	var east := focus_east + roundi((at.x - _view.size.x / 2.0) * metres_per_pixel() * 100.0)
	var north := focus_north - roundi((at.y - _view.size.y / 2.0) * metres_per_pixel() * 100.0)
	var within := roundi(TAP_REACH * metres_per_pixel() * 100.0)
	var camp := world.nearest_camp(east, north, within)
	if camp >= 0:
		world.call_home(camp)
		_called = "Camp %d called home at %s" % [camp + 1, world.time_text()]


## The benchmark's camera at a real second into its tour: circling, zooming and turning, so every
## area is drawn near and far, and none is left out for long. Implements PLT-04.
func tour(t: float) -> void:
	var square := world.crowd_square()
	var around := TAU * t / TOUR_CIRCLE
	focus_east = (
		int(square["west"]) + int(square["side"]) / 2 + roundi(cos(around) * TOUR_RADIUS * 100.0)
	)
	focus_north = (
		int(square["south"]) + int(square["side"]) / 2 + roundi(sin(around) * TOUR_RADIUS * 100.0)
	)
	# evenly from closest to farthest and back, as a pinch would
	var zoom := 0.5 - 0.5 * cos(TAU * t / TOUR_ZOOM)
	_camera.size = CLOSEST * pow(FARTHEST / CLOSEST, zoom)
	_camera.rotation_degrees = Vector3(-90.0, 360.0 * fmod(t / TOUR_TURN, 1.0), 0.0)


## Moves the view by a drag of some pixels: the ground follows the finger.
func pan_by(pixels: Vector2) -> void:
	var metres := pixels * metres_per_pixel()
	focus_east -= roundi(metres.x * 100.0)
	focus_north += roundi(metres.y * 100.0)


## Zooms out by a factor, or in when it is below 1.
func zoom_by(factor: float) -> void:
	_camera.size = clampf(_camera.size * factor, CLOSEST, FARTHEST)


func _spread() -> float:
	if _touches.size() < 2:
		return 0.0
	var points: Array = _touches.values()
	return (points[0] as Vector2).distance_to(points[1])


func _compose() -> void:
	var heading := _label(22, TEXT)
	heading.text = "People test · Simulation markers"
	move_child(heading, 0)
	move_child(_clock, 1)
	move_child(_view_box, 2)
	_compact = _label(16, TEXT)
	move_child(_compact, 3)
	var dock := HBoxContainer.new()
	add_child(dock)
	move_child(dock, 4)
	_drawer = ScrollContainer.new()
	_drawer.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_drawer.custom_minimum_size.y = 160
	_drawer.hide()
	add_child(_drawer)
	var details := VBoxContainer.new()
	details.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_drawer.add_child(details)
	for control: Control in [bar, _counters, legend, _pin]:
		control.reparent(details)
	bar._pause.hide()
	for label: String in ["Pause", "Speed", "Details"]:
		var button := Button.new()
		button.text = label
		button.custom_minimum_size.y = 48
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		dock.add_child(button)
		if label == "Pause":
			_pause_button = button
			button.pressed.connect(bar.toggle_pause)
		else:
			button.pressed.connect(_open_drawer.bind(label))


func _open_drawer(which: String) -> void:
	var speed := which == "Speed"
	var same := _drawer.visible and bar.visible == speed
	_drawer.visible = not same
	bar.visible = speed
	for control: Control in [_counters, legend, _pin]:
		control.visible = not speed
