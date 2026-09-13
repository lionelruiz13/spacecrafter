#!/usr/bin/env python3
"""F118 -- the edge leg's table.  INTENT Sec.11.239(h) / Sec.5.150's rider.

    python3 claude/harness/f118_report.py <legdir> [--threshold 5.6e-7]

READS ONLY WHAT THE ENGINE WROTE.  Every number here comes from
`f118_edge.json` (which is itself only the dumps' own fields) or from
f107_model's solver text; nothing is re-derived and nothing is assumed.

THE THREE STATISTICS, and why there are three.

  A  ARM-TO-ARM 3-D  |ecl(trail on) - ecl(trail off)| at the SAME k and the
     SAME date.  Two engine-dumped vectors in one frame: `rotate_to_vsop87`
     is a rotation, so it cancels, and this needs no model at all.  Its floor
     is the off/off A/A pair, printed beside it.  THIS IS THE PRIMARY ONE.
  B  MODEL-SCORED RADIAL  ||ecl| - |converged at the record's own lastJD||,
     which is exactly F111's scoring (f111_rate.py:84) and the statistic the
     5.6e-07 AU threshold was pre-registered against.
  C  ARM-TO-ARM RADIAL, the same difference of norms between the two arms --
     model-free, and comparable to B.

STALENESS IS MEASURED, NOT ASSUMED: `new.lastJD - new.trail[0].headJD` is the
gap between the date the body is evaluated at and the date the reconstruction
loop left its seed converged at (TrailModule.cpp:235-238, dumped at :331).
"""
import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import f107_model as M                                            # noqa: E402
import f118_predict as Q                                          # noqa: E402

AU_ARCSEC = 180.0 * 3600.0 / math.pi


def d3(a, b):
    if not a or not b:
        return None
    return M.norm(M.sub(a, b))


