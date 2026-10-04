## P3's kit (IMPLEMENTATION α0.2c): shapes built at load from the catalogue's parameters
## (catalogue.json), in the look's format (look/shape.gd): shared shapes, plants and an animal in
## either of their materials (PRE-46), and the block figure in any pose of its movements, with a
## face of eyes and a mouth (PRE-27, PRE-44). Pre-production code (research 00).
extends RefCounted

const Shape := preload("res://look/shape.gd")


## A shape of the catalogue in its first or second material (which).
static func build(e: Dictionary, which: int, rows: Dictionary) -> ArrayMesh:
	var m := Shape.new()
	var row: float = rows[(e.materials as Array)[which]]
	var pat := _pattern(e, which)
	match e.kind:
		"boulder":
			var s: Array = e.size
			var half := Vector3(s[0], s[1], s[2]) * 0.5
			m.ball(Vector3(0.0, half.y * 0.9, 0.0), 1.0, row, 0.0, true, Shape.PAT.rock, half)
		"log":
			var h: float = e.length * 0.5
			var r: float = e.radius
			m.pole(Vector3(-h, r, 0.0), Vector3(h, r, 0.0), r, row, 0.0, Shape.PAT.bark, 7)
		"stump":
			var r: float = e.radius
			var profile := [Vector2(r * 1.2, 0.0), Vector2(r, 0.12), Vector2(r, e.height)]
			m.lathe(Vector3.ZERO, profile, 8, row, 0.0, Shape.PAT.bark)
		"hearth":
			_hearth(m, e, row, rows)
		"lathe":
			var profile := []
			for p: Array in e.profile:
				profile.append(Vector2(p[0], p[1]))
			m.lathe(Vector3.ZERO, profile, 10, row, 0.0, pat)
		"rack":
			_rack(m, e, row, rows, which)
		"windbreak":
			_windbreak(m, e, row, rows)
		"leanto":
			_lean_to(m, e, row, rows, pat)
		"tent":
			_tent(m, e, row, rows)
		"hut":
			_hut(m, e, row, rows, pat)
		"broadleaf":
			var h: float = e.height
			m.pole(Vector3.ZERO, Vector3(0.0, h * 0.55, 0.0), 0.16, rows.bark, 0.0, Shape.PAT.bark)
			for c: Vector3 in [
				Vector3(0.0, h * 0.7, 0.0),
				Vector3(0.7, h * 0.6, 0.3),
				Vector3(-0.6, h * 0.62, -0.4)
			]:
				m.ball(c, h * 0.24, row, Shape.FOLIAGE)
		"pine":
			var h: float = e.height
			m.pole(Vector3.ZERO, Vector3(0.0, h * 0.3, 0.0), 0.14, rows.bark, 0.0, Shape.PAT.bark)
			for k in 3:
				m.xf = Transform3D(Basis.IDENTITY, Vector3(0.0, h * (0.2 + 0.22 * k), 0.0))
				m.cone(h * (0.26 - 0.06 * k), h * 0.4, 8, row, Shape.FOLIAGE)
		"deer":
			_deer(m, row, rows)
	return m.commit()


## The surface pattern of a shape in one of its materials: its own for each material, one for both,
## or none.
static func _pattern(e: Dictionary, which: int) -> int:
	if e.has("patterns"):
		return Shape.PAT.get((e.patterns as Array)[which], 0)
	return Shape.PAT.get(e.get("pattern", "none"), 0)


## A ring of stones round a bed of charcoal.
static func _hearth(m: Shape, e: Dictionary, row: float, rows: Dictionary) -> void:
	var n: int = e.stones
	var r: float = e.radius
	for k in n:
		var a := TAU * k / n
		var at := Vector3(cos(a) * r, 0.07, sin(a) * r)
		m.ball(at, 1.0, row, 0.0, false, Shape.PAT.rock, Vector3(0.16, 0.1, 0.14))
	var bed := Vector3(r * 0.7, 0.05, r * 0.7)
	m.ball(Vector3.ZERO, 1.0, rows.charcoal, 0.0, false, 0, bed)


