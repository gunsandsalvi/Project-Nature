## The Compare page, headless in the cloud (A5.5): it loads its ground and asks ten pairs;
## answering them all gives the code the cloud reads, and the right answers are those that chose
## the better way.
extends GdUnitTestSuite

const ComparePage := preload("res://pages/compare.gd")


# checks: PRE-01
func test_ten_answers_give_a_code_and_count_the_better_ways_chosen() -> void:
	var page: VBoxContainer = auto_free(ComparePage.new())
	add_child(page)
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	assert_int(page.pairs.size()).is_equal(10)
	for pair: Dictionary in page.pairs:
		assert_str(page.code).is_empty()
		page.choose(pair["better_first"])
	assert_int(page.right()).is_equal(10)
	assert_int(page.code.length()).is_equal(13)
	# a new test starts empty; always the top picture is right where the better way was on top
	page.start()
	assert_str(page.code).is_empty()
	for i in 10:
		page.choose(true)
	var on_top: int = (
		page.pairs.filter(func(p: Dictionary) -> bool: return p["better_first"]).size()
	)
	assert_int(page.right()).is_equal(on_top)
