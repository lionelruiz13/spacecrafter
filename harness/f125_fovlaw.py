#!/usr/bin/env python3
"""F125 - is the fov seam red FOR THE PREDICTED REASON? (INTENT 11.263, 11.251(b), 11.250(d)(2))

    f125_fovlaw.py <final.json[.gz]> [--expect two-laws]

Input: a dual dump carrying the F121 per-frame seam ring (`f121_run.sh <label> f121_drive.py`
with F121_LEG=census). Since the per-frame follow was deleted, every zoom COMMAND gives each
engine its own plan, so the fov channel is off its floor mid-ramp. That is "predicted red"
only if the cause is the two LAWS and nothing else. The laws are written here from the
source, never fitted to a run:

  old  (projector.cpp, Projector::updateAutoZoom): fov = start + (aim - start) * c,
       c = 1 - (1 - k)^3 when start > aim, k^3 otherwise; k += dt_ms / (D * 1000) AFTER use
  new  (Camera.hpp calculateZoomCoef, Camera.cpp Camera::update): timer += dt_s, then
       half = src * (dst / src)^e, e = 2 u^2 (u <= 0.5) else 1 - 2 (1 - u)^2, u = timer / D

Each zoom plan found in the ring is replayed on the run's OWN dt column and judged:
  law_old / law_new   each engine follows ITS law, max residual <= LAW_TOL deg
  RESID_separation    the recorded mid-ramp separation equals the replayed one (the predicted
                      residual: GREEN = it is there with its cause; it goes RED when one
                      engine owns the fov (B42) - the expected sign of the fix, not a regression)
  rest_at_floor       once both plans ended the engines agree within REST_TOL (a key step on
                      a plan would leave them apart AT REST: INTENT 5.166 - another cause)
Exit 0 all PASS, 1 any FAIL, 2 no plan found.
"""
import gzip, json, math, os, sys
import numpy as np

LAW_TOL = 0.5      # deg; one frame of either law at its steepest is about 1 deg at 60 fps
SEP_TOL = 0.5      # deg
REST_TOL = 1e-4    # deg; the channel's float32 floor is 5.0e-06 (INTENT 11.245(d))
FAILS = []


def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok:
        FAILS.append(name)


def load(path):
    if not path.endswith(".gz") and not os.path.exists(path):
        path += ".gz"
    op = gzip.open if path.endswith(".gz") else open
    with op(path, "rt") as f:
        head = json.loads(f.readline())
    return head["seam"]["steps"]


def full_new(s):
    return s["halfFovNew"] * 360.0 / math.pi


def plans(steps):
    """Maximal runs of frames where either engine has a zoom plan in flight."""
    act = [(s["zoomTNew"] > 0) or abs(s["aimFovOld"] - s["fovOld"]) > 1e-9 for s in steps]
    out, i, n = [], 1, len(steps)
    while i < n:
        if act[i]:
            a = i
            while i < n and (act[i] or (i + 2 < n and (act[i + 1] or act[i + 2]))):
                i += 1
            out.append((a, i - 1))
        i += 1
    return out


def replay_old(steps, a, b, start, aim, dur, off):
    # `speed` and `coef` are float in projector.hpp's auto_zoom: a plan whose dt sum lands
    # exactly on its duration ends one frame later than double arithmetic says.
    f32 = np.float32
    sim, coef, live, speed = {}, f32(0), True, f32(1) / f32(dur * 1000)
    for j in range(a + off, b + 1):
        if live:
            c = f32(1) - (f32(1) - coef) ** 3 if start > aim else coef ** 3
            fov = start + (aim - start) * float(c)
            coef = f32(coef + speed * f32(steps[j]["dt"]))
            if coef >= 1.0:
                live, fov = False, aim
        else:
            fov = aim
        sim[j] = fov
    return sim


