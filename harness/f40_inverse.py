#!/usr/bin/env python3
"""F40 / INTENT §5.80 — THE freeMode CONVERTER AS THE COMPOSER'S EXACT INVERSE.

    cd claude/harness && DISPLAY=:2 ./f40_inverse.py <absOutdir> --bin B --expect pre|post

ONE instrument, TWO binaries.  Every claim below is scored against BOTH
hypotheses and the loser is reported with its own residual, so a PRE run and a
POST run are the same measurement read two ways rather than two harnesses:

    H_A         `position` = spheToRect(-lon,lat)*distance      (today, §5.80)
    H_composer  `position` = -posePart(lon,lat,distance)        (the fix, §11.153)
                 posePart(l,p,d) = d*(cos p sin l, -cos p cos l, sin p)
                                 = viewMat()'s ANCHORED branch solved for the eye

`--expect post` gates on H_composer, `--expect pre` on H_A; the numbers printed
are identical either way.  The eye is always read the convention-proof way from
the DRAWN matrix, E = -Rot^T . t (`Camera::getReferenceRelativePosition`), so
nothing here trusts the pose members it is testing.

WHAT EACH LEG DISCRIMINATES

  L0  A/A control            no command: 0 AU, 0 px.  The floor every other
                             number is read against.
  L1  anchored calibration   E == Z(theta).pB — the composer, and it is
                             UNCHANGED by this task (the anchored place is the
                             baseline; §11.52(b)).
  L2  the entry toggle       POST: the eye does not move (that IS the fix).
                             PRE: it moves to -Z(theta).pA, the 137.06 deg
                             swing §11.144(c) predicted and measured.
                             Also: `get status position` reports the ENTRY
                             TRIPLE on BOTH binaries — the §11.144 rider 2
                             observable, PRESERVED rather than settled.
  L3  moveTo, both modes     the SAME command issued free and anchored: PRE
                             16 700 km apart (§11.144(g)), POST the same place.
  L4  descend / moveEyeRel   UNCHANGED, and this leg exists to prove it: the
                             predictions are F34's, the tolerance is F34's, and
                             both binaries must pass them identically.
  L5  the reversible pair    entered TWICE, the second entry starting from the
                             state the first exit produced, with a flight
                             between the traverses so it is not an A/A^-1
                             identity.  POST: every teleport is 0 km.
  L6  screen, body in frame  the composed screen across the toggle vs the A/A
                             control.  POST: equal to the control.
  L7  the SHIPPED place      §5.80's own headline case (43d18'N 5d22'E, 75 m)
                             at the SHIPPED flags: PRE 124.6797 deg / 11 298.6 km
                             / ~3.3 Mpx, POST 0/0/floor.
  L8  MARS                   §5.80's third witness (5.5e+04 km from Mars, the
                             6354 px screen), reference-switched — the fix must
                             hold at a body the observer is not standing on.
  L9  anchored regression    a fixed anchored itinerary, dumped and shot, for
                             BIT-IDENTITY between the two binaries (compared by
                             f40_cmp.py — this leg only records).

TOLERANCES, mechanism-named, taken from F34 unchanged (§11.144's own floors):
  POSE_FLOOR 3.0e-07 AU   float32 1 AU-scale subtraction inside -Rot^T.t
  MEMBER_FLOOR 1.0e-09 AU member-to-member, no 1 AU subtraction
  ANG_FLOOR 1.0e-04 rad   6e-08 AU over a 4e-05 AU arm
"""

import argparse, json, math, sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f27_reply import Session
import dumpread                          # the ONE dump reader (§11.152(p)(2))

HERE = Path(__file__).resolve().parent
JD0 = 2461233.5
AU_KM = 149597870.691                    # sc_const.hpp:45
POSE_FLOOR = 3.0e-07
MEMBER_FLOOR = 1.0e-09
ANG_FLOOR = 1.0e-04

CHECKS, DATA = [], {}
EXPECT = "post"


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""),
          flush=True)
    return bool(ok)


def note(label, detail):
    DATA.setdefault("notes", []).append({"label": label, "detail": detail})
    print("     " + label + "  -- " + detail, flush=True)


# ------------------------------------------------------ the two hypotheses
def zrot(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]])


def pB(lam, phi, d):
    """posePart / viewMat()'s anchored branch solved for the eye."""
    return np.array([d * math.cos(phi) * math.sin(lam),
                     -d * math.cos(phi) * math.cos(lam),
                     d * math.sin(phi)])


