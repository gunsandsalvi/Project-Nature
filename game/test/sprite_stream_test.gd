## Checks PRE-03 PLT-04 WLD-13 (T2.9a.2): main-thread bundle publication and disposal.
extends GdUnitTestSuite

const Stream := preload("res://fixtures/sprite_stream.gd")
var stream: Node
var identity := {
	"world_id": "test",
	"data_hash": "data",
	"look_hash": "look",
	"renderer": "test",
	"format_version": 1,
	"epoch": 1
}


func before_test() -> void:
	stream = auto_free(Stream.new())
	add_child(stream)
	stream.set_process(false)
	assert_bool(stream.begin(identity, {}).ok).is_true()
	(
		assert_bool(
			(
				stream
				. ledger
				. manifest(
					{
						"epoch": 1,
						"revision": 1,
						"second": 0,
						"surfaces": {},
						"casters": {},
						"appearances": {}
					}
				)
				. ok
			)
		)
		. is_true()
	)


func _ready_bundle() -> int:
	var key := identity.duplicate()
	key.erase("epoch")
	key.merge(
		{
			"asset": "test",
			"family": "far",
			"form": "individual",
			"level": 0,
			"tile_east": 0,
			"tile_north": 0,
			"surface_revision": 0,
			"caster_revision": 0,
			"appearance_revision": 0
		}
	)
	var answer: Dictionary = stream.request(
		{
			"key": key,
			"parent_key": {},
			"generation": 1,
			"revision": 1,
			"input": {},
			"reserve": {"input": 65536, "prepared": 48, "resident": 48, "category": "sprites"}
		}
	)
	assert_bool(answer.ok).is_true()
	var jobs: Array = stream.ledger.take_jobs(1)
	assert_int(jobs.size()).is_equal(1)
	var images := {}
	for channel: String in ["colour", "normal", "material"]:
		var image := Image.create(2, 2, false, Image.FORMAT_RGBA8)
		image.fill(Color.WHITE)
		images[channel] = image
	(
		assert_bool(
			(
				stream
				. ledger
				. ready(
					answer.token,
					{
						"key": key,
						"epoch": 1,
						"generation": 1,
						"revision": 1,
						"cpu_bytes": 48,
						"images": images,
						"problem": ""
					}
				)
				. ok
			)
		)
		. is_true()
	)
	stream.ledger.disposed(answer.token, "input")
	return answer.token


func test_bundle_becomes_visible_only_after_all_three_main_thread_uploads() -> void:
	var token := _ready_bundle()
	stream._process(0.016)
	assert_bool(stream.bundle(token).is_empty()).is_true()
	stream._upload_cooldown = 0
	stream._process(0.016)
	assert_bool(stream.bundle(token).is_empty()).is_true()
	stream._upload_cooldown = 0
	stream._process(0.016)
	assert_int(stream.bundle(token).textures.size()).is_equal(3)
	assert_int(stream.ledger.status().resident_bytes).is_equal(48)
	assert_int(stream.ledger.status().prepared_bytes).is_equal(48)


func test_world_swap_during_partial_upload_never_publishes_old_images() -> void:
	var token := _ready_bundle()
	stream._process(0.016)
	var next := identity.duplicate()
	next.epoch = 2
	next.world_id = "another"
	stream.begin(next, {})
	stream._upload_cooldown = 0
	stream._process(0.016)
	assert_bool(stream.bundle(token).is_empty()).is_true()
	assert_int(stream.ledger.status().resident_bytes).is_equal(16)
	for frame in 3:
		await get_tree().process_frame
	stream._process(0.016)
	assert_int(stream.ledger.status().resident_bytes).is_equal(0)
	assert_int(stream.ledger.status().prepared_bytes).is_equal(0)