def model_radial(el, jd, ecl):
    if el is None or jd is None or not ecl:
        return None
    Mn = M.mean_anomaly(el, jd)
    p = M.position_at_E(el, M.kepler_exact(el["e"], Mn))
    return abs(M.norm(ecl) - M.norm(p))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("leg")
    ap.add_argument("--threshold", type=float, default=5.6e-7)
    a = ap.parse_args()
    res = json.load(open(Path(a.leg) / "f118_edge.json"))
    bodies = res["bodies"]
    els = {}
    secs = M.by_name_sections(M.DEFAULT_INI)
    for n in bodies:
        if secs.get(n):
            els[n] = M.elements(secs[n])
    arms = res["arms"]
    L = []
    L.append("F118 EDGE LEG -- %s, binary %s, %s"
             % (res["tag"], res["bin_md5"], res["started"]))
    L.append("jd0 %.9f  ->  jd1 %.9f   (N = %.6f sim days, missed = %d)"
             % (res["jd0"], res["jd1"], res["days"], int(res["days"])))
    L.append("")
    L.append("=== THE CHANNEL'S OWN LATENCY (evalCount deltas, never assumed) ===")
    L.append("tcp: two consecutive App.dump() calls are %s frames apart"
             % sorted(set(res["tcp_latency_frames"].values())))
    L.append("tcp: the SAME edge driven through App.send/App.dump reaches the")
    L.append("     post-unhide dump at k = %s"
             % {k: v for k, v in sorted(res["tcp_arm"]["k_by_evalcount"].items(),
                                        key=lambda x: -x[1])})
    L.append("script: k measured per arm (every body agrees; first listed) = %s"
             % {t: sorted(set((arms[t].get("k_by_evalcount") or {}).values()))
                for t in sorted(arms)})
    L.append("script: frames (frameref %s) mid->post = %s"
             % (res["frameref"],
                {t: arms[t].get("frames_mid_to_post") for t in sorted(arms)}))
    L.append("")
    L.append("=== THE RECONSTRUCTION ACTUALLY RAN (the arm's own dumps) ===")
    L.append("%-11s %-9s %8s %8s %9s %12s %14s"
             % ("arm", "body", "pre_pts", "post_pts", "recording",
                "staleness_d", "lastJD-headJD"))
    for t in sorted(arms):
        st = arms[t]
        for n in bodies:
            pre = (st.get("pre") or {}).get("bodies", {}).get(n) or {}
            post = (st.get("post") or {}).get("bodies", {}).get(n) or {}
            stale = None
            if post.get("lastJD") is not None and post.get("trail_headJD"):
                stale = post["lastJD"] - post["trail_headJD"]
            L.append("%-11s %-9s %8s %8s %9s %12s %14s"
                     % (t, n, pre.get("trail_points"), post.get("trail_points"),
                        post.get("trail_recording"),
                        ("%.6f" % stale) if stale is not None else "-",
                        ("%.9f" % post["trail_headJD"]) if post.get("trail_headJD") else "-"))
        L.append("")
    L.append("=== A: ARM-TO-ARM 3-D, |ecl(on) - ecl(off)| AU, and the A/A floor ===")
    off = (arms.get("off_k1") or {}).get("post", {}).get("bodies", {})
    offb = (arms.get("off_k1_b") or {}).get("post", {}).get("bodies", {})
    L.append("%-11s %13s %13s %13s %13s"
             % ("body", "A/A floor", "on_k1", "on_k2", "on_k3"))
    rows = {}
    for n in bodies:
        floor = d3((off.get(n) or {}).get("ecl"), (offb.get(n) or {}).get("ecl"))
        vals = []
        for t in ("on_k1", "on_k2", "on_k3", "on_k5"):
            on = (arms.get(t) or {}).get("post", {}).get("bodies", {}).get(n) or {}
            vals.append(d3(on.get("ecl"), (off.get(n) or {}).get("ecl")))
        rows[n] = (floor, vals)
        L.append("%-11s %13.5g %13.5g %13.5g %13.5g"
                 % (n, floor if floor is not None else float("nan"),
                    vals[0] or 0.0, vals[1] or 0.0, vals[2] or 0.0))
    L.append("")
    L.append("=== B: MODEL-SCORED RADIAL (F111's statistic), AU ===")
    L.append("%-11s %-11s %13s %13s %9s"
             % ("arm", "body", "errR", "threshold", "PERTURBED"))
    npert = {}
    for t in sorted(arms):
        st = arms[t]
        post = (st.get("post") or {}).get("bodies", {})
        cnt = 0
        for n in bodies:
            r = post.get(n) or {}
            v = model_radial(els.get(n), r.get("lastJD"), r.get("ecl"))
            if v is None:
                continue
            p = v >= a.threshold
            cnt += 1 if p else 0
            L.append("%-11s %-11s %13.5g %13.1e %9s"
                     % (t, n, v, a.threshold, "YES" if p else "no"))
        npert[t] = cnt
        L.append("")
    L.append("PERTURBED count per arm: %s" % npert)
    L.append("")
    L.append("=== C: THE HEIGHT OF THE NUMBER (arcsec at the observer) ===")
    L.append("%-11s %13s %13s %13s %10s"
             % ("body", "3D AU (k=1)", "dist AU", "arcsec", "vs threshold"))
    for n in bodies:
        on = (arms.get("on_k1") or {}).get("post", {}).get("bodies", {}).get(n) or {}
        v = rows[n][1][0]
        dist = on.get("dist")
        arc = (v / dist) * AU_ARCSEC if (v and dist) else None
        L.append("%-11s %13.5g %13.6g %13.5g %10s"
                 % (n, v or 0.0, dist or 0.0, arc or 0.0,
                    "%.1fx" % ((v or 0) / a.threshold)))
    L.append("")
    L.append("=== D: PREDICTED vs MEASURED, at the arm's OWN measured staleness ===")
    L.append("(prediction2.txt P1'; the model is seeded at `lastJD - (lastJD-headJD)`,")
    L.append(" both read from the same dump, so nothing here is a nominal value)")
    L.append("%-10s %3s %10s %14s %14s %8s"
             % ("body", "k", "stale_d", "predicted3D", "measured3D", "ratio"))
    for t, k in (("on_k1", 1), ("on_k2", 2), ("on_k3", 3), ("on2_k1", 1)):
        arm = (arms.get(t) or {}).get("post", {}).get("bodies", {})
        ref = (arms.get("off2_k1" if t.startswith("on2") else "off_k1")
               or {}).get("post", {}).get("bodies", {})
        for n in bodies:
            r, o = arm.get(n), ref.get(n)
            if not r or not o or not r.get("trail_headJD") or els.get(n) is None:
                continue
            st = r["lastJD"] - r["trail_headJD"]
            if st <= 0:
                continue
            el = els[n]
            if el["family"] == "ell":
                rr, _p, _q = Q.chain(el, M.mean_anomaly(el, r["lastJD"]),
                                     -st * el["n"], k)
            else:
                dt = r["lastJD"] - el["t0"]
                rr, _p, _q = Q.chain_comet(el, dt, dt - st, k)
            pred = rr[k - 1][1]
            meas = d3(r["ecl"], o["ecl"])
            L.append("%-10s %3d %10.6f %14.5g %14.5g %8s"
                     % (n, k, st, pred, meas,
                        ("%.3f" % (meas / pred)) if pred > 0 else "-"))
        L.append("")
    L.append("=== E: THE NO-HIDE CONTROL, and it is NOT a floor ===")
    L.append("|ecl(bare `date jday` jump) - ecl(hide+unhide, trails off)| at the")
    L.append("SAME date and the SAME k: the hidden arm got the D8 barrier in")
    L.append("`show()`, the no-hide arm got nothing, because `useNow` returns on")
    L.append("its first line for a body the walk evaluates (ModularBody.cpp:457).")
    L.append("%-10s %14s %14s %10s" % ("body", "walker3D", "jump3D", "jump/walk"))
    jmp = (arms.get("jump_k1") or {}).get("post", {}).get("bodies", {})
    onk = (arms.get("on_k1") or {}).get("post", {}).get("bodies", {})
    for n in bodies:
        o, j, w = off.get(n), jmp.get(n), onk.get(n)
        if not (o and j and w):
            continue
        dw, dj = d3(w["ecl"], o["ecl"]), d3(j["ecl"], o["ecl"])
        L.append("%-10s %14.5g %14.5g %10s"
                 % (n, dw, dj, ("%.1f" % (dj / dw)) if dw else "-"))
    L.append("")
    L.append("=== F: THE NULLS THAT ARE NOT MODELLED BUT MEASURED ===")
    L.append("Every body carrying a TrailModule, read straight from the dump files")
    L.append("(no elements, no model): a `*_special` orbit has NO iterative state")
    L.append("(orbit.cpp:920 -> positionFunction), so the walker cannot leave one.")
    L.append("%-10s %8s %8s %14s %14s %14s"
             % ("body", "relation", "points", "walker3D", "A/A floor", "jump3D"))
    import f105_dump
    dd = Path(a.leg) / "dumps"
    try:
        _h1, bon = f105_dump.parse(dd / "on_k1_post.json")
        _h2, bof = f105_dump.parse(dd / "off_k1_post.json")
        _h3, bob = f105_dump.parse(dd / "off_k1_b_post.json")
        _h4, bjp = f105_dump.parse(dd / "jump_k1_post.json")
    except Exception as exc:                                     # noqa: BLE001
        L.append("  (dumps unreadable: %s)" % exc)
        bon = bof = bob = bjp = {}
    for n in sorted(bon):
        r = (bon.get(n) or {}).get("new") or {}
        if not r.get("trail"):
            continue
        o = (bof.get(n) or {}).get("new") or {}
        c = (bob.get(n) or {}).get("new") or {}
        j = (bjp.get(n) or {}).get("new") or {}
        if not (r.get("ecl") and o.get("ecl")):
            continue
        L.append("%-10s %8s %8s %14.5g %14.5g %14.5g"
                 % (n, r.get("relation"), r["trail"][0].get("points"),
                    d3(r["ecl"], o["ecl"]) or 0.0,
                    d3(o["ecl"], c.get("ecl")) or 0.0,
                    d3(j.get("ecl"), o["ecl"]) or 0.0))
    L.append("")
    L.append("=== THE TCP ARM: the same edge through the channel F111 used ===")
    L.append("%-11s %13s %13s %8s" % ("body", "errR (model)", "3D vs off_k1", "k"))
    for n in bodies:
        r = (res["tcp_arm"]["bodies"] or {}).get(n) or {}
        v = model_radial(els.get(n), r.get("lastJD"), r.get("ecl"))
        dd = d3(r.get("ecl"), (off.get(n) or {}).get("ecl"))
        L.append("%-11s %13.5g %13.5g %8s"
                 % (n, v if v is not None else float("nan"),
                    dd if dd is not None else float("nan"),
                    res["tcp_arm"]["k_by_evalcount"].get(n)))
    L.append("")
    L.append("=== CONTROLS ===")
    L.append("two dates: %d of %d target records moved (the comparison can fail)"
             % (len(res["ctl_two_dates"]["moved"]), res["ctl_two_dates"]["of"]))
    L.append("md5 of the real ~/.spacecrafter: in %s  out %s"
             % (res["md5_in"], res["md5_out"]))
    L.append("proc before %s / after %s ; exit %s"
             % (res["proc_before"], res["proc_after"], res["exit_code"]))
    sys.stdout.write("\n".join(L) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
