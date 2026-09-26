#!/usr/bin/env python3
# F131 offline analyser: applies the criteria of INTENT 11.292 P5 to the records of f131_obs.py.
#   analyse.py state <dir> <tag>          one dump: both observers, fov, tone, stars
#   analyse.py ring <dir> <tag> [a] [b]   the per-frame seam ring of a dump, frames a..b
#   analyse.py obs1 <dir>                 criterion (1) on a launch record
import json, math, re, sys
import numpy as np
from PIL import Image

AU = 149597870.7


def clean(ln):
    """Replace bare nan / inf tokens only (b21_keypath._clean also rewrites keys such as worldAdaptationLuminance)."""
    ln = re.sub(r'(?<![A-Za-z_"])-?nan(?![A-Za-z_"])', 'null', ln.strip())
    ln = re.sub(r'(?<![A-Za-z_"])-inf(?![A-Za-z_"])', '-1e308', ln)
    return re.sub(r'(?<![A-Za-z_"])inf(?![A-Za-z_"])', '1e308', ln)


def load(d, tag):
    """Return (header, bodies) of dump <d>/<tag>.json."""
    hdr, bodies = None, {}
    for ln in open("%s/%s.json" % (d, tag)):
        try:
            o = json.loads(clean(ln))
        except Exception:
            continue
        if o.get("type") == "header":
            hdr = o
        elif o.get("type") == "body":
            bodies[o["name"]] = o
    return hdr, bodies


def state(d, tag):
    h, b = load(d, tag)
    c, ctl, ov = h["camera"], h["control"], h["oldView"]
    s = {"tag": tag, "jd": h["jd"], "drawn": ctl["drawnPath"], "cam_ref": c["reference"], "cam_free": c["freeMode"],
         "cam_bound": c["boundToSurface"], "cam_tracked": c["tracked"], "cam_heading_deg": math.degrees(c["heading"]),
         "lat": ctl["latitude"], "lon": ctl["longitude"], "alt_m": ctl["altitude"], "heading": ctl["heading"],
         "old_home": ov["observer"]["homePlanet"], "old_onBody": ov["observer"]["onBody"],
         "fov_old": ov["projector"]["fov"], "fov_new": 2 * math.degrees(c["halfFov"]),
         "skyBrightness": ov["tone"]["skyBrightness"], "atmShown": ov["tone"]["atmosphereShown"],
         "atmFade": ov["tone"]["atmosphereFadeIntensity"], "worldAdapt": ov["tone"]["worldAdaptationLuminance"],
         "stars_nbToDraw": ov["stars"]["nbStarsToDraw"], "stars_fader": ov["stars"]["fader"],
         "anchors": h["anchors"], "cam_rootPos": c["rootPos"], "cam_pos_len_km": math.sqrt(sum(x * x for x in c["position"])) * AU,
         "cam_dist_member_km": c["distance"] * AU, "cam_selected": c["selected"], "absFwd": c["absFwd"]}
    return s, h, b


def ring(h):
    return h["seam"]["steps"]


def star_points(png):
    """Count star-like points: pixels 40 grey above the 11x11 median, in the upper sky half of the disc."""
    from scipy.ndimage import median_filter, maximum_filter
    img = np.asarray(Image.open(png).convert("L"), dtype=np.float32)
    med = median_filter(img, size=11)
    peak = (img - med > 40) & (img == maximum_filter(img, size=5))
    n = img.shape[0]
    yy, xx = np.mgrid[0:n, 0:n]
    r = np.hypot(xx - (n - 1) / 2, yy - (n - 1) / 2)
    return int((peak & (r < 0.40 * n)).sum())


def sunset_jd(jd, lon, lat):
    """Port of TimeMgr::dateSunSet (src/coreModule/time_mgr.cpp), line for line."""
    d2r, r2d = math.pi / 180.0, 180.0 / math.pi
    f32 = lambda x: float(np.float32(x))
    n = round(jd - 2451545.0009 - lon / 360.0)
    temp = f32(lon / 360.0)
    J = temp + n + f32(2451545.0009)
    M = float(np.fmod(np.float32(357.5291 + 0.98560028 * (J - 2451545.0)), np.float32(360.0)))
    C = (1.9148 * math.sin(d2r * M)) + (0.0200 * math.sin(d2r * 2.0 * M)) + (0.0003 * math.sin(d2r * 3.0 * M))
    el = float(np.fmod(np.float32(M + 102.9372 + C + 180.0), np.float32(360.0)))
    s = r2d * math.asin(math.sin(d2r * el) * math.sin(d2r * 23.45))
    Ho = (math.sin(-0.83 * d2r) - (math.sin(d2r * lat) * math.sin(d2r * s))) / (math.cos(d2r * lat) * math.cos(d2r * s))
    if abs(Ho) > 1:
        return J + 0.5
    w = r2d * math.acos(Ho)
    jset = 2451545.0009 + (w + lon) / 360.0 + n + (0.0053 * math.sin(d2r * M)) - (0.0069 * math.sin(d2r * 2.0 * el))
    nova = jset - 2.0 * ((1.0 / 24.0) * (lon / 15.0))
    if nova - jd >= 0.99:
        nova -= 1
    if jd - nova >= 0.99:
        nova += 1
    return nova


