#!/usr/bin/env python3
"""F111 -- CATCHING THE ORBIT-LINE SAMPLER'S TRANSIENT IN THE ENGINE, at a time
rate where EVERY frame resamples.  INTENT Sec.5.150 / Sec.11.229(g) / Sec.11.239.

    cd claude/harness && DISPLAY=:2 python3 f111_rate.py <absOutdir> \
        --tag <name> --bin <path> [--stages 1e8,7e8] [--dumps 4]

WHY A RATE LEG AND NOT F107's PINNED ONE.  At a pinned clock the resample fires
ONCE (OrbitModule.cpp:132 never fires again at a frozen date) and the body is
re-evaluated every frame, so the perturbation is gone hundreds of frames before
a dump can arrive -- Sec.11.229(f3)'s null, which its own text says is not evidence
of absence.  At a rate where dJD per frame >= vis/180 the resample fires EVERY
frame, so the perturbation is the STEADY STATE and any dump carries it.

WHAT MAKES THE READING POSSIBLE.  `ModularBody::useNow` returns on its first
line for a body the walk evaluates (`!renderHidden`, ModularBody.cpp:457), so
"a dump is a use" does NOT re-converge a WALKED body: the dump reads the walk's
own position.  The clock is running, so two dumps are never at the same date and
a dump-to-dump comparison is unavailable; the comparison is therefore each
record against the MODEL's converged position at the record's OWN dumped
`lastJD` (f111_predict.score), whose floor on this corpus is 5.6e-09 AU.

THE ARMS, all inside ONE launch:
  c0,c1   rate 0, two dumps at TWO DATES -- the control that shows the
          comparison able to fail.
  s0      rate 0, flag ON, settled: F107's own arm, reproduced.
  <R>_off flag OFF at rate R, N dumps: the within-launch control.
  <R>_on  flag ON at rate R, N dumps: the catch.
  <R>_off2 flag OFF again at rate R, N dumps: the flag put back.
Screenshots are taken at the default camera and at a camera aimed so the
irregular satellites' orbits are on screen.

BOUNDARY.  Nothing is written outside <absOutdir> and its farm; the real
~/.spacecrafter is md5'd in and out and never opened for writing.
"""
import argparse
import hashlib
import json
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, build_farm, cam_fields, no_instance, JD   # noqa: E402
from f99_locguard import lock_state                                   # noqa: E402
import f105_dump                                                      # noqa: E402
import f107_model as M                                                # noqa: E402
import f111_predict as P                                              # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
THRESHOLD = 5.6e-7          # the pre-registered PERTURBED threshold, AU


def md5_home():
    return {n: hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
            for n in ("config.ini", "ssystem.ini")}


def take(app, tag):
    app.dump(tag, pause=0.6, keep=True)
    p = app.out / "dumps" / ("%s_%s.json" % (app.name, tag))
    h, b = f105_dump.parse(p)
    return h, b, p


def score(bodies, rate, fps, sampling):
    """Per record: ||ecl| - |model converged at its own lastJD||, and the
    perturbation the model predicts for that same date at this stage's rate."""
    out = {}
    for n, el, vis, branch, e in sampling:
        rec = bodies.get(n)
        if not rec or not rec.get("new"):
            continue
        new = rec["new"]
        jd, got = new.get("lastJD"), new.get("ecl")
        if jd is None or not got:
            continue
        off, dJD = P.stage_offset(vis, rate, fps) if vis > 0 else (P.NOMINAL_OFFSET, 0.)
        Mn = M.mean_anomaly(el, jd)
        rex, _ = P.radial(el, M.kepler_exact(el["e"], Mn))
        rpe, _ = P.radial(el, P.perturbed_E(el, Mn, off))
        out[n] = {"lastJD": jd, "resid": abs(M.norm(got) - rex),
                  "pred": abs(rpe - rex), "r": M.norm(got),
                  "every": bool(vis > 0 and ((rate is None) or
                                             (dJD >= vis / P.ORBIT_POINTS)))}
    return out


def arm(app, tag, n, rate, fps, sampling, res):
    """N dumps in one arm; the arm's statistic is the MAX resid per record
    (pre-registered in prediction.txt R2: one dump can land where dr/dE ~ 0)."""
    per, jds = {}, []
    for k in range(n):
        h, b, _ = take(app, "%s_%d" % (tag, k))
        jds.append(h.get("jd"))
        s = score(b, rate, fps, sampling)
        for name, v in s.items():
            cur = per.setdefault(name, dict(v))
            if v["resid"] > cur["resid"]:
                per[name] = dict(v)
            cur = per[name]
            cur["pred_max"] = max(cur.get("pred_max", 0.0), v["pred"])
    pert = sorted((n_ for n_, v in per.items() if v["resid"] >= THRESHOLD),
                  key=lambda x: -per[x]["resid"])
    res[tag] = {"jds": jds, "n_dumps": n, "perturbed": pert,
                "n_perturbed": len(pert), "n_scored": len(per),
                "every_frame": sorted(n_ for n_, v in per.items() if v["every"]),
                "worst": (pert[0] if pert else None),
                "worst_resid": (per[pert[0]]["resid"] if pert else 0.0),
                "per_body": {k: {kk: vv for kk, vv in v.items()}
                             for k, v in sorted(per.items())}}
    print("  %-14s %d dumps: %2d PERTURBED of %d scored%s"
          % (tag, n, len(pert), len(per),
             ("  worst %s %.4g AU" % (pert[0], per[pert[0]]["resid"]))
             if pert else ""), flush=True)
    return per


