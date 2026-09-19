#!/usr/bin/env python3
"""F123 driver -- THE PRESENCE LADDER for the near-body disappearance.

One leg per launch (F123_LEG in the environment).  Legs:

  moon5    anchored on the Moon at the SHIPPED display scale (x5), altitude
           stepped ACROSS the predicted threshold (8687.0 km above the
           displayed datum), one grab per DRAWN path per rung + the dump.
  moon1    the same ladder with `flag moon_scaled off`, the ramp waited out BY
           MEASUREMENT (S5.109), threshold re-based to 1737.4 km.
  fov      altitude held at two rungs (one side of the threshold each) with
           fov 180 and fov 60 -- P4's fov-independence leg.
  earth    Earth at 3000 km altitude, the scene INTENT/11.105 BLOCKER 1
           measured, swept over four longitudes so the lit one is in the set.
  default  THE REGRESSION SURFACE: the shipped default view, standing on
           Earth.  Grabs the launch transient first (no quiesce sleep in the
           runner for this leg), then an A/A pair, then both drawn paths.

WHAT IS ASSERTED IS PRESENCE, NEVER A DIFF (S11.99(e): an A/B diff cancels a
missing disc -- the two frames agree about the pixels the body did not paint).
Presence is read as the count of lit pixels inside a CENTRED DISC that lies
wholly inside the body's own disc at every rung of its ladder, so "the body is
there" is a positive count and "the body is gone" is the star field's residue.

THE ILLUMINATION IS CALIBRATED IN-RUN, NOT ASSUMED.  A body's disc is only
bright where the sun reaches it, and the sub-solar longitude at a pinned date
is not a value this harness may recall (S11.51(d)).  So the first rung of a
body ladder sweeps four longitudes 90 deg apart and the leg continues at the
one whose OLD-path frame is brightest -- the old path draws the body at every
rung by construction (P3), so it is the honest calibrator.
"""
import json, math, os, socket, sys, time
import numpy as np
from PIL import Image

OUT = sys.argv[1]
LEG = os.environ.get("F123_LEG", "moon5")
JD = 2461234.0          # pinned date (b3_ladder's earth-site jd, reused)
LONS = [0, 90, 180, 270]
DISC_R = 200            # px radius of the presence disc (see module docstring)
LIT = 32                # per-channel lit threshold (INTENT 11.105's "px > 32")

RESULTS = {"leg": LEG, "jd": JD, "rungs": []}


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print("%.3f >> %s" % (time.time(), cmd), flush=True)


def shot(sock, name, pause=2.5):
    """One screenshot, waited for on disk (the app writes it a frame later)."""
    p = os.path.join(OUT, name + ".png")
    try:
        os.remove(p)
    except OSError:
        pass
    send(sock, "body action screenshot filename %s" % p, pause)
    last = -1
    for _ in range(40):
        if os.path.exists(p):
            sz = os.path.getsize(p)
            if sz > 0 and sz == last:
                return p
            last = sz
        time.sleep(0.25)
    print("MISSING SHOT %s" % p, flush=True)
    return None


def measure(png):
    """lit px in the centred presence disc, lit px over the whole frame, and
    the disc's mean luminance -- all three recorded, none derived from the
    other frame."""
    if not png or not os.path.exists(png):
        return None
    a = np.asarray(Image.open(png).convert("RGB"), dtype=np.int16)
    h, w = a.shape[0], a.shape[1]
    yy, xx = np.ogrid[:h, :w]
    disc = (xx - w / 2.0) ** 2 + (yy - h / 2.0) ** 2 <= DISC_R * DISC_R
    lum = a.max(axis=2)
    return {"png": os.path.basename(png), "w": w, "h": h,
            "lit_disc": int((lum[disc] > LIT).sum()),
            "disc_px": int(disc.sum()),
            "lit_frame": int((lum > LIT).sum()),
            "mean_disc": float(lum[disc].mean())}


def dump(sock, name, pause=2.0):
    p = os.path.join(OUT, name + ".json")
    try:
        os.remove(p)
    except OSError:
        pass
    send(sock, "body action dual_dump filename %s" % p, pause)
    for _ in range(40):
        if os.path.exists(p) and os.path.getsize(p) > 0:
            break
        time.sleep(0.25)
    bodies = {}
    try:
        with open(p) as f:
            for line in f:
                line = line.strip().rstrip(",")
                if not line.startswith("{"):
                    continue
                try:
                    o = json.loads(line)
                except Exception:
                    continue
                if o.get("type") == "body" and o.get("new") is not None:
                    bodies[o["name"]] = o["new"]
    except OSError:
        pass
    return bodies