## A drying rack of poles: two A-frames and a bar, with two skins hung on it, of the rack's
## material's hang (hide on wood, leather on bark).
static func _rack(m: Shape, e: Dictionary, row: float, rows: Dictionary, which: int) -> void:
	var w: float = e.width * 0.5
	var h: float = e.height
	for x: float in [-w, w]:
		for z: float in [-0.4, 0.4]:
			m.pole(Vector3(x, 0.0, z), Vector3(x, h, 0.0), 0.04, row, 0.0, Shape.PAT.bark)
	m.pole(Vector3(-w, h, 0.0), Vector3(w, h, 0.0), 0.04, row, 0.0, Shape.PAT.bark)
	var hang: float = rows[(e.hang as Array)[which]]
	for k in 2:
		var at := Vector3(-0.45 + 0.9 * k, h - 0.45, 0.0)
		m.box(at, Vector3(0.6, 0.8, 0.03), hang, 0.0, Shape.PAT.hide)


## A windbreak as the art book builds one (the painter's things.js): a row of poles leaning back
## from the fire's side, +z, a bar across them, and brush packed against them, its top ragged, with
## twigs sticking up from it.
static func _windbreak(m: Shape, e: Dictionary, row: float, rows: Dictionary) -> void:
	var rng := RandomNumberGenerator.new()
	rng.seed = 7
	var w: float = e.width * 0.5
	var h: float = e.height
	var lean := h * 0.45
	var poles := roundi(w * 2.0 / 0.4)
	for k in poles:
		var x := -w + (k + 0.5) * w * 2.0 / poles
		var top := Vector3(x + rng.randf_range(-0.1, 0.1), h * rng.randf_range(0.9, 1.08), -lean)
		m.pole(Vector3(x, -0.05, 0.0), top, 0.035, rows.wood, 0.0, Shape.PAT.bark, 4)
	m.pole(
		Vector3(-w, h * 0.8, -lean * 0.8),
		Vector3(w, h * 0.8, -lean * 0.8),
		0.03,
		rows.wood,
		0.0,
		0,
		4
	)
	var segs := roundi(w * 2.0 / 0.25)
	var tops := PackedFloat32Array()
	for k in segs + 1:
		tops.append(rng.randf_range(0.8, 0.97))
	var n := Vector3(0.0, lean, h).normalized()
	for k in segs:
		var x0 := -w + k * w * 2.0 / segs
		var x1 := x0 + w * 2.0 / segs
		m.quad(
			Vector3(x0, 0.02, -0.03),
			Vector3(x1, 0.02, -0.03),
			Vector3(x1, h * tops[k + 1], -lean * tops[k + 1] - 0.03),
			Vector3(x0, h * tops[k], -lean * tops[k] - 0.03),
			n,
			row,
			0.0,
			Shape.PAT.thatch
		)
	for k in poles * 2:
		var x := rng.randf_range(-w, w)
		var t := rng.randf_range(0.84, 0.95)
		var foot := Vector3(x, h * t - 0.06, -lean * t - 0.05)
		for twig in 2:
			var tip := foot + Vector3(rng.randf_range(-0.12, 0.12), rng.randf_range(0.15, 0.3), 0.0)
			m.pole(foot, tip, 0.012, rows.bark if twig == 0 else row, 0.0, 0, 3)