def pA(lam, phi, d):
    """spheToRect(-lon, lat) * distance — the pre-fix converter."""
    return np.array([d * math.cos(phi) * math.cos(lam),
                     -d * math.cos(phi) * math.sin(lam),
                     d * math.sin(phi)])


def member_pred(lam, phi, d):
    """What `position` must hold after entering free flight, per hypothesis."""
    return {"A": pA(lam, phi, d), "composer": -pB(lam, phi, d)}


def place_of(P, hyp):
    """The (lon, lat, distance) triple the EXIT recovers from `position`."""
    if hyp == "A":                                   # rectToSphe, negate lng
        return (-math.atan2(P[1], P[0]), math.asin(P[2] / np.linalg.norm(P)),
                float(np.linalg.norm(P)))
    q = -P                                           # posePartToPose(-position)
    return (math.atan2(q[0], -q[1]), math.asin(q[2] / np.linalg.norm(q)),
            float(np.linalg.norm(q)))


def eye_after_entry(lam, phi, d, th, hyp):
    """The DRAWN eye after `free_mode on`: E = -Z(theta).position."""
    return -zrot(th) @ member_pred(lam, phi, d)[hyp]


def swing_A(lam, phi):
    return math.acos(max(-1.0, min(1.0, math.cos(phi) ** 2 * (1 - math.sin(2 * lam)) - 1)))


# ------------------------------------------------------------------ readers
def mat_of(m16):
    m = np.array(m16, dtype=float).reshape(4, 4).T   # r is COLUMN-major
    return m[:3, :3], m[:3, 3]


def eye_of(m16):
    R, t = mat_of(m16)
    return -R.T @ t


def ang(u, v):
    n = np.linalg.norm(u) * np.linalg.norm(v)
    return float("nan") if n == 0 else math.acos(
        max(-1.0, min(1.0, float(np.dot(u, v)) / n)))


def px(a, b, thr=8):
    A = np.asarray(Image.open(a).convert("L"), dtype=int)
    B = np.asarray(Image.open(b).convert("L"), dtype=int)
    return int((np.abs(A - B) > thr).sum())


class App:
    def __init__(self, sess, drv, outdir, tag):
        self.sess, self.drv, self.outdir, self.tag = sess, drv, outdir, tag
        self.n = 0
        self.log = []

    def cmd(self, c, pause=0.8):
        self.drv.send(c, pause)
        self.log.append(c)

    def dump(self, name):
        self.n += 1
        p = self.outdir / f"dump_{self.n:03d}_{name}.json"
        self.drv.send(f"body action dual_dump filename {p}", 1.4)
        head, pairs, mnew, mold = dumpread.load_dump(p)
        cam = head["camera"]
        bodies = {r["name"]: r for r in pairs}
        st = {"tag": name, "file": str(p), "ref": cam["reference"],
              "free": cam["freeMode"], "bound": cam["boundToSurface"],
              "lon": cam["longitude"], "lat": cam["latitude"], "dist": cam["distance"],
              "position": list(cam["position"]), "mat": list(cam["mat"]),
              "E": eye_of(cam["mat"]).tolist(), "rootPos": list(cam["rootPos"]),
              "selDist": cam.get("selDist"), "refDist": cam.get("refDist"),
              "control": {k: head["control"][k]["new"]
                          for k in ("latitude", "longitude", "altitude")},
              "jd": head.get("jd")}
        nb = (bodies.get(st["ref"], {}).get("new")) or {}
        st["axisRot"] = nb.get("axisRot")
        st["theta"] = (nb["axisRot"] + math.pi / 2) if "axisRot" in nb else None
        st["refBody"] = {k: nb.get(k) for k in
                         ("ecl", "eclDisplay", "dist", "scaledDatumRadius", "screen")}
        return st

    def place(self):
        """`get status position` — the shipped readout (F27, §11.135)."""
        self.drv.sock.sendall(b"get status position\n")
        r, _, _ = self.drv.poll_for_reply(4.0)
        return r

    def shot(self, name):
        p = self.outdir / f"shot_{name}.png"
        self.drv.send(f"body action screenshot filename {p}", 2.5)
        return p