AU_KM = 149597870.0


def rec(bodies, name):
    m = bodies.get(name) or {}
    return {"dist_km": m.get("dist", 0) * AU_KM,
            "scaledDatumRadius_km": m.get("scaledDatumRadius", 0) * AU_KM,
            "boundingRadius_km": m.get("boundingRadius", 0) * AU_KM,
            "screenSize": m.get("screenSize"),
            "scaling": m.get("scaling"), "scalingTarget": m.get("scalingTarget"),
            "visible": m.get("visible"), "routing": m.get("routing")}


def both_paths(sock, tag):
    """One grab per DRAWN path.  The new path is the shipped default, so it is
    grabbed first and the old-path detour is closed behind it."""
    new = measure(shot(sock, tag + "_new"))
    send(sock, "flag experimental_path off", 2.0)
    old = measure(shot(sock, tag + "_old"))
    send(sock, "flag experimental_path on", 2.0)
    return old, new


def wait_scale_settled(sock, body, tag, tries=20):
    """S5.109: `flag moon_scaled off` is a RAMP, and `moveto ... alt` counts
    from the display-scaled datum at the INSTANT of the command and never
    re-converges.  Settled is asserted by MEASUREMENT, never slept."""
    prev, trace = None, []
    for _ in range(tries):
        m = rec(dump(sock, "%s_settle" % tag, 0.9), body)
        trace.append(m)
        sc, tgt = m.get("scaling"), m.get("scalingTarget")
        if sc is not None and tgt is not None:
            if abs(sc - tgt) < 1e-4 and prev is not None and abs(sc - prev) < 1e-4:
                RESULTS["settle_%s" % tag] = trace
                return m
            prev = sc
        else:
            last = m["scaledDatumRadius_km"]
            if prev is not None and abs(last - prev) <= 1e-6 * max(1.0, abs(last)):
                RESULTS["settle_%s" % tag] = trace
                return m
            prev = last
    RESULTS["settle_%s" % tag] = trace
    print("SCALE NEVER SETTLED for %s" % body, flush=True)
    return None


def calibrate_lon(sock, body, alt_m, tag):
    """Four longitudes 90 deg apart, judged on the OLD path (which draws the
    body at every rung by construction).  Returns the brightest."""
    best, best_lit = LONS[0], -1
    for lon in LONS:
        send(sock, "moveto lat 0 lon %d alt %d duration 0" % (lon, alt_m), 2.5)
        b = dump(sock, "%s_cal%d" % (tag, lon), 1.5)
        old, new = both_paths(sock, "%s_cal%d" % (tag, lon))
        r = {"rung": "cal", "lon": lon, "alt_m": alt_m,
             "body": rec(b, body), "old": old, "new": new}
        RESULTS["rungs"].append(r)
        print("CAL lon=%3d  dist=%.1f km  old.lit_disc=%s  new.lit_disc=%s"
              % (lon, r["body"]["dist_km"], old and old["lit_disc"],
                 new and new["lit_disc"]), flush=True)
        if old and old["lit_disc"] > best_lit:
            best_lit, best = old["lit_disc"], lon
    print("CALIBRATED lon=%d (old lit_disc %d of %d)"
          % (best, best_lit, math.pi * DISC_R * DISC_R), flush=True)
    RESULTS["lon"] = best
    return best


