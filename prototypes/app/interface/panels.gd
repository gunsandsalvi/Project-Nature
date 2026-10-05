## P12's panels (IMPLEMENTATION α0.7a, PRE-35, PRE-05, A15): Aru's card, the book of ages open
## at the age of hesoru, and the views, with the words and pictures of the art book's plates, laid
## out in art pixels on any of the grounds (styles.gd). A control is any node with an "action"; the
## screen finds them, and reads every touch itself. Pre-production code (research 00).
extends RefCounted

const PixelFont := preload("res://interface/pixel_font.gd")
const HandText := preload("res://interface/hand_text.gd")
const Grounds := preload("res://interface/styles.gd")
const ART := "res://interface/art/"
const SIZE := 11  # the plain font's line, one art pixel a font pixel
const MARK := Color("d9a441")  # the ochre squares beside the body's marks


## Words in the plain font, wrapped to the width given, in a colour of the ground.
static func words(text: String, colour: Color, width := 0, by := 1) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_font_override("font", PixelFont.plain())
	label.add_theme_font_size_override("font_size", SIZE * by)
	label.add_theme_color_override("font_color", colour)
	label.add_theme_constant_override("line_spacing", 0)
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if width > 0:
		label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		label.custom_minimum_size.x = width
	return label


static func picture(name: String) -> TextureRect:
	var rect := TextureRect.new()
	rect.texture = load(ART + name + ".png")
	rect.stretch_mode = TextureRect.STRETCH_KEEP
	rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return rect


## A heading in small capitals, with its rule beneath, as the card's sections have.
static func heading(text: String, ground: Dictionary, width: int) -> Control:
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 1)
	box.add_child(words(text.to_upper(), ground.head))
	var rule := ColorRect.new()
	rule.color = ground.rule
	rule.custom_minimum_size = Vector2(width, 1)
	box.add_child(rule)
	return box


## A control: its words in a box with a line under, as the plates draw them; the touch reads it
## by its action, through a hit area the screen makes at least 48 dp.
static func control(text: String, action: String, ground: Dictionary) -> Control:
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 1)
	box.set_meta("action", action)
	var label := words(text, ground.ink)
	box.add_child(label)
	var rule := ColorRect.new()
	rule.color = ground.edge
	rule.custom_minimum_size = Vector2(PixelFont.measure(text) + 6, 1)
	box.add_child(rule)
	label.custom_minimum_size.x = PixelFont.measure(text) + 6
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	# its width from the pixel font's own measure, which holds before the label is drawn
	box.custom_minimum_size.x = PixelFont.measure(text) + 6
	return box


static func column(gap: int) -> VBoxContainer:
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", gap)
	return box


static func row(gap: int) -> HBoxContainer:
	var box := HBoxContainer.new()
	box.add_theme_constant_override("separation", gap)
	return box


## Aru's card, as the art book's card plate has it (PRE-35), `width` art pixels wide: who she is,
## what she does and why, her ambition, her body and her recent talk.
static func card(ground: Dictionary, width: int) -> VBoxContainer:
	var inner := width - 16
	var page := column(4)
	var top := row(8)
	top.add_child(picture("aru"))
	var who := column(1)
	who.add_child(HandText.new("Aru", 2, ground.ink, 7))
	who.add_child(words('"river stone"', ground.dim))
	who.add_child(words("Woman, 31 · the Tavu", ground.ink))
	who.add_child(words("Potter · mother of Sefi", ground.ink))
	who.add_child(words("Mood: proud, a little tired", ground.accent))
	top.add_child(who)
	page.add_child(top)
	page.add_child(heading("Doing, and why", ground, inner))
	var doing := (
		"Firing pots for the winter stores, "
		+ "because last year the grain went damp in the baskets."
	)
	page.add_child(words(doing, ground.ink, inner))
	page.add_child(heading("Ambition", ground, inner))
	page.add_child(words("To teach Sefi the whole craft before winter.", ground.ink, inner))
	var close := row(6)
	close.add_child(Bar.new(ground, 0.45))
	close.add_child(words("close: Sefi coils, not yet fires", ground.dim))
	page.add_child(close)
	page.add_child(heading("Body", ground, inner))
	var body := row(8)
	body.add_child(picture("body"))
	var said := column(0)
	said.add_child(words("Strong, well fed.", ground.ink))
	for mark: String in ["Left hand: old burn, healed.", "Back: sore from lifting."]:
		var line := row(3)
		var square := ColorRect.new()
		square.color = MARK
		square.custom_minimum_size = Vector2(4, 4)
		square.size_flags_vertical = Control.SIZE_SHRINK_CENTER
		line.add_child(square)
		line.add_child(words(mark, ground.ink))
		said.add_child(line)
	said.add_child(words("No illness.", ground.ink))
	body.add_child(said)
	page.add_child(body)
	page.add_child(heading("Recent talk", ground, inner))
	var talk := row(10)
	for topic: String in ["pots", "goats", "fire"]:
		var one := row(4)
		one.add_child(picture("talk-" + topic))
		var label := words(topic, ground.ink)
		label.size_flags_vertical = Control.SIZE_SHRINK_CENTER
		one.add_child(label)
		talk.add_child(one)
	page.add_child(talk)
	return page


