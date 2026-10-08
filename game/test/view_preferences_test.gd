## Checks PRE-03 WLD-13 PLT-07: optional view state reopens and corruption cannot move the camera.
extends GdUnitTestSuite

const Preferences := preload("res://fixtures/view_preferences.gd")
const PATH := "user://test-view-state/view.cfg"


func test_view_state_reopens_with_exact_identity_and_rejects_nonfinite_camera() -> void:
	var saved := {
		"east": 1048576.125,
		"north": -2097152.5,
		"density": 0.000244140625,
		"manual_rate": 60.0,
		"scene": "River",
		"light": "Dusk",
		"selected": 1152921504606846978,
		"locked": true,
		"origin_east_cm": 100659476,
		"origin_north_cm": 49353014,
		"raster_origin_east_cm": 3100659476,
		"raster_origin_north_cm": -1950646986
	}
	assert_int(Preferences.write(PATH, saved)).is_equal(OK)
	var reopened := Preferences.read(PATH)
	assert_dict(reopened).is_equal(saved)
	var corrupt := ConfigFile.new()
	corrupt.load(PATH)
	corrupt.set_value("view", "east", "not a number")
	corrupt.save(PATH)
	assert_bool(Preferences.read(PATH).is_empty()).is_true()
	DirAccess.remove_absolute(PATH)