def ladder(sock, body, lon, alts, tag):
    for alt in alts:
        send(sock, "moveto lat 0 lon %d alt %d duration 0" % (lon, alt), 2.5)
        b = dump(sock, "%s_a%d" % (tag, alt // 1000), 1.5)
        old, new = both_paths(sock, "%s_a%d" % (tag, alt // 1000))
        r = {"rung": alt, "lon": lon, "alt_m": alt,
             "body": rec(b, body), "old": old, "new": new}
        RESULTS["rungs"].append(r)
        print("RUNG alt=%9d m  dist=%10.1f km  screenSize=%s  routing=%s  "
              "old.lit_disc=%s  new.lit_disc=%s"
              % (alt, r["body"]["dist_km"], r["body"]["screenSize"],
                 r["body"]["routing"], old and old["lit_disc"],
                 new and new["lit_disc"]), flush=True)


def preamble(s):
    send(s, "timerate rate 0", 1.0)
    send(s, "deselect", 0.6)
    send(s, "date jday %.1f" % JD, 2.0)
    send(s, "meteors zhr 0", 0.6)


def anchor_on(s, body):
    send(s, "set home_planet %s" % body, 3.0)
    send(s, "select planet %s pointer off" % body, 1.5)
    send(s, "flag track_object on", 4.0)
    send(s, "zoom fov 180 duration 0", 2.0)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
print("leg=%s out=%s" % (LEG, OUT), flush=True)

if LEG == "default":
    # THE LAUNCH TRANSIENT FIRST -- nothing is sent before this grab but the
    # grab itself (N2's `loaded` latch predicts near components drawn until
    # every module reports isLoaded, then gone in one frame).
    for i in range(6):
        RESULTS["rungs"].append({"rung": "t%d" % i, "t": time.time(),
                                 "new": measure(shot(s, "t%d_new" % i, 1.2))})
        print("TRANSIENT t%d lit_disc=%s lit_frame=%s"
              % (i, RESULTS["rungs"][-1]["new"]["lit_disc"],
                 RESULTS["rungs"][-1]["new"]["lit_frame"]), flush=True)
    time.sleep(20)
    RESULTS["rungs"].append({"rung": "settled",
                             "new": measure(shot(s, "settled_new"))})
    print("SETTLED lit_disc=%s" % RESULTS["rungs"][-1]["new"]["lit_disc"], flush=True)
    # Now the pinned-date default view: the A/A floor first (two grabs of the
    # SAME path with nothing between them), then one grab per drawn path.
    preamble(s)
    time.sleep(3)
    aa1 = measure(shot(s, "aa1_new"))
    aa2 = measure(shot(s, "aa2_new"))
    old, new = both_paths(s, "def")
    b = dump(s, "def_dump", 2.0)
    RESULTS["rungs"].append({"rung": "default", "aa1": aa1, "aa2": aa2,
                             "old": old, "new": new,
                             "body": rec(b, "Earth")})
    print("DEFAULT aa1=%s aa2=%s old=%s new=%s dist=%.3f km"
          % (aa1["lit_disc"], aa2["lit_disc"], old["lit_disc"], new["lit_disc"],
             RESULTS["rungs"][-1]["body"]["dist_km"]), flush=True)
elif LEG == "moon5":
    preamble(s)
    anchor_on(s, "Moon")
    lon = calibrate_lon(s, "Moon", 12000000, "m5")
    ladder(s, "Moon", lon, [12000000, 9000000, 8700000, 8670000, 8000000, 5000000], "m5")
elif LEG == "moon1":
    preamble(s)
    anchor_on(s, "Moon")
    send(s, "flag moon_scaled off", 2.0)
    wait_scale_settled(s, "Moon", "m1")
    lon = calibrate_lon(s, "Moon", 3000000, "m1")
    ladder(s, "Moon", lon, [3000000, 2000000, 1800000, 1700000, 1000000], "m1")
elif LEG == "fov":
    preamble(s)
    anchor_on(s, "Moon")
    lon = calibrate_lon(s, "Moon", 12000000, "fv")
    for alt in (9000000, 8000000):
        for fov in (180, 60):
            send(s, "moveto lat 0 lon %d alt %d duration 0" % (lon, alt), 2.0)
            send(s, "zoom fov %d duration 0" % fov, 2.5)
            b = dump(s, "fv_a%d_f%d" % (alt // 1000, fov), 1.5)
            old, new = both_paths(s, "fv_a%d_f%d" % (alt // 1000, fov))
            RESULTS["rungs"].append({"rung": "alt%d_fov%d" % (alt // 1000, fov),
                                     "lon": lon, "alt_m": alt, "fov": fov,
                                     "body": rec(b, "Moon"), "old": old, "new": new})
            print("FOV alt=%d fov=%d dist=%.1f screenSize=%s old=%s new=%s"
                  % (alt, fov, RESULTS["rungs"][-1]["body"]["dist_km"],
                     RESULTS["rungs"][-1]["body"]["screenSize"],
                     old["lit_disc"], new["lit_disc"]), flush=True)
    send(s, "zoom fov 180 duration 0", 1.0)
elif LEG == "earth":
    preamble(s)
    anchor_on(s, "Earth")
    lon = calibrate_lon(s, "Earth", 3000000, "e3")
    ladder(s, "Earth", lon, [3000000], "e3")
else:
    raise SystemExit("unknown leg %r" % LEG)

with open(os.path.join(OUT, "ladder.json"), "w") as f:
    json.dump(RESULTS, f, indent=1)
print("done", flush=True)
