## P8's ground as a tree of chunks (A8.1, CDLOD): 4 × 2 roots of 500 km, each splitting into four
## down to chunks 30.5 m across, every chunk N points a side, made on Godot's worker threads from
## the seed alone (WLD-13) and drawn through the RenderingServer with one material, which places
## each on the sphere. A chunk splits while the camera is nearer than SPLIT of its sides, and
## morphs into its parent over the last third of its parent's reach.
## Pre-production code (research 00): the app's README names its items.
extends RefCounted

const ROOT := 500000.0
const DEEPEST := 14
const N := 33
const SPLIT := 2.4
## The most chunks being made at once, and the most kept made, drawn or not.
const MAKING := 6
const KEEP := 900

var gen: RefCounted
var world_m := Vector2(2.0e6, 1.0e6)
var scenario: RID
var material: ShaderMaterial
## The origin the picture's coordinates start from, which moves with the focus (A8.2), and the
## frames the tree has chosen.
var origin := Vector2.ZERO
var frame := 0
## Each chunk made, by its key (depth << 40 | i << 20 | j): its mesh and instance, its corner, the
## lowest and highest it is drawn, and the frame the tree last reached it; those asked for and the
## worker threads' tasks making them; those drawn; and how many the picture lacks.
var chunks := {}
var asked := {}
var tasks := {}
var drawn := {}
var missing := 0
## The east-west stretch round the focus, less one, and how far it reaches (zoom_true), which the
## culling bounds.
var stretch := 0.0
var reach := 5000.0

var _indices := PackedInt32Array()
var _done := []
var _done_lock := Mutex.new()
## This frame's camera planes, the focus in the picture, the focus's latitude's cosine, the
## camera's angle round the planet from the focus and how far round its horizon lies.
var _planes: Array[Plane] = []
var _at := Vector3.ZERO
var _cos_focus := 1.0
var _under := 0.0
var _horizon := -1.0


func _init(generator: RefCounted, world: Vector2, scene: RID, ground: ShaderMaterial) -> void:
	gen = generator
	world_m = world
	scenario = scene
	material = ground
	_indices = chunk_indices(N)


## Moves the origin to `to`, and every chunk's place in the picture with it.
func rebase(to: Vector2) -> void:
	origin = to
	for key: int in chunks:
		_set_place(key)


## Waits for the chunks being made and frees every one, once the zoom closes.
func free_all() -> void:
	for id: int in tasks.keys():
		WorkerThreadPool.wait_for_task_completion(id)
	for key: int in chunks:
		RenderingServer.free_rid(chunks[key].instance)
		RenderingServer.free_rid(chunks[key].mesh)
	chunks.clear()


## The chunk tree (A8.1): from the roots down, a chunk the camera can see splits into its four
## while the camera is nearer than SPLIT of its sides and all four are made; those not yet made are
## asked for, nearest first, and their parent drawn meanwhile, so nothing is ever missing from the
## picture. A chunk the camera cannot see is neither drawn nor split.
func select(camera: Camera3D, focus: Vector2) -> void:
	frame += 1
	var cam := camera.global_position
	_at = local(focus)
	_planes = camera.get_frustum()
	var radius := world_m.x / TAU
	_cos_focus = cos(PI * (focus.y / world_m.y - 0.5))
	# the camera's angle round the planet from the focus, and how far round its horizon lies
	var from_middle := cam - (_at - Vector3(0.0, radius, 0.0))
	var eye := from_middle.length()
	_under = acos(clampf(from_middle.y / eye, -1.0, 1.0))
	_horizon = acos(radius / eye) if eye > radius else INF
	var ground := Vector2(focus.x + cam.x - _at.x, focus.y - (cam.z - _at.z))
	var want := []
	var shown := {}
	missing = 0
	for j in 2:
		for i in 4:
			_visit(0, i, j, ground, cam.y, want, shown)
	for key: int in drawn:
		if not shown.has(key) and chunks.has(key):
			RenderingServer.instance_set_visible(chunks[key].instance, false)
	for key: int in shown:
		if not drawn.has(key):
			RenderingServer.instance_set_visible(chunks[key].instance, true)
	drawn = shown
	want.sort_custom(func(a: Array, b: Array) -> bool: return a[0] < b[0])
	for w: Array in want:
		if tasks.size() >= MAKING:
			break
		if not asked.has(w[1]):
			_ask(w[1], w[2], w[3], w[4])
	_forget_old()


