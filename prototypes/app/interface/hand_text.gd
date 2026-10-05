## A title in the pixel handwriting (IMPLEMENTATION α0.7a, PRE-35), drawn as font.js draws it,
## from the glyphs pixel_font.gd reads: each glyph's rows leant one pixel for every few drawn, a
## letter now and then a pixel high, and a letter that ends at the foot joined to a small one after
## it. Pre-production code (research 00).
extends Control

const PixelFont := preload("res://interface/pixel_font.gd")

var text := ""
var scale_by := 2
var ink := Color.BLACK
var seed := 1


func _init(words: String, by: int, colour: Color, seeded := 1) -> void:
	text = words
	scale_by = by
	ink = colour
	seed = seeded
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	var d: Dictionary = PixelFont.data()
	custom_minimum_size = Vector2(PixelFont.measure(text, true) * by + by, (int(d.cap) + 3) * by)


func _draw() -> void:
	var d: Dictionary = PixelFont.data()
	var lean_every: float = d.lean_every
	var wobble: float = d.wobble
	var ends_low := String(d.ends_low)
	var starts_x := String(d.starts_x)
	var s := seed
	var cx := 0
	var prev := ""
	var top := scale_by
	for ch in text:
		var rows: Array = PixelFont.glyph(ch)
		var w := String(rows[0]).length()
		s = (s * 16807) % 2147483647
		var dy := (-scale_by) if float(s) / 2147483647.0 < wobble else 0
		for r in rows.size():
			for c in w:
				if String(rows[r])[c] == "#":
					_put(cx + c * scale_by, r, top + r * scale_by + dy, lean_every)
		if prev != "" and ends_low.contains(prev) and starts_x.contains(ch):
			_put(cx - scale_by, 6, top + 6 * scale_by + dy, lean_every)
		cx += (w + 1) * scale_by
		prev = ch


## One font pixel at row r: a block of the scale, each of its rows leant on its own.
func _put(px: int, r: int, py: int, lean_every: float) -> void:
	for k in scale_by:
		var lean := floori((6 * scale_by - (r * scale_by + k)) / lean_every)
		draw_rect(Rect2(px + lean, py + k, scale_by, 1), ink)
