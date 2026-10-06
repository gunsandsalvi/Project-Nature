## Calibration scene C2's plants (A18.1, α2.2b): the liked camp's airy plants as stand-ins at its
## density at the closest zoom (A4.6: about 450 to 900 copies), grass tufts, reed beds along a
## bank, bushes and flower clumps, each form a few cards with a design of its own drawn by code at
## 64 texture pixels a metre. Drawn the ways C2 compares, all else equal: plain cut-out cards, cards
## cut close to their designs, solid cores with cut-out fringes, and close-cut cards with alpha to
## coverage. A plant stands where a hash of its place puts it, so every view of the same ground
## shows the same plants. Built for the Calibrate and Compare pages. Implements PRE-46 and PLT-04.
class_name CalibrationPlants
extends RefCounted

const CUT := preload("res://look/leaves-cut.gdshader")
const SOLID := preload("res://look/leaves-solid.gdshader")
const COVERAGE := preload("res://look/leaves-coverage.gdshader")
## Texture pixels a metre (A5.3), and a design's layer: 1 m by 1.5 m.
const TEXELS := 64.0
const LAYER := Vector2i(64, 96)
## The forms, each a layer: its design's rectangle in the layer, and the point its cards stand on.
const TUFT := 0
const BUSH := 1
const REED := 2
const FLOWERS := 3
const RECTS: Array[Rect2i] = [
	Rect2i(16, 48, 32, 48), Rect2i(0, 32, 64, 64), Rect2i(16, 0, 32, 96), Rect2i(16, 64, 32, 32)
]
const ORIGINS: Array[Vector2] = [Vector2(32, 96), Vector2(32, 64), Vector2(32, 96), Vector2(32, 96)]
## A design cut close: the outline where this many lines round it touch its leaves, a texture pixel
## out, so no leaf is cut.
const AROUND := 16
## A bush's design has a dense middle of this radius in texture pixels, and its solid core stays
## two inside it, clear of the filter's blend at the edge.
const DENSE := 20.0
const CORE := 18.0
## The ground's cells and the bushes' cells, in metres; a bush keeps this much round it clear.
const CELL := 0.5
const BUSH_CELL := 3.0
const BUSH_CLEAR := 0.7
## Past the screen's edge, plants are set out this far in metres, so none that leans in is missing.
const MARGIN := 1.5
## A bush always stands here, just south-west of the focus, so the smallest view, the cloud's,
## holds every form.
const FOCUS_BUSH := Vector2(-0.6, 1.2)

## The designs, their outlines and each way's meshes, made once.
static var _kit := {}


## The plants a view of the ground shows, drawn one way, under a node: a MultiMesh for each form,
## none for "none". screen: the screen's size as the camera's rays take it. The node's metadata
## holds the copies, the draws as Godot counts them (an instance each) and the triangles it draws.
static func plants(under: Node3D, camera: Camera3D, screen: Vector2, way: String) -> Node3D:
	var holder := Node3D.new()
	under.add_child(holder)
	holder.set_meta("copies", 0)
	holder.set_meta("draws", 0)
	holder.set_meta("triangles", 0)
	if way == "none":
		return holder
	var meshes: Array = kit()["meshes"][way]
	var places := _places(camera, screen)
	var copies := 0
	var draws := 0
	var triangles := 0
	for form in places.size():
		var at: Array = places[form]
		if at.is_empty():
			continue
		var mesh: ArrayMesh = meshes[form]
		var multimesh := MultiMesh.new()
		multimesh.transform_format = MultiMesh.TRANSFORM_3D
		multimesh.mesh = mesh
		multimesh.instance_count = at.size()
		for i in at.size():
			multimesh.set_instance_transform(i, at[i])
		var node := MultiMeshInstance3D.new()
		node.multimesh = multimesh
		# small plants cast no sun shadow (A4.4)
		node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
		holder.add_child(node)
		copies += at.size()
		# Godot counts a draw for each instance seen, whatever its surfaces
		draws += 1
		for s in mesh.get_surface_count():
			triangles += at.size() * mesh.surface_get_array_len(s) / 3
	holder.set_meta("copies", copies)
	holder.set_meta("draws", draws)
	holder.set_meta("triangles", triangles)
	return holder