func _visit(
	depth: int, i: int, j: int, ground: Vector2, height: float, want: Array, shown: Dictionary
) -> void:
	var side := ROOT / float(1 << depth)
	var key := (depth << 40) | (i << 20) | j
	if not chunks.has(key):
		missing += 1
		want.append([float(depth), key, depth, i, j])
		return
	var chunk: Dictionary = chunks[key]
	chunk.used = frame
	if not _seen(chunk, side):
		return
	var east := float(i) * side
	var north := float(j) * side
	# the camera's distance from the chunk's box, its heights included, as the shader measures the
	# morph from each point: measured from the sea instead, chunks on high ground split too late, and
	# their edges had not finished morphing into the coarser chunks beside them
	var dx := _east_gap(ground.x, east, side)
	var dy := maxf(maxf(north - ground.y, ground.y - (north + side)), 0.0)
	var dz := maxf(maxf(height - chunk.high, chunk.low - height), 0.0)
	var dist := sqrt(dx * dx + dy * dy + dz * dz)
	if depth < DEEPEST and dist < SPLIT * side:
		var ready := true
		for c in 4:
			var ci := i * 2 + c % 2
			var cj := j * 2 + c / 2
			var child := ((depth + 1) << 40) | (ci << 20) | cj
			if chunks.has(child):
				chunks[child].used = frame  # kept while it waits for the others
			else:
				ready = false
				missing += 1
				if not asked.has(child):
					want.append([dist, child, depth + 1, ci, cj])
		if ready:
			for c in 4:
				_visit(depth + 1, i * 2 + c % 2, j * 2 + c / 2, ground, height, want, shown)
			return
	shown[key] = true


## Whether any of a chunk can be in the picture. First, whether it lies behind the planet's
## horizon as the camera sees it, its highest point too; then its box as it lies flat, widened by
## as much as the sphere can move it (planet_place in planet.gdshaderinc), against the camera's six
## planes; its skirts left out, since they show only where the chunk's own edge could be seen.
## The sphere only lowers the ground, by up to d² / 2r at d metres from the focus, and moves it
## across by the error of its east-west scale there, by the north the meridians' meeting gives,
## d² / 2r at most, and by less of the third order: near the focus the box stays tight.
func _seen(chunk: Dictionary, side: float) -> bool:
	var radius := world_m.x / TAU
	var x0: float = chunk.east - origin.x
	x0 -= world_m.x * floorf(x0 / world_m.x + 0.5)
	var z1: float = origin.y - chunk.north
	var high: float = chunk.high
	var lo := Vector3(x0, chunk.low, z1 - side)
	var hi := Vector3(x0 + side, high, z1)
	var near_x := maxf(maxf(lo.x - _at.x, _at.x - hi.x), 0.0)
	var near_z := maxf(maxf(lo.z - _at.z, _at.z - hi.z), 0.0)
	# the horizon: its nearest point's angle round the planet from the focus, against the camera's
	var s_lat := sin(minf(near_z / radius, PI) * 0.5)
	var s_lon := sin(minf(near_x / radius, PI) * 0.5)
	var hav := minf(s_lat * s_lat + chunk.cos_least * _cos_focus * s_lon * s_lon, 1.0)
	if 2.0 * asin(sqrt(hav)) - _under > _horizon + chunk.lift + 0.002:
		return false
	# the box
	var far_x := maxf(absf(lo.x - _at.x), absf(hi.x - _at.x))
	var far_z := maxf(absf(lo.z - _at.z), absf(hi.z - _at.z))
	var far := sqrt(far_x * far_x + far_z * far_z)
	var k_most := 1.0 + stretch * exp(-(near_x * near_x + near_z * near_z) / (reach * reach))
	var k_least := 1.0 + stretch * exp(-far * far / (reach * reach))
	var scale_error := maxf(
		absf(chunk.cos_most * k_most - 1.0), absf(chunk.cos_least * k_least - 1.0)
	)
	var bent := k_most * far
	var curve := bent * bent / (2.0 * radius) * (1.0 + high / radius)
	var across := (
		scale_error * far_x
		+ 1.1 * curve
		+ bent * bent * bent / (6.0 * radius * radius)
		+ high / radius * bent
	)
	lo -= Vector3(across, curve + 1.0, across)
	hi += Vector3(across, 1.0, across)
	var mid := (lo + hi) * 0.5
	var half := (hi - lo) * 0.5
	for p: Plane in _planes:
		var reach := (
			absf(p.normal.x) * half.x + absf(p.normal.y) * half.y + absf(p.normal.z) * half.z
		)
		if p.distance_to(mid) > reach:
			return false
	return true


