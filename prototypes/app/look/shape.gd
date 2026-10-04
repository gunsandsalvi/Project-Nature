## A shape in the look's format, built at load (PRE-46): per vertex its material row, step bias,
## pattern and flags (custom0), and its place in its own part (custom1), with Godot's clockwise
## front faces. Parts are placed by xf, so a figure's limbs can turn at their joints. Used by P2's
## camp and P3's kit. Pre-production code (research 00): thrown away with the prototypes.
extends RefCounted

## The look's flags (look.gdshaderinc).
const EMISSIVE := 1.0
const FOLIAGE := 2.0
const NO_OUTLINE := 4.0
const CREATURE := 32.0
## Parts that take their copy's own material, pattern and wear (solid.gdshader, A6.2).
const COPY := 64.0
## The look's surface patterns (look.gdshaderinc's pattern()).
const PAT := {"none": 0, "ground": 1, "rock": 2, "bark": 3, "hide": 5, "thatch": 6, "face": 7}

## Where the part being built stands, and steps up or down from its light's own.
var xf := Transform3D.IDENTITY
var bias := 0
var _points := PackedVector3Array()
var _normals := PackedVector3Array()
var _attrs := PackedByteArray()
var _locs := PackedFloat32Array()


## One face; its vertices' places in their part (loc) default to where they stand.
func tri(
	a: Vector3, b: Vector3, c: Vector3, n: Vector3, row: float, flags: float, pat := 0, loc := []
) -> void:
	var la: Vector3 = loc[0] if loc.size() == 3 else a
	var lb: Vector3 = loc[1] if loc.size() == 3 else b
	var lc: Vector3 = loc[2] if loc.size() == 3 else c
	var wa := xf * a
	var wb := xf * b
	var wc := xf * c
	var wn := (xf.basis * n).normalized()
	# Godot's front faces wind clockwise seen from outside, so their right-hand normal points in
	if (wb - wa).cross(wc - wa).dot(wn) > 0.0:
		var t := wb
		wb = wc
		wc = t
		var u := lb
		lb = lc
		lc = u
	for v in [[wa, la], [wb, lb], [wc, lc]]:
		_points.append(v[0])
		_normals.append(wn)
		_attrs.append_array(PackedByteArray([int(row), 128 + bias, int(pat), int(flags)]))
		var l: Vector3 = v[1]
		_locs.append_array(PackedFloat32Array([l.x, l.y, l.z, 0.0]))


func quad(
	a: Vector3, b: Vector3, c: Vector3, d: Vector3, n: Vector3, row: float, flags: float, pat := 0
) -> void:
	tri(a, b, c, n, row, flags, pat)
	tri(a, c, d, n, row, flags, pat)


## A box; with norm its vertices' places run -0.5 to 0.5 across it, as the face pattern reads them.
func box(centre: Vector3, size: Vector3, row: float, flags: float, pat := 0, norm := false) -> void:
	var h := size * 0.5
	for axis in 3:
		for side in [-1.0, 1.0]:
			var n := Vector3.ZERO
			n[axis] = side
			var u := Vector3.ZERO
			u[(axis + 1) % 3] = h[(axis + 1) % 3]
			var v := Vector3.ZERO
			v[(axis + 2) % 3] = h[(axis + 2) % 3]
			var c := centre + n * h[axis]
			var corners := [c - u - v, c + u - v, c + u + v, c - u + v]
			var locs := []
			for p: Vector3 in corners:
				locs.append((p - centre) / size if norm else p)
			tri(corners[0], corners[1], corners[2], n, row, flags, pat, [locs[0], locs[1], locs[2]])
			tri(corners[0], corners[2], corners[3], n, row, flags, pat, [locs[0], locs[2], locs[3]])


## A round shape: an octahedron's faces, split once and pushed out to the sphere when fine,
## stretched by scale.
func ball(
	centre: Vector3,
	radius: float,
	row: float,
	flags: float,
	fine := true,
	pat := 0,
	scale := Vector3.ONE
) -> void:
	var o := [Vector3.UP, Vector3.DOWN, Vector3.LEFT, Vector3.RIGHT, Vector3.FORWARD, Vector3.BACK]
	var faces := [
		[0, 2, 4], [0, 4, 3], [0, 3, 5], [0, 5, 2], [1, 4, 2], [1, 3, 4], [1, 5, 3], [1, 2, 5]
	]
	for f in faces:
		var a: Vector3 = o[f[0]]
		var b: Vector3 = o[f[1]]
		var c: Vector3 = o[f[2]]
		var parts := [[a, b, c]]
		if fine:
			var ab := (a + b).normalized()
			var bc := (b + c).normalized()
			var ca := (c + a).normalized()
			parts = [[a, ab, ca], [ab, b, bc], [ca, bc, c], [ab, bc, ca]]
		for t in parts:
			var p0: Vector3 = t[0] * scale
			var p1: Vector3 = t[1] * scale
			var p2: Vector3 = t[2] * scale
			var n: Vector3 = ((t[0] + t[1] + t[2]) / scale).normalized()
			tri(
				centre + p0 * radius, centre + p1 * radius, centre + p2 * radius, n, row, flags, pat
			)


