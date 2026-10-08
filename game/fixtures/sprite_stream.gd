## Implements PRE-03 PLT-04 WLD-13 (T2.9a.2): one worker, main-thread upload and publication.
## The service survives page/world swaps; consumers release bundles after detaching draw references.
extends Node

signal published(token: int)

const Prepare := preload("res://fixtures/sprite_prepare.gd")
const CHANNELS := ["colour", "normal", "material"]
var ledger := KdStream.new()
var headroom_us := 2000
var last_upload_us := 0
var problem := ""
var _worker: Thread
var _job: Dictionary = {}
var _upload: Dictionary = {}
var _resident: Dictionary = {}
var _retired_gpu: Dictionary = {}
var _upload_cooldown := 0
var _retired_allocations: Dictionary = {}


static func service(tree: SceneTree) -> Node:
	var found := tree.root.get_node_or_null("SpriteStream")
	if found == null:
		found = load("res://fixtures/sprite_stream.gd").new()
		found.name = "SpriteStream"
		tree.root.add_child(found)
	return found


func begin(identity: Dictionary, limits: Dictionary) -> Dictionary:
	# Existing worker ownership remains counted until its completion is polled.
	var answer: Dictionary = ledger.begin(identity, limits)
	problem = answer.problem
	return answer


func request(spec: Dictionary) -> Dictionary:
	return ledger.request(spec)


func bundle(token: int) -> Dictionary:
	return _resident.get(token, {})


func release(token: int) -> void:
	# Caller has removed its sprite/polygon references before this acknowledgement.
	ledger.cancel(token)
	_resident.erase(token)
	_retired_gpu[token] = Engine.get_process_frames() + 2


func reserve_allocation(spec: Dictionary) -> Dictionary:
	return ledger.reserve_allocation(spec)


func retire_allocation(token: int) -> void:
	if token != 0:
		_retired_allocations[token] = Engine.get_process_frames() + 2


func _process(_delta: float) -> void:
	for token: int in _retired_allocations.keys():
		if Engine.get_process_frames() >= _retired_allocations[token]:
			ledger.release_allocation(token)
			_retired_allocations.erase(token)
	_finish_worker()
	_dispose_retired()
	if _upload.is_empty():
		var ready: Array = ledger.take_ready(1)
		if not ready.is_empty():
			_upload = ready[0]
			_upload.textures = {}
			_upload.next_channel = 0
	if not _upload.is_empty():
		_upload_one()
	if _worker == null:
		var jobs: Array = ledger.take_jobs(1)
		if not jobs.is_empty():
			_job = jobs[0]
			_worker = Thread.new()
			var started := _worker.start(Prepare.prepare.bind(_job))
			if started != OK:
				var failed := _job.duplicate(true)
				failed.problem = "Could not start sprite preparation."
				failed.cpu_bytes = 0
				ledger.ready(_job.token, failed)
				ledger.disposed(_job.token, "input")
				_worker = null
				_job.clear()


func _finish_worker() -> void:
	if _worker == null or _worker.is_alive():
		return
	var result: Dictionary = _worker.wait_to_finish()
	var token: int = _job.token
	_worker = null
	_job.clear()
	var answer: Dictionary = ledger.ready(token, result)
	result.clear()
	ledger.disposed(token, "input")
	if not answer.ok:
		problem = answer.problem
		ledger.disposed(token, "prepared")


func _upload_one() -> void:
	if _upload_cooldown > 0:
		_upload_cooldown -= 1
		return
	var token: int = _upload.token
	var channel: String = CHANNELS[_upload.next_channel]
	var image: Image = _upload.images[channel]
	var bytes := image.get_width() * image.get_height() * 4
	var answer: Dictionary = ledger.stage(token, channel, bytes)
	if not answer.ok:
		# Headroom may be held by a retiring mask/target. Keep this complete CPU parent.
		for row: Dictionary in ledger.status().jobs:
			if int(row.token) == token and row.state == "ready":
				return
		problem = answer.problem
		_abandon_upload()
		return
	var started := Time.get_ticks_usec()
	var texture := ImageTexture.create_from_image(image)
	last_upload_us = Time.get_ticks_usec() - started
	_upload.textures[channel] = texture
	answer = ledger.uploaded(token, channel, bytes, last_upload_us)
	if not answer.ok:
		problem = answer.problem
		_abandon_upload()
		return
	_upload.next_channel += 1
	# At most one bounded channel per frame; measured slow uploads leave recovery frames.
	_upload_cooldown = mini(8, maxi(0, ceili(float(last_upload_us) / maxi(1, headroom_us)) - 1))
	if _upload.next_channel == CHANNELS.size():
		answer = ledger.publish(token)
		if answer.ok:
			# Keep the colour image for exact picking; it remains in the CPU ledger until release.
			_resident[token] = {
				"textures": _upload.textures, "images": _upload.images, "key": _upload.key
			}
			_upload.clear()
			problem = ""
			published.emit(token)
		else:
			problem = answer.problem
			_abandon_upload()


func _abandon_upload() -> void:
	var token: int = _upload.token
	ledger.cancel(token)
	_upload.clear()
	ledger.disposed(token, "staging")
	_retired_gpu[token] = Engine.get_process_frames() + 2


func _dispose_retired() -> void:
	var owned := {}
	for job: Dictionary in ledger.status().jobs:
		owned[job.token] = job.worker_owned
	for token: int in _retired_gpu.keys():
		if owned.get(token, false):
			continue
		if Engine.get_process_frames() >= int(_retired_gpu[token]):
			ledger.disposed(token, "prepared")
			ledger.disposed(token, "gpu")
			_retired_gpu.erase(token)
	var current: Dictionary = ledger.status()
	for job: Dictionary in current.jobs:
		if job.state != "retiring" or job.worker_owned:
			continue
		var token: int = job.token
		if _resident.has(token) or _retired_gpu.has(token) or int(_upload.get("token", 0)) == token:
			continue
		ledger.disposed(token, "input")
		ledger.disposed(token, "prepared")
		if not _retired_gpu.has(token):
			ledger.disposed(token, "staging")
			ledger.disposed(token, "gpu")


func _exit_tree() -> void:
	# Only application shutdown waits; switching worlds leaves this root service running.
	if _worker != null:
		_worker.wait_to_finish()
		_worker = null
