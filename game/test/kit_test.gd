## The Kit page, headless in the cloud (A6.5, A17): it reads the build's families and recipes, puts
## every recipe's thing together and every part of the kit on the ground, and says what each is;
## a seed makes the same thing every time; and what cannot be put together says why.
extends GdUnitTestSuite

const KitPage := preload("res://pages/kit.gd")


func _page() -> Control:
	var page: Control = auto_free(KitPage.new())
	add_child(page)
	return page


## The recipe the build lists with the most copies, which is the one a seed has most to vary.
func _busiest(page: Control) -> String:
	var busiest := ""
	var most := 0
	for model: String in page.models:
		var copies: int = page.kit.model_info(model).get("copies", 0)
		if copies > most:
			busiest = model
			most = copies
	return busiest


# checks: PRE-46 PRE-22
func test_the_kit_page_puts_every_recipe_together_and_says_what_it_shows() -> void:
	var page := _page()
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	assert_array(Array(page.models)).is_not_empty()
	# every recipe has its place on the ground, and the first is the one on show
	assert_int(page.places.size()).is_equal(page.models.size())
	assert_str(page.showing).is_equal(page.models[0])
	for model: String in page.models:
		var info: Dictionary = page.kit.model_info(model)
		assert_str(info.get("problem", "")).is_empty()
		assert_int(info["triangles"]).is_greater(0)
		assert_int(info["copies"]).is_greater(0)
	# the sheet's words: the thing, its parts, its copies and triangles, and what each role wears
	var words: String = page.shown[page.shown.size() - 1]
	assert_str(words).starts_with(page.models[0])
	assert_str(words).contains("triangles")
	assert_str(words).contains("roles: ")


# checks: PRE-46
func test_every_part_of_the_kit_stands_alone_and_says_how_far_its_texture_stretches() -> void:
	var page := _page()
	await await_idle_frame()
	page.show_parts()
	assert_str(page.problem).is_empty()
	assert_str(page.mode).is_equal("parts")
	var parts := 0
	for family: String in GameData.models(GameData.build()):
		for part: String in page.kit.parts(family):
			parts += 1
			var info: Dictionary = page.kit.part_info(family, part)
			# the line the kit's check holds every part to is 1.5 to 1 (A6.4)
			assert_float(info["stretch"]).is_less_equal(1.5)
			assert_int(info["triangles"]).is_greater(0)
	assert_int(parts).is_greater(0)
	assert_int(page.places.size()).is_equal(parts)
	assert_str(page.shown[page.shown.size() - 1]).contains("stretch at most")
	# and back to the things
	page.show_things()
	assert_str(page.mode).is_equal("things")
	assert_int(page.places.size()).is_equal(page.models.size())


# checks: PRE-46 TIM-16
func test_a_seed_makes_the_same_thing_every_time_and_another_seed_another() -> void:
	var page := _page()
	await await_idle_frame()
	var model := _busiest(page)
	assert_str(model).is_not_empty()
	var a: Dictionary = page.kit.place(page.scenario(), model, 5, 0, -2400, 0, 0.0, {})
	var b: Dictionary = page.kit.place(page.scenario(), model, 5, 0, -3000, 0, 0.0, {})
	var c: Dictionary = page.kit.place(page.scenario(), model, 6, 0, -3600, 0, 0.0, {})
	for made: Dictionary in [a, b, c]:
		assert_str(made["problem"]).is_empty()
	for key: String in ["triangles", "lowest", "highest"]:
		assert_that(b[key]).is_equal(a[key])
	var differs := false
	for key: String in ["triangles", "lowest", "highest"]:
		differs = differs or c[key] != a[key]
	assert_bool(differs).is_true()
	for made: Dictionary in [a, b, c]:
		page.kit.remove(made["id"])


# checks: PRE-46
func test_a_thing_that_cannot_be_put_together_says_why_and_puts_nothing_down() -> void:
	var page := _page()
	await await_idle_frame()
	var missing: Dictionary = page.kit.place(
		page.scenario(), "art:no_such_thing", 1, 0, 0, 0, 0.0, {}
	)
	assert_str(missing["problem"]).contains("no model art:no_such_thing")
	assert_bool(missing.has("id")).is_false()
	var part: Dictionary = page.kit.place_part(
		page.scenario(), "no_family", "no_part", 0, 0, 0, 0.0, {}
	)
	assert_str(part["problem"]).contains("no part no_part")
	# a role given no texture is named, and nothing of the thing is left on the screen
	var model := _busiest(page)
	var info: Dictionary = page.kit.model_info(model)
	var role: String = (info["roles"] as Dictionary).keys()[0]
	var bare: Dictionary = page.kit.place(
		page.scenario(), model, 1, 0, 0, 0, 0.0, {role: "art:not_a_texture"}
	)
	assert_str(bare["problem"]).contains("art:not_a_texture")
	assert_bool(bare.has("id")).is_false()
