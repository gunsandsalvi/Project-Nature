## The crowd's drawing and the Crowd page, headless in the cloud (A17): the buffers read back from
## the dummy renderer hold every walker where the world has it at the screen's game time, split
## into areas that each hold their own; and the page runs ten thousand walkers within a frame.
extends GdUnitTestSuite

const CrowdPage := preload("res://pages/crowd.gd")
const STRIDE := 16
const Folders := preload("res://test/folders.gd")
const TEST_WORLDS := "user://test-worlds"


func after_test() -> void:
	Folders.remove(TEST_WORLDS)


## The Crowd page with its world kept in a folder of its own under the tests' folder, the size of a
## phone's screen.
func _page(name: String) -> VBoxContainer:
	var page: VBoxContainer = auto_free(CrowdPage.new())
	page.root = TEST_WORLDS
	page.folder = "%s/%s" % [TEST_WORLDS, name]
	page.size = Vector2(540, 1100)
	return page


## A crowd of four camps run to a morning moment, and a drawer for it.
func _small_crowd(moment: int) -> Array:
	var world := KdWorld.new()
	GameData.load_into(world)
	world.start_crowd(7, 4)
	world.run_until(moment)
	var crowd := KdCrowd.new()
	crowd.set_world(world)
	return [world, crowd]


func _multimesh() -> MultiMesh:
	var multimesh := MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D
	multimesh.use_colors = true
	return multimesh


# checks: WLD-13 PLT-01
func test_the_buffer_holds_every_walker_where_the_world_has_it() -> void:
	var moment := 9 * 3600 + 17
	var made := _small_crowd(moment)
	var world: KdWorld = made[0]
	var crowd: KdCrowd = made[1]
	var square := world.crowd_square()
	var east: int = square["west"] + square["side"] / 3
	var north: int = square["south"] + square["side"] / 2
	var then := world.places(east, north)
	assert_int(then.size()).is_equal(200)
	# the world runs on half an hour, and the screen still draws the moment it shows
	world.run_until(moment + 1800)
	var area := _multimesh()
	crowd.set_areas([area.get_rid()], 1)
	assert_int(crowd.draw(float(moment), east, north, 2.0)).is_equal(100)
	assert_int(_wrong(RenderingServer.multimesh_get_buffer(area.get_rid()), then)).is_equal(0)
	var now := world.places(east, north)
	crowd.draw(float(moment + 1800), east, north, 2.0)
	assert_int(_wrong(RenderingServer.multimesh_get_buffer(area.get_rid()), now)).is_equal(0)
	# in half an hour of the morning, walkers have moved
	var moved := 0
	for i in 100:
		if absf(then[2 * i] - now[2 * i]) + absf(then[2 * i + 1] - now[2 * i + 1]) > 1.0:
			moved += 1
	assert_int(moved).is_greater(10)


## How many of the buffer's walkers are not at their places, east then north for each.
func _wrong(buffer: PackedFloat32Array, places: PackedFloat64Array) -> int:
	var wrong := 0
	for i in places.size() / 2:
		# east is x and north is -z, in metres from the view's centre
		var x := buffer[i * STRIDE + 3]
		var z := buffer[i * STRIDE + 11]
		if absf(x - places[2 * i]) > 0.01 or absf(z + places[2 * i + 1]) > 0.01:
			wrong += 1
	return wrong


# checks: PLT-01
func test_each_area_holds_the_walkers_in_its_part_of_the_square() -> void:
	var moment := 11 * 3600
	var made := _small_crowd(moment)
	var world: KdWorld = made[0]
	var crowd: KdCrowd = made[1]
	var square := world.crowd_square()
	var west: int = square["west"]
	var south: int = square["south"]
	var side: int = square["side"]
	var areas: Array[MultiMesh] = []
	var rids := []
	for i in 4:
		areas.append(_multimesh())
		rids.append(areas[i].get_rid())
	crowd.set_areas(rids, 2)
	crowd.draw(float(moment), west, south, 2.0)
	var counts := crowd.area_counts()
	var all := 0
	var outside := 0
	for a in 4:
		all += counts[a]
		var buffer := RenderingServer.multimesh_get_buffer(rids[a])
		for i in counts[a]:
			# metres east and north of the square's south-west corner
			var e := buffer[i * STRIDE + 3]
			var n := -buffer[i * STRIDE + 11]
			var half := side / 200.0
			if int(e >= half) != a % 2 or int(n >= half) != a / 2:
				outside += 1
	assert_int(all).is_equal(100)
	assert_int(outside).is_equal(0)