## The card's controls: the deeper views it links to, and the ground to try next.
static func card_controls(ground: Dictionary) -> Array:
	var out := []
	for name: String in ["Mind", "Family", "Crafts", "Follow"]:
		out.append(control(name, "deeper:" + name, ground))
	return out


## The book of ages open at the age of hesoru (PRE-05, PRE-39), as the art book's book plate has it.
static func book(ground: Dictionary, width: int) -> VBoxContainer:
	var inner := width - 16
	var page := column(3)
	var header := row(0)
	var title := words("THE BOOK OF AGES", ground.dim)
	title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	header.add_child(title)
	header.add_child(words("iii", ground.dim))
	page.add_child(header)
	page.add_child(HandText.new("The age of hesoru", 2, ground.ink, 3))
	page.add_child(words("fire from wood · from Year 31", ground.accent))
	var drawing := picture("shelter")
	drawing.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	page.add_child(drawing)
	page.add_child(words("The shelter by the stream, as it was kept.", ground.dim, inner))
	var entries := [
		[
			"Year 31, autumn, day 4",
			(
				"Ume of the Tavu carried a burning branch from the lightning fire to the shelter "
				+ "and kept it alive through three nights of rain. The band called it hesoru."
			)
		],
		["yours", "Yours: lightning on the oak by the stream, Year 31, autumn, day 3."],
		[
			"Year 33, spring, day 18",
			"The first hearth ringed with stones, so the fire stays where it is put."
		],
		[
			"Year 38, winter, day 9",
			"Meat dried over smoke lasted until the thaw for the first time."
		],
		[
			"Year 44, winter, day 2",
			"Ume died, aged 58. Her fire was carried to two new bands upstream."
		],
		["Year 51, summer, day 20", "The Tavu painted a red deer on the shelter wall."],
	]
	for entry: Array in entries:
		if entry[0] == "yours":
			page.add_child(_yours(entry[1], ground, inner))
			continue
		page.add_child(words(entry[0], ground.accent))
		page.add_child(words(entry[1], ground.ink, inner))
	var ume := row(8)
	ume.add_child(picture("ume"))
	var about := column(2)
	about.add_child(words("Ume, who kept the fire", ground.ink))
	about.add_child(words("of the Tavu, died Year 44, aged 58", ground.dim))
	about.add_child(words("taught the fire to Asi and Koro", ground.accent))
	about.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	ume.add_child(about)
	page.add_child(ume)
	return page


## Your own act, marked apart from the text with a bar beside it (GOD-07), and what came of it.
static func _yours(text: String, ground: Dictionary, width: int) -> Control:
	var line := row(5)
	var bar := ColorRect.new()
	bar.color = ground.link
	bar.custom_minimum_size = Vector2(2, 0)
	line.add_child(bar)
	var said := column(0)
	said.add_child(words(text, ground.link, width - 8))
	var link := words("What came of it", ground.link)
	link.set_meta("action", "deeper:What came of it")
	said.add_child(link)
	line.add_child(said)
	return line


## The book's tabs, where a thumb reaches them: at its foot in portrait.
static func book_controls(ground: Dictionary) -> Array:
	var out := []
	for name: String in ["Ages", "Peoples", "Followed", "Waiting 2", "Species"]:
		out.append(control(name, "tab:" + name, ground))
	return out


## The views behind the handle (PRE-33), each a row: its name and what it holds now.
static func views(ground: Dictionary, width: int) -> VBoxContainer:
	var page := column(0)
	page.add_child(words("VIEWS", ground.head))
	var rows := [
		["Book of ages", "The age of hesoru · 7 ages · 2 moments waiting", "book"],
		["Followed", "Aru, Sefi and Tor", "deeper:Followed"],
		["Your acts", "two at work: rain, a dream", "deeper:Your acts"],
		["Map overlays", "peoples, crafts, beliefs, land, weather", "deeper:Map overlays"],
	]
	for item: Array in rows:
		var one := column(0)
		one.set_meta("action", item[2])
		one.add_child(words(item[0], ground.ink))
		one.add_child(words(item[1], ground.dim, width - 16))
		page.add_child(one)
	return page


## Ambition's bar: how close it is, in the ground's accent.
class Bar:
	extends Control

	var ground: Dictionary
	var share := 0.5

	func _init(chosen: Dictionary, how_far: float) -> void:
		ground = chosen
		share = how_far
		custom_minimum_size = Vector2(62, 5)
		size_flags_vertical = Control.SIZE_SHRINK_CENTER
		mouse_filter = Control.MOUSE_FILTER_IGNORE

	func _draw() -> void:
		draw_rect(Rect2(0, 0, size.x, size.y), ground.edge)
		draw_rect(Rect2(1, 1, size.x - 2, size.y - 2), ground.rule)
		draw_rect(Rect2(1, 1, roundi((size.x - 2) * share), size.y - 2), ground.accent)
