## The path of smoke from a fire (PRE-30), worked out from the scene at load: up, and where rock
## lies close overhead, along beneath it toward where it ends, then up and away on the wind,
## widening as it goes. smoke.gdshader draws the volume round it. Pre-production code (research 00).
extends RefCounted


## The smoke's path from a fire: up, and where rock lies close overhead, along beneath it toward
## where it ends, then up and away on the wind, widening as it goes.
static func find(fire: Vector3, size: float, scene: Node3D) -> Array[Vector4]:
	var tris := rock_above(scene, fire, 9.0)
	var path: Array[Vector4] = []
	var p := fire + Vector3(0.0, 0.15, 0.0)
	var r := 0.45 * size
	var wind := Vector3(0.55, 0.0, 0.3).normalized()
	for step in 32:
		path.append(Vector4(p.x, p.y, p.z, r))
		var move := Vector3.UP * 0.4 + wind * (0.03 + 0.012 * step)
		if cast(tris, p, Vector3.UP, 30.0) < r + 0.5:
			var best := Vector3.ZERO
			var best_up := -1.0
			for k in 8:
				var d := Vector3(cos(TAU * k / 8.0), 0.0, sin(TAU * k / 8.0))
				if cast(tris, p, d, 1.2) < 1.2:
					continue
				var room := cast(tris, p + d * 1.2, Vector3.UP, 30.0)
				if room > best_up:
					best_up = room
					best = d
			move = best * 0.4
		p += move
		r = minf(r * 1.07 + 0.02, 1.8 * size)
	return path


## The triangles of the scene's solid shapes above a point and within reach of it, sideways, where
## each stands.
static func rock_above(scene: Node3D, at: Vector3, reach: float) -> PackedVector3Array:
	var out := PackedVector3Array()
	var near := AABB(at - Vector3(reach, 0.2, reach), Vector3(reach * 2.0, 40.0, reach * 2.0))
	for node in scene.get_children():
		if not node.has_meta("kind") or node.get_meta("kind") != "solid":
			continue
		var mesh: Mesh = (node as MeshInstance3D).mesh
		var xf := (node as Node3D).transform
		if not (xf * mesh.get_aabb()).intersects(near):
			continue
		for s in mesh.get_surface_count():
			var arrays := mesh.surface_get_arrays(s)
			var v: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
			var index := PackedInt32Array(range(v.size()))
			if arrays[Mesh.ARRAY_INDEX] != null:
				index = arrays[Mesh.ARRAY_INDEX]
			for i in range(0, index.size() - 2, 3):
				var a := xf * v[index[i]]
				var b := xf * v[index[i + 1]]
				var c := xf * v[index[i + 2]]
				if near.has_point(a) or near.has_point(b) or near.has_point(c):
					out.append_array([a, b, c])
	return out


## How far a ray goes before it meets one of the triangles, up to far.
static func cast(tris: PackedVector3Array, from: Vector3, dir: Vector3, far: float) -> float:
	var best := far
	for i in range(0, tris.size(), 3):
		var hit: Variant = Geometry3D.ray_intersects_triangle(
			from, dir, tris[i], tris[i + 1], tris[i + 2]
		)
		if hit != null:
			best = minf(best, from.distance_to(hit))
	return best