## The designs as a texture array, each form's outline and each way's meshes, made at first use.
static func kit() -> Dictionary:
	if not _kit.is_empty():
		return _kit
	var designs: Array[Image] = []
	var outlines: Array[PackedVector2Array] = []
	for form in RECTS.size():
		var design := _design(form)
		outlines.append(_outline(design, form))
		designs.append(_with_levels(design))
	var array := Texture2DArray.new()
	array.create_from_images(designs)
	var cut := _material(CUT, array, 1)
	var solid := _material(SOLID, array, 0)
	var coverage := _material(COVERAGE, array, 0)
	var meshes := {}
	for way: String in ["plain", "close", "cores", "coverage"]:
		var each: Array[ArrayMesh] = []
		for form in RECTS.size():
			each.append(_mesh(form, way, outlines[form], cut, solid, coverage))
		meshes[way] = each
	_kit = {"designs": array, "outlines": outlines, "meshes": meshes}
	return _kit


## A form's cards as a mesh drawn one way: each card its design's whole rectangle (plain), its close
## outline (close, coverage), or that outline as a solid core and the fringe round it (cores, for
## the one design with a dense middle; the rest stay as close).
static func _mesh(
	form: int,
	way: String,
	outline: PackedVector2Array,
	cut: Material,
	solid: Material,
	coverage: Material
) -> ArrayMesh:
	var mesh := ArrayMesh.new()
	var whole := _rect_polygon(RECTS[form])
	if way == "plain":
		_surface(mesh, form, _fan(whole), cut)
	elif way == "coverage":
		_surface(mesh, form, _fan(outline), coverage)
	elif way == "cores" and form == BUSH:
		var core := PackedVector2Array()
		for p in outline:
			core.append(ORIGINS[form] + (p - ORIGINS[form]).normalized() * CORE)
		_surface(mesh, form, _fan(core), solid)
		var ring := PackedVector2Array()
		for k in outline.size():
			var n := (k + 1) % outline.size()
			ring.append_array([outline[k], outline[n], core[n], outline[k], core[n], core[k]])
		_surface(mesh, form, ring, cut)
	else:
		_surface(mesh, form, _fan(outline), cut)
	return mesh


## One surface of a form's cards: the triangles, given as points of the design's layer in texture
## pixels, three a triangle, laid on every card of the form.
static func _surface(
	mesh: ArrayMesh, form: int, triangles: PackedVector2Array, material: Material
) -> void:
	var tool := SurfaceTool.new()
	tool.begin(Mesh.PRIMITIVE_TRIANGLES)
	var origin: Vector2 = ORIGINS[form]
	for card: Transform3D in _cards(form):
		var normal := (card.basis * Vector3.BACK).normalized()
		for t in triangles:
			tool.set_normal(normal)
			tool.set_uv(Vector2(t.x / LAYER.x, t.y / LAYER.y))
			tool.set_uv2(Vector2(form, 0.0))
			tool.add_vertex(
				card * Vector3((t.x - origin.x) / TEXELS, (origin.y - t.y) / TEXELS, 0.0)
			)
	tool.commit(mesh)
	mesh.surface_set_material(mesh.get_surface_count() - 1, material)


## Where a form's cards stand in one copy: tufts three crossed, reeds four, flowers two, and a bush
## fourteen leaf clusters round a dome.
static func _cards(form: int) -> Array[Transform3D]:
	var out: Array[Transform3D] = []
	if form == BUSH:
		var rng := RandomNumberGenerator.new()
		rng.seed = 77
		for i in 14:
			var turn := (
				Basis(Vector3.UP, rng.randf() * TAU)
				* Basis(Vector3.RIGHT, rng.randf_range(-0.45, 0.45))
			)
			var at := Vector3(
				rng.randf_range(-1, 1), rng.randf_range(-1, 1), rng.randf_range(-1, 1)
			)
			at = at.limit_length(1.0) * 0.35 + Vector3(0.0, 0.62, 0.0)
			out.append(Transform3D(turn, at))
		return out
	var crossed: int = {TUFT: 3, REED: 4, FLOWERS: 2}[form]
	for i in crossed:
		out.append(Transform3D(Basis(Vector3.UP, PI * i / crossed), Vector3.ZERO))
	return out


