## The art book's pixel fonts in Godot (IMPLEMENTATION α0.7a, PRE-35, A15), from glyphs.json, which
## tools/pixel-font.py writes from font.js, where they are designed: the plain font for reading, a
## bitmap font of one art pixel to a font pixel that labels draw at whole multiples only; and the
## handwriting for big titles, slanted, wobbling and joined as the art book draws it.
## Pre-production code (research 00).
extends RefCounted

const GLYPHS := "res://interface/glyphs.json"
## The plain font's one size: its line, from the cap's top less two rows above for marks.
const ABOVE := 2

static var _data := {}
static var _plain: FontFile


static func data() -> Dictionary:
	if _data.is_empty():
		_data = JSON.parse_string(FileAccess.get_file_as_string(GLYPHS))
	return _data


static func glyph(ch: String) -> Array:
	var glyphs: Dictionary = data().glyphs
	return glyphs.get(ch, glyphs["?"])


## The plain font, made once: every glyph in one atlas, a pixel each, at its one size, the line's
## height; a label asking twice the size draws it twice as large, whole pixels kept (titles at 2).
static func plain() -> FontFile:
	if _plain != null:
		return _plain
	var d := data()
	var line := int(d.line)
	var cap := int(d.cap)
	var glyphs: Dictionary = d.glyphs
	var width := 0
	for ch: String in glyphs:
		width += String(glyphs[ch][0]).length() + 1
	var atlas := Image.create(width, 9, false, Image.FORMAT_LA8)
	var font := FontFile.new()
	font.antialiasing = TextServer.FONT_ANTIALIASING_NONE
	font.subpixel_positioning = TextServer.SUBPIXEL_POSITIONING_DISABLED
	font.hinting = TextServer.HINTING_NONE
	font.generate_mipmaps = false
	font.fixed_size = line
	font.fixed_size_scale_mode = TextServer.FIXED_SIZE_SCALE_INTEGER_ONLY
	var size := Vector2i(line, 0)
	font.set_cache_ascent(0, line, ABOVE + cap)
	font.set_cache_descent(0, line, line - ABOVE - cap)
	var x := 0
	for ch: String in glyphs:
		var rows: Array = glyphs[ch]
		var w := String(rows[0]).length()
		for r in rows.size():
			for c in w:
				if String(rows[r])[c] == "#":
					atlas.set_pixel(x + c, r, Color.WHITE)
		var code := ch.unicode_at(0)
		font.set_glyph_advance(0, line, code, Vector2(w + 1, 0))
		font.set_glyph_offset(0, size, code, Vector2(0, -cap))
		font.set_glyph_size(0, size, code, Vector2(w, rows.size()))
		font.set_glyph_uv_rect(0, size, code, Rect2(x, 0, w, rows.size()))
		font.set_glyph_texture_idx(0, size, code, 0)
		x += w + 1
	font.set_texture_image(0, size, 0, atlas)
	_plain = font
	return font


## Text's width in font pixels at one pixel each, as font.js measures it.
static func measure(text: String, hand := false) -> int:
	var w := 0
	for ch in text:
		w += String(glyph(ch)[0]).length() + 1
	return maxi(0, w - 1) + (int(data().hand_extra) if hand else 0)