# checks: PLT-01 TIM-01
func test_the_page_draws_ten_thousand_walkers_within_a_frame() -> void:
	var page := _page("draws")
	add_child(page)
	await await_idle_frame()
	page.bar.choose_speed(2)
	var slowest := 0
	for i in 30:
		await await_idle_frame()
		var started := Time.get_ticks_usec()
		page.crowd.draw(page.world.screen_time(), page.focus_east, page.focus_north, 2.0)
		slowest = maxi(slowest, Time.get_ticks_usec() - started)
	var counters: Dictionary = page.world.counters()
	assert_int(counters["walkers"]).is_equal(10000)
	var drawn := 0
	for n in page.crowd.area_counts():
		drawn += n
	assert_int(drawn).is_equal(10000)
	assert_int(counters["events"]).is_greater(0)
	# the drawing's own work stays a small part of a frame, even on the cloud's slow cores
	assert_int(slowest).override_failure_message("slowest draw %d µs" % slowest).is_less(8000)


# checks: PLT-01
func test_a_drag_moves_the_ground_with_the_finger_and_the_zoom_stays_in_bounds() -> void:
	var page := _page("drag")
	add_child(page)
	await await_idle_frame()
	var east: int = page.focus_east
	var north: int = page.focus_north
	var per_pixel: float = page.metres_per_pixel()
	# dragging right and down moves the view west and north, so the ground follows the finger
	page.pan_by(Vector2(100.0, 50.0))
	assert_int(page.focus_east).is_equal(east - roundi(100.0 * per_pixel * 100.0))
	assert_int(page.focus_north).is_equal(north + roundi(50.0 * per_pixel * 100.0))
	for i in 100:
		page.zoom_by(0.5)
	assert_float(page.metres_per_pixel() * page._view.size.y).is_equal_approx(
		CrowdPage.CLOSEST, 0.01
	)
	for i in 100:
		page.zoom_by(2.0)
	assert_float(page.metres_per_pixel() * page._view.size.y).is_equal_approx(
		CrowdPage.FARTHEST, 0.01
	)


# checks: TIM-05 PLT-07
func test_the_world_saved_as_the_page_closes_opens_again_where_it_was() -> void:
	var page := _page("again")
	add_child(page)
	await await_idle_frame()
	assert_bool(page.opened["made"]).is_true()
	page.bar.choose_speed(2)
	for i in 20:
		await await_idle_frame()
	# a camp called home: its command is written to the journal before it acts
	page.tap(page._view.size / 2.0)
	await await_idle_frame()
	page.world.save_now()
	var digest: String = page.world.digest()
	var frontier: int = page.world.frontier()
	assert_str(digest).is_not_empty()
	assert_int(page.world.counters()["saves"]).is_greater(0)
	remove_child(page)
	page.free()
	# opened again, before its first frame moves it on
	var again := _page("again")
	add_child(again)
	assert_bool(again.opened["made"]).is_false()
	assert_int(again.opened["frontier"]).is_equal(frontier)
	assert_str(again.world.digest()).is_equal(digest)
	assert_str(again.opened["snapshot"]).is_not_empty()


# checks: PLT-07
func test_a_tap_on_a_camp_calls_it_home() -> void:
	var page := _page("tap")
	add_child(page)
	await await_idle_frame()
	page.zoom_by(0.05)
	# a tap far from any camp calls none
	page.tap(Vector2.ZERO)
	assert_str(page._called).is_empty()
	# with a camp at the middle of the view, a tap there calls it home, and the journal keeps it
	var camp: int = page.world.nearest_camp(page.focus_east, page.focus_north, 2000000)
	assert_int(camp).is_greater_equal(0)
	var at: PackedInt64Array = page.world.camp_at(camp)
	page.focus_east = at[0]
	page.focus_north = at[1]
	page.tap(page._view.size / 2.0)
	assert_str(page._called).starts_with("Camp %d called home" % (camp + 1))
	page.world.save_now()
	var journal := FileAccess.get_file_as_bytes("%s/journal.log" % page.folder)
	assert_int(journal.size()).is_greater(0)
