## The graphics engine's own clocks (A18.1, PLT-04): the time the graphics chip and the main thread
## spent on every viewport a page draws, its window's and any of its own, as Godot's release build
## measures them; it times a whole viewport, never a pass, so a part's cost comes from switching it
## off. Written once for the Bench and Calibrate pages. Implements PLT-04.
class_name Timing
extends RefCounted


## Every viewport a page draws, its window's and any of its own, each timed from now on.
static func viewports(page: Node) -> Array[Viewport]:
	var out: Array[Viewport] = [page.get_viewport()]
	for node: Node in page.find_children("*", "SubViewport", true, false):
		out.append(node as Viewport)
	for viewport: Viewport in out:
		RenderingServer.viewport_set_measure_render_time(viewport.get_viewport_rid(), true)
	return out


## The graphics chip's time on the viewports' last measured frame, in milliseconds: 0 where the
## phone gives none. A viewport drawn only now and then, which says how often in its metadata
## "drawn_a_second", counts its last drawing's time for its share of the frames, since Godot keeps
## a viewport's last time until it draws again.
static func gpu_ms(viewports: Array[Viewport]) -> float:
	var ms := 0.0
	for viewport: Viewport in viewports:
		var rid := viewport.get_viewport_rid()
		ms += RenderingServer.viewport_get_measured_render_time_gpu(rid) * _share(viewport)
	return ms


## The main thread's time drawing the viewports' last frame, in milliseconds: the frame's setup and
## each viewport's culling and draw recording, without scripts; a viewport drawn now and then for
## its share of the frames.
static func cpu_ms(viewports: Array[Viewport]) -> float:
	var ms := RenderingServer.get_frame_setup_time_cpu()
	for viewport: Viewport in viewports:
		var rid := viewport.get_viewport_rid()
		ms += RenderingServer.viewport_get_measured_render_time_cpu(rid) * _share(viewport)
	return ms


## The share of the frames a viewport is drawn in: 1, or its drawings a second over the frames a
## second.
static func _share(viewport: Viewport) -> float:
	var fps := Engine.get_frames_per_second()
	if not viewport.has_meta("drawn_a_second") or fps <= 0.0:
		return 1.0
	return minf(1.0, float(viewport.get_meta("drawn_a_second")) / fps)
