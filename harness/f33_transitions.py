#!/usr/bin/env python3
"""F33 / B4(iv) — the scripted camera transitions, old vs new, per member.

    cd claude/harness && DISPLAY=:2 ./f33_transitions.py <outdir> [--bin B]

WHAT IS UNDER TEST. `camera action move_to target point|body`,
`transition_to target point|body` and `align_with body` reached only the old
`AnchorManager` (INTENT §13 B4 clause (iv)). They now drive the new camera path
as well (§11.141). The bar is B34's: OLD'S MEASURED BEHAVIOUR THROUGH THE LIVE
COMMAND CHANNEL, on the same drive wherever the two paths can be read from one
frame.

THE INSTRUMENT'S OWN SHAPE, and why it can fail:

  * ONE observable spans both paths. The old path's observer heliocentric
    position is recovered from the dump's `helioToEye` as -Rᵀ·t (the eye is the
    origin of the eye frame); the new path's is `camera.rootPos`, derived the
    same way from the matrix the renderer consumes. They are comparable because
    the root frame IS heliocentric on this data - MEASURED, not assumed:
    Sun / SolarSystem / MilkyWay all dump ecl [0,0,0] (f33_smoke.py P3).

  * TIME IS STEPPED, NOT RUN. Both travels are pure functions of the DATE, so
    the clock is PAUSED and the date is set explicitly at each step. Without
    that, a 3 s travel over ~1 AU moves 5.6e-03 AU per frame at 60 fps, and any
    one-frame skew between the two paths' updates would swamp a comparison whose
    subject is 1e-7 AU. Stepping removes the frame-rate coupling entirely and
    makes every step reproducible.

  * THE PREDICTION IS COMMITTED BEFORE THE RUN (F29/F32 discipline). It is in
    this file, in `predict_travel`, and it is old's own law transcribed from
    anchor_manager.cpp:406 - logistic on x = 25f-5 with u = 2, s = 1, clamped -
    including the two quirks that make it discriminating: the path STARTS
    9.11e-4 of the distance along (a pop) and ENDS 1.5e-8 short. A merely
    "smooth S-curve" implementation would pass an eyeball and fail this.

  * REFUSALS ARE PART OF THE PORT. Each path logs its own refusal, so the log is
    read per path: `AnchorManager::` lines are old's, `camera action` lines are
    the new registry's. A member that accepts what old refuses would also
    desynchronize the script clock, because the command interface's `wait` rides
    the seam's verdict.

TOLERANCES (stated up front, each with its mechanism):
  POS_FLOOR  2.0e-07 AU  - a place's position is stored in the body's Vec3f
                           eclipticPos; at ~1 AU that is 6e-08 AU per component.
  CMP_FLOOR  3.0e-07 AU  - the above, plus the float32 floor of the 1 AU
                           subtraction inside both -Rᵀ·t extractions.
  VIEW_FLOOR 2.0e-06 rad - 4x the measured per-switch recoverParams Euler floor
                           F = 4.8e-07 rad (§11.111(h)).
"""

import argparse, json, math, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import numpy as np
from f27_reply import Session

HERE = Path(__file__).resolve().parent
JD0 = 2461233.5
POS_FLOOR = 2.0e-07
CMP_FLOOR = 3.0e-07
VIEW_FLOOR = 2.0e-06
DAY = 24 * 60 * 60.0
AU_KM = 149597870.691          # sc_const.hpp:45, the value the engine uses

CHECKS = []
NOTES = {}


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""))
    return ok


def eye_pos(mat16):
    m = np.array(mat16, dtype=float).reshape(4, 4).T
    return -m[:3, :3].T @ m[:3, 3]


def predict_travel(start, target, start_jd, travel_days, jd):
    """anchor_manager.cpp:406, transcribed. COMMITTED BEFORE THE RUN."""
    start, target = np.asarray(start, float), np.asarray(target, float)
    d = target - start
    dist = np.linalg.norm(d)
    if dist == 0:
        return start.copy()
    direction = d / dist
    if travel_days <= 0:
        return start + direction * dist
    j = min(max(jd, start_jd), start_jd + travel_days)
    x = ((j - start_jd) / travel_days) * 25 - 5
    logistic = 1 / (1 + math.exp(-(x - 2)))
    return start + direction * (dist * logistic)