## Where each form's copies stand in the ground a view shows, by a hash of each cell's place: reed
## beds along a winding bank with water beyond it, a trodden path with few tufts, bushes on their
## own cells, and the meadow's tufts and flowers between.
static func _places(camera: Camera3D, screen: Vector2) -> Array:
	var out := [[], [], [], []]
	var corners := PackedVector2Array()
	for c: Vector2 in [Vector2.ZERO, Vector2(screen.x, 0.0), screen, Vector2(0.0, screen.y)]:
		var g := CalibrationDrawing.ground_at(camera, c)
		corners.append(Vector2(g.x, g.z))
	var grown := Geometry2D.offset_polygon(corners, MARGIN)
	var area: PackedVector2Array = grown[0] if not grown.is_empty() else corners
	var low := area[0]
	var high := area[0]
	for p in area:
		low = low.min(p)
		high = high.max(p)
	var bushes: Array[Vector2] = []
	if Geometry2D.is_point_in_polygon(FOCUS_BUSH, area):
		bushes.append(FOCUS_BUSH)
		out[BUSH].append(_copy(FOCUS_BUSH, 0, 0, 11))
	for bj in range(floori(low.y / BUSH_CELL), ceili(high.y / BUSH_CELL)):
		for bi in range(floori(low.x / BUSH_CELL), ceili(high.x / BUSH_CELL)):
			if _unit(bi, bj, 7) >= 0.35:
				continue
			var p := (
				Vector2(bi, bj)
				+ Vector2(0.2, 0.2)
				+ Vector2(_unit(bi, bj, 8), _unit(bi, bj, 9)) * 0.6
			)
			p *= BUSH_CELL
			if _zone(p) == "meadow" and Geometry2D.is_point_in_polygon(p, area):
				bushes.append(p)
				out[BUSH].append(_copy(p, bi, bj, 10))
	for j in range(floori(low.y / CELL), ceili(high.y / CELL)):
		for i in range(floori(low.x / CELL), ceili(high.x / CELL)):
			var p := (Vector2(i, j) + Vector2(0.5, 0.5)) * CELL
			p += (Vector2(_unit(i, j, 1), _unit(i, j, 2)) - Vector2(0.5, 0.5)) * 0.4
			if not Geometry2D.is_point_in_polygon(p, area):
				continue
			var u := _unit(i, j, 0)
			match _zone(p):
				"bank":
					if u < 0.7:
						out[REED].append(_copy(p, i, j, 3))
				"path":
					if u < 0.1:
						out[TUFT].append(_copy(p, i, j, 3))
				"meadow":
					if bushes.any(func(b: Vector2) -> bool: return b.distance_to(p) < BUSH_CLEAR):
						continue
					if u < 0.45:
						out[TUFT].append(_copy(p, i, j, 3))
					elif u < 0.53:
						out[FLOWERS].append(_copy(p, i, j, 3))
	return out


## The zone a point of the ground lies in, from its place in metres east (x) and south (y): "water"
## west of a winding bank, the "bank" itself, a "path" north to south, or the "meadow".
static func _zone(p: Vector2) -> String:
	var north := -p.y
	var bank := -2.6 + 1.4 * sin(north / 7.0)
	if p.x < bank - 1.2:
		return "water"
	if absf(p.x - bank) < 1.2:
		return "bank"
	if absf(p.x - (1.6 + 0.9 * sin(north / 5.0 + 1.0))) < 0.6:
		return "path"
	return "meadow"


## A copy at a point of the ground, turned and sized by its cell's hash.
static func _copy(p: Vector2, i: int, j: int, salt: int) -> Transform3D:
	var size := 0.85 + 0.3 * _unit(i, j, salt + 1)
	var turn := Basis(Vector3.UP, _unit(i, j, salt) * TAU).scaled(Vector3.ONE * size)
	return Transform3D(turn, Vector3(p.x, 0.0, p.y))


## A number from 0 to 1 for a cell and a salt, the same on every run and every machine.
static func _unit(i: int, j: int, salt: int) -> float:
	var h := (i * 73856093) ^ (j * 19349663) ^ (salt * 83492791)
	h = (h ^ (h >> 13)) * 1274126177
	h = h ^ (h >> 16)
	return float(h & 0xFFFF) / 65536.0