## The gap east or west from a place to a span of the world, the shorter way round.
func _east_gap(x: float, east: float, side: float) -> float:
	var mid := east + side * 0.5 - x
	mid -= world_m.x * floorf(mid / world_m.x + 0.5)
	return maxf(absf(mid) - side * 0.5, 0.0)


func _ask(key: int, depth: int, i: int, j: int) -> void:
	var side := ROOT / float(1 << depth)
	asked[key] = true
	var id := WorkerThreadPool.add_task(
		_make_chunk.bind(key, depth, float(i) * side, float(j) * side, side / float(N - 1))
	)
	tasks[id] = key


## On a worker thread: a chunk's ground.
func _make_chunk(key: int, depth: int, east: float, north: float, spacing: float) -> void:
	var arrays: Array = gen.chunk(0, east, north, N, spacing)
	_done_lock.lock()
	_done.append([key, depth, east, north, spacing, arrays])
	_done_lock.unlock()


## The chunks the worker threads have made, given their meshes, hidden until the tree draws them.
func collect() -> void:
	for id: int in tasks.keys():
		if WorkerThreadPool.is_task_completed(id):
			WorkerThreadPool.wait_for_task_completion(id)
			tasks.erase(id)
	_done_lock.lock()
	var done := _done
	_done = []
	_done_lock.unlock()
	for d: Array in done:
		var key: int = d[0]
		asked.erase(key)
		var made: Array = d[5]
		if made.size() < 5 or chunks.has(key):
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
		# placed by its shader on the sphere, so never culled by where its flat points lie: the tree
		# culls it instead (_seen)
		var huge := AABB(Vector3(-4.0e6, -4.0e6, -4.0e6), Vector3(8.0e6, 8.0e6, 8.0e6))
		rs.mesh_set_custom_aabb(mesh, huge)
		var instance := rs.instance_create2(mesh, scenario)
		rs.instance_geometry_set_material_override(instance, material.get_rid())
		var depth: int = d[1]
		var side := ROOT / float(1 << depth)
		rs.instance_geometry_set_shader_parameter(instance, "spacing", d[4])
		var reach := SPLIT * side * 2.0
		var morph := Vector2(reach * 0.7, reach) if depth > 0 else Vector2(1e12, 2e12)
		rs.instance_geometry_set_shader_parameter(instance, "morph", morph)
		rs.instance_set_visible(instance, false)
		# for the culling: its lowest and highest, the cosines of the latitudes it spans, and how
		# far round the planet its highest point lifts the horizon
		var heights: Vector2 = made[4]
		var high := maxf(heights.y, 0.0)
		var radius := world_m.x / TAU
		var south := clampf(PI * (d[3] / world_m.y - 0.5), -0.5 * PI, 0.5 * PI)
		var north := clampf(PI * ((d[3] + side) / world_m.y - 0.5), -0.5 * PI, 0.5 * PI)
		chunks[key] = {
			"mesh": mesh,
			"instance": instance,
			"east": d[2],
			"north": d[3],
			"low": heights.x,
			"high": high,
			"cos_most": 1.0 if south <= 0.0 and north >= 0.0 else maxf(cos(south), cos(north)),
			"cos_least": minf(cos(south), cos(north)),
			"lift": acos(radius / (radius + high)),
			"used": frame,
		}
		_set_place(key)


## A chunk's place in the picture: its corner from the moving origin, east the shorter way round,
## north as it is, since the poles are the globe's edges.
func _set_place(key: int) -> void:
	var c: Dictionary = chunks[key]
	var dx: float = c.east - origin.x
	dx -= world_m.x * floorf(dx / world_m.x + 0.5)
	var dy: float = c.north - origin.y
	RenderingServer.instance_set_transform(c.instance, Transform3D(Basis(), Vector3(dx, 0.0, -dy)))


## The chunks the tree has not reached for longest, freed once more than KEEP are made; never one
## it reached this frame, drawn, split, out of sight or waiting for its brothers, nor a root.
func _forget_old() -> void:
	if chunks.size() <= KEEP:
		return
	var old := []
	for key: int in chunks:
		if chunks[key].used < frame and key >> 40 > 0:
			old.append([chunks[key].used, key])
	old.sort()
	for k in mini(old.size(), chunks.size() - KEEP):
		var key: int = old[k][1]
		RenderingServer.free_rid(chunks[key].instance)
		RenderingServer.free_rid(chunks[key].mesh)
		chunks.erase(key)


## A place in the world's metres, in the picture's from the moving origin: x east the shorter way
## round, z south.
func local(m: Vector2) -> Vector3:
	var d := m - origin
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
