#!/usr/bin/env python3
"""F18 / INTENT §5.53 - what actually changes when a body crosses the big-texture gate.

The row says the reduced and the full level of ONE colour map do not render alike
(Sun disc centre [253,246,218] vs [250,199,107]) and reasons that since a
downscale preserves the mean, the cause must be in the reduction/cache path.

This measures the step on TWO bodies and states the prediction first, because
the two predictions are different in kind and only one cause produces both:

    ratio = rendered disc mean at the REDUCED level / at the FULL level

  A) a code-side transform in the reduction path would give ONE signature
     (the same ratio shape on every body).
  B) the loader preferring an authored `<name>-preview.<ext>` file over any
     downscale (s_texture.cpp asyncLoad) gives the ratio of the TWO FILES'
     own means - per body, unrelated between bodies:
        Sun   sun-preview.jpg [254.0,246.2,223.5] / sun.jpg [250.7,195.9,94.5]
              -> [1.01, 1.26, 2.37]   strongly colour-shifted
        Moon  moon-preview.jpg [184.1,183.3,181.5] / moon.jpg [150.5,147.0,146.1]
              -> [1.22, 1.25, 1.24]   near-neutral, ~+24% on all channels

The disc mean is used rather than the centre pixel, and the ratio rather than
either level alone, so that lighting and projection cancel: the two captures
differ ONLY in fov, so the illuminated hemisphere is identical.

The whole-file mean is an approximation of the visible hemisphere's mean, so a
few percent of residual is expected and is NOT evidence either way; what
discriminates is the SHAPE (colour-shifted vs neutral) and the magnitude class.

Usage: f18_level.py <outdir> <body>
"""
import socket, sys, time, os, json, hashlib
import numpy as np
from PIL import Image

OUT = sys.argv[1]
BODY = sys.argv[2] if len(sys.argv) > 2 else "Sun"
# Optional jd: the step is a TEXTURE ratio, so the body has to be lit for the
# texture to be what the pixels carry. At the b12 date the Moon is near new -
# disc mean 5.3/255, where quantisation crushes any ratio toward 1. The lit
# date is found by measurement (elongation from the app's own dump), not recall.
JD = sys.argv[3] if len(sys.argv) > 3 else "2461233.5"
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
REF_FOV = 60.0
BELOW, ABOVE = 0.60, 1.60          # factors on the reported gate


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def shot(sock, name, pause=3.0):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)


def dump(sock, name, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return f"{OUT}/{name}.json"


def read(path, name):
    head, body = None, None
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "header":
            head = d
        elif d.get("type") == "body" and d.get("name") == name:
            body = d.get("new") if isinstance(d.get("new"), dict) else None
    return head, body


def disc_mean(name, r_px):
    """Mean RGB over the inner 70% of the disc - away from the limb, where the
    silhouette's partial pixels and the terminator would weight the two
    captures differently."""
    a = np.asarray(Image.open(os.path.join(OUT, name + ".png")).convert("RGB"),
                   dtype=np.float64)
    h, w, _ = a.shape
    cy, cx = h // 2, w // 2
    yy, xx = np.mgrid[0:h, 0:w]
    m = ((xx - cx) ** 2 + (yy - cy) ** 2) <= (0.7 * r_px) ** 2
    lit = m & (a.sum(axis=2) > 12)      # drop the unlit hemisphere
    return ([round(v, 2) for v in a[lit].mean(axis=0)] if lit.sum() else None,
            int(m.sum()), int(lit.sum()))


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)
send(s, f"date jday {JD}", 1)
send(s, "timerate rate 0", 1)
for f in ("landscape", "atmosphere", "star_twinkle", "stars", "show_fps",
          "planet_names", "planet_orbits", "object_trails",
          "constellation_drawing", "constellation_art", "cardinal_points",
          "milky_way", "nebulae"):
    send(s, f"flag {f} off", 0.4)
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, f"select planet {BODY} pointer off", 1)
send(s, "flag track_object on", 8)
send(s, "timerate rate 0", 1)
send(s, f"body name {BODY} color halo r 0 g 0 b 0", 1)

send(s, f"zoom fov {REF_FOV} duration 0", 3)
h0, b0 = read(dump(s, "ref"), BODY)
gate = h0["gates"]["screenSize"]["bigTexture"]
res = {"body": BODY, "jd": JD, "gate_screenSize": gate, "gates": h0["gates"], "levels": {}}

for label, factor in (("reduced", BELOW), ("full", ABOVE)):
    want = gate * factor
    fov = REF_FOV * b0["screenSize"] / want
    send(s, f"zoom fov {fov:.6f} duration 0", 4)
    # The full level arrives ASYNCHRONOUSLY (getBigTexture + rebind), so settle
    # and take the LAST of three - a capture taken before the swap would read
    # the reduced level and quietly report "no step".
    for i in range(3):
        shot(s, f"{label}_{i}")
    _, b = read(dump(s, f"{label}_geom"), BODY)
    r_px = b["screenSize"] * 2048 / 2.0
    rgb, npx, nlit = disc_mean(f"{label}_2", r_px)
    prev, _, _ = disc_mean(f"{label}_1", r_px)
    res["levels"][label] = {"fov": fov, "screenSize": b["screenSize"],
                            "r_px": round(r_px, 2), "disc_mean_rgb": rgb,
                            "settled_same_as_previous": rgb == prev,
                            "npx": npx, "nlit": nlit}
    print(f"### {label}: ss {b['screenSize']:.5f} rgb {rgb}", flush=True)

send(s, "zoom fov 180 duration 0", 2)
r, f_ = res["levels"]["reduced"]["disc_mean_rgb"], res["levels"]["full"]["disc_mean_rgb"]
res["ratio_reduced_over_full"] = ([round(a / b, 4) for a, b in zip(r, f_)]
                                  if r and f_ and all(f_) else None)
res["ssystem_md5"] = hashlib.md5(open(SSY, "rb").read()).hexdigest()
with open(f"{OUT}/f18_level_result.json", "w") as fp:
    json.dump(res, fp, indent=2)
print(json.dumps(res, indent=2), flush=True)
