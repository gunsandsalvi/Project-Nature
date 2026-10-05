## The grounds a card or the book of ages can stand on (IMPLEMENTATION α0.7a, PRE-35), for you to
## choose from: the art book's paper and four others, each with the colours its words take. Each is
## drawn pixel by pixel from a fixed pattern, so it is crisp and the same every time.
## Pre-production code (research 00).
extends Control

const GROUNDS := [
	{
		"name": "Paper",
		"says": "the art book's journal paper",
		"back": Color("efe4c8"),
		"spot": Color("e2d3ae"),
		"pattern": "mottle",
		"ink": Color("2e2620"),
		"dim": Color("8a6a50"),
		"head": Color("8a6a50"),
		"accent": Color("9a3a2a"),
		"link": Color("3f6488"),
		"rule": Color("d6c6a2"),
		"edge": Color("8a6a50"),
	},
	{
		"name": "Night",
		"says": "the dark of the time controls and the views",
		"back": Color(0.086, 0.075, 0.114, 0.95),
		"spot": Color(0, 0, 0, 0),
		"pattern": "plain",
		"ink": Color("ebe5da"),
		"dim": Color("a39ca9"),
		"head": Color("f6a33c"),
		"accent": Color("f08a64"),
		"link": Color("8fb7ff"),
		"rule": Color("3a3346"),
		"edge": Color("4a4258"),
	},
	{
		"name": "Hide",
		"says": "tanned hide, stitched at its edge",
		"back": Color("3e2c20"),
		"spot": Color("35251a"),
		"pattern": "mottle",
		"ink": Color("f2e3c6"),
		"dim": Color("c4a782"),
		"head": Color("e3ad52"),
		"accent": Color("ec8a5a"),
		"link": Color("a9c8e8"),
		"rule": Color("5e4532"),
		"edge": Color("a07a52"),
	},
	{
		"name": "Slate",
		"says": "dark stone, with chalk and ochre",
		"back": Color("2e343e"),
		"spot": Color("272c35"),
		"pattern": "speckle",
		"ink": Color("e9edf0"),
		"dim": Color("9da7b3"),
		"head": Color("e7a65a"),
		"accent": Color("ee8a68"),
		"link": Color("9cc4ff"),
		"rule": Color("47505e"),
		"edge": Color("566070"),
	},
	{
		"name": "Glass",
		"says": "dark glass the world shows through, its words shadowed",
		"shadow": Color(0.02, 0.016, 0.03, 0.9),
		"back": Color(0.055, 0.047, 0.078, 0.9),
		"spot": Color(0, 0, 0, 0),
		"pattern": "screen",
		"ink": Color("f4eee2"),
		"dim": Color("c8c0d0"),
		"head": Color("f6a33c"),
		"accent": Color("ff9a78"),
		"link": Color("a9ccff"),
		"rule": Color("5a5268"),
		"edge": Color("6a6278"),
	},
]

var ground: Dictionary
var _drawn: ImageTexture


func _init(chosen: Dictionary) -> void:
	ground = chosen
	mouse_filter = Control.MOUSE_FILTER_IGNORE


## A pixel's own number, the same every time, for the patterns.
static func hashed(x: int, y: int) -> int:
	var h := (x * 374761393 + y * 668265263) & 0x7FFFFFFF
	h = ((h ^ (h >> 13)) * 1274126177) & 0x7FFFFFFF
	return h ^ (h >> 16)


func _draw() -> void:
	var w := int(size.x)
	var h := int(size.y)
	if w <= 0 or h <= 0:
		return
	if _drawn == null or _drawn.get_size() != Vector2(w, h):
		_drawn = ImageTexture.create_from_image(picture(ground, w, h))
	draw_texture(_drawn, Vector2.ZERO)


## The ground as a picture w by h, made once for each size: its colour or screen, its pattern and
## its edge.
static func picture(chosen: Dictionary, w: int, h: int) -> Image:
	var back: Color = chosen.back
	var spot: Color = chosen.spot
	var edge: Color = chosen.edge
	var im := Image.create(w, h, false, Image.FORMAT_RGBA8)
	match String(chosen.pattern):
		"screen":
			# one pixel in four left clear, so the world shows through without a blur (PRE-01)
			for y in h:
				for x in w:
					if (x & 1) == 0 or (y & 1) == 0:
						im.set_pixel(x, y, back)
		_:
			im.fill(back)
	match String(chosen.pattern):
		"mottle":
			# soft spots of the darker tone, a few pixels across, as the art book's paper has
			for cy in range(0, h, 7):
				for cx in range(0, w, 7):
					var n := hashed(cx, cy)
					if n % 5 != 0:
						continue
					var r := 1 + n % 3
					var ox := cx + (n >> 4) % 7
					var oy := cy + (n >> 8) % 7
					for dy in range(-r, r + 1):
						var span := r - absi(dy)
						for dx in range(-span, span + 1):
							_dot(im, ox + dx, oy + dy, spot)
		"speckle":
			for y in h:
				for x in w:
					var n := hashed(x, y) % 100
					if n < 6:
						im.set_pixel(x, y, spot)
					elif n < 8:
						im.set_pixel(x, y, chosen.rule)
	for x in w:
		im.set_pixel(x, 0, edge)
		im.set_pixel(x, h - 1, edge)
	for y in h:
		im.set_pixel(0, y, edge)
		im.set_pixel(w - 1, y, edge)
	if String(chosen.name) == "Hide":
		# stitches three pixels in from the edge, two on and two off
		for x in range(4, w - 4, 4):
			for k in 2:
				_dot(im, x + k, 3, edge)
				_dot(im, x + k, h - 4, edge)
		for y in range(4, h - 4, 4):
			for k in 2:
				_dot(im, 3, y + k, edge)
				_dot(im, w - 4, y + k, edge)
	return im


static func _dot(im: Image, x: int, y: int, colour: Color) -> void:
	if x >= 0 and y >= 0 and x < im.get_width() and y < im.get_height():
		im.set_pixel(x, y, colour)
