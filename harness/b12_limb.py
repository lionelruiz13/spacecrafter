#!/usr/bin/env python3
"""B12 - does the disc carry the limb-darkening law, and is everything else inert?

    ./b12_limb.py <post_dir> [ref_dir]

    ref_dir = the pre-change captures (the inertness comparison), or a SECOND
    run of the same binary (the cross-launch noise floor, which is what makes
    the inertness numbers readable).

THE PREDICTION IS FIXED BEFORE THE RUN (claude/b12-design.md §5.1, §10.3):

  L(mu) = (3*mu + 2)/5      Eddington grey atmosphere, centre-normalised.

  Measured as the ratio NEW/OLD on the same scene with the additive big halo
  removed: both paths draw the same texture on the same sphere, and the old
  path's fragment is the texel unmodified (body_sun.frag), so the ratio IS the
  law - the colour target is B8G8R8A8_UNORM (app.cpp:323), so no transfer
  function stands between the shader output and the stored pixel.

  Far-limit values (x = r/r_limb):
      x    0.00  0.50  0.80  0.95  1.00
    L(x)  1.000 0.920 0.760 0.587 0.400
  mu(x) is computed EXACTLY for each scene's own d/R, so the reported
  prediction is the finite-distance one; the table above is its d>>R limit.

Two independent readings of the same claim:
  - ring medians (robust to the map's own inhomogeneity),
  - PER PIXEL: |new - old*L(mu(pixel))| over the disc, which tests that the law
    is applied per fragment and leaves nowhere for a fitted global factor to
    hide.

FAILURE MODES this can show: a flat ratio (no law), a ratio on a different
curve (wrong law / double-counted texture), a centre ratio != 1 (the parity
anchor broken), a per-pixel residual above quantisation (the law applied to
something other than the drawn fragment).
"""
import sys, os, json, math
import numpy as np
from PIL import Image

POST = sys.argv[1]
REF = sys.argv[2] if len(sys.argv) > 2 else None

PAIRS = [("s1", "s1_geom", "s1_disc_new", "s1_disc_old"),
         ("s4", "s4_geom", "s4_new", "s4_old"),
         ("s5", "s5_geom", "s5_new", "s5_old"),
         ("s3_a", "s3_geom_a", "s3_a_new", "s3_a_old"),
         ("s3_b", "s3_geom_b", "s3_b_new", "s3_b_old")]
INERT = ["n1_earth_surface", "n2_moon", "n3_mars"]
CHANGED = ["n4_sun_fisheye", "s1_disc_new", "s2_disc_halo_new", "s4_new", "s5_new",
           "s3_composed_new_a", "s3_a_new"]
OLD_SIDE = ["s1_disc_old", "s2_disc_halo_old", "s4_old", "s5_old", "s3_a_old"]


def img(d, n):
    return np.asarray(Image.open(os.path.join(d, n + ".png")).convert("RGB"), dtype=np.float64)


def lum(a):
    """Sum of channels - the §11.44 scale (max 765), so the recorded medians
    (new 21 vs old 725) stay directly comparable."""
    return a.sum(axis=2)


def diff(d, a, b):
    x, y = img(d, a), img(d, b)
    dd = np.abs(x - y).max(axis=2)
    return {f"px>{t}": int((dd > t).sum()) for t in (0, 2, 8, 16, 32)} | {"max": float(dd.max())}


def cross_diff(da, db, n):
    try:
        x, y = img(da, n), img(db, n)
    except FileNotFoundError:
        return {"missing": n}
    if x.shape != y.shape:
        return {"shape_mismatch": [list(x.shape), list(y.shape)]}
    dd = np.abs(x - y).max(axis=2)
    ys, xs = np.nonzero(dd > 0)
    box = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())] if len(xs) else None
    return {f"px>{t}": int((dd > t).sum()) for t in (0, 2, 8, 16, 32)} | {
        "max": float(dd.max()), "bbox_xyxy": box}


def mu_of_x(x, dr):
    """Exact cosine of the emission angle at fractional apparent radius x, for an
    observer at distance d = dr * R from a sphere of radius R.

        mu(psi)    = (dr*cos psi - 1) / sqrt(1 + dr^2 - 2*dr*cos psi)
        theta(psi) = atan2(sin psi, dr - cos psi)      (apparent offset angle)
    The limb is mu = 0 at cos psi = 1/dr, where theta = asin(1/dr). A fisheye
    maps radius linearly in angle, so x = theta / theta_limb.
    """
    if x <= 0:
        return 1.0
    if x >= 1:
        return 0.0
    th_limb = math.asin(1.0 / dr)
    lo, hi = 0.0, math.acos(1.0 / dr)
    for _ in range(60):
        mid = 0.5 * (lo + hi)
        if math.atan2(math.sin(mid), dr - math.cos(mid)) / th_limb < x:
            lo = mid
        else:
            hi = mid
    c = math.cos(0.5 * (lo + hi))
    return max((dr * c - 1.0) / math.sqrt(1.0 + dr * dr - 2.0 * dr * c), 0.0)


