## Checks PRE-42 PRE-43 PRE-46 PLT-04 (T2.9a.2/4): source channels stay truthful and immutable.
extends GdUnitTestSuite

const Prepare := preload("res://test/support/fixtures/sprite_prepare.gd")


func test_south_normal_is_converted_once_without_changing_the_source() -> void:
	var image := Image.create_from_data(
		1, 1, false, Image.FORMAT_RGBA8, PackedByteArray([128, 255, 128, 255])
	)
	var converted: Image = Prepare.converted(image, "normal", "world-east-south-up", "fixture27-v1")
	assert_int(converted.get_data()[1]).is_equal(0)
	assert_int(image.get_data()[1]).is_equal(255)
	var north: Image = Prepare.converted(converted, "normal", "world-east-north-up", "fixture27-v1")
	assert_array(north.get_data()).is_equal(converted.get_data())


func test_material_ids_map_exactly_and_preserve_coverage() -> void:
	var bytes := PackedByteArray()
	for id in range(1, 9):
		bytes.append_array(PackedByteArray([id, 0, 0, id * 20]))
	var image := Image.create_from_data(8, 1, false, Image.FORMAT_RGBA8, bytes)
	var converted: Image = Prepare.converted(
		image, "material", "world-east-south-up", "fixture27-v1"
	)
	var expected := [1, 4, 1, 7, 7, 3, 2, 2]
	for index in 8:
		assert_int(converted.get_data()[index * 4]).is_equal(expected[index])
		assert_int(converted.get_data()[index * 4 + 3]).is_equal((index + 1) * 20)
	assert_array(image.get_data()).is_equal(bytes)


func test_unknown_material_is_rejected_instead_of_drawing_a_wrong_surface() -> void:
	var image := Image.create_from_data(
		1, 1, false, Image.FORMAT_RGBA8, PackedByteArray([20, 0, 0, 255])
	)
	(
		assert_object(Prepare.converted(image, "material", "world-east-south-up", "fixture27-v1"))
		. is_null()
	)
