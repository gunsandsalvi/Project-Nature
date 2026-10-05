"""P12's pictures (IMPLEMENTATION α0.7a): the stand-in world and the small pictures the interface shows, cut from the
art book's plates at their true size, one art pixel to one pixel, so the app draws them as the book does.

- world/<stop>.png and world/<stop>-land.png: the art book's zoom stops at noon, held upright and sideways
  (art/book/plates/zoom), from the person to the globe.
- art/: from the interface plates (art/book/plates/ui), Aru's portrait and body, the talk pictures and the time
  controls from the card and the sheet, and the book's drawing and portrait of Ume, each with the plate's ground
  around it made clear.

    python3 prototypes/app/interface/make_art.py

Pre-production code (research 00).
"""

from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
PLATES = ROOT / "art" / "book" / "plates"
STOPS = ("person", "closecamp", "camp", "valley", "region", "map", "globe")
FRAME = (138, 106, 80)  # the plates' brown picture frame


def frame_box(im, region):
    """The box of the brown frame inside a region of a plate: a picture with its frame."""
    x0, y0, x1, y1 = region
    xs, ys = [], []
    for y in range(y0, y1):
        for x in range(x0, x1):
            if im.getpixel((x, y))[:3] == FRAME:
                xs.append(x)
                ys.append(y)
    return (min(xs), min(ys), max(xs) + 1, max(ys) + 1)


def clear(im, ground, slack=6):
    """A picture with the plate's ground around it, and colours within slack of it, made clear."""
    out = im.convert("RGBA")
    px = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, _ = px[x, y]
            if max(abs(r - ground[0]), abs(g - ground[1]), abs(b - ground[2])) <= slack:
                px[x, y] = (0, 0, 0, 0)
    return out


def main():
    world = HERE / "world"
    art = HERE / "art"
    world.mkdir(exist_ok=True)
    art.mkdir(exist_ok=True)
    for stop in STOPS:
        Image.open(PLATES / "zoom" / f"{stop}-noon.png").convert("RGB").save(world / f"{stop}.png", optimize=True)
        Image.open(PLATES / "zoom" / f"{stop}-noon-land.png").convert("RGB").save(
            world / f"{stop}-land.png", optimize=True
        )
    card = Image.open(PLATES / "ui" / "card.png").convert("RGB")
    # each search kept left of the text beside it, whose dimmer lines share the frame's brown
    card.crop(frame_box(card, (4, 375, 84, 475))).save(art / "aru.png")
    # the body, its paper and the paper's darker spots made clear; the talk pictures keep their square, a small tile
    # as their bubbles in the world are
    body = card.crop((14, 571, 38, 619)).convert("RGBA")
    px = body.load()
    for y in range(body.height):
        for x in range(body.width):
            r, g, b, _ = px[x, y]
            if r > 200 and g > 185 and b > 150:
                px[x, y] = (0, 0, 0, 0)
    body.save(art / "body.png")
    for name, x in (("pots", 12), ("goats", 74), ("fire", 136)):
        icon = card.crop((x, 636, x + 19, 656))
        clear(icon, icon.getpixel((0, 0)), 3).save(art / f"talk-{name}.png")
    book = Image.open(PLATES / "ui" / "book.png").convert("RGB")
    book.crop(frame_box(book, (4, 82, 332, 245))).save(art / "shelter.png")
    book.crop(frame_box(book, (4, 505, 80, 615))).save(art / "ume.png")
    # the time controls, each a round button on the sheet's dark bar
    sheet = Image.open(PLATES / "ui" / "sheet.png").convert("RGB")
    for name, cx in (("pause", 39), ("play", 68), ("dial", 96), ("lock", 126), ("skip", 154)):
        button = sheet.crop((cx - 12, 128, cx + 13, 153))
        clear(button, button.getpixel((0, 0)), 2).save(art / f"time-{name}.png")
    print(f"make_art: {len(STOPS) * 2} world pictures, the card's, the book's and the time controls'")


if __name__ == "__main__":
    main()
