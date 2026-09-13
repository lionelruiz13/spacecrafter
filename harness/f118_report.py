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
    L.append("script: k measured per arm = %s"
             % {t: arms[t].get("k_by_evalcount", {}).get("Elara")
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
