## Checks PRE-31: direct entry and shell navigation both build a drawing.
extends GdUnitTestSuite

const Examples := preload("res://pages/examples.gd")


func test_direct_entry_builds_the_view_and_service_without_parent_busy_errors() -> void:
	var page: Control = Examples.new()
	page.frozen = true
	add_child(page)
	await await_idle_frame()
	page._process(0)
	assert_object(page.drawing).is_not_null()
	assert_bool(page._stream.is_inside_tree()).is_true()
	assert_int(page._viewport.size.x).is_greater(2)
	page.free()


func test_shell_navigation_builds_the_same_view() -> void:
	var shell: Control = preload("res://main.gd").new()
	add_child(shell)
	shell.open_page("Examples")
	await await_idle_frame()
	shell._page._process(0)
	assert_object(shell._page.drawing).is_not_null()
	assert_bool(shell._page._stream.is_inside_tree()).is_true()
	shell.free()
