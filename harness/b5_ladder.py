#!/usr/bin/env python3
"""B5-oort-2 EARTH-ANCHORED POWER-OF-TWO distance ladder (INTENT §6.9 / §11.96
[vixy 2026-07-24]).

Vixy's protocol, verbatim: "taking screenshot at each power of two of the
distance in Mm, starting from 1Mm of distance (anchored on earth) in both path,
and compare pixels between captures of same-distance."

WHY THIS IS A VALID INSTRUMENT NOW: B5-oort-2 gave the oort cloud a FROZEN-SEED
PRNG (coreModule/oort.cpp oortRng), so the old-path and new-path clouds are
POINT-identical (same seed, same draw order). Cross-path oort divergence at any
rung is therefore REAL PATH signal, not a statistical cloud difference (the pilot
could only measure statistical similarity, §11.96(a)).

ANCHORED (NOT free mode): reference stays Earth; `moveto altitude 2^k Mm` raises
the observer above Earth. This exercises the §11.83(c) anchored draw path
(drawSystem-direct, never collapses to a dot) and side-steps the §11.96(c)
free-mode AoI reference hijack. The Sun is aimed (select+track) so the
heliocentric oort shell is inside the 340-deg fisheye; tracking is turned OFF
before every shot (§11.94(e): new-path bit-stability requires track-off).

THE TWO GATES the ladder probes (different quantities - the crux of the
"showing too early" suspicion):
  OLD path  : observer->getAltitude() in [1e13, 1e16] m  (altitude ABOVE EARTH)
              [solarSystemModule.cpp:203; oort.cpp:141]
  NEW path  : the oort BODY's regime - near component hidden only while
              screenSize>=0.2 AND distance<scaledRadius (=oort body radius peg),
              else drawn [ModularBody.cpp:369-409] - distance is HELIOCENTRIC
              (observer -> oort body at the Sun).

MODES
  capture <new|old|preold> <outdir> <start> <stop> <step>
     pins the path (experimental_path on=new, off=old/preold), walks the
     altitude exponents arange(start,stop+, step) (2^x Mm), and per rung saves
     {path}_{x}_on.png / _off.png (oort flag on/off = the oort ALONE) plus a
     camera+oort dump. 'preold' behaves like 'old' but is captured with the
     PRE-seed-change binary (SC_BIN override in the runner) for the Part-1
     old-cloud delta.
  compare <outdir>
     offline: per-rung table (rung, dist, old px, new px, cross-path channels),
     first-pixel rung per path, and the old-cloud before/after delta.
"""
import socket, time, json, sys, os, glob
import numpy as np
from PIL import Image

AU_M = 149597870700.0
MM_M = 1.0e6           # metres per Mm

def img(p):
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)

def px32(a, b):
    """count/bbox/max of pixels whose per-channel |delta| exceeds 32 (b26 class)."""
    d = np.abs(a - b)
    mask = (d > 32).any(axis=2)
    n = int(mask.sum())
    bbox = None
    if n:
        ys, xs = np.nonzero(mask)
        bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
    return n, bbox, int(d.max())

def chan_counts(a, b):
    """per-channel px>32 counts (R,G,B) - the 'count channels' the spec asks."""
    d = np.abs(a - b)
    return [int((d[:, :, c] > 32).sum()) for c in range(3)]