## A lean-to: a ridge pole on two posts, open on the +z side, a thick roof of the material laid
## in three overlapping courses of sheets from the ridge down to the ground behind, each course's
## lower edge ragged, and its two ends closed with brush; a fire set under its open side fills it
## with smoke that flows out under the ridge (PRE-30).
static func _lean_to(m: Shape, e: Dictionary, row: float, rows: Dictionary, pat: int) -> void:
	var rng := RandomNumberGenerator.new()
	rng.seed = 5
	var w: float = e.width * 0.5
	var h: float = e.height
	var d: float = e.depth
	for x: float in [-w + 0.1, w - 0.1]:
		m.pole(
			Vector3(x, 0.0, 0.0), Vector3(x, h + 0.12, 0.0), 0.05, rows.wood, 0.0, Shape.PAT.bark
		)
	m.pole(
		Vector3(-w - 0.1, h, 0.0), Vector3(w + 0.1, h, 0.0), 0.05, rows.wood, 0.0, Shape.PAT.bark
	)
	var n := Vector3(0.0, d, h).normalized()
	var down := Vector3(0.0, -h, -d)
	var ridge := Vector3(0.0, h, 0.0)
	# the underside, a hand's breadth below the courses, so a fire's light passes beneath the roof
	# (look.gdshaderinc's fire_shadow)
	var under := -n * 0.06
	m.quad(
		Vector3(-w, h, 0.0) + under,
		Vector3(-w, 0.0, -d) + under,
		Vector3(w, 0.0, -d) + under,
		Vector3(w, h, 0.0) + under,
		-n,
		row,
		0.0,
		pat
	)
	var courses := 3
	for c in courses:
		var lift := n * (0.04 * (courses - c))
		var x := -w
		while x < w - 0.01:
			var x1 := minf(x + rng.randf_range(0.35, 0.55), w)
			var s0 := maxf(float(c) / courses - 0.06, 0.0)
			var s1 := minf(float(c + 1) / courses + rng.randf_range(0.0, 0.06), 1.0)
			var a := ridge + down * s0 + lift
			var b := ridge + down * s1 + lift
			m.quad(
				Vector3(x, a.y, a.z),
				Vector3(x, b.y, b.z),
				Vector3(x1, b.y, b.z),
				Vector3(x1, a.y, a.z),
				n,
				row,
				0.0,
				pat
			)
			# the sheet's lower edge, its thickness facing down the slope
			var edge := down.normalized()
			m.quad(
				Vector3(x, b.y, b.z),
				Vector3(x, b.y, b.z) - lift - n * 0.02,
				Vector3(x1, b.y, b.z) - lift - n * 0.02,
				Vector3(x1, b.y, b.z),
				edge,
				row,
				0.0,
				pat
			)
			x = x1
	for side: float in [-1.0, 1.0]:
		m.tri(
			Vector3(side * w, 0.0, 0.0),
			Vector3(side * w, h, 0.0),
			Vector3(side * w, 0.0, -d),
			Vector3(side, 0.0, 0.0),
			rows.drygrass,
			0.0,
			Shape.PAT.thatch
		)


## A tent of hides on poles: a cone of ten faces, the one toward +z holding the doorway in its own
## surface, so nothing stands out of it, and the poles crossing above its top, as the art book's
## tents have them.
static func _tent(m: Shape, e: Dictionary, row: float, rows: Dictionary) -> void:
	var r: float = e.radius
	var h: float = e.height
	var sides := 10
	var top := Vector3(0.0, h, 0.0)
	for i in sides:
		var a0 := TAU * i / sides
		var a1 := TAU * (i + 1) / sides
		var p0 := Vector3(cos(a0) * r, 0.0, sin(a0) * r)
		var p1 := Vector3(cos(a1) * r, 0.0, sin(a1) * r)
		var n := ((p0 + p1).normalized() * h + Vector3(0.0, r, 0.0)).normalized()
		if i != 2:
			m.tri(p0, p1, top, n, row, 0.0, Shape.PAT.hide)
			continue
		# the face toward +z (72° to 108°): the doorway, a third of its width and a metre high, and
		# the hide round it
		var at := func(s: float, y: float) -> Vector3:
			return p0.lerp(p1, s) * (1.0 - y / h) + Vector3.UP * y
		var door := 1.0
		m.quad(
			at.call(0.0, 0.0),
			at.call(1.0 / 3.0, 0.0),
			at.call(1.0 / 3.0, door),
			at.call(0.0, door),
			n,
			row,
			0.0,
			Shape.PAT.hide
		)
		m.quad(
			at.call(2.0 / 3.0, 0.0),
			at.call(1.0, 0.0),
			at.call(1.0, door),
			at.call(2.0 / 3.0, door),
			n,
			row,
			0.0,
			Shape.PAT.hide
		)
		m.quad(
			at.call(1.0 / 3.0, 0.0),
			at.call(2.0 / 3.0, 0.0),
			at.call(2.0 / 3.0, door),
			at.call(1.0 / 3.0, door),
			n,
			rows.charcoal,
			0.0
		)
		m.tri(at.call(0.0, door), at.call(1.0, door), top, n, row, 0.0, Shape.PAT.hide)
	for k in 5:
		var a := TAU * (k + 0.3) / 5.0
		var foot := Vector3(cos(a) * r * 0.97, 0.0, sin(a) * r * 0.97)
		m.pole(
			foot, top + (top - foot).normalized() * 0.45, 0.03, rows.wood, 0.0, Shape.PAT.bark, 4
		)


