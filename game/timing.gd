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
## phone gives none.
static func gpu_ms(viewports: Array[Viewport]) -> float:
	var ms := 0.0
	for viewport: Viewport in viewports:
		ms += RenderingServer.viewport_get_measured_render_time_gpu(viewport.get_viewport_rid())
	return ms


## The main thread's time drawing the viewports' last frame, in milliseconds: the frame's setup and
## each viewport's culling and draw recording, without scripts.
static func cpu_ms(viewports: Array[Viewport]) -> float:
	var ms := RenderingServer.get_frame_setup_time_cpu()
	for viewport: Viewport in viewports:
		ms += RenderingServer.viewport_get_measured_render_time_cpu(viewport.get_viewport_rid())
	return ms
