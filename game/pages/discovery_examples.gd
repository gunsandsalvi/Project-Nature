## Captured playable examples from ordinary saved camps.
extends VBoxContainer


func _ready() -> void:
	add_theme_constant_override("separation", 16)
	var title := Label.new()
	title.text = "Examples"
	add_child(title)
	var words := Label.new()
	words.text = (
		"First flake\nA captured camp after someone noticed a useful edge during ordinary work. "
		+ "Follow the maker, inspect their tool, then let the camp continue."
	)
	words.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(words)
	var button := Button.new()
	button.text = "Open First flake"
	button.custom_minimum_size.y = 64
	button.pressed.connect(_open_flake)
	add_child(button)


func _open_flake() -> void:
	var shell: Node = self
	while shell != null and not shell.has_method("open_page"):
		shell = shell.get_parent()
	if shell != null:
		shell.open_page.call_deferred("FirstFlake")