def score_entry(app, before, tag, expect):
    """The toggle ON, scored both ways.  Returns (after, dict)."""
    lam, phi, d, th = before["lon"], before["lat"], before["dist"], before["theta"]
    app.cmd("camera action free_mode state on", 1.5)
    after = app.dump(tag)
    P = np.array(after["position"])
    E0, E1 = np.array(before["E"]), np.array(after["E"])
    res = {}
    for hyp in ("A", "composer"):
        res[hyp] = {
            "member": float(np.linalg.norm(P - member_pred(lam, phi, d)[hyp])),
            "eye": float(np.linalg.norm(E1 - eye_after_entry(lam, phi, d, th, hyp))),
        }
    d_move = float(np.linalg.norm(E1 - E0))
    out = {"before": before, "after": after, "res": res,
           "move_AU": d_move, "move_km": d_move * AU_KM,
           "swing_deg": math.degrees(ang(E0, E1)),
           "swing_A_pred_deg": math.degrees(swing_A(lam, phi)),
           "radius_before_AU": float(np.linalg.norm(E0)),
           "radius_after_AU": float(np.linalg.norm(E1))}
    w, l = (expect, "A" if expect == "composer" else "composer")
    chk(res[w]["member"] < MEMBER_FLOOR and res[w]["member"] < res[l]["member"] / 100,
        f"[{tag}] member: `position` is the {w}'s",
        f"res_{w} = {res[w]['member']:.3e} AU vs res_{l} = {res[l]['member']:.3e} AU")
    chk(res[w]["eye"] < POSE_FLOOR and res[w]["eye"] < res[l]["eye"] / 100,
        f"[{tag}] the DRAWN eye after `free_mode on` is the {w}'s",
        f"res_{w} = {res[w]['eye']:.3e} AU vs res_{l} = {res[l]['eye']:.3e} AU")
    if expect == "composer":
        chk(d_move < POSE_FLOOR, f"[{tag}] THE FIX: the observer does not move",
            f"{d_move:.3e} AU = {d_move*AU_KM:.4f} km ; swing {out['swing_deg']:.4f} deg")
    else:
        chk(abs(out["swing_deg"] - out["swing_A_pred_deg"]) < math.degrees(ANG_FLOOR),
            f"[{tag}] the observer teleports by the predicted swing",
            f"measured {out['swing_deg']:.4f} deg, predicted {out['swing_A_pred_deg']:.4f}; "
            f"chord {out['move_km']:.1f} km")
    return after, out