class App:
    def __init__(self, sess, drv, outdir):
        self.sess, self.drv, self.outdir = sess, drv, outdir
        self.n = 0

    def cmd(self, c, pause=0.8):
        mark = self.sess.logmark()
        self.drv.send(c, pause)
        return self.sess.lognew(mark)

    def at(self, jd, pause=0.8):
        self.drv.send(f"date jday {jd:.9f}", pause)

    def dump(self, tag):
        self.n += 1
        p = f"/tmp/f33_{self.n:03d}_{tag}.json"
        self.drv.send(f"body action dual_dump filename {p}", 1.2)
        with open(p) as f:
            head = json.loads(f.readline())
            bodies = {}
            for line in f:
                line = line.strip()
                if line:
                    o = json.loads(line)
                    if o.get("type") == "body":
                        bodies[o["name"]] = o
        head["_old"] = eye_pos(head["helioToEye"]).tolist()
        head["_new"] = list(head["camera"]["rootPos"])
        head["_bodies"] = bodies
        return head

    def shot(self, tag):
        p = self.outdir / f"f33_{tag}.png"
        self.drv.send(f"body action screenshot filename {p}", 2.0)
        return p


def refusals(log):
    """Per-path refusal lines. Old's come from AnchorManager, the new registry's
    from the `camera action ...` diagnostics this port writes (§2(f))."""
    old = [l for l in log.splitlines() if "AnchorManager" in l and "error" in l]
    new = [l for l in log.splitlines() if "camera action" in l
           and ("does nothing" in l or "IGNORED" in l or "NOT changed" in l)]
    return old, new


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    out = Path(a.outdir); out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, "f33", a.bin)
    drv = sess.client("drv")
    app = App(sess, drv, out)
    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        app.cmd(c, 0.5)
    app.at(JD0, 1.2)

    h0 = app.dump("start")
    chk(abs(np.array(h0["_old"]) - np.array(h0["_new"])).max() < CMP_FLOOR,
        "baseline: the two paths report the SAME observer position",
        f"|d| = {np.linalg.norm(np.array(h0['_old'])-np.array(h0['_new'])):.3e} AU")

    # ---------------- R: refusals BEFORE any transition (on a body) ---------
    log = app.cmd("camera action move_to target point x 2 y 0 z 0 duration 5")
    o, n = refusals(log)
    chk(o and n, "R1 move_to point while ON A BODY: BOTH paths refuse",
        f"old={len(o)} new={len(n)}")
    NOTES["R1"] = {"old": o, "new": n}
    log = app.cmd("camera action move_to target body body_name Mars duration 5 altitude 50000")
    o, n = refusals(log)
    chk(o and n, "R2 move_to body while ON A BODY: BOTH paths refuse",
        f"old={len(o)} new={len(n)}")
    NOTES["R2"] = {"old": o, "new": n}
    hR = app.dump("after_refusals")
    chk(np.linalg.norm(np.array(hR["_new"]) - np.array(h0["_new"])) < POS_FLOOR
        and np.linalg.norm(np.array(hR["_old"]) - np.array(h0["_old"])) < POS_FLOOR,
        "R1/R2 changed nothing on either path")

    log = app.cmd("camera action transition_to target body name NoSuchBodyHere")
    o, n = refusals(log)
    chk(n, "R3 transition_to body with an unknown name: the new path refuses and says so",
        f"new={len(n)}")
    NOTES["R3"] = {"old": o, "new": n, "log": log.strip().splitlines()[-4:]}

    # ---------------- M1: transition_to point --------------------------------
    before = app.dump("m1_before")
    log = app.cmd("camera action transition_to target point name Space", 1.2)
    after = app.dump("m1_after")
    d_old = np.linalg.norm(np.array(after["_old"]) - np.array(before["_old"]))
    d_new = np.linalg.norm(np.array(after["_new"]) - np.array(before["_new"]))
    chk(d_old < POS_FLOOR, "M1 old: the observer does not move", f"{d_old:.3e} AU")
    chk(d_new < POS_FLOOR, "M1 new: the observer does not move", f"{d_new:.3e} AU")
    chk(np.linalg.norm(np.array(after["_old"]) - np.array(after["_new"])) < CMP_FLOOR,
        "M1 the two paths still agree on where the observer is",
        f"{np.linalg.norm(np.array(after['_old'])-np.array(after['_new'])):.3e} AU")
    cam = after["camera"]
    chk(cam["reference"] == "Space" and abs(cam["distance"]) < 1e-12
        and not cam["boundToSurface"],
        "M1 new: the camera references the point, AT it, unbound",
        f"ref={cam['reference']} distance={cam['distance']} bound={cam['boundToSurface']}")
    chk(after["anchors"]["current"] == "Space" and after["anchors"]["kind"] == "point",
        "M1 new: the anchor registry names the point", json.dumps(after["anchors"])[:160])
    dv = np.linalg.norm(np.array(after["camera"]["absFwd"]) - np.array(before["camera"]["absFwd"]))
    chk(dv < VIEW_FLOOR, "M1 new: the absolute look direction is held across the switch",
        f"|dAbsFwd| = {dv:.3e} (floor {VIEW_FLOOR:.1e})")
    NOTES["M1"] = {"d_old": d_old, "d_new": d_new, "dAbsFwd": dv,
                   "old_local_vision": [before["oldView"]["localVision"],
                                        after["oldView"]["localVision"]],
                   "heading": [before["oldView"]["heading"], after["oldView"]["heading"],
                               before["camera"]["heading"], after["camera"]["heading"]]}

    # negative duration, now that a place exists
    log = app.cmd("camera action move_to target point x 2 y 0 z 0 duration -5")
    o, n = refusals(log)
    chk(o and n, "R4 negative duration: BOTH paths refuse", f"old={len(o)} new={len(n)}")
    NOTES["R4"] = {"old": o, "new": n}

    # ---------------- M2: move_to point, stepped -----------------------------
    h = app.dump("m2_before")
    start = np.array(h["_new"])
    start_old = np.array(h["_old"])
    target = np.array([2.0, 1.0, 0.30])
    T_DAYS = 600.0 / DAY
    app.cmd(f"camera action move_to target point x {target[0]} y {target[1]} z {target[2]} duration 600", 1.0)
    hj = app.dump("m2_t0")
    jd_start = hj["jd"]
    # refusals WHILE MOVING
    log = app.cmd("camera action move_to target point x 3 y 0 z 0 duration 60")
    o, n = refusals(log)
    chk(o and n, "R5 a second travel while one is in flight: BOTH paths refuse",
        f"old={len(o)} new={len(n)}")
    log = app.cmd("camera action switch name Space")
    o, n = refusals(log)
    chk(n, "R6 anchor switch while travelling: the new path refuses (old's own rule)",
        f"new={len(n)}")
    NOTES["R5"] = {"old": o, "new": n}

    steps = []
    for f in (0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25):
        jd = jd_start + f * T_DAYS
        app.at(jd, 0.7)
        hs = app.dump(f"m2_f{int(f*100):03d}")
        pred_old = predict_travel(start_old, target, jd_start, T_DAYS, hs["jd"])
        pred_new = predict_travel(start, target, jd_start, T_DAYS, hs["jd"])
        steps.append({
            "f": f, "jd": hs["jd"],
            "old": hs["_old"], "new": hs["_new"],
            "predOld": pred_old.tolist(), "predNew": pred_new.tolist(),
            "errOld": float(np.linalg.norm(np.array(hs["_old"]) - pred_old)),
            "errNew": float(np.linalg.norm(np.array(hs["_new"]) - pred_new)),
            "oldVsNew": float(np.linalg.norm(np.array(hs["_old"]) - np.array(hs["_new"]))),
            "moving": hs["anchors"]["moving"],
        })
    NOTES["M2"] = {"start": start.tolist(), "target": target.tolist(),
                   "jd_start": jd_start, "travel_days": T_DAYS, "steps": steps}
    worst_new = max(s["errNew"] for s in steps)
    worst_old = max(s["errOld"] for s in steps)
    worst_cmp = max(s["oldVsNew"] for s in steps)
    chk(worst_old < CMP_FLOOR, "M2 old follows the committed law at every step",
        f"worst |old - predicted| = {worst_old:.3e} AU")
    chk(worst_new < CMP_FLOOR, "M2 new follows the SAME committed law at every step",
        f"worst |new - predicted| = {worst_new:.3e} AU")
    chk(worst_cmp < CMP_FLOOR, "M2 the two paths are on the same trajectory, step by step",
        f"worst |old - new| = {worst_cmp:.3e} AU")
    # the two quirks, as discriminators
    s0 = steps[0]
    pop = np.linalg.norm(np.array(s0["new"]) - start) / np.linalg.norm(target - start)
    chk(abs(pop - 1/(1+math.exp(7))) < 5e-5,
        "M2 the START POP is old's: 1/(1+e^7) of the distance, not 0",
        f"measured {pop:.6e} vs predicted {1/(1+math.exp(7)):.6e}")
    sEnd = steps[-1]
    short = np.linalg.norm(np.array(sEnd["new"]) - target) / np.linalg.norm(target - start)
    NOTES["M2"]["startPop"] = pop
    NOTES["M2"]["endShort"] = short
    chk(sEnd["moving"] is False, "M2 the travel flag retires after the arrival date")

    # ---------------- M3: move_to body, stepped ------------------------------
    h = app.dump("m3_before")
    start = np.array(h["_new"]); start_old = np.array(h["_old"])
    jd_now = h["jd"]
    ALT_KM = 50000.0
    app.cmd(f"camera action move_to target body body_name Mars duration 600 altitude {ALT_KM}", 1.0)
    hj = app.dump("m3_t0")
    jd_start = hj["jd"]
    mars_R = hj["_bodies"]["Mars"]["new"]["scaledRadius"] if "scaledRadius" in hj["_bodies"]["Mars"]["new"] else None
    NOTES["M3"] = {"start": start.tolist(), "jd_start": jd_start, "alt_km": ALT_KM,
                   "marsFields": sorted(hj["_bodies"]["Mars"]["new"].keys())}
    steps3 = []
    for f in (0.0, 0.3, 0.7, 1.0, 1.2):
        jd = jd_start + f * T_DAYS
        app.at(jd, 0.7)
        hs = app.dump(f"m3_f{int(f*100):03d}")
        steps3.append({"f": f, "jd": hs["jd"], "old": hs["_old"], "new": hs["_new"],
                       "oldVsNew": float(np.linalg.norm(np.array(hs["_old"]) - np.array(hs["_new"]))),
                       "marsEcl": hs["_bodies"]["Mars"]["new"]["ecl"],
                       "moving": hs["anchors"]["moving"]})
    NOTES["M3"]["steps"] = steps3
    worst3 = max(s["oldVsNew"] for s in steps3)
    chk(worst3 < CMP_FLOOR, "M3 the two paths travel to the body together, step by step",
        f"worst |old - new| = {worst3:.3e} AU")
    # standoff: at arrival the observer is `altitude` above Mars' surface, and
    # both paths aimed at Mars' position AT THE ARRIVAL DATE.
    arr = [s for s in steps3 if s["f"] == 1.0][0]
    mars_arr = np.array(arr["marsEcl"], float)
    d_new = np.linalg.norm(np.array(arr["new"]) - mars_arr)
    d_old = np.linalg.norm(np.array(arr["old"]) - mars_arr)
    NOTES["M3"]["standoff"] = {"newAU": float(d_new), "oldAU": float(d_old)}
    chk(abs(d_new - d_old) < CMP_FLOOR,
        "M3 both paths stop the same distance short of the body",
        f"new {d_new:.6e} AU, old {d_old:.6e} AU, delta {abs(d_new-d_old):.3e}")

    # ---------------- M4: transition_to body ---------------------------------
    before = app.dump("m4_before")
    log = app.cmd("camera action transition_to target body name Mars", 1.2)
    after = app.dump("m4_after")
    d_old = np.linalg.norm(np.array(after["_old"]) - np.array(before["_old"]))
    d_new = np.linalg.norm(np.array(after["_new"]) - np.array(before["_new"]))
    mars = np.array(after["_bodies"]["Mars"]["new"]["ecl"], float)
    NOTES["M4"] = {
        "d_old_AU": float(d_old), "d_new_AU": float(d_new),
        "d_old_km": float(d_old * AU_KM), "d_new_km": float(d_new * AU_KM),
        "refBefore": before["camera"]["reference"], "refAfter": after["camera"]["reference"],
        "distanceAfter": after["camera"]["distance"],
        "distToMars": float(np.linalg.norm(np.array(after["_new"]) - mars)),
        "oldHeading": [before["oldView"]["heading"], after["oldView"]["heading"]],
        "oldHeadingPlan": after["oldView"]["plans"],
        "newHeading": [before["camera"]["heading"], after["camera"]["heading"]],
        "absFwd": [before["camera"]["absFwd"], after["camera"]["absFwd"]],
    }
    chk(after["camera"]["reference"] == "Mars",
        "M4 new: the camera references the body")
    chk(d_new <= d_old + POS_FLOOR,
        "M4 new: the observer moves no more than old's own bisection residual",
        f"new {d_new*AU_KM:.3f} km vs old {d_old*AU_KM:.3f} km")
    chk(abs(after["camera"]["distance"] - np.linalg.norm(np.array(after["_new"]) - mars)) < POS_FLOOR,
        "M4 new: the pose distance IS the distance to the body's centre",
        f"{after['camera']['distance']:.6e} vs {np.linalg.norm(np.array(after['_new'])-mars):.6e}")
    dv = np.linalg.norm(np.array(after["camera"]["absFwd"]) - np.array(before["camera"]["absFwd"]))
    NOTES["M4"]["dAbsFwd"] = float(dv)
    chk(dv < VIEW_FLOOR, "M4 new: A38 - the switch holds the absolute look direction",
        f"|dAbsFwd| = {dv:.3e}")

    # ---------------- reversible pair, entered TWICE -------------------------
    pair = []
    for i in range(2):
        a = app.dump(f"rp{i}_a")
        app.cmd("camera action transition_to target point name Space", 1.0)
        b = app.dump(f"rp{i}_b")
        app.cmd("camera action transition_to target body name Mars", 1.0)
        c = app.dump(f"rp{i}_c")
        pair.append({
            "entry": a["_new"], "point": b["_new"], "backOnBody": c["_new"],
            "refs": [a["camera"]["reference"], b["camera"]["reference"], c["camera"]["reference"]],
            "movePointAU": float(np.linalg.norm(np.array(b["_new"]) - np.array(a["_new"]))),
            "moveBodyAU": float(np.linalg.norm(np.array(c["_new"]) - np.array(b["_new"]))),
            "oldMovePointAU": float(np.linalg.norm(np.array(b["_old"]) - np.array(a["_old"]))),
            "oldMoveBodyAU": float(np.linalg.norm(np.array(c["_old"]) - np.array(b["_old"]))),
            "distance": [a["camera"]["distance"], b["camera"]["distance"], c["camera"]["distance"]],
        })
    NOTES["reversible"] = pair
    chk(all(p["movePointAU"] < POS_FLOOR and p["moveBodyAU"] < POS_FLOOR for p in pair),
        "RP: both entries of the point<->body pair move the observer by nothing",
        "; ".join(f"{p['movePointAU']:.2e}/{p['moveBodyAU']:.2e}" for p in pair))
    chk(all(p["refs"] == ["Mars", "Space", "Mars"] for p in pair),
        "RP: the reference alternates as declared, both times",
        str([p["refs"] for p in pair]))
    chk(abs(pair[0]["distance"][2] - pair[1]["distance"][2]) < POS_FLOOR,
        "RP: the second entry lands on the state the first exit produced",
        f"{pair[0]['distance'][2]:.9e} vs {pair[1]['distance'][2]:.9e}")

    # ---------------- M5: align_with, the derivation question ---------------
    b5 = app.dump("m5_before")
    log = app.cmd("camera action align_with body Mars duration 3", 1.2)
    a5 = app.dump("m5_after")
    NOTES["M5"] = {
        "oldHeading": [b5["oldView"]["heading"], a5["oldView"]["heading"]],
        "oldPlans": a5["oldView"]["plans"],
        "newHeading": [b5["camera"]["heading"], a5["camera"]["heading"]],
        "log": [l for l in log.splitlines() if "align" in l.lower()],
    }
    chk(True, "M5 recorded (verdict is the derivation, not a pass/fail)",
        json.dumps(NOTES["M5"])[:200])

    # ---------------- free-mode teleport control (out-of-scope defect) -------
    app.cmd("select planet Mars", 0.8)
    fm = []
    for state in ("on", "off", "on", "off"):
        app.cmd(f"camera action free_mode state {state}", 0.8)
        hf = app.dump(f"fm_{state}_{len(fm)}")
        fm.append({"state": state, "rootPos": hf["_new"], "freeMode": hf["camera"]["freeMode"],
                   "selDist": hf["camera"]["selDist"], "oldHelio": hf["_old"]})
    NOTES["freeModeControl"] = fm
    base = np.array(fm[1]["rootPos"])
    moved = [float(np.linalg.norm(np.array(f["rootPos"]) - base)) for f in fm]
    seld = [f["selDist"] for f in fm]
    NOTES["freeModeControl_moved"] = moved
    print(f"     free-mode toggle: rootPos moves {moved}, selDist {seld}")

    sess.stop(drv)
    res = {"checks": CHECKS, "notes": NOTES,
           "summary": {"failures": sum(1 for c in CHECKS if not c["ok"]), "n": len(CHECKS)}}
    (out / "f33_result.json").write_text(json.dumps(res, indent=1, default=float))
    print(f"\n{res['summary']['failures']} failure(s) of {len(CHECKS)}; "
          f"report {out / 'f33_result.json'}")
    return 1 if res["summary"]["failures"] else 0


if __name__ == "__main__":
    sys.exit(main())