# ---------------------------------------------------------------- capture ----
def do_capture():
    path = sys.argv[2]                 # new | old | preold | prenew
    OUT = sys.argv[3]
    os.makedirs(OUT, exist_ok=True)
    # exponents: either "@file" (one per line) or start stop step (arange).
    if sys.argv[4].startswith("@"):
        with open(sys.argv[4][1:]) as f:
            exps = [round(float(t), 4) for t in f.read().split() if t.strip()]
    else:
        start, stop, step = float(sys.argv[4]), float(sys.argv[5]), float(sys.argv[6])
        exps = []
        x = start
        while x <= stop + 1e-9:
            exps.append(round(x, 4))
            x += step
    exps = sorted(set(exps))

    def send(s, cmd, pause=0.5):
        s.sendall((cmd + "\n").encode()); time.sleep(pause)
        try:
            s.settimeout(0.3); s.recv(8192)
        except socket.timeout:
            pass
        s.settimeout(None)
        print(f">> {cmd}", flush=True)

    def shot(s, name, pause=2.5):
        send(s, f"body action screenshot filename {OUT}/{name}.png", pause)

    def dump(s, name):
        send(s, f"body action dual_dump filename {OUT}/{name}.json", 1.5)
        cam = None; oort = None
        with open(f"{OUT}/{name}.json") as f:
            for line in f:
                try:
                    d = json.loads(line)
                except Exception:
                    continue
                if d.get("type") == "header" and "camera" in d:
                    cam = d["camera"]
                elif d.get("type") == "body" and d.get("name") == "Oort":
                    oort = d.get("new")
        return cam, oort

    s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
    pin_new = path in ("new", "prenew")
    send(s, f"flag experimental_path {'on' if pin_new else 'off'}", 2)
    send(s, "date jday 2461234.0", 1)
    send(s, "timerate rate 0", 1)
    send(s, "meteors zhr 0", 1)
    send(s, "flag atmosphere off", 1)
    send(s, "flag landscape off", 1)
    send(s, "flag star off", 1)
    send(s, "flag milky_way off", 1)
    send(s, "flag constellation_art off", 1)
    send(s, "select planet Sun", 1)               # aim target (anchored, not free)

    rows = []
    for x in exps:
        alt_m = (2.0 ** x) * MM_M
        tag = f"{x:.2f}"
        send(s, "flag track_object on", 1)
        send(s, f"moveto altitude {alt_m:.0f} duration 0", 3)
        send(s, "flag track_object on", 3)         # re-aim the Sun after the move
        send(s, "flag track_object off", 2)        # tracking OFF before shots
        cam, oort = dump(s, f"{path}_{tag}_dump")
        send(s, "flag oort on", 3)                 # 3s: settle the OLD fader (2000ms)
        shot(s, f"{path}_{tag}_on")
        send(s, "flag oort off", 3)
        shot(s, f"{path}_{tag}_off")
        row = {"exp": x, "tag": tag, "alt_m": alt_m, "alt_AU": alt_m / AU_M,
               "reference": cam.get("reference") if cam else None,
               "refDist": cam.get("refDist") if cam else None,
               "cam_dist": cam.get("distance") if cam else None,
               "alt": cam.get("alt") if cam else None,
               "az": cam.get("az") if cam else None,
               "absFwd": cam.get("absFwd") if cam else None,
               "oort_dist": oort.get("dist") if oort else None,
               "oort_screen": oort.get("screenSize") if oort else None,
               "oort_scaledR": oort.get("scaledDatumRadius") if oort else None,
               "oort_visible": oort.get("visible") if oort else None,
               "oort_relation": oort.get("relation") if oort else None}
        rows.append(row)
        print(f"[{path} x={tag}] alt={alt_m:.3e}m ({alt_m/AU_M:.2f}AU) "
              f"ref={row['reference']} refDist={row['refDist']} "
              f"oort_dist={row['oort_dist']} oort_screen={row['oort_screen']} "
              f"scaledR={row['oort_scaledR']} vis={row['oort_visible']}", flush=True)
    # MERGE with any prior capture of this path (coarse + fine accumulate).
    rpath = f"{OUT}/{path}_rows.json"
    merged = {}
    if os.path.exists(rpath):
        for r in json.load(open(rpath)):
            merged[r["tag"]] = r
    for r in rows:
        merged[r["tag"]] = r
    json.dump([merged[t] for t in sorted(merged, key=lambda t: float(t))],
              open(rpath, "w"), indent=1)
    print(f"=== capture {path}: {len(rows)} rungs ({len(merged)} total) -> {OUT} ===", flush=True)

