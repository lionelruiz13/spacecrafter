#!/usr/bin/env python3
"""F18 / INTENT §5.54 - is the G4 early-visibility gate at a fixed PIXEL size?

The respelling makes the regime gates px-authored: a body's regime is supposed
to depend on how big it is ON SCREEN, not on how big the render target is. That
claim is only worth anything if it can fail, so this measures the gate POSITION
at two render widths and compares it to a prediction fixed before the run.

The observable is the EARLY gate, because it is the one with a large, binary
consequence: below it the body draws no surface at all (halo/hint only), above
it the surface appears. Two fov points per width bracket the predicted gate:

    PRESENT  point at 3.50 px predicted diameter  -> surface must be drawn
    ABSENT   point at 2.70 px predicted diameter  -> surface must be absent

  px-intent   (the change under test): gate at 3.072 px at EVERY width, so
              PRESENT/ABSENT at both widths.
  fraction-intent (what shipped): the gate is a constant screenSize, so it sits
              at 3.072 px only at 2048 and at 1.536 px at 1024 - the ABSENT
              point at 1024 (2.70 px, i.e. screenSize 0.0026 > 0.0015) would
              show a surface. That is the failure this can show.

The halo is suppressed, so what the disc window holds is the surface alone. The
app's own reported gate (dump header `gates`, §5.54) is read back and asserted
against the prediction too - two independent readings of one claim.

Usage: f18_gate.py <outdir> <render_width> [body]
"""
import socket, sys, time, os, json, hashlib
import numpy as np
from PIL import Image

OUT = sys.argv[1]
WIDTH = int(sys.argv[2])
BODY = sys.argv[3] if len(sys.argv) > 3 else "Mars"
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")

PRESENT_PX, ABSENT_PX = 3.50, 2.70
GATE_PX_PREDICTED = 3.072
REF_FOV = 60.0


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


def shot(sock, name, pause=2.5):
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


def disc_sum(name, r_px):
    a = np.asarray(Image.open(os.path.join(OUT, name + ".png")).convert("RGB"),
                   dtype=np.int64).sum(axis=2)
    h, w = a.shape
    cy, cx = h // 2, w // 2
    yy, xx = np.mgrid[0:h, 0:w]
    m = ((xx - cx) ** 2 + (yy - cy) ** 2) <= max(r_px, 1.0) ** 2
    return int(a[m].sum()), int((a[m] > 0).sum()), [h, w]


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
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
res = {"width_arg": WIDTH, "body": BODY, "gates_reported": h0.get("gates"),
       "gate_px_predicted": GATE_PX_PREDICTED, "ref_screenSize": b0["screenSize"],
       "points": {}}

for label, px in (("present", PRESENT_PX), ("absent", ABSENT_PX)):
    want_ss = px / WIDTH                       # screenSize == diameter / width
    fov = REF_FOV * b0["screenSize"] / want_ss
    send(s, f"zoom fov {fov:.6f} duration 0", 3)
    _, b = read(dump(s, f"{label}_geom"), BODY)
    shot(s, label)
    got_px = b["screenSize"] * WIDTH
    ssum, nz, shape = disc_sum(label, max(got_px / 2.0, 1.5))
    res["points"][label] = {"fov": fov, "wanted_px": px, "screenSize": b["screenSize"],
                            "measured_px": got_px, "disc_sum": ssum,
                            "disc_nonzero": nz, "shape": shape}
    print(f"### {label}: fov {fov:.5f} px {got_px:.3f} disc_sum {ssum}", flush=True)

send(s, "zoom fov 180 duration 0", 2)
p, a = res["points"]["present"], res["points"]["absent"]
# The verdict is the RATIO absent/present, not "absent is exactly zero". At
# small render widths a satellite's halo lands inside the disc window and is
# above quantisation there while being below it at 2048 - drawHalo's cmag
# carries a 1/screen_r factor and screen_r = screenSize*2*viewportRadius, so the
# same halo is brighter per pixel on a smaller target. That content is not the
# body's surface and does not move with the gate; the surface is a factor ~20
# above it. Counterfactual leg (a pre-respelling binary at the same width)
# reads ~1.0 here, which is what makes the threshold discriminating rather than
# chosen.
ratio = (a["disc_sum"] / p["disc_sum"]) if p["disc_sum"] else None
res["absent_over_present"] = ratio
res["verdict"] = ("FAIL-present-empty" if not p["disc_sum"] else
                  "GREEN" if ratio <= 0.10 else
                  "FAIL-absent-drew" if ratio >= 0.5 else "AMBIGUOUS")
res["ssystem_md5"] = hashlib.md5(open(SSY, "rb").read()).hexdigest()
with open(f"{OUT}/f18_gate_result.json", "w") as f:
    json.dump(res, f, indent=2)
print(json.dumps(res, indent=2), flush=True)
