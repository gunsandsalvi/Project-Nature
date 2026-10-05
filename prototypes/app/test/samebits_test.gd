## P5's screen and its extension: the toy world run by the app on one thread and on four, against
## the cloud's recorded digest, and the line for the chat.
extends GdUnitTestSuite

const SAMEBITS := preload("res://samebits/samebits.gd")


# checks: RES-05, TIM-16
func test_the_app_runs_the_cloud_s_world_and_gets_its_digest() -> void:
	assert_bool(ClassDB.class_exists("SameBits")).is_true()
	var days: int = ClassDB.class_call_static("SameBits", "days")
	var one: PackedStringArray = ClassDB.class_call_static("SameBits", "run", 1, days)
	var four: PackedStringArray = ClassDB.class_call_static("SameBits", "run", 4, days)
	# the digest, then each day's checksum
	assert_int(one.size()).is_equal(days + 1)
	assert_array(Array(four)).is_equal(Array(one))
	assert_str(one[0]).is_equal(ClassDB.class_call_static("SameBits", "cloud_digest"))


# checks: RES-05
func test_the_line_for_the_chat() -> void:
	var same := {1: ["1bbbe1d787b4d4fe", 2500], 4: ["1bbbe1d787b4d4fe", 2300]}
	assert_str(SAMEBITS.line(same, "1bbbe1d787b4d4fe", "α0.4a")).is_equal(
		(
			"P5 α0.4a | 1 thread 1bbbe1d787b4d4fe 2.5 s | 4 threads 1bbbe1d787b4d4fe 2.3 s"
			+ " | cloud 1bbbe1d787b4d4fe | same"
		)
	)
	var off := {1: ["1bbbe1d787b4d4fe", 2500], 4: ["0000000000000001", 2300]}
	assert_str(SAMEBITS.line(off, "1bbbe1d787b4d4fe", "")).ends_with("| DIFFERENT")