## A round hut of the material, its doorway on the +z side in the dome's own surface, so nothing
## stands out of it; pole ends at its top and pegs round its foot, as the art book's domes have.
## Its cover takes a copy's own material, pattern and wear where it is drawn as copies
## (solid.gdshader, A6.2), so one shape makes a hut of birch bark and one of reed, each worn in its
## own places (PRE-42, PRE-43).
static func _hut(m: Shape, e: Dictionary, row: float, rows: Dictionary, pat: int) -> void:
	var r: float = e.radius
	var h: float = e.height
	var sides := 12
	var rings := 5
	for i in sides:
		# turned half a face, so one face looks straight down +z
		var a0 := TAU * (i - 0.5) / sides
		var a1 := TAU * (i + 0.5) / sides
		for k in rings:
			var b0 := PI * 0.5 * k / rings
			var b1 := PI * 0.5 * (k + 1) / rings
			var mid := (a0 + a1) * 0.5
			var up := (b0 + b1) * 0.5
			var n := Vector3(cos(mid) * cos(up), sin(up), sin(mid) * cos(up)).normalized()
			var doorway := i == 3 and k < 2
			m.quad(
				_dome(r, h, a0, b0),
				_dome(r, h, a1, b0),
				_dome(r, h, a1, b1),
				_dome(r, h, a0, b1),
				n,
				rows.charcoal if doorway else row,
				0.0 if doorway else Shape.COPY,
				0 if doorway else pat
			)
	for k in 3:
		var a := TAU * k / 3.0
		var out := Vector3(cos(a) * 0.25, 0.0, sin(a) * 0.25)
		m.pole(
			Vector3(0.0, h - 0.1, 0.0) - out,
			Vector3(0.0, h + 0.3, 0.0) + out,
			0.03,
			rows.wood,
			0.0,
			0,
			4
		)
	for k in 10:
		var a := TAU * (k + 0.5) / 10.0
		if absf(angle_difference(a, PI * 0.5)) < 0.4:
			continue
		var at := Vector3(cos(a) * (r + 0.05), 0.12, sin(a) * (r + 0.05))
		m.box(at, Vector3(0.07, 0.24, 0.07), rows.wood, 0.0)


## A point of a dome of radius r and height h, at an angle round and an angle up.
static func _dome(r: float, h: float, round_angle: float, up: float) -> Vector3:
	return Vector3(cos(round_angle) * cos(up) * r, sin(up) * h, sin(round_angle) * cos(up) * r)


## A deer stand-in facing +z: body, legs, neck, head, ears, nose and tail, as one shape (PRE-27).
static func _deer(m: Shape, row: float, rows: Dictionary) -> void:
	var f := Shape.CREATURE
	m.box(Vector3(0.0, 0.95, 0.0), Vector3(0.36, 0.4, 1.1), row, f, Shape.PAT.hide)
	for x: float in [-0.12, 0.12]:
		for z: float in [-0.42, 0.42]:
			m.box(Vector3(x, 0.4, z), Vector3(0.09, 0.8, 0.1), row, f)
	m.xf = Transform3D(Basis(Vector3.RIGHT, deg_to_rad(35.0)), Vector3(0.0, 1.1, 0.5))
	m.box(Vector3(0.0, 0.22, 0.0), Vector3(0.16, 0.45, 0.18), row, f)
	m.xf = Transform3D(Basis.IDENTITY, Vector3(0.0, 1.45, 0.72))
	m.box(Vector3(0.0, 0.0, 0.08), Vector3(0.16, 0.18, 0.32), row, f)
	for x: float in [-0.1, 0.1]:
		m.box(Vector3(x, 0.14, -0.02), Vector3(0.05, 0.14, 0.03), row, f)
	m.box(Vector3(0.0, -0.02, 0.25), Vector3(0.07, 0.06, 0.05), rows.charcoal, f)
	m.xf = Transform3D.IDENTITY
	m.box(Vector3(0.0, 1.05, -0.57), Vector3(0.08, 0.14, 0.06), rows.white, f)