## A form's design, drawn by code into its layer, the rest of the layer clear.
static func _design(form: int) -> Image:
	var image := Image.create(LAYER.x, LAYER.y, false, Image.FORMAT_RGBA8)
	var rng := RandomNumberGenerator.new()
	rng.seed = 1000 + form
	match form:
		TUFT:
			_blades(
				image, rng, form, 9, Vector2(26, 46), 0.6, 1, Color("#3f5a22"), Color("#b5c46a")
			)
		BUSH:
			_cluster(image, rng)
		REED:
			_blades(
				image, rng, form, 7, Vector2(66, 92), 0.14, 2, Color("#4b5e2a"), Color("#a9b36a")
			)
			for k in 2:
				var x := 26 + rng.randi_range(0, 12)
				image.fill_rect(Rect2i(x, 2 + k * 6, 3, 9), Color("#6b4a2c"))
		FLOWERS:
			_blades(
				image, rng, form, 6, Vector2(10, 26), 0.5, 1, Color("#4d6a2a"), Color("#7f9a45")
			)
			for k in 6:
				var at := Vector2i(26 + rng.randi_range(0, 12), 70 + rng.randi_range(0, 12))
				var head: Color = [Color("#e8d24a"), Color("#f2efe2"), Color("#9a7cc4")][k % 3]
				image.fill_rect(Rect2i(at, Vector2i(3, 3)), head)
	_bleed(image)
	return image


## Blades or stems from the foot of a form's design, fanning out, darker at the foot.
static func _blades(
	image: Image,
	rng: RandomNumberGenerator,
	form: int,
	count: int,
	lengths: Vector2,
	spread: float,
	width: int,
	foot: Color,
	tip: Color
) -> void:
	var rect: Rect2i = RECTS[form]
	var base: Vector2 = ORIGINS[form] - Vector2(0.0, 1.0)
	for b in count:
		var from := base + Vector2(rng.randf_range(-4.0, 4.0), 0.0)
		var angle := rng.randf_range(-spread, spread)
		var bend := rng.randf_range(-0.006, 0.006)
		var length := rng.randf_range(lengths.x, lengths.y)
		for t in int(length):
			var p := from + Vector2(sin(angle) * t + bend * t * t, -cos(angle) * t)
			var colour := foot.lerp(tip, t / length)
			for w in width:
				var q := Vector2i(roundi(p.x) + w, roundi(p.y))
				if rect.has_point(q):
					image.set_pixelv(q, colour)


## A bush's leaf cluster: a dense middle of mottled leaves, and single leaves round it with gaps.
static func _cluster(image: Image, rng: RandomNumberGenerator) -> void:
	var centre: Vector2 = ORIGINS[BUSH]
	var greens := [Color("#2f4a1c"), Color("#3e5f22"), Color("#4f7330"), Color("#6a8a3a")]
	for y in range(int(centre.y - DENSE), int(centre.y + DENSE) + 1):
		for x in range(int(centre.x - DENSE), int(centre.x + DENSE) + 1):
			if Vector2(x + 0.5, y + 0.5).distance_to(centre) <= DENSE:
				image.set_pixel(x, y, greens[rng.randi_range(0, 2)])
	for k in 46:
		var a := rng.randf() * TAU
		var r := rng.randf_range(16.0, 25.0)
		var leaf := centre + Vector2.from_angle(a) * r
		var along := Vector2.from_angle(a + rng.randf_range(-0.4, 0.4))
		var colour: Color = greens[rng.randi_range(1, 3)]
		for y in range(int(leaf.y) - 5, int(leaf.y) + 6):
			for x in range(int(leaf.x) - 5, int(leaf.x) + 6):
				var d := Vector2(x + 0.5, y + 0.5) - leaf
				var u := d.dot(along)
				var v := d.dot(along.orthogonal())
				if (u * u) / 20.0 + (v * v) / 4.8 <= 1.0:
					image.set_pixel(x, y, colour)


## Colour bled into the clear pixels beside a design's, so the filter's blend at an edge keeps the
## leaf's colour (A6.1: cut-out levels keep their colour at the edge).
static func _bleed(image: Image) -> void:
	var source := image.duplicate() as Image
	for y in image.get_height():
		for x in image.get_width():
			if source.get_pixel(x, y).a > 0.5:
				continue
			var sum := Color(0, 0, 0, 0)
			var n := 0
			for d: Vector2i in [Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)]:
				var q := Vector2i(x, y) + d
				if (
					Rect2i(Vector2i.ZERO, image.get_size()).has_point(q)
					and source.get_pixelv(q).a > 0.5
				):
					sum += source.get_pixelv(q)
					n += 1
			if n > 0:
				image.set_pixel(x, y, Color(sum.r / n, sum.g / n, sum.b / n, 0.0))


