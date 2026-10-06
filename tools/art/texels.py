"""Small helpers the art lane's tools share: pictures as arrays of texture pixels, lossless saving, digests, a blur
that wraps round a tile, and the sRGB curve for work in linear light.

Pictures are numpy arrays of shape (height, width, 3), 8-bit sRGB. Nothing here measures colour: OKLab, accents and
texture pixel contrast come only from `kindling look` (art/BRIEF.md, rule 7).

Implements PRE-22, see A5.4.
"""

import hashlib

import numpy as np
from PIL import Image


def load(path):
    """A picture as an (h, w, 3) uint8 array, any alpha dropped."""
    with Image.open(path) as im:
        return np.asarray(im.convert("RGB"), dtype=np.uint8).copy()


def save_png(a, path):
    """Saves a picture as lossless PNG with no metadata, so its digest depends on its pixels alone."""
    Image.fromarray(np.ascontiguousarray(a, dtype=np.uint8), "RGB").save(path, format="PNG", optimize=True)


def save_webp(a, path, quality=None):
    """Saves a picture as WebP with no metadata: lossless when no quality is given, else lossy at that quality."""
    im = Image.fromarray(np.ascontiguousarray(a, dtype=np.uint8), "RGB")
    if quality is None:
        im.save(path, format="WEBP", lossless=True, quality=100, method=6, exact=True)
    else:
        im.save(path, format="WEBP", quality=quality, method=6)


def sha256(path):
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def enlarge(a, k):
    """Each texture pixel as a k x k square of picture pixels."""
    return np.repeat(np.repeat(a, k, axis=0), k, axis=1)


def gauss_kernel(sigma):
    r = max(1, int(np.ceil(3 * sigma)))
    x = np.arange(-r, r + 1, dtype=np.float64)
    k = np.exp(-0.5 * (x / sigma) ** 2)
    return k / k.sum()


def blur(a, sigma, wrap=True):
    """A separable Gaussian blur over the first two axes; edges wrap round a tile, or reflect for a picture."""
    out = np.asarray(a, dtype=np.float64)
    if sigma <= 0:
        return out.copy()
    k = gauss_kernel(sigma)
    r = len(k) // 2
    for axis in (0, 1):
        n = out.shape[axis]
        if wrap:
            idx = np.arange(-r, n + r) % n
        else:
            idx = np.abs(np.arange(-r, n + r))
            idx = np.where(idx >= n, 2 * (n - 1) - idx, idx).clip(0, n - 1)
        p = np.take(out, idx, axis=axis)
        acc = np.zeros_like(out)
        for i, w in enumerate(k):
            acc += w * np.take(p, np.arange(i, i + n), axis=axis)
        out = acc
    return out


def to_linear(a):
    """8-bit sRGB to linear light, 0 to 1."""
    c = np.asarray(a, dtype=np.float64) / 255.0
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def to_srgb(lin):
    """Linear light, 0 to 1, to 8-bit sRGB."""
    c = np.clip(lin, 0.0, 1.0)
    c = np.where(c <= 0.0031308, c * 12.92, 1.055 * c ** (1 / 2.4) - 0.055)
    return np.round(c * 255.0).astype(np.uint8)


def luminance(a):
    """Linear-light luminance (Rec. 709 weights) of an 8-bit sRGB picture: a light level, not a colour measure."""
    return to_linear(a) @ np.array([0.2126, 0.7152, 0.0722])
