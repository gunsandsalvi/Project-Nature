## The Time page, headless in the cloud (A17): the calendar starts at the start of history, and the
## speeds you choose change the speed it really runs at.
extends GdUnitTestSuite

const TimePage := preload("res://pages/time.gd")


# checks: TIM-14
func test_the_calendar_starts_at_year_1_spring_day_1() -> void:
	var page: VBoxContainer = auto_free(TimePage.new())
	add_child(page)
	await await_idle_frame()
	assert_str(page.date_text()).is_equal("Year 1, spring, day 1")
	assert_str(page.world.time_text()).is_equal("00:00")


# checks: TIM-01 TIM-10
func test_its_speeds_change_the_speed_shown() -> void:
	var page: VBoxContainer = auto_free(TimePage.new())
	add_child(page)
	await await_idle_frame()
	page.choose_speed(1)
	await get_tree().create_timer(1.5).timeout
	assert_float(page.world.speed_shown()).is_between(50.0, 70.0)
	page.choose_speed(3)
	await get_tree().create_timer(1.5).timeout
	assert_float(page.world.speed_shown()).is_between(18000.0, 25000.0)
	page.toggle_pause()
	await get_tree().create_timer(1.5).timeout
	assert_float(page.world.speed_shown()).is_equal(0.0)
	assert_bool(page.world.screen_time() <= float(page.world.frontier())).is_true()


# checks: TIM-01
func test_speeds_read_as_you_would_say_them() -> void:
	assert_str(TimePage.speed_words(1.0, false)).is_equal("real speed, a game second a second")
	assert_str(TimePage.speed_words(60.0, false)).is_equal("1 game hour a real minute")
	assert_str(TimePage.speed_words(480.0, false)).is_equal("8 game hours a real minute")
	assert_str(TimePage.speed_words(21600.0, false)).is_equal("1 game season a real minute")
	assert_str(TimePage.speed_words(259200.0, false)).is_equal("3 game years a real minute")
	assert_str(TimePage.speed_words(5.0e6, false)).is_equal("58 game years a real minute")
	assert_str(TimePage.speed_words(0.0, true)).is_equal("paused")


# checks: TIM-01
func test_its_speeds_come_from_the_tuning_file() -> void:
	var page: VBoxContainer = auto_free(TimePage.new())
	add_child(page)
	await await_idle_frame()
	assert_array(page.speeds).is_equal([1.0, 60.0, 480.0, 21600.0, 259200.0, TimePage.TOP])
	assert_str(TimePage.button_words(1.0)).is_equal("Real")
	assert_str(TimePage.button_words(60.0)).is_equal("1 hour a minute")
	assert_str(TimePage.button_words(259200.0)).is_equal("3 years a minute")
	assert_str(TimePage.button_words(TimePage.TOP)).is_equal("Top")