def law_lut(dr, n=4097):
    xs = np.linspace(0.0, 1.0, n)
    return xs, np.array([(3.0 * mu_of_x(float(x), dr) + 2.0) / 5.0 for x in xs])


def scene(d, gk, nname, oname, G):
    g = G.get(gk)
    if not g:
        return {"no_geometry": gk}
    new, old = img(d, nname), img(d, oname)
    h, w = new.shape[:2]
    cx = (g["screen"][0] * 0.5 + 0.5) * w
    cy = (1.0 - (g["screen"][1] * 0.5 + 0.5)) * h
    r_limb = g["screenSize"] * (w / 2.0)
    dr = g["dist"] / g["boundingRadius"]
    yy, xx = np.mgrid[0:h, 0:w]
    rr = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / r_limb
    xs, ls = law_lut(dr)
    out = {"centre_px": [round(cx, 1), round(cy, 1)], "r_limb_px": round(r_limb, 2),
           "d_over_R": round(dr, 3), "screenSize": g["screenSize"],
           "routing": g.get("routing"), "modules": g.get("modules")}
    # ---- ring medians ----------------------------------------------------
    nl, ol = lum(new), lum(old)
    nb = 20 if r_limb > 40 else 5
    prof = []
    for i in range(nb):
        lo, hi = i / nb, (i + 1) / nb
        m = (rr >= lo) & (rr < hi)
        if not m.any():
            continue
        x = 0.5 * (lo + hi)
        vn, vo = float(np.median(nl[m])), float(np.median(ol[m]))
        pred = float(np.interp(x, xs, ls))
        prof.append({"x": round(x, 3), "px": int(m.sum()), "new": round(vn, 1),
                     "old": round(vo, 1),
                     "ratio": None if not vo else round(vn / vo, 4),
                     "pred": round(pred, 4),
                     "err": None if not vo else round(vn / vo - pred, 4)})
    errs = [abs(p["err"]) for p in prof if p["err"] is not None]
    out["ring_profile"] = prof
    out["centre_ratio"] = prof[0]["ratio"] if prof else None
    out["new_median_r0"] = prof[0]["new"] if prof else None
    out["old_median_r0"] = prof[0]["old"] if prof else None
    out["ring_max_abs_err"] = round(max(errs), 4) if errs else None
    out["ring_rms_err"] = round(float(np.sqrt(np.mean(np.square(errs)))), 4) if errs else None
    # ---- per-pixel attribution -------------------------------------------
    # Interior only (r < 0.97): the silhouette row mixes the disc with the
    # background through multisample coverage, which no fragment law describes.
    m = (rr < 0.97) & (old.max(axis=2) > 8)
    if m.sum() > 32:
        L = np.interp(rr[m], xs, ls)[:, None]
        resid = new[m] - old[m] * L
        out["per_pixel"] = {
            "n": int(m.sum()),
            "mean_abs": round(float(np.abs(resid).mean()), 4),
            "p99_abs": round(float(np.percentile(np.abs(resid), 99)), 4),
            "max_abs": round(float(np.abs(resid).max()), 4),
            "bias": round(float(resid.mean()), 4),
            "scale_note": "per-channel 0..255",
        }
    return out


R = {}
G = json.load(open(os.path.join(POST, "geometry.json")))
R["md5_ok"] = G.get("md5_ok")
R["noise_floor_same_launch"] = diff(POST, "s0_floor_a", "s0_floor_b")
for tag, gk, nn, on in PAIRS:
    try:
        R[tag] = scene(POST, gk, nn, on, G)
    except FileNotFoundError as e:
        R[tag] = {"missing": str(e)}

if REF:
    R["ref_dir"] = REF
    R["inert"] = {n: cross_diff(REF, POST, n) for n in INERT}
    R["expected_to_change"] = {n: cross_diff(REF, POST, n) for n in CHANGED}
    R["old_path_side"] = {n: cross_diff(REF, POST, n) for n in OLD_SIDE}
    try:
        Gp = json.load(open(os.path.join(REF, "geometry.json")))
        R["ref_s1"] = scene(REF, "s1_geom", "s1_disc_new", "s1_disc_old", Gp)
        R["ref_s1"].pop("ring_profile", None)
    except Exception as e:
        R["ref_s1"] = {"error": str(e)}

print(json.dumps(R, indent=2))
with open(os.path.join(POST, "b12_limb.json"), "w") as f:
    json.dump(R, f, indent=2)