## A solid of turning: the profile's points (radius, height) round the upright axis at base.
func lathe(
	base: Vector3, profile: Array, sides: int, row: float, flags: float, pat := 0, cap := true
) -> void:
	for i in sides:
		var a0 := TAU * i / sides
		var a1 := TAU * (i + 1) / sides
		var d0 := Vector3(cos(a0), 0.0, sin(a0))
		var d1 := Vector3(cos(a1), 0.0, sin(a1))
		for k in profile.size() - 1:
			var p: Vector2 = profile[k]
			var q: Vector2 = profile[k + 1]
			var v00 := base + d0 * p.x + Vector3.UP * p.y
			var v01 := base + d1 * p.x + Vector3.UP * p.y
			var v10 := base + d0 * q.x + Vector3.UP * q.y
			var v11 := base + d1 * q.x + Vector3.UP * q.y
			var slope := Vector2(q.y - p.y, p.x - q.x).normalized()
			var n := ((d0 + d1).normalized() * slope.x + Vector3.UP * slope.y).normalized()
			quad(v00, v01, v11, v10, n, row, flags, pat)
		if cap:
			var top: Vector2 = profile[-1]
			var c := base + Vector3.UP * top.y
			tri(c, c + d0 * top.x, c + d1 * top.x, Vector3.UP, row, flags, pat)


## A cone of sides faces, base on the ground at the origin of the part.
func cone(radius: float, height: float, sides: int, row: float, flags: float, pat := 0) -> void:
	var top := Vector3(0.0, height, 0.0)
	for i in sides:
		var a0 := TAU * i / sides
		var a1 := TAU * (i + 1) / sides
		var p0 := Vector3(cos(a0) * radius, 0.0, sin(a0) * radius)
		var p1 := Vector3(cos(a1) * radius, 0.0, sin(a1) * radius)
		var n := (p0 + p1).normalized() * height + Vector3(0.0, radius, 0.0)
		tri(p0, p1, top, n.normalized(), row, flags, pat)


## A round pole between two points, of sides faces.
func pole(
	from: Vector3, to: Vector3, radius: float, row: float, flags: float, pat := 0, sides := 5
) -> void:
	var axis := (to - from).normalized()
	var side := axis.cross(Vector3.UP if absf(axis.y) < 0.9 else Vector3.RIGHT).normalized()
	var up := side.cross(axis)
	for i in sides:
		var a0 := TAU * i / sides
		var a1 := TAU * (i + 1) / sides
		var o0 := (side * cos(a0) + up * sin(a0)) * radius
		var o1 := (side * cos(a1) + up * sin(a1)) * radius
		var n := (o0 + o1).normalized()
		quad(from + o0, from + o1, to + o1, to + o0, n, row, flags, pat)


func commit() -> ArrayMesh:
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = _points
	arrays[Mesh.ARRAY_NORMAL] = _normals
	arrays[Mesh.ARRAY_CUSTOM0] = _attrs
	arrays[Mesh.ARRAY_CUSTOM1] = _locs
	var format := (
		Mesh.ARRAY_CUSTOM_RGBA8_UNORM << Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT
		| Mesh.ARRAY_CUSTOM_RGBA_FLOAT << Mesh.ARRAY_FORMAT_CUSTOM1_SHIFT
	)
	var mesh := ArrayMesh.new()
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays, [], {}, format)
	return mesh


## A flat square of ground, side metres across, all of one of the ground's four covers.
static func plain(side: float, cover: int) -> ArrayMesh:
	var h := side * 0.5
	var points := PackedVector3Array(
		[
			Vector3(-h, 0, -h),
			Vector3(h, 0, -h),
			Vector3(h, 0, h),
			Vector3(-h, 0, -h),
			Vector3(h, 0, h),
			Vector3(-h, 0, h)
		]
	)
	var normals := PackedVector3Array()
	var weights := PackedByteArray()
	for i in 6:
		normals.append(Vector3.UP)
		var w := PackedByteArray([0, 0, 0, 0])
		w[cover] = 255
		weights.append_array(w)
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = points
	arrays[Mesh.ARRAY_NORMAL] = normals
	arrays[Mesh.ARRAY_CUSTOM0] = weights
	var mesh := ArrayMesh.new()
	var format := Mesh.ARRAY_CUSTOM_RGBA8_UNORM << Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays, [], {}, format)
	return mesh
