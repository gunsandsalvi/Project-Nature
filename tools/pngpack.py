"""PLT-06 PRC-11: compact runtime PNG encoding without changing a decoded RGBA byte."""

import io

from PIL import Image


def lossless(picture):
    """Use an exact palette or omit an entirely opaque alpha channel; keep the smaller PNG."""
    with Image.open(io.BytesIO(picture)) as source:
        # Preserve colour-profile interpretation by retaining the original encoding.
        if any(key in source.info for key in ("icc_profile", "gamma", "srgb")):
            return picture
        image = source.convert("RGBA")
        target = image
        colours = image.getcolors(256)
        if colours is not None:
            palette = sorted(colour for _, colour in colours)
            lookup = {colour: index for index, colour in enumerate(palette)}
            raw = image.tobytes()
            indices = bytes(lookup[tuple(raw[index : index + 4])] for index in range(0, len(raw), 4))
            target = Image.frombytes("P", image.size, indices)
            target.putpalette([value for colour in palette for value in colour[:3]])
            if any(colour[3] != 255 for colour in palette):
                target.info["transparency"] = bytes(colour[3] for colour in palette)
        elif image.getchannel("A").getextrema() == (255, 255):
            target = image.convert("RGB")
        buffer = io.BytesIO()
        target.save(buffer, format="PNG", optimize=True, compress_level=9)
        packed = buffer.getvalue()
        # Fail packaging instead of distributing any unexpected codec conversion.
        with Image.open(io.BytesIO(packed)) as decoded:
            if decoded.convert("RGBA").tobytes() != image.tobytes():
                raise ValueError("lossless PNG packing changed decoded pixels")
        return packed if len(packed) < len(picture) else picture