# ---------------------------------------------------------------- compare ----
def do_compare():
    OUT = sys.argv[2]
    def load_rows(path):
        p = f"{OUT}/{path}_rows.json"
        return {r["tag"]: r for r in json.load(open(p))} if os.path.exists(p) else {}
    new = load_rows("new"); old = load_rows("old"); pre = load_rows("preold")
    tags = sorted(set(new) | set(old), key=lambda t: float(t))

    # The oort is BLUE (config oort_color 0,0.5,1.0, set on BOTH paths -
    # core.cpp:1461 old / OortLoader new). A flag-toggle transient (§11.96(f))
    # shifts REDDISH central body/halo content by a sub-pixel and pollutes raw
    # px32(on,off) (measured 23-112 px, meanRGB~[107,32,32]). Isolate the oort by
    # its BLUE contribution: the oort adds blue, the transient adds red -> the
    # blue-delta is transient-immune (measured 0 blue where raw was 112).
    def blue_add(on, off):
        dB = on[:, :, 2] - off[:, :, 2]                  # blue added when oort ON
        return (dB > 32) & (on[:, :, 2] > on[:, :, 0])    # blue-dominant pixels

    def oort_footprint(path, tag):
        on = f"{OUT}/{path}_{tag}_on.png"; off = f"{OUT}/{path}_{tag}_off.png"
        if not (os.path.exists(on) and os.path.exists(off)):
            return None
        a, b = img(on), img(off)
        m = blue_add(a, b); n = int(m.sum())
        raw = px32(a, b)[0]                                # aux: full toggle diff
        bbox = None; mx = 0
        if n:
            ys, xs = np.nonzero(m); bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
            mx = int((a[:, :, 2] - b[:, :, 2])[m].max())
        return n, bbox, mx, raw                           # blue px, bbox, blueMax, raw px32

    def crosspath_oort(tag):
        """oort-ISOLATED cross-path diff on BLUE: |(new_on-new_off)-(old_on-old_off)|
        on the blue channel. Removes the path-differing body scene AND the red
        transient; leaves the oort's cross-path divergence. With the frozen seed
        the clouds are point-identical, so this is pure PATH signal."""
        try:
            n_on = img(f"{OUT}/new_{tag}_on.png"); n_off = img(f"{OUT}/new_{tag}_off.png")
            o_on = img(f"{OUT}/old_{tag}_on.png"); o_off = img(f"{OUT}/old_{tag}_off.png")
        except FileNotFoundError:
            return None
        ndB = n_on[:, :, 2] - n_off[:, :, 2]
        odB = o_on[:, :, 2] - o_off[:, :, 2]
        diff = np.abs(ndB - odB)
        mask = diff > 32; n = int(mask.sum())
        bbox = None
        if n:
            ys, xs = np.nonzero(mask); bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
        fulldiff = np.abs((n_on - n_off) - (o_on - o_off))     # full-RGB per-channel
        chans = [int((fulldiff[:, :, c] > 32).sum()) for c in range(3)]
        return n, bbox, int(diff.max()), chans

    def crosspath_full(tag):
        try:
            return px32(img(f"{OUT}/new_{tag}_on.png"), img(f"{OUT}/old_{tag}_on.png"))
        except FileNotFoundError:
            return None

    def oldcloud_delta(tag):
        """old cloud POINTS before(preold) vs after(old) the reseed - same path,
        same matrix, only the seed changed. Blue-isolated (on-off) both sides."""
        try:
            a_on = img(f"{OUT}/preold_{tag}_on.png"); a_off = img(f"{OUT}/preold_{tag}_off.png")
            b_on = img(f"{OUT}/old_{tag}_on.png"); b_off = img(f"{OUT}/old_{tag}_off.png")
        except FileNotFoundError:
            return None
        adB = a_on[:, :, 2] - a_off[:, :, 2]; bdB = b_on[:, :, 2] - b_off[:, :, 2]
        diff = np.abs(adB - bdB)
        n = int((diff > 32).sum())
        fa = int(blue_add(a_on, a_off).sum())    # pre-reseed old-cloud footprint
        fb = int(blue_add(b_on, b_off).sum())    # post-reseed old-cloud footprint
        return n, int(diff.max()), fa, fb

    SHOW = 100   # blue-oort px floor for "shown" (the red transient adds 0 blue)
    table = []
    print("\n rung(2^x Mm)      alt_AU   ref        | OLD px(bbox,max)      NEW px(bbox,max)      | xpath-oort px/max ch[R,G,B]  xpath-full px", flush=True)
    print("-" * 155, flush=True)
    first_old = first_new = None
    for tag in tags:
        of = oort_footprint("old", tag); nf = oort_footprint("new", tag)
        xo = crosspath_oort(tag); xf = crosspath_full(tag)
        r = (new.get(tag) or old.get(tag) or {})
        au = r.get("alt_AU"); ref = r.get("reference")
        oldpx = of[0] if of else None; newpx = nf[0] if nf else None
        if first_old is None and oldpx is not None and oldpx > SHOW:
            first_old = tag
        if first_new is None and newpx is not None and newpx > SHOW:
            first_new = tag
        rec = {"tag": tag, "exp": float(tag), "alt_AU": au, "reference": ref,
               "old_oort_px": oldpx, "old_oort_bbox": of[1] if of else None,
               "old_oort_max": of[2] if of else None, "old_raw_px": of[3] if of else None,
               "new_oort_px": newpx, "new_oort_bbox": nf[1] if nf else None,
               "new_oort_max": nf[2] if nf else None, "new_raw_px": nf[3] if nf else None,
               "xpath_oort_px": xo[0] if xo else None, "xpath_oort_max": xo[2] if xo else None,
               "xpath_oort_chan": xo[3] if xo else None, "xpath_oort_bbox": xo[1] if xo else None,
               "xpath_full_px": xf[0] if xf else None,
               "oort_dist_AU": (r.get("oort_dist") / AU_M) if r.get("oort_dist") else None,
               "oort_screen": r.get("oort_screen"), "oort_scaledR_AU": (r.get("oort_scaledR") / AU_M) if r.get("oort_scaledR") else None}
        oc = oldcloud_delta(tag)
        if oc:
            rec["oldcloud_delta_px"] = oc[0]; rec["oldcloud_delta_max"] = oc[1]
            rec["oldcloud_pre_footprint"] = oc[2]; rec["oldcloud_post_footprint"] = oc[3]
        table.append(rec)
        aus = f"{au:9.2f}" if au is not None else "     n/a"
        print(f"  2^{tag:>6}  {aus}  {str(ref):<9} | "
              f"{str(oldpx):>7} {str(of[1] if of else '-'):<19} m{of[2] if of else 0:<3} "
              f"{str(newpx):>7} {str(nf[1] if nf else '-'):<19} m{nf[2] if nf else 0:<3} | "
              f"{str(xo[0] if xo else '-'):>7}/{xo[2] if xo else 0:<3} {str(xo[3] if xo else '-'):<15} "
              f"{str(xf[0] if xf else '-'):>7}", flush=True)

    print("\n--- FIRST-PIXEL (oort footprint > %d px) ---" % SHOW, flush=True)
    def au_of(tag):
        r = new.get(tag) or old.get(tag) or {}
        return r.get("alt_AU")
    print(f"OLD first-show rung: 2^{first_old} Mm ({au_of(first_old) if first_old else None} AU)  "
          f"[gate = altitude above Earth in 1e13..1e16 m = 66.8..66845 AU]", flush=True)
    print(f"NEW first-show rung: 2^{first_new} Mm ({au_of(first_new) if first_new else None} AU)  "
          f"[gate = oort-body regime, ModularBody.cpp:385]", flush=True)

    oc_tags = [t for t in tags if any(x in pre for x in [t])]
    if pre:
        print("\n--- OLD-CLOUD before/after reseed (Part 1 delta) ---", flush=True)
        for rec in table:
            if rec.get("oldcloud_delta_px") is not None:
                print(f"  2^{rec['tag']}: pre_footprint={rec['oldcloud_pre_footprint']} "
                      f"post_footprint={rec['oldcloud_post_footprint']} "
                      f"point-delta px>32={rec['oldcloud_delta_px']} max={rec['oldcloud_delta_max']}", flush=True)

    json.dump({"table": table, "first_old": first_old, "first_new": first_new,
               "show_floor": SHOW}, open(f"{OUT}/ladder_result.json", "w"), indent=1)
    print(f"\n=== compare -> {OUT}/ladder_result.json ===", flush=True)

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1] not in ("capture", "compare"):
        print(__doc__); sys.exit(2)
    if sys.argv[1] == "capture":
        do_capture()
    else:
        do_compare()
