## Checks PRE-03 PRE-22 PLT-04 WLD-13 (T2.9a.2): retry, fallback and eviction use the actual controller ledger.
extends GdUnitTestSuite

const Atlas := preload("res://fixtures/streamed_atlas.gd")
const Stream := preload("res://fixtures/sprite_stream.gd")
var stream: Node
var atlas: RefCounted
var identity := {
	"world_id": "atlas-regression",
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
						"second": 12.5,
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
	atlas = Atlas.new()
	atlas._service = stream
	atlas._identity = identity.duplicate()
	atlas._revision = 1
	for family_name: String in ["far", "middle"]:
		atlas._families["boulder/" + family_name] = {
			"asset": "boulder",
			"family": family_name,
			"density": 4 if family_name == "far" else 16,
			"page_width": 2,
			"page_height": 2,
			"colour": "art:unit",
			"normal": "art:unit",
			"material": "art:unit",
			"cells": [{"pivot_x_256": 256, "pivot_y_256": 256}]
		}
	atlas._descriptors[GameData.texture_path("art:unit")] = {
		"path": "unit-unused", "max_bytes": 32, "sha256": "unit-unused"
	}


func _frame(density: float) -> Dictionary:
	return {"density": density, "target_density": density}


func _publish(token: int) -> void:
	var jobs: Array = stream.ledger.take_jobs(1)
	assert_int(jobs.size()).is_equal(1)
	if jobs.is_empty():
		return
	assert_int(jobs[0].token).is_equal(token)
	var images := {}
	for channel: String in ["colour", "normal", "material"]:
		images[channel] = Image.create(2, 2, false, Image.FORMAT_RGBA8)
		images[channel].fill(Color.WHITE)
	var job: Dictionary = jobs[0]
	(
		assert_bool(
			(
				stream
				. ledger
				. ready(
					token,
					{
						"epoch": job.epoch,
						"generation": job.generation,
						"revision": job.revision,
						"key": job.key,
						"images": images,
						"cpu_bytes": 48,
						"problem": ""
					}
				)
				. ok
			)
		)
		. is_true()
	)
	stream.ledger.disposed(token, "input")
	for channel: String in ["colour", "normal", "material"]:
		assert_bool(stream.ledger.stage(token, channel, 16).ok).is_true()
		assert_bool(stream.ledger.uploaded(token, channel, 16, 1).ok).is_true()
	assert_bool(stream.ledger.publish(token).ok).is_true()
	# No GPU resource is allocated in this fixture; owned images stand in for a published bundle.
	stream._resident[token] = {"textures": {}, "images": images, "key": job.key}


func test_failed_slot_retries_after_backoff_instead_of_staying_permanently_missing() -> void:
	atlas._request("boulder", 4, {})
	var old_token: int = atlas._requests["boulder/4/0"].token
	var jobs: Array = stream.ledger.take_jobs(1)
	assert_int(jobs.size()).is_equal(1)
	(
		assert_bool(
			(
				stream
				. ledger
				. ready(
					old_token,
					{"problem": "deliberate decoder failure", "cpu_bytes": 0, "images": {}}
				)
				. ok
			)
		)
		. is_false()
	)
	atlas._reconcile(_frame(4))
	assert_bool(atlas._requests.has("boulder/4/0")).is_false()
	assert_bool(atlas._retry_at.has("boulder/4/0")).is_true()
	atlas._request("boulder", 4, {})
	assert_bool(atlas._requests.has("boulder/4/0")).is_false()
	# Expire the clock directly: the test needs no real one-second sleep.
	atlas._retry_at["boulder/4/0"] = 0
	atlas._request("boulder", 4, {})
	assert_bool(atlas._requests.has("boulder/4/0")).is_true()
	if atlas._requests.has("boulder/4/0"):
		assert_int(atlas._requests["boulder/4/0"].token).is_not_equal(old_token)


func test_zoom_out_evicts_obsolete_fine_bundle_and_keeps_visible_parent_fallback() -> void:
	atlas._request("boulder", 4, {})
	var far: Dictionary = atlas._requests["boulder/4/0"]
	_publish(far.token)
	atlas._request("boulder", 16, far.key)
	var fine: Dictionary = atlas._requests["boulder/16/0"]
	_publish(fine.token)
	assert_int(stream.ledger.status().resident_bytes).is_equal(96)
	atlas._reconcile(_frame(4))
	assert_bool(atlas._requests.has("boulder/16/0")).is_false()
	assert_bool(atlas._requests.has("boulder/4/0")).is_true()
	assert_bool(stream.bundle(fine.token).is_empty()).is_true()
	var fallback: Dictionary = atlas.sample({"name": "boulder"}, 16)
	assert_float(fallback.get("density", 0)).is_equal(4.0)
	assert_object(fallback.get("image")).is_equal(stream.bundle(far.token).images.colour)
	# Detached handles retain the old charge through two process frames.
	assert_int(stream.ledger.status().resident_bytes).is_equal(96)
	for frame in 3:
		await get_tree().process_frame
	stream._process(0)
	assert_int(stream.ledger.status().resident_bytes).is_equal(48)