## A design with its own levels down to one pixel, each a cut-out of the one above: a pixel is a
## leaf's if two of its four are, so the leaves keep their cover at every level (A6.1).
static func _with_levels(image: Image) -> Image:
	var data := image.get_data()
	var level := image
	while level.get_width() > 1 or level.get_height() > 1:
		var w := maxi(1, level.get_width() / 2)
		var h := maxi(1, level.get_height() / 2)
		var next := Image.create(w, h, false, Image.FORMAT_RGBA8)
		for y in h:
			for x in w:
				var sum := Color(0, 0, 0, 0)
				var n := 0
				for d: Vector2i in [Vector2i(0, 0), Vector2i(1, 0), Vector2i(0, 1), Vector2i(1, 1)]:
					var q := Vector2i(
						mini(2 * x + d.x, level.get_width() - 1),
						mini(2 * y + d.y, level.get_height() - 1)
					)
					var c := level.get_pixelv(q)
					if c.a > 0.5:
						sum += c
						n += 1
				if n > 0:
					next.set_pixel(
						x, y, Color(sum.r / n, sum.g / n, sum.b / n, 1.0 if n >= 2 else 0.0)
					)
		data.append_array(next.get_data())
		level = next
	return Image.create_from_data(
		image.get_width(), image.get_height(), true, Image.FORMAT_RGBA8, data
	)


## A design's close outline in its layer's texture pixels: where AROUND lines round it touch its
## leaves from outside, a texture pixel out, kept inside its rectangle; convex, and round the point
## its cards stand on for a bush, whose core it rings.
static func _outline(design: Image, form: int) -> PackedVector2Array:
	var rect: Rect2i = RECTS[form]
	var centre := Vector2(rect.get_center()) if form != BUSH else ORIGINS[BUSH]
	var reach := PackedFloat64Array()
	reach.resize(AROUND)
	reach.fill(-1.0e9)
	var directions: Array[Vector2] = []
	for k in AROUND:
		directions.append(Vector2.from_angle(TAU * k / AROUND))
	for y in range(rect.position.y, rect.end.y):
		for x in range(rect.position.x, rect.end.x):
			if design.get_pixel(x, y).a < 0.5:
				continue
			for k in AROUND:
				var d := directions[k]
				# the pixel's corner farthest along the direction
				var corner := Vector2(
					x + (1.0 if d.x > 0.0 else 0.0), y + (1.0 if d.y > 0.0 else 0.0)
				)
				reach[k] = maxf(reach[k], (corner - centre).dot(d))
	var polygon := PackedVector2Array()
	for k in AROUND:
		var a := directions[k]
		var b := directions[(k + 1) % AROUND]
		var ha := reach[k] + 1.0
		var hb := reach[(k + 1) % AROUND] + 1.0
		var det := a.x * b.y - a.y * b.x
		polygon.append(centre + Vector2((ha * b.y - hb * a.y) / det, (a.x * hb - b.x * ha) / det))
	var kept := Geometry2D.intersect_polygons(polygon, _rect_polygon(rect))
	return kept[0] if not kept.is_empty() else polygon


## A convex polygon as a fan of triangles, three points each.
static func _fan(polygon: PackedVector2Array) -> PackedVector2Array:
	var out := PackedVector2Array()
	for k in range(1, polygon.size() - 1):
		out.append_array([polygon[0], polygon[k], polygon[k + 1]])
	return out


static func _rect_polygon(rect: Rect2i) -> PackedVector2Array:
	var r := Rect2(rect)
	return PackedVector2Array(
		[r.position, Vector2(r.end.x, r.position.y), r.end, Vector2(r.position.x, r.end.y)]
	)


static func _material(shader: Shader, designs: Texture2DArray, priority: int) -> ShaderMaterial:
	var material := ShaderMaterial.new()
	material.shader = shader
	material.set_shader_parameter("kd_leaves", designs)
	# cut-out leaves after solid things, so the chip skips their hidden parts (A4.1)
	material.render_priority = priority
	return material
