## Implements PRE-26 PRE-30 PLT-02: independent integer bitmap text and physical touch sizes.
extends RefCounted

const Preferences := preload("res://ui/preferences.gd")


static func font_size(logical: float, density: float) -> int:
	var larger := 1.5 if Preferences.value("large_text") else 1.0
	return 16 * ceili(maxf(16.0, logical) * larger * density / 16.0)


static func page(root: Control, density: float) -> void:
	var nodes: Array[Node] = [root]
	nodes.append_array(root.find_children("*", "Control", true, false))
	for child: Node in nodes:
		var control := child as Control
		control.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		if control is Button:
			control.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		if not control.has_meta("ui_minimum"):
			control.set_meta("ui_minimum", control.custom_minimum_size)
		var minimum: Vector2 = control.get_meta("ui_minimum")
		if control is BaseButton or control is LineEdit or control is Range:
			minimum.y = maxf(48.0, minimum.y)
		control.custom_minimum_size = minimum * density
		if control is Label or control is BaseButton or control is LineEdit or control is TextEdit:
			if not control.has_meta("ui_font_size"):
				control.set_meta("ui_font_size", control.get_theme_font_size("font_size"))
			control.add_theme_font_size_override(
				"font_size", font_size(float(control.get_meta("ui_font_size")), density)
			)
		if control is BoxContainer:
			if not control.has_meta("ui_gap"):
				control.set_meta("ui_gap", maxi(8, control.get_theme_constant("separation")))
			control.add_theme_constant_override(
				"separation", roundi(int(control.get_meta("ui_gap")) * density)
			)

		if control is GridContainer:
			for key: String in ["h_separation", "v_separation"]:
				if not control.has_meta("ui_" + key):
					control.set_meta("ui_" + key, maxi(8, control.get_theme_constant(key)))
				control.add_theme_constant_override(
					key, roundi(int(control.get_meta("ui_" + key)) * density)
				)

		if control is RichTextLabel:
			for key: String in [
				"normal_font_size",
				"bold_font_size",
				"italics_font_size",
				"bold_italics_font_size",
				"mono_font_size"
			]:
				control.add_theme_font_size_override(key, font_size(16, density))
		if control.has_method("set_ui_density"):
			control.set_ui_density(density)