## The block figure in a pose (PRE-27, PRE-44): angles in degrees at the hips, knees, shoulders and
## elbows, the body's bend and the head's nod, the pelvis dropped to sit or kneel, and what the
## hands hold; facing +z, with a face of eyes and a mouth on its head (the look's face pattern).
## Its skin, tunic, legs and hair are the figure's own, or those given (mats).
static func figure(rows: Dictionary, d: Dictionary, pose: Dictionary, mats := []) -> ArrayMesh:
	var m := Shape.new()
	if mats.is_empty():
		mats = d.materials
	var skin: float = rows[mats[0]]
	var top: float = rows[mats[1]]
	var legs: float = rows[mats[2]]
	var hair: float = rows[mats[3]]
	var f := Shape.CREATURE
	var thigh: float = d.thigh
	var shin: float = d.shin
	var torso: float = d.torso
	var upper: float = d.upper_arm
	var fore: float = d.forearm
	var pelvis := Vector3(0.0, thigh + shin - float(pose.get("drop", 0.0)), 0.0)
	for side: float in [-1.0, 1.0]:
		var k := "l" if side < 0.0 else "r"
		var hip := Transform3D(
			_turn(-_angle(pose, k + "hip")), pelvis + Vector3(side * float(d.hip_gap), 0.0, 0.0)
		)
		m.xf = hip
		m.box(Vector3(0.0, -thigh * 0.5, 0.0), Vector3(0.15, thigh, 0.17), legs, f)
		m.xf = hip * Transform3D(_turn(_angle(pose, k + "knee")), Vector3(0.0, -thigh, 0.0))
		m.box(Vector3(0.0, -shin * 0.5, 0.0), Vector3(0.13, shin, 0.15), legs, f)
		m.box(Vector3(0.0, -shin + 0.03, 0.05), Vector3(0.13, 0.06, 0.24), rows.leather, f)
	var waist := Transform3D(_turn(_angle(pose, "bend")), pelvis)
	m.xf = waist
	m.box(Vector3(0.0, torso * 0.5, 0.0), Vector3(0.4, torso, 0.24), top, f, Shape.PAT.hide)
	# a belt at the waist, a small detail the art book's figures carry
	m.box(Vector3(0.0, 0.05, 0.0), Vector3(0.42, 0.06, 0.26), rows.leather, f)
	var hands: Array[Transform3D] = []
	for side: float in [-1.0, 1.0]:
		var k := "l" if side < 0.0 else "r"
		var at := Vector3(side * float(d.shoulder_gap), torso - 0.05, 0.0)
		var shoulder := waist * Transform3D(_turn(-_angle(pose, k + "sh")), at)
		m.xf = shoulder
		m.box(Vector3(0.0, -upper * 0.5, 0.0), Vector3(0.11, upper, 0.11), top, f)
		var elbow := (
			shoulder * Transform3D(_turn(-_angle(pose, k + "el")), Vector3(0.0, -upper, 0.0))
		)
		m.xf = elbow
		m.box(Vector3(0.0, -fore * 0.5, 0.0), Vector3(0.1, fore, 0.1), skin, f)
		hands.append(elbow * Transform3D(Basis.IDENTITY, Vector3(0.0, -fore - 0.04, 0.0)))
	var head: Array = d.head
	var size := Vector3(head[0], head[1], head[2])
	m.xf = waist * Transform3D(_turn(_angle(pose, "head")), Vector3(0.0, torso, 0.0))
	m.box(Vector3(0.0, 0.04, 0.0), Vector3(0.12, 0.08, 0.12), skin, f)
	m.box(Vector3(0.0, 0.08 + size.y * 0.5, 0.0), size, skin, f, Shape.PAT.face, true)
	var cap := Vector3(size.x + 0.02, 0.06, size.z + 0.02)
	m.box(Vector3(0.0, 0.1 + size.y, -0.01), cap, hair, f)
	var back := Vector3(size.x + 0.02, size.y * 0.8, 0.04)
	m.box(Vector3(0.0, 0.08 + size.y * 0.6, -size.z * 0.5 - 0.02), back, hair, f)
	_item(m, rows, pose.get("item", ""), waist, hands, torso)
	return m.commit()


