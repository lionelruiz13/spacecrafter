#!/usr/bin/env python3
"""F14 — the PREDICTION artifact, written and committed BEFORE the fix lands.

    ./f14_predict.py <outdir>            # write predictions from the pre capture
    ./f14_predict.py <outdir> --check    # score the post capture against them

WHY IT IS A SEPARATE, EARLIER COMMIT. A residual is only accepted when its
magnitude and mechanism were PREDICTED from the attributed cause; a prediction
written after the measurement is a fit. This file is committed at the pre-fix
checkpoint, so every number below is on the record before the conversion is
touched, and `--check` can only confirm or refute it.

WHAT IS PREDICTED, and from what.
  * `offset_post = offset_pre + 90 deg` for EXACTLY the bodies that declare
    `rot_pole_w0`, and nothing else moves. Derived, not fitted: the corrected
    conversion solves the same equation on the texture's centre column instead
    of the mesh x_hat, and those two mesh directions are 90 deg apart
    [SphereObjL.cpp:153: u = theta/360 - 0.25 => u 0.5 <-> theta 270, u 0.75
    <-> theta 0]. atan2(pm.ey, pm.ex) = atan2(-(pm.ex), pm.ey) + 90 identically,
    so the delta is exactly +90 for every body, independent of its pole.
  * `u_pm 0.75 -> 0.50`; `u_sub` drops by exactly 0.25.
  * The rendered image: the body turns 90 deg about its own pole and NOTHING
    else changes, so the post-fix frame is the pre-fix geometry with the map
    rotated by -90 deg [derived: RE_post = RE_pre . Rz(pi/2)]. That image is
    synthesised here and written out; `--check` correlates the LIVE post-fix
    screenshot against it and against the null hypothesis (no rotation).
  * The 4 registration-bearing planets are the independent corpus anchor: the
    corrected conversion, applied to their FETCHED W0, must reproduce the
    `rot_rotation_offset` their file already carries - a value nobody derived
    from this code. Predicted residuals are the §11.101(b2) u-offsets.
"""

import json, math, sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f14_meridian as F

# The 4 planets whose texture IS longitudinally registered (§11.101(b2)); the
# other three (Venus, Jupiter, Uranus) have nothing to register - cloud maps and
# a featureless disc - and are excluded for that stated reason, not by fit.
# (ra0, de0, W0) as used by b14_w0_planetscan.py, whose provenance is the
# §11.86 cited pck00011 fetch. No value is introduced here.
PLANET_ANCHORS = {
    "Saturn":  (40.5908,  83.537,   38.90),
    "Mercury": (281.001,  61.45,   329.5988),
    "Mars":    (317.6725, 52.88212, 176.049863),
    "Neptune": (299.33,   42.95,   249.978),
}


def corrected_offset(ra0, de0, W0):
    """The conversion as it must read AFTER the fix, in closed form."""
    p = F.M_J2VSOP @ F.s2r(ra0*F.d2r, de0*F.d2r)
    ra = math.atan2(p[1], p[0]); de = math.asin(p[2]/np.linalg.norm(p))
    obl, node = math.pi/2 - de, ra + math.pi/2
    pm, _ = F.iau_prime_meridian(ra0, de0, W0, 0.0, F.J2000)
    Axz = F.Az(node) @ F.Ax(obl)
    ex, ey = Axz @ np.array([1., 0, 0]), Axz @ np.array([0, 1., 0])
    return math.degrees(math.atan2(pm @ ey, pm @ ex)) % 360.0


def load_pre(out, moon):
    jd, body, hops = F.read_dump(out / f"f14_pre_{moon}.json", moon)
    return jd, body, hops


def frame(out, tag, moon, ini):
    """(observation, model pieces) for one capture - the f14_meridian chain."""
    jd, body, hops = F.read_dump(out / f"f14_{tag}_{moon}.json", moon)
    h = hops[0]
    RE = F.R3(body["mat"]) @ F.Az(body["axisRot"] + math.pi/2)
    RM2E = F.R3(h["tilt"]) @ F.R3(h["spin"])
    helio = np.zeros(3)
    for hop in hops:
        helio = helio + np.array(hop["ecl"])
    s_mesh = RM2E.T @ (-helio/np.linalg.norm(helio))
    full = Image.open(out / f"f14_{tag}_{moon}.png").convert("L")
    W, H = full.size
    cxf = W/2 + body["screen"][0]*(H/2)
    cyf = H/2 - body["screen"][1]*(H/2)
    Rf = body["screenSize"]*(H/2)
    N, hb = 256, 2.0*Rf
    x0, y0 = cxf - hb, cyf - hb
    img = np.asarray(full.resize((N, N), Image.BILINEAR,
                                 box=(x0, y0, x0+2*hb, y0+2*hb))).astype(float)
    sc = N/(2*hb)
    tex = np.asarray(Image.open(F.TEXDIR / ini[moon.lower()]["tex_map"]).convert("L")).astype(float)
    return dict(jd=jd, body=body, hops=hops, RE=RE, s_mesh=s_mesh, img=img,
                tex=tex, cx=(cxf-x0)*sc, cy=(cyf-y0)*sc, R=Rf*sc)