def old_half_resid(bodies, sampling):
    """F-1's live test: if the two paths shared ONE Orbit object the OLD half
    would carry the same transient (Body::compute_position -> positionAtTime ->
    iterativeLastE).  The old record's own position, scored the same way."""
    out = {}
    for n, el, vis, branch, e in sampling:
        rec = bodies.get(n)
        old = (rec or {}).get("old") or {}
        ecl = old.get("ecl") or old.get("eclipticPos")
        if not ecl:
            continue
        # the old half carries no per-body lastJD; use the header date's model
        out[n] = M.norm(ecl)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--stages", default="1e8,7e8")
    ap.add_argument("--dumps", type=int, default=4)
    ap.add_argument("--fps", type=float, default=P.FPS)
    ap.add_argument("--jd", type=float, default=JD)
    ap.add_argument("--jd2", type=float, default=JD + 1.0)
    ap.add_argument("--settle", type=float, default=2.0)
    a = ap.parse_args()

    out = Path(a.out).resolve() / a.tag
    out.mkdir(parents=True, exist_ok=True)
    sampling = [r for r in P.sampling_bodies() if r[2] > 0]

    res = {"tag": a.tag, "bin": a.bin, "stages": a.stages, "dumps": a.dumps,
           "threshold_AU": THRESHOLD, "fps_assumed": a.fps, "jd": a.jd,
           "bin_md5": hashlib.md5(open(a.bin, "rb").read()).hexdigest()[:8],
           "started": time.strftime("%Y-%m-%dT%H:%M:%S"),
           "sampling_bodies": [r[0] for r in sampling]}
    hits = no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)
    res["proc_before"], res["lock_before"] = hits, lock_state()
    res["md5_in"] = md5_home()

    farm = out / "farm"
    build_farm(farm)
    log = f105_dump.Log(farm / ".spacecrafter")
    app = App(a.bin, farm, out, a.tag)
    try:
        res["tcp_seconds"] = app.start()
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("meteors zhr 0")
        app.send("date jday %.9f" % a.jd, 1.5)
        app.settle_scale()

        # --- the control that shows the comparison able to fail --------------
        h, b, _ = take(app, "c0_pinned")
        res["camera"] = cam_fields(h)
        res["n_records"] = len(b)
        s_a = score(b, None, a.fps, sampling)
        app.send("date jday %.9f" % a.jd2, 1.5)
        h2, b2, _ = take(app, "c1_pinned_other_date")
        moved = [n for n in s_a
                 if abs(M.norm((b2.get(n) or {}).get("new", {}).get("ecl") or [0, 0, 0])
                        - s_a[n]["r"]) > 1e-9]
        res["ctl_two_dates"] = {"jd0": h.get("jd"), "jd1": h2.get("jd"),
                                "moved_of_scored": [len(moved), len(s_a)]}
        print("  control two dates: %d of %d sampling records moved"
              % (len(moved), len(s_a)), flush=True)
        app.send("date jday %.9f" % a.jd, 1.5)

        # --- F107's own arm, reproduced: pinned clock, flag on ---------------
        arm(app, "pinned_flag_off", 1, None, a.fps, sampling, res)
        app.send("flag satellites_orbits on", 2.0)
        arm(app, "pinned_flag_on_settled", 1, None, a.fps, sampling, res)
        shot_pinned_on = app.shot("shot_pinned_flag_on")
        app.send("flag satellites_orbits off", 1.0)
        shot_pinned_off = app.shot("shot_pinned_flag_off")
        res["shots_pinned"] = [str(shot_pinned_off), str(shot_pinned_on)]

        # --- the rate stages -------------------------------------------------
        for rs in a.stages.split(","):
            R = float(rs)
            tag = "R%g" % R
            app.send("timerate rate %.0f" % R, a.settle)
            h, b, _ = take(app, "%s_probe" % tag)
            res.setdefault("timeSpeed", {})[tag] = h.get("timeSpeed")
            print("  stage %s: header timeSpeed = %s (expected %.9g)"
                  % (tag, h.get("timeSpeed"), R / 86400.0), flush=True)
            arm(app, "%s_off" % tag, a.dumps, R, a.fps, sampling, res)
            app.send("flag satellites_orbits on", a.settle)
            per_on = arm(app, "%s_on" % tag, a.dumps, R, a.fps, sampling, res)
            h3, b3, _ = take(app, "%s_on_oldhalf" % tag)
            res.setdefault("old_half_norm", {})[tag] = old_half_resid(b3, sampling)
            app.send("flag satellites_orbits off", a.settle)
            arm(app, "%s_off2" % tag, a.dumps, R, a.fps, sampling, res)
            del per_on
        app.send("timerate rate 0", 1.0)
        res["log_lines"] = log.lines()
    finally:
        res["exit_code"] = app.stop()
        res["lock_after"] = lock_state()
        res["md5_out"] = md5_home()
        res["proc_after"] = no_instance()
        (out / "f111_rate.json").write_text(json.dumps(res, indent=1))

    if res["md5_in"] != res["md5_out"]:
        raise SystemExit("BOUNDARY BREACH: the real ~/.spacecrafter moved: %s -> %s"
                         % (res["md5_in"], res["md5_out"]))
    summary = {k: v for k, v in res.items()
               if k not in ("log_lines", "old_half_norm")
               and not (isinstance(v, dict) and "per_body" in v)}
    for k, v in res.items():
        if isinstance(v, dict) and "per_body" in v:
            summary[k] = {kk: vv for kk, vv in v.items() if kk != "per_body"}
    print(json.dumps(summary, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