def main():
    global EXPECT
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    ap.add_argument("--expect", choices=("pre", "post"), default="post")
    a = ap.parse_args()
    EXPECT = "A" if a.expect == "pre" else "composer"
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    DATA["expect"] = a.expect
    DATA["binary"] = a.bin

    sess = Session(out, f"f40{a.expect}", a.bin)
    drv = sess.client("drv")
    app = App(sess, drv, out, a.expect)
    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        app.cmd(c, 0.5)
    app.cmd(f"date jday {JD0:.9f}", 1.2)
    LAM0, PHI0, ALT0 = 70.0, 30.0, 1.0e6      # F34's discriminating place
    app.cmd(f"moveto lat {PHI0} lon {LAM0} alt {ALT0} duration 0", 1.2)

    # ---------------- L0: the A/A control ----------------------------------
    s1, sh1 = app.dump("aa1"), app.shot("aa1")
    s2, sh2 = app.dump("aa2"), app.shot("aa2")
    d_aa = float(np.linalg.norm(np.array(s2["E"]) - np.array(s1["E"])))
    px_aa = px(sh1, sh2)
    chk(d_aa == 0.0, "L0 A/A control: no command, the eye does not move", f"{d_aa:.3e} AU")
    chk(px_aa == 0, "L0 A/A control: no command, the screen is identical", f"{px_aa} px>8")
    DATA["L0"] = {"dE": d_aa, "px": px_aa}

    # ---------------- L1: the composer, calibrated -------------------------
    st = s2
    E = np.array(st["E"])
    th = st["theta"]
    rB = float(np.linalg.norm(E - zrot(th) @ pB(st["lon"], st["lat"], st["dist"])))
    rA = float(np.linalg.norm(E - zrot(th) @ pA(st["lon"], st["lat"], st["dist"])))
    chk(rB < POSE_FLOOR and rB < rA / 100,
        "L1 anchored: the drawn eye IS Z(theta).pB — the composer, UNCHANGED here",
        f"res_B = {rB:.3e} AU vs res_A = {rA:.3e} AU (theta = {th:.6f} rad)")
    th_fit = math.atan2(E[1], E[0]) - math.atan2(*reversed(list(
        pB(st["lon"], st["lat"], st["dist"])[:2])))
    th_fit = (th_fit + math.pi) % (2 * math.pi) - math.pi
    th_dump = (th + math.pi) % (2 * math.pi) - math.pi
    chk(abs(th_fit - th_dump) < 1e-4,
        "L1 the surface fold read from axisRot IS the one in the matrix",
        f"fitted {th_fit:.6f} vs axisRot+pi/2 {th_dump:.6f} rad")
    altRef = st["dist"] - st["control"]["altitude"] / (1000 * AU_KM)
    DATA["L1"] = {"res_B": rB, "res_A": rA, "theta_fit": th_fit, "theta_dump": th_dump,
                  "altitudeReference_AU": altRef, "distance_AU": st["dist"]}
    place0 = app.place()

    # ---------------- L2: the entry toggle ---------------------------------
    after, L2 = score_entry(app, st, "L2_entry", EXPECT)
    sh_on = app.shot("L2_entry")
    place1 = app.place()
    L2["px"] = px(sh2, sh_on)
    L2["px_control"] = px_aa
    if EXPECT == "composer":
        chk(L2["px"] == px_aa, "L2 screen: the toggle leaves the composed frame alone",
            f"{L2['px']} px>8 against the A/A control's {px_aa}")
    else:
        chk(L2["px"] > px_aa, "L2 screen: the toggle changes the composed frame",
            f"{L2['px']} px>8 against the A/A control's {px_aa}")
    chk(abs(np.linalg.norm(after["E"]) - np.linalg.norm(st["E"])) < POSE_FLOOR,
        "L2 the distance to the reference is preserved (why selDist is blind)",
        f"{np.linalg.norm(st['E']):.9e} -> {np.linalg.norm(after['E']):.9e} AU")
    # THE RIDER, PRESERVED: `get status position` must answer the ENTRY TRIPLE on
    # BOTH binaries.  It does so on the pre binary because getPlace inverts what
    # the converter just wrote (§11.144(f)); it must keep doing so after the fix,
    # or the fix has SETTLED the second §11.144 rider instead of leaving it open.
    chk(place0 and place1 and abs(place0["lat"] - place1["lat"]) < 1e-6
        and abs(place0["lon"] - place1["lon"]) < 1e-6
        and abs(place0["alt"] - place1["alt"]) < 1e-2,
        "L2 readout: `get status position` answers the ENTRY TRIPLE (rider 2 untouched)",
        f"{place0} -> {place1}")
    L2["place_before"], L2["place_after"] = place0, place1
    DATA["L2"] = L2

    # ---------------- L3: moveTo in both modes -----------------------------
    LAM1, PHI1, ALT1 = -130.0, -20.0, 2.0e6
    app.cmd(f"moveto lat {PHI1} lon {LAM1} alt {ALT1} duration 0", 1.5)
    m_free = app.dump("L3_moveto_free")
    lam1, phi1 = math.radians(LAM1), math.radians(PHI1)
    d1 = altRef + ALT1 / (1000 * AU_KM)
    th1 = m_free["theta"]
    E1 = np.array(m_free["E"])
    rA1 = float(np.linalg.norm(E1 + zrot(th1) @ pA(lam1, phi1, d1)))
    rB1 = float(np.linalg.norm(E1 - zrot(th1) @ pB(lam1, phi1, d1)))
    w, l = ((rB1, rA1), ("composer", "A")) if EXPECT == "composer" else ((rA1, rB1), ("A", "composer"))
    chk(w[0] < POSE_FLOOR and w[0] < w[1] / 100,
        f"L3 moveTo free branch lands at the {l[0]}'s place for the COMMANDED triple",
        f"res_{l[0]} = {w[0]:.3e} AU vs res_{l[1]} = {w[1]:.3e} AU")
    app.cmd("camera action free_mode state off", 1.5)
    app.cmd(f"moveto lat {PHI1} lon {LAM1} alt {ALT1} duration 0", 1.5)
    m_anch = app.dump("L3_moveto_anchored")
    E1a = np.array(m_anch["E"])
    rB1a = float(np.linalg.norm(E1a - zrot(m_anch["theta"]) @ pB(lam1, phi1, m_anch["dist"])))
    chk(rB1a < POSE_FLOOR, "L3 control: the SAME command anchored lands at Z(theta).pB",
        f"res_B = {rB1a:.3e} AU")
    sep = float(np.linalg.norm(E1a - E1) * AU_KM)
    DATA["L3"] = {"free": m_free, "anchored": m_anch, "res_A": rA1, "res_B": rB1,
                  "control_res_B": rB1a, "separation_km": sep,
                  "separation_deg": math.degrees(ang(E1a, E1))}
    if EXPECT == "composer":
        # FLOOR, named rather than rounded: the two branches reach the same eye
        # by DIFFERENT arithmetic (one builds -posePart and lets viewMat
        # translate by it, the other writes `distance` and lets viewMat compose
        # T(0,0,-d).X.Z), so they may differ by a few float32 ulp of the eye's
        # own magnitude - 5.6e-05 AU here, whose ulp is 3.3e-12 AU = 0.50 m.
        # 20 m = 40 ulp, still 8e+05 times below the pre-fix separation of
        # 16 700 km.  (The first spelling gated at 1 m, BELOW the representable
        # floor, and failed at 1.631 m = 3.3 ulp: a gate error, attributed at
        # the digit.)
        ulp_m = float(np.linalg.norm(E1a)) * 2**-24 * AU_KM * 1000
        chk(sep * 1000 < 20.0, "L3 ONE command, ONE place, in both modes",
            f"{sep*1000:.3f} m apart = {sep*1000/ulp_m:.1f} float32 ulp "
            f"({ulp_m:.2f} m); pre-fix 16 700 km")
    else:
        chk(sep > 1000, "L3 one command, two modes, two PLACES",
            f"{sep:.1f} km apart, {math.degrees(ang(E1a, E1)):.3f} deg")

    # ---------------- L4: descend / moveEyeRel — UNCHANGED -----------------
    app.cmd("camera action free_mode state on", 1.5)
    app.cmd("select planet Earth pointer off", 1.0)
    app.cmd("flag track_object on", 6.0)
    app.cmd("flag track_object off", 1.5)
    aim1, aim2 = app.dump("L4_aim1"), app.dump("L4_aim2")
    chk(np.array_equal(np.array(aim1["mat"])[:12], np.array(aim2["mat"])[:12]),
        "L4 precondition: with tracking off the view rotation is static",
        "mat rotation identical across two dumps")
    d0 = aim1
    R0, _ = mat_of(d0["mat"])
    up = np.array(R0[2, :])
    g = altRef
    COEF_UP = 1.25
    step_pred = max((COEF_UP - 1) * (float(np.linalg.norm(d0["E"])) - g), 1e-6 * g)
    predU = step_pred * up
    app.cmd(f"camera action descend coef {COEF_UP}", 1.2)
    du = app.dump("L4_desc_up")
    dE = np.array(du["E"]) - np.array(d0["E"])
    rU, rUflip = float(np.linalg.norm(dE - predU)), float(np.linalg.norm(dE + predU))
    chk(rU < POSE_FLOOR and rU < rUflip / 100,
        "L4 descend(ascend): dE == +step*(Rot^T.z) — F34's prediction, UNCHANGED",
        f"res = {rU:.3e} AU vs flipped {rUflip:.3e} AU ; "
        f"|dE| = {np.linalg.norm(dE)*AU_KM:.3f} km predicted {step_pred*AU_KM:.3f} km")
    r_comp = float(np.linalg.norm(np.array(du["E"]) + zrot(du["theta"]) @ np.array(du["position"])))
    chk(r_comp < POSE_FLOOR,
        "L4 after a flight the free composer holds (E == -Z(theta).position)",
        f"res = {r_comp:.3e} AU")
    COEF_DOWN = 0.8
    R1, C1v = mat_of(du["mat"])
    fwd1 = np.array(-R1[2, :])
    Cz, disc = C1v[2], C1v[2] ** 2 - (float(C1v @ C1v) - g * g)
    s = (-Cz - math.sqrt(disc)) if disc >= 0 else -1.0
    if s > 0:
        branch, predD = "ray", (1 - COEF_DOWN) * s * fwd1
    else:
        alt_l = max(float(np.linalg.norm(du["E"])) - g, 0.0)
        branch = "radial"
        predD = np.array(du["E"]) * ((COEF_DOWN - 1) * alt_l / float(np.linalg.norm(du["E"])))
    app.cmd(f"camera action descend coef {COEF_DOWN}", 1.2)
    dd = app.dump("L4_desc_down")
    dE2 = np.array(dd["E"]) - np.array(du["E"])
    rD, rDflip = float(np.linalg.norm(dE2 - predD)), float(np.linalg.norm(dE2 + predD))
    chk(rD < POSE_FLOOR and rD < rDflip / 100,
        f"L4 descend(descend), {branch} branch — F34's prediction, UNCHANGED",
        f"res = {rD:.3e} AU vs flipped {rDflip:.3e} AU ; "
        f"|dE| = {np.linalg.norm(dE2)*AU_KM:.3f} km predicted {np.linalg.norm(predD)*AU_KM:.3f} km")
    DATA["L4"] = {"res_up": rU, "res_up_flipped": rUflip, "res_down": rD,
                  "res_down_flipped": rDflip, "branch": branch,
                  "step_up_km": step_pred * AU_KM,
                  "dE_up_km": float(np.linalg.norm(dE) * AU_KM),
                  "dE_down_km": float(np.linalg.norm(dE2) * AU_KM),
                  "res_free_composer": r_comp}

    # ---------------- L5: the reversible pair, TWICE ------------------------
    trav, cur = [], dd
    for i in (1, 2):
        P = np.array(cur["position"])
        lam2, phi2, d2v = place_of(P, EXPECT)
        pred_off = zrot(cur["theta"]) @ pB(lam2, phi2, d2v)
        app.cmd("camera action free_mode state off", 1.5)
        off = app.dump(f"L5_exit_{i}")
        Eo = np.array(off["E"])
        r_off = float(np.linalg.norm(Eo - pred_off))
        r_hold = float(np.linalg.norm(Eo - np.array(cur["E"])))
        exit_km = r_hold * AU_KM
        chk(r_off < POSE_FLOOR,
            f"L5 exit #{i}: `free_mode off` lands where the {EXPECT} says",
            f"res = {r_off:.3e} AU ; exit teleport {exit_km:.4f} km")
        if EXPECT == "composer":
            chk(r_hold < POSE_FLOOR, f"L5 exit #{i}: THE FIX — the observer does not move",
                f"{r_hold:.3e} AU = {exit_km:.4f} km")
        # re-enter from the state the exit produced
        pred_on = eye_after_entry(off["lon"], off["lat"], off["dist"], off["theta"], EXPECT)
        app.cmd("camera action free_mode state on", 1.5)
        on = app.dump(f"L5_entry_{i+1}")
        r_on = float(np.linalg.norm(np.array(on["E"]) - pred_on))
        entry_km = float(np.linalg.norm(np.array(on["E"]) - Eo) * AU_KM)
        chk(r_on < POSE_FLOOR,
            f"L5 entry #{i+1}: predicted from the state the previous exit produced",
            f"res = {r_on:.3e} AU ; teleport {entry_km:.4f} km")
        trav.append({"exit_res": r_off, "exit_km": exit_km, "entry_res": r_on,
                     "entry_km": entry_km, "exit_place": [lam2, phi2, d2v],
                     "exit_state": off, "entry_state": on})
        app.cmd("camera action descend coef 1.1", 1.2)   # fly between the traverses
        cur = app.dump(f"L5_fly_{i}")
    DATA["L5"] = trav

    # ---------------- L6: the screen, body in frame ------------------------
    app.cmd("flag track_object on", 6.0)
    app.cmd("flag track_object off", 1.5)
    sh_f1, s_f1 = app.shot("L6_free_1"), app.dump("L6_free_1")
    app.cmd("camera action free_mode state off", 1.5)
    sh_a, s_a = app.shot("L6_anchored"), app.dump("L6_anchored")
    app.cmd("camera action free_mode state on", 1.5)
    sh_f2, s_f2 = app.shot("L6_free_2"), app.dump("L6_free_2")
    px_ab, px_ctl = px(sh_f1, sh_a), px(sh_f1, sh_f2)
    d_pair = float(np.linalg.norm(np.array(s_f2["E"]) - np.array(s_f1["E"])))
    DATA["L6"] = {"px_ab": px_ab, "px_pair": px_ctl, "pair_dE_AU": d_pair,
                  "teleport_km": float(np.linalg.norm(
                      np.array(s_a["E"]) - np.array(s_f1["E"])) * AU_KM)}
    if EXPECT == "composer":
        chk(px_ab == px_ctl == 0 and d_pair < 1e-11,
            "L6 SCREEN: the toggle leaves the frame alone, and so does the pair",
            f"free->anchored {px_ab} px>8 ; free->free (A/A) {px_ctl} px>8 ; "
            f"pair {d_pair:.3e} AU")
    else:
        chk(px_ab > px_ctl and px_ctl == 0 and d_pair < 1e-11,
            "L6 SCREEN: the toggle moves the frame; the toggle PAIR does not",
            f"free->anchored {px_ab} px>8 ; free->free (A/A) {px_ctl} px>8 ; "
            f"pair {d_pair:.3e} AU")

    # ---------------- L7: the SHIPPED place --------------------------------
    # TWO sub-legs, because the shipped flags carry a SECOND mode-dependence
    # that is not the converter's: `EnvironmentManager::update` sets
    # `onBody = !camera.isFreeMode() && !reference->isSystem()`
    # [observed: EnvironmentManager.cpp:81], so entering free flight takes the
    # ANCHOR branch and the GROUNDED environment members (landscape + fog) stop
    # updating.  That change is ~79 % of the shipped-flags frame and it survives
    # the fix, because the fix moves the observer's place and not the
    # environment's regime.  L7a (flags OFF) is the converter's own claim;
    # L7b (flags ON) is §5.80's headline scene, recorded with that attribution
    # and gated only on the OBSERVER.  f40_env.py measures the four flag cells
    # on both binaries.
    app.cmd("camera action free_mode state off", 1.5)
    LAM2, PHI2, ALT2 = 5.0 + 22.0 / 60.0, 43.0 + 18.0 / 60.0, 75.0
    app.cmd(f"moveto lat {PHI2} lon {LAM2} alt {ALT2} duration 0", 1.5)
    L7 = {"lon_deg": LAM2, "lat_deg": PHI2, "alt_m": ALT2}
    for cell, flags in (("off", []), ("on", ["atmosphere", "landscape"])):
        for f in flags:
            app.cmd(f"flag {f} on", 6.0 if f == "landscape" else 1.0)
        c1, c2 = app.shot(f"L7{cell}_ctl1"), app.shot(f"L7{cell}_ctl2")
        floor = px(c1, c2)
        sh0, s0 = app.shot(f"L7{cell}_anchored"), app.dump(f"L7{cell}_anchored")
        app.cmd("camera action free_mode state on", 1.5)
        sh1, s1 = app.shot(f"L7{cell}_free"), app.dump(f"L7{cell}_free")
        app.cmd("camera action free_mode state off", 1.5)
        sh2, s2 = app.shot(f"L7{cell}_anchored_2"), app.dump(f"L7{cell}_anchored_2")
        E0, E1 = np.array(s0["E"]), np.array(s1["E"])
        L7[cell] = {"swing_deg": math.degrees(ang(E0, E1)),
                    "chord_km": float(np.linalg.norm(E1 - E0) * AU_KM),
                    "px_ab": px(sh0, sh1), "px_pair": px(sh0, sh2), "px_floor": floor,
                    "pair_dE_AU": float(np.linalg.norm(np.array(s2["E"]) - E0))}
        for f in flags:
            app.cmd(f"flag {f} off", 0.8)
    sw2_pred = math.degrees(swing_A(math.radians(LAM2), math.radians(PHI2)))
    L7["swing_A_pred_deg"] = sw2_pred
    DATA["L7"] = L7
    if EXPECT == "composer":
        # The FLOOR here is measured in the same leg, not assumed: `pair_dE_AU`
        # is |E(anchored_2) - E(anchored)|, two ANCHORED dumps with a toggle
        # pair between them, i.e. the null control for exactly this comparison.
        # float32 ulp at 4.26e-05 AU is 0.38 m, so a sub-metre reading is one
        # ulp and cannot be made smaller by any converter.
        m_off = L7["off"]["chord_km"] * 1000.0
        m_on = L7["on"]["chord_km"] * 1000.0
        f_off = L7["off"]["pair_dE_AU"] * AU_KM * 1000.0
        f_on = L7["on"]["pair_dE_AU"] * AU_KM * 1000.0
        chk(m_off < 20.0 and m_on < 20.0,
            "L7 SHIPPED place: the toggle does not move the observer (either flag cell)",
            f"flags off {m_off:.3f} m (null control {f_off:.3f} m) ; flags on "
            f"{m_on:.3f} m (null control {f_on:.3f} m) ; pre-fix 11 298 585 m")
        chk(L7["off"]["px_ab"] <= max(L7["off"]["px_floor"], 1) * 4,
            "L7a SHIPPED place, flags OFF: the composed frame is unchanged",
            f"{L7['off']['px_ab']} px>8 against the adjacent floor "
            f"{L7['off']['px_floor']} px>8 (round trip {L7['off']['px_pair']})")
        note("L7b SHIPPED place, flags ON: the frame still changes, and it is NOT "
             "the observer", f"{L7['on']['px_ab']} px>8 while the observer moves "
             f"{L7['on']['chord_km']*1000:.3f} m — EnvironmentManager.cpp:81's "
             f"onBody gate, see f40_env.py")
    else:
        chk(abs(L7["on"]["swing_deg"] - sw2_pred) < math.degrees(ANG_FLOOR)
            and L7["on"]["px_ab"] > 1000,
            "L7 SHIPPED place: §5.80's own headline, reproduced from the prediction",
            f"swing {L7['on']['swing_deg']:.4f} deg (pred {sw2_pred:.4f}) ; chord "
            f"{L7['on']['chord_km']:.1f} km ; flags ON {L7['on']['px_ab']} px>8 "
            f"(floor {L7['on']['px_floor']}) ; flags OFF {L7['off']['px_ab']} px>8 "
            f"(floor {L7['off']['px_floor']})")

    # ---------------- L8: MARS, §5.80's third witness ----------------------
    # THE PLACE IS CHOSEN FROM THE CLOSED FORM, not for roundness: the swing is
    # acos(cos^2(phi)(1 - sin 2 lam) - 1), so at (lam, phi) = (-25 deg, 0) it is
    # 40.0 deg - large enough to be a real screen effect, small enough that the
    # reference stays INSIDE the dome across the toggle.  The first spelling used
    # (47, 12), whose swing is 176.09 deg: Mars went from dead centre to BEHIND
    # the observer (`visible: false`, eye-frame z = +3.89e-04 AU), so the frame
    # lost its only body and 34 px>8 was measuring the star field - which the OLD
    # path draws, and the old path has no free mode, so it cannot move.  A gate
    # error, attributed at the dump before anything was changed (F34's rule).
    app.cmd("set home_planet Mars", 6.0)
    app.cmd("select planet Mars pointer off", 1.0)
    app.cmd("moveto lat 0 lon -25 alt 55000000 duration 0", 2.0)
    app.cmd("flag track_object on", 6.0)
    app.cmd("flag track_object off", 1.5)
    m_ctl1, m_ctl2 = app.shot("L8_ctl1"), app.shot("L8_ctl2")
    px_mfloor = px(m_ctl1, m_ctl2)
    sh_m0, s_m0 = app.shot("L8_anchored"), app.dump("L8_anchored")
    m_after, L8 = score_entry(app, s_m0, "L8_entry", EXPECT)
    sh_m1 = app.shot("L8_free")
    L8["px_ab"] = px(sh_m0, sh_m1)
    L8["px_floor"] = px_mfloor
    L8["ref"] = s_m0["ref"]
    chk(s_m0["ref"] == "Mars", "L8 precondition: the reference IS Mars", s_m0["ref"])
    if EXPECT == "composer":
        chk(L8["px_ab"] <= max(px_mfloor, 1) * 4,
            "L8 MARS: the toggle leaves the frame alone",
            f"{L8['px_ab']} px>8 against the adjacent floor {px_mfloor}")
    else:
        chk(L8["px_ab"] > max(px_mfloor, 1) * 4,
            "L8 MARS: §5.80's third witness, the composed screen",
            f"{L8['px_ab']} px>8 against the adjacent floor {px_mfloor} ; "
            f"chord {L8['move_km']:.1f} km ; swing {L8['swing_deg']:.4f} deg")
    app.cmd("camera action free_mode state off", 1.5)
    DATA["L8"] = L8

    # ---------------- L9: the ANCHORED regression itinerary ----------------
    # Anchored places only, dumped and shot: the two binaries must agree BIT for
    # BIT (f40_cmp.py).  This is the "unchanged by construction" half of the DoD
    # measured on the free-mode-adjacent surface rather than argued.
    app.cmd("select planet Earth pointer off", 1.0)
    app.cmd("set home_planet Earth", 6.0)
    reg = []
    for i, (la, lo, al) in enumerate([(0.0, 0.0, 100.0), (43.3, 5.3667, 75.0),
                                      (-33.9, 151.2, 1000.0), (89.9, -179.9, 500000.0),
                                      (0.0, 60.0, 2000.0)]):
        app.cmd(f"moveto lat {la} lon {lo} alt {al} duration 0", 1.5)
        d = app.dump(f"L9_anchored_{i}")
        s = app.shot(f"L9_anchored_{i}")
        reg.append({"cmd": [la, lo, al], "dump": d, "shot": str(s),
                    "place": app.place()})
    DATA["L9"] = reg
    chk(all(not r["dump"]["free"] for r in reg),
        "L9 the regression itinerary is anchored throughout", f"{len(reg)} places")

    res = {"checks": CHECKS, "data": DATA, "commands": app.log,
           "failures": [c for c in CHECKS if not c["ok"]]}
    (out / "f40_result.json").write_text(json.dumps(res, indent=1, default=str))
    n_ok = sum(1 for c in CHECKS if c["ok"])
    print(f"\nf40_inverse [{a.expect}]: {n_ok}/{len(CHECKS)} checks OK", flush=True)
    sess.stop(drv)
    return 0 if n_ok == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
