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


# checks: PRE-01, PRE-46
func test_the_leaf_edges_test_asks_its_own_question_and_its_code_counts_the_same_way() -> void:
	var page: VBoxContainer = auto_free(ComparePage.new())
	add_child(page)
	await await_idle_frame()
	page.start(page.LEAF_EDGES)
	assert_int(page.comparison).is_equal(2)
	assert_int(page.pairs.size()).is_equal(10)
	# the leaves' views keep to the plants' bank and path, within 3 m east or west
	for pair: Dictionary in page.pairs:
		assert_int(absi(int(pair["east"]))).is_less_equal(300)
	for pair: Dictionary in page.pairs:
		page.choose(not pair["better_first"])
	assert_int(page.right()).is_equal(0)
	assert_int(page.code.length()).is_equal(13)
	# "Again" keeps the comparison
	page.start(page.comparison)
	assert_int(page.comparison).is_equal(2)
	assert_str(page.code).is_empty()


# checks: PRE-01, PRE-30
func test_the_fire_test_draws_two_grounds_and_leaves_one_after() -> void:
	var page: VBoxContainer = auto_free(ComparePage.new())
	add_child(page)
	await await_idle_frame()
	page.start(page.FIRE_SHADOWS)
	assert_int(page.comparison).is_equal(3)
	assert_str(page.problem).is_empty()
	# the fires' views keep round the camp, within 1.5 m
	for pair: Dictionary in page.pairs:
		assert_int(absi(int(pair["north"]))).is_less_equal(150)
	for pair: Dictionary in page.pairs:
		page.choose(pair["better_first"])
	assert_int(page.right()).is_equal(10)
	# another test brings the day back, with one ground
	page.start(page.SHARPNESS)
	assert_object(page._mapped).is_null()