def score_against(f, ref_albedo, ref_lam, ref_mask):
    obs = f["img"]/np.where(ref_lam > 0.05, ref_lam, 1.0)
    use = ref_mask & (ref_lam > 0.35) & (f["img"] > 8)
    return F.ncc(F.highpass(obs, use), F.highpass(ref_albedo, use), use)


def main(argv):
    out = Path(argv[1]).resolve()
    check = "--check" in argv
    ini = F.ini_bodies()
    pred_path = out / "f14_prediction.json"

    if not check:
        pred = {"note": "written BEFORE the fix, from the pre-fix capture",
                "offsets": {}, "bodies": {}, "planet_anchor": {}}
        _, _, hops_all = load_pre(out, "Iapetus")
        # every hopped body, keyed or not - the "nothing else moves" half
        for line in open(out / "f14_pre_Iapetus.json"):
            line = line.strip().rstrip(",")
            if not line:
                continue
            try:
                o = json.loads(line)
            except json.JSONDecodeError:
                continue
            if o.get("type") != "hops":
                continue
            nm = o["name"]
            off = o["new"][0]["offset"]
            keyed = "rot_pole_w0" in ini.get(nm.lower(), {})
            pred["offsets"][nm] = {"pre": off, "keyed": keyed,
                                   "post": (off + 90.0) % 360.0 if keyed else off}
        for _, moon, _, _ in F.SCENES:
            f = frame(out, "pre", moon, ini)
            a, lam, m = F.synth(f["tex"], f["RE"], f["s_mesh"], f["cx"], f["cy"],
                                f["R"], f["img"].shape, -math.pi/2, True, -1.0)
            Image.fromarray(np.clip(a*lam, 0, 255).astype(np.uint8)).save(
                out / f"f14_pred_post_{moon}.png")
            a0, lam0, m0 = F.synth(f["tex"], f["RE"], f["s_mesh"], f["cx"], f["cy"],
                                   f["R"], f["img"].shape, 0.0, True, -1.0)
            Image.fromarray(np.clip(a0*lam0, 0, 255).astype(np.uint8)).save(
                out / f"f14_pred_null_{moon}.png")
            d_mesh = f["RE"].T @ (-np.array(f["body"]["mat"][12:15])
                                  / np.linalg.norm(f["body"]["mat"][12:15]))
            u_sub = F.mesh_u(d_mesh)
            # how much of the visible disc the DARK terrain covers, before/after
            def darkfrac(delta):
                aa, ll, mm = F.synth(f["tex"], f["RE"], f["s_mesh"], f["cx"], f["cy"],
                                     f["R"], f["img"].shape, delta, True, -1.0)
                use = mm & (ll > 0.35)
                return float((aa[use] < 0.5*f["tex"].max()).mean())
            pred["bodies"][moon] = {
                "u_pm_pre": 0.75, "u_pm_post": 0.50,
                "u_sub_pre": u_sub, "u_sub_post": (u_sub - 0.25) % 1.0,
                "dark_centroid_u": F.dark_centroid_u(F.TEXDIR / ini[moon.lower()]["tex_map"]),
                "dark_fraction_of_lit_disc_pre": darkfrac(0.0),
                "dark_fraction_of_lit_disc_post": darkfrac(-math.pi/2),
                "predicted_image": f"f14_pred_post_{moon}.png",
                "null_image": f"f14_pred_null_{moon}.png",
            }
        for nm, (ra0, de0, W0) in PLANET_ANCHORS.items():
            fileoff = float(ini[nm.lower()]["rot_rotation_offset"])
            conv = corrected_offset(ra0, de0, W0)
            pred["planet_anchor"][nm] = {
                "file_offset": fileoff, "corrected_conversion": conv,
                "residual_deg": ((conv - fileoff + 180) % 360) - 180,
                "note": "the planets carry no rot_pole_w0, so the conversion "
                        "never runs for them; this is the corpus CHECK that the "
                        "corrected conversion targets the shipped convention"}
        pred_path.write_text(json.dumps(pred, indent=1))
        print(json.dumps(pred["planet_anchor"], indent=1))
        for m, b in pred["bodies"].items():
            print(f"{m:9s} u_sub {b['u_sub_pre']:.4f} -> {b['u_sub_post']:.4f}   "
                  f"dark fraction of lit disc {b['dark_fraction_of_lit_disc_pre']:.3f}"
                  f" -> {b['dark_fraction_of_lit_disc_post']:.3f}")
        print(f"keyed bodies: {sum(1 for v in pred['offsets'].values() if v['keyed'])}"
              f" of {len(pred['offsets'])} hopped")
        print("written:", pred_path)
        return 0

    # ---------------------------------------------------------------- --check
    pred = json.loads(pred_path.read_text())
    fails = []
    # The --check leg wrote NO artifact: its two gate values (the NCC pair at
    # :206, the changed-pixel count at :214) lived only in the printed line
    # (§11.167(i)). Recorded below, observation only - nothing reads this dict
    # and no gate consults it.
    scored = {}
    for _, moon, _, _ in F.SCENES:
        post = frame(out, "post", moon, ini)
        pre = frame(out, "pre", moon, ini)
        # the PREDICTED post-fix frame, rebuilt from the PRE-fix capture
        aP, lamP, mP = F.synth(pre["tex"], pre["RE"], pre["s_mesh"], pre["cx"],
                               pre["cy"], pre["R"], pre["img"].shape, -math.pi/2, True, -1.0)
        a0, lam0, m0 = F.synth(pre["tex"], pre["RE"], pre["s_mesh"], pre["cx"],
                               pre["cy"], pre["R"], pre["img"].shape, 0.0, True, -1.0)
        s_pred = score_against(post, aP, lamP, mP)
        s_null = score_against(post, a0, lam0, m0)
        # px difference on the FULL-resolution frames - the terminal observable,
        # not the 256-px working crop (a downsample halves the count).
        fa = np.asarray(Image.open(out / f"f14_pre_{moon}.png").convert("L")).astype(int)
        fb = np.asarray(Image.open(out / f"f14_post_{moon}.png").convert("L")).astype(int)
        px = int((np.abs(fa - fb) > 32).sum())
        px8 = int((np.abs(fa - fb) > 8).sum())
        off_pre = pre["hops"][0]["offset"]
        off_post = post["hops"][0]["offset"]
        d_off = ((off_post - off_pre) % 360.0)
        exp_off = pred["offsets"][moon]["post"]
        line = (f"{moon:9s} offset {off_pre:.6f} -> {off_post:.6f} (d {d_off:+.6f}, "
                f"predicted {exp_off:.6f}) | live post vs PREDICTED image "
                f"{s_pred:+.3f}, vs NULL {s_null:+.3f} | pre-vs-post full-frame "
                f"px>32 {px}, px>8 {px8}")
        print(line)
        if abs(((off_post - exp_off + 180) % 360) - 180) > 2e-4:
            fails.append(f"{moon}: offset {off_post} != predicted {exp_off}")
        if not (s_pred > 0.25 and s_pred - s_null > 0.25):
            fails.append(f"{moon}: the live post-fix frame does not select the "
                         f"predicted image ({s_pred:.3f} vs null {s_null:.3f})")
        # A 90 deg turn of a ~450-px disc must move a large fraction of it. The
        # floor is stated per body as a fraction of the DISC AREA, so a bland
        # map (Proteus) is judged against its own scene rather than a constant.
        area = math.pi*(pred["bodies"][moon].get("disc_radius_px",
                        post["body"]["screenSize"]*1024.0))**2
        scored[moon] = {"ncc_vs_predicted": s_pred, "ncc_vs_null": s_null,
                        "px_gt32": px, "px_gt8": px8, "disc_area_px": area,
                        "offset_pre": off_pre, "offset_post": off_post}
        if px8 < 0.20*area:
            fails.append(f"{moon}: only {px8} px>8 changed between the binaries "
                         f"({px8/area:.1%} of the {area:.0f}-px disc) - a 90 deg "
                         f"turn cannot be that small")
    (out / "f14_check_results.json").write_text(json.dumps(
        {"file": "f14_predict.py --check", "bodies": scored, "fails": fails},
        indent=1))
    for m in fails:
        print("FAIL:", m)
    print("RESULT:", "ALL OK" if not fails else f"{len(fails)} FAILURE(S)")
    return 0 if not fails else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
