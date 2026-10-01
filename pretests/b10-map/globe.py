"""B10 part B: drawing the torus as a globe (WLD-01, WLD-02, WLD-03). Throwaway analysis.

Mapping: map column x -> longitude 2*pi*x/W; map row y -> latitude 90 - 180*y/H degrees
(y = 0 is the north edge, y = H the south edge; both are the pole seam). Globe radius
R = W / (2*pi), so the equator keeps its true length, and because W = 2H every meridian
(pole to pole = pi*R = H) keeps its true length too. Only east-west lengths shrink, by cos(lat).
"""

import math
import random

W, H = 2000.0, 1000.0  # km (WLD-03)
R = W / (2 * math.pi)


def to_globe(x, y):
    lon = 2 * math.pi * x / W
    lat = math.pi / 2 - math.pi * y / H
    return (R * math.cos(lat) * math.cos(lon), R * math.cos(lat) * math.sin(lon), R * math.sin(lat))


def great_circle(p, q):
    dot = sum(a * b for a, b in zip(p, q)) / (R * R)
    return R * math.acos(max(-1.0, min(1.0, dot)))


def map_distance(p, q, wrap_ns):
    dx = abs(q[0] - p[0]) % W
    dx = min(dx, W - dx)
    dy = abs(q[1] - p[1])
    if wrap_ns:
        dy = min(dy % H, H - dy % H)
    return math.hypot(dx, dy)


def tissot():
    print("Local distortion by latitude (east-west scale, north-south scale, area, worst angle):")
    print("  lat | E-W  | N-S  | area | worst angle change | map rows -> globe")
    for lat in (0, 15, 30, 45, 60, 70, 75, 80, 85, 89):
        k = math.cos(math.radians(lat))
        w = 2 * math.degrees(math.asin((1 - k) / (1 + k)))
        print(f"  {lat:>3} | {k:.2f} | 1.00 | {k:.2f} | {w:5.1f} deg | a 1 km square shows as {k:.2f} x 1 km")
    print()


def area_shares():
    # Latitude is linear in y, so map area is uniform in latitude.
    print("Share of the map by how much the globe shrinks it:")
    for lat in (60, 75.5, 84.3):
        k = math.cos(math.radians(lat))
        print(f"  beyond {lat:>4} deg latitude (area scale under {k:.2f}): {100 * (1 - lat / 90):.1f}% of the map, "
              f"{100 * (1 - math.sin(math.radians(lat))):.1f}% of the globe")
    print(f"  mean area scale over the map: {2 / math.pi:.3f} (globe area {4 * math.pi * R * R / 1e6:.2f} M km2 "
          f"for a {W * H / 1e6:.0f} M km2 map)")
    for cap_km in (50, 100):
        lat = 90 - 180 * cap_km / H
        print(f"  an ice cap {cap_km} km wide on each side of the seam (beyond {lat:.0f} deg): "
              f"{100 * 2 * cap_km / H:.0f}% of the map, {100 * (1 - math.sin(math.radians(lat))):.1f}% of the globe")
    print()


def pairs(n=200_000, seed=10):
    rnd = random.Random(seed)
    print("Distance read off the globe (great circle) / distance on the map, random pairs:")
    for label, ymax_lat in (("whole map", 90.0), ("both points within 60 deg of the equator", 60.0)):
        ratios = []
        y0 = H / 2 - H * ymax_lat / 180
        y1 = H / 2 + H * ymax_lat / 180
        for _ in range(n):
            p = (rnd.uniform(0, W), rnd.uniform(y0, y1))
            q = (rnd.uniform(0, W), rnd.uniform(y0, y1))
            d = map_distance(p, q, wrap_ns=False)  # travel never crosses the pole seam
            if d < 1.0:
                continue
            ratios.append(great_circle(to_globe(*p), to_globe(*q)) / d)
        ratios.sort()
        m = len(ratios)
        pct = lambda f: ratios[min(m - 1, int(f * m))]
        print(f"  {label}: median {pct(0.5):.2f}, 5% {pct(0.05):.2f}, 95% {pct(0.95):.2f}, min {ratios[0]:.2f}, max {ratios[-1]:.2f}")
    print()


def seams():
    print("Seams on the globe:")
    a, b = to_globe(1000, 0.0), to_globe(1000, H - 1e-9)
    print(f"  north-south seam: map rows y=0 and y=H (neighbours on the torus) land {great_circle(a, b):.0f} km apart, "
          "at the two poles; every point of row 0 lands on the north pole, every point of row H on the south pole")
    a, b = to_globe(0.0, 300), to_globe(W - 1e-9, 300)
    print(f"  east-west seam: columns x=0 and x=W land {great_circle(a, b):.6f} km apart (continuous)")
    print()


def equal_area_alternative():
    print("Rejected alternative, equal-area globe (rows placed by sin(latitude)):")
    for lat in (30, 60, 75):
        y_frac = 0.5 - lat / 180  # where that simulated latitude sits on the map
        shown = math.degrees(math.asin(1 - 2 * y_frac))
        print(f"  simulated {lat} deg would be drawn at {shown:.1f} deg")
    print()


if __name__ == "__main__":
    print(f"Globe radius for display: {R:.1f} km (equator {W:.0f} km, meridian pole to pole {math.pi * R:.0f} km)\n")
    tissot()
    area_shares()
    pairs()
    seams()
    equal_area_alternative()