def main():
    cmd = sys.argv[1]
    if cmd == "state":
        s, _, _ = state(sys.argv[2], sys.argv[3])
        print(json.dumps(s, indent=1))
    elif cmd == "ring":
        _, h, _ = state(sys.argv[2], sys.argv[3])
        st = ring(h)
        a = int(sys.argv[4]) if len(sys.argv) > 4 else 0
        b = int(sys.argv[5]) if len(sys.argv) > 5 else st[-1]["frame"]
        for x in st:
            if a <= x["frame"] <= b:
                print(json.dumps(x))


if __name__ == "__main__":
    main()


def race(d, tag="final"):
    """Test P5 (2)'s race on a fly launch: accepted iff jd(frame before line 38) >= the new travel's arrival."""
    h, b = load(d, tag)
    st, tr = h["seam"]["steps"], h["seam"]["travels"]
    new = [t for t in tr if t["engine"] == 1 and t["endTime"] - t["startTime"] > 1e-6]
    if not new:
        return {"install": None, "note": "no timed travel on the new registry"}
    t = new[-1]
    byf = {s["frame"]: s for s in st}
    f0 = t["frame"]
    acc, k = 0, f0
    while acc < 3000:
        k += 1
        acc += byf[k]["dt"]
    dt0 = byf[f0]["dt"]
    s_km1 = acc - byf[k]["dt"]
    jd_before = byf[k - 1]["jd"]
    log = open(d + "/log_spacecrafter.txt", errors="replace").read()
    refused = "a scripted travel is in flight" in log
    return {"install_frame": f0, "line38_frame": k, "dt0_ms": dt0, "S_ms": s_km1, "dt0_plus_S": dt0 + s_km1,
            "predicted_accept_ms_form": dt0 + s_km1 >= 3000,
            "jd_before_line38": jd_before, "travel_endTime": t["endTime"], "jd_margin_s": (jd_before - t["endTime"]) * 86400,
            "predicted_accept_jd_form": jd_before >= t["endTime"], "log_says_refused": refused,
            "camera_reference_final": h["camera"]["reference"], "old_home_final": h["oldView"]["observer"]["homePlanet"]}


def wheel(d, tag="final", out=None):
    """Per-frame fov table of each wheel window and each engine's time to cover half of its own path."""
    h, b = load(d, tag)
    st = h["seam"]["steps"]
    rows, wins, cur = [], [], None
    for i, s in enumerate(st):
        new = 2 * math.degrees(s["halfFovNew"])
        active = s["zoomTNew"] > 0 or abs(s["aimFovOld"] - s["fovOld"]) > 1e-9
        rows.append((s["frame"], s["dt"], s["fovOld"], new, s["fovOld"] - new, s["aimFovOld"], active))
        if active and cur is None:
            cur = [i, i]
        elif active:
            cur[1] = i
        elif cur is not None and i - cur[1] > 3:
            wins.append(tuple(cur)); cur = None
    if cur:
        wins.append(tuple(cur))
    res = []
    for a, bnd in wins:
        start_old, start_new = rows[a - 1][2], rows[a - 1][3]
        end_old, end_new = rows[min(bnd + 3, len(rows) - 1)][2], rows[min(bnd + 3, len(rows) - 1)][3]
        t, t50o, t50n, mx = 0.0, None, None, 0.0
        for r in rows[a:bnd + 4]:
            t += r[1] / 1000.0
            if t50o is None and abs(r[2] - start_old) >= 0.5 * abs(end_old - start_old):
                t50o = t
            if t50n is None and abs(r[3] - start_new) >= 0.5 * abs(end_new - start_new):
                t50n = t
            mx = max(mx, abs(r[4]))
        rest = max(abs(r[4]) for r in rows[bnd + 1:bnd + 31]) if bnd + 1 < len(rows) else None
        res.append({"frames": (rows[a][0], rows[bnd][0]), "start": (round(start_old, 4), round(start_new, 4)),
                    "end": (round(end_old, 4), round(end_new, 4)), "max_abs_delta_deg": round(mx, 4),
                    "t50_stars_s": t50o, "t50_bodies_s": t50n,
                    "lag_stars_minus_bodies_ms": None if t50o is None or t50n is None else round((t50o - t50n) * 1000, 1),
                    "rest_30f_max_delta": rest})
    if out:
        with open(out, "w") as f:
            f.write("frame dt_ms fov_old_deg fov_new_deg delta_deg aim_old_deg active\n")
            for a, bnd in wins:
                for r in rows[a - 2:bnd + 6]:
                    f.write("%d %d %.6f %.6f %.6f %.6f %d\n" % (r[0], r[1], r[2], r[3], r[4], r[5], r[6]))
                f.write("\n")
    return res