def replay_new(steps, a, b, src, dst, dur, off):
    sim, timer = {}, 0.0
    for j in range(a + off, b + 1):
        timer += steps[j]["dt"] / 1000.0
        if timer > dur:
            half = dst
        else:
            u = timer / dur
            e = 2 * u * u if u <= 0.5 else 1 - 2 * (1 - u) ** 2
            half = src * (dst / src) ** e
        sim[j] = half * 360.0 / math.pi
    return sim


def best(steps, a, b, rec, make):
    """The command lands inside a frame: try the plan's first frame at a-1 .. a+2."""
    found = None
    for off in (-1, 0, 1, 2):
        if a + off < 1:
            continue
        sim = make(off)
        res = max(abs(sim[j] - rec(steps[j])) for j in sim)
        if found is None or res < found[0]:
            found = (res, off, sim)
    return found


def main():
    args = [x for x in sys.argv[1:] if not x.startswith("--")]
    if not args:
        raise SystemExit(__doc__)
    steps = load(args[0])
    found = plans(steps)
    print("== %s: %d frames, %d zoom plan(s) ==" % (args[0], len(steps), len(found)), flush=True)
    if not found:
        print("no zoom plan in the ring - nothing judged")
        return 2
    for n, (a, b) in enumerate(found):
        seg = steps[a:b + 1]
        dur = max(s["zoomTNew"] for s in seg)
        start_old = steps[a - 1]["fovOld"]
        old_planned = any(abs(s["aimFovOld"] - s["fovOld"]) > 1e-9 for s in seg)
        aim_old = seg[0]["aimFovOld"] if old_planned else seg[0]["fovOld"]
        newseg = [s for s in seg if s["zoomTNew"] > 0]
        tag = "plan%d[f%d-%d]" % (n, steps[a]["frame"], steps[b]["frame"])
        if not newseg or dur <= 0:
            check(tag + "_new_has_a_plan", False, "the Camera ran no plan on this window")
            continue
        src, dst = newseg[0]["zoomSrcNew"], newseg[0]["zoomDstNew"]
        print("-- %s: old %.4f -> %.4f deg (%s), new %.4f -> %.4f deg over %.3f s, %d frames"
              % (tag, start_old, aim_old, "cubic plan" if old_planned else "SNAP",
                 src * 360 / math.pi, dst * 360 / math.pi, dur, len(seg)), flush=True)
        if old_planned:
            ro, oo, so = best(steps, a, b, lambda s: s["fovOld"],
                              lambda off: replay_old(steps, a, b, start_old, aim_old, dur, off))
        else:
            so = {j: aim_old for j in range(a, b + 1)}
            ro, oo = max(abs(steps[j]["fovOld"] - aim_old) for j in so), 0
        rn, on, sn = best(steps, a, b, full_new,
                          lambda off: replay_new(steps, a, b, src, dst, dur, off))
        check(tag + "_law_old", ro <= LAW_TOL,
              "projector against its %s: max residual %.4f deg (first frame a%+d)"
              % ("one-sided cubic" if old_planned else "snap", ro, oo))
        check(tag + "_law_new", rn <= LAW_TOL,
              "Camera against its geometric two-phase law: max residual %.4f deg (first frame a%+d)"
              % (rn, on))
        common = sorted(set(so) & set(sn))
        sep_rec = max(abs(steps[j]["fovOld"] - full_new(steps[j])) for j in common)
        sep_sim = max(abs(so[j] - sn[j]) for j in common)
        check("RESID_" + tag + "_separation", abs(sep_rec - sep_sim) <= SEP_TOL and sep_rec > 1.0,
              "recorded max |old - new| %.4f deg, the two laws replayed on this run's dt give %.4f"
              % (sep_rec, sep_sim))
        tail = steps[b + 1:b + 31]
        if tail:
            rest = max(abs(s["fovOld"] - full_new(s)) for s in tail)
            check(tag + "_rest_at_floor", rest <= REST_TOL,
                  "30 frames after both plans ended: max |old - new| %.3e deg" % rest)
    print("\n%s" % ("OK" if not FAILS else "%d FAILS: %s" % (len(FAILS), ", ".join(FAILS))))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