## What the figure's hands hold: a bundle on the shoulder, a core and a hammerstone, or a scraper
## over a hide pegged out on the ground before it.
static func _item(
	m: Shape, rows: Dictionary, item: String, waist: Transform3D, hands: Array, torso: float
) -> void:
	var f := Shape.CREATURE
	match item:
		"bundle":
			m.xf = waist
			var at := Vector3(0.2, torso + 0.2, -0.06)
			m.box(at, Vector3(0.62, 0.42, 0.42), rows.hide, f, Shape.PAT.hide)
		"stone":
			m.xf = hands[0]
			m.ball(Vector3.ZERO, 0.07, rows.flint, f, false)
			m.xf = hands[1]
			m.ball(Vector3.ZERO, 0.06, rows.rock, f, false)
		"hide":
			m.xf = Transform3D.IDENTITY
			m.box(Vector3(0.0, 0.01, 0.62), Vector3(0.9, 0.02, 0.7), rows.hide, 0.0, Shape.PAT.hide)
			m.xf = hands[1]
			m.box(Vector3.ZERO, Vector3(0.08, 0.05, 0.12), rows.flint, f)
	m.xf = Transform3D.IDENTITY


## A movement's poses step by step, about 10 a second (PRE-44): each key pose, then the poses
## between it and the next, so a pose held for several steps moves on at every step instead.
static func steps(move: Dictionary) -> Array:
	var keys: Array = move.poses
	var hold: int = move.hold
	var out := []
	for i in keys.size():
		var a: Dictionary = keys[i]
		var b: Dictionary = keys[(i + 1) % keys.size()]
		for k in hold:
			var t := float(k) / hold
			var pose := a.duplicate()
			for key: String in a.keys() + b.keys():
				if key == "item":
					continue
				pose[key] = lerpf(float(a.get(key, 0.0)), float(b.get(key, 0.0)), t)
			out.append(pose)
	return out


## A flower of the meadow: a small head of three petals held above the grass on a stem, in its
## colour's row, lit as foliage and drawn without an outline.
static func flower(rows: Dictionary, row: float) -> ArrayMesh:
	var m := Shape.new()
	var f := Shape.FOLIAGE + Shape.NO_OUTLINE
	m.pole(Vector3.ZERO, Vector3(0.0, 0.16, 0.0), 0.01, rows.grass, f, 0, 3)
	m.bias = 1
	for k in 3:
		var a := TAU * k / 3.0
		m.box(Vector3(cos(a) * 0.03, 0.18, sin(a) * 0.03), Vector3(0.05, 0.04, 0.05), row, f)
	return m.commit()


## A tuft of grass: five thin blades leaning out from a point, lit as the ground at its foot and a
## step lighter, as the art book's tufts are, and drawn without an outline, which would make a
## pixel-sized tuft all edge.
static func tuft(rows: Dictionary) -> ArrayMesh:
	var m := Shape.new()
	m.bias = 1
	for k in 5:
		var a := TAU * k / 5.0 + 0.4
		var out := Vector3(cos(a), 0.0, sin(a))
		var side := Vector3(-out.z, 0.0, out.x) * 0.025
		var tip := out * 0.09 + Vector3(0.0, 0.16 + 0.05 * (k % 2), 0.0)
		m.tri(-side, side, tip, Vector3.UP, rows.grass, Shape.FOLIAGE + Shape.NO_OUTLINE)
	return m.commit()


## A turn about the figure's side-to-side axis: positive tips a part standing up forward, and
## swings a part hanging down back.
static func _turn(radians: float) -> Basis:
	return Basis(Vector3.RIGHT, radians)


## A joint's angle in the pose, in radians; 0 where the pose leaves it out.
static func _angle(pose: Dictionary, key: String) -> float:
	return deg_to_rad(float(pose.get(key, 0.0)))
