#!/usr/bin/env python3
"""F107 -- SCORE THE REPLAY ON THE TREE'S OWN TEXT, against the landed dump.

    ./f107_replay.py <absOutdir> --pre-src <root> [--post-src <root>]
                     [--calls 5] [--jd 2461233.5]

WHAT IT DOES.  `f107_model.py` predicts (an independent Python re-derivation);
this driver puts the ENGINE'S OWN `EllipticalOrbit::eccentricAnomaly` and
`positionAtE` -- sliced out of the given trees by `f104_solver.py` and compiled
with the tree's own `iterative_orbits.hpp`, so the constant is the tree's -- in
front of exactly the mean anomalies the mechanism says the engine would hand
them, and scores three things:

  1. slice vs model, per body and arm: the engine's text must agree with the
     re-derivation to double precision.  A disagreement is a finding about the
     MODEL (the tree wins), and it is printed, never absorbed.
  2. the slice's fifth iterate, converted to a position through the sliced
     `positionAtE`, against the LANDED DUMP's float32 `ecl` -- the claim that
     the first use of a parked body starts from a seed the CONSTRUCTOR left at
     JD 0 (ModularBody.cpp:121-128) stands or falls here.
  3. Eris's pre-vs-post displacement against F104's landed 2.029337245 AU.

WHO OWNS WHAT.  The DATES are the caller's: they come from the field file and
from the light-travel/`distance` coupling of the first use
(ModularBody.hpp:698-702, :961), neither of which lives in the solver.  The
ITERATION is the slice's.  Keeping the split explicit is what makes the
agreement in (1) evidence about the reading rather than about arithmetic.
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import f107_model as M                                            # noqa: E402
import f104_solver                                                # noqa: E402

ARMS = (("pre", 1, True), ("post", 2, True), ("mut", 1, False))


def build_slice(outdir, src):
    exe, md5, steps = f104_solver.build(str(outdir), str(src))
    return str(exe), md5, steps


def rows_for(secs, names, jd, obs, steps, ctor, calls):
    """One TSV row per body: the arm's own coupled dates, as mean anomalies
    (ell) or dt values (comet).  The model computes the dates; the slice
    iterates."""
    out = []
    detail = {}
    for n in names:
        el = M.elements(secs[n])
        if el is None:
            continue
        r = M.replay(el, jd, obs, steps, calls, ctor)
        detail[n] = (el, r)
        if el["family"] == "ell":
            ctor_col = ("%.17g" % r["ctor"]["M"]) if r["ctor"] else "-"
            cols = [n, "ell", "%.17g" % el["e"], "%.17g" % el["q"],
                    "%.17g" % el["Om"], "%.17g" % el["inc"], "%.17g" % el["w"],
                    ctor_col] + ["%.17g" % c["M"] for c in r["calls"]]
        else:
            ctor_col = ("%.17g" % (0.0 - el["t0"])) if r["ctor"] else "-"
            cols = [n, "comet", "%.17g" % el["e"], "%.17g" % el["q"],
                    "%.17g" % el["n"], ctor_col] \
                + ["%.17g" % (c["jd"] - el["t0"]) for c in r["calls"]]
        out.append("\t".join(cols))
    return out, detail


def run_slice(exe, tsv):
    r = subprocess.run([exe, "--replay", str(tsv)], capture_output=True,
                       text=True)
    if r.returncode != 0:
        raise SystemExit("slice --replay failed: %s%s" % (r.stdout, r.stderr))
    return r.stdout


def parse_replay(txt):
    """{(name, arm): [ {call, iterate, residual, pos} ... ]}"""
    out = {}
    for line in txt.splitlines():
        if line.startswith("#") or line.startswith("name\t") \
                or line.startswith("pos_slice="):
            continue
        c = line.split("\t")
        if len(c) < 8:
            continue
        num = lambda s: None if s == "-" else float(s)
        out.setdefault((c[0], c[1]), []).append(
            {"call": int(c[2]), "iterate": num(c[3]), "res": num(c[4]),
             "pos": [num(c[5]), num(c[6]), num(c[7])]})
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--pre-src", required=True)
    ap.add_argument("--post-src", default=str(HERE.parent.parent / "src"))
    ap.add_argument("--ini", default=str(M.DEFAULT_INI))
    ap.add_argument("--census", default=str(M.DEFAULT_CENSUS))
    ap.add_argument("--dump", default=str(M.DEFAULT_DUMP))
    ap.add_argument("--post-dump",
                    default=str(HERE / "artifacts" / "f104" / "leg_post"
                               / "pinned_p0_launch.json.gz"))
    ap.add_argument("--jd", type=float, default=M.LAUNCH_JD)
    ap.add_argument("--calls", type=int, default=M.RESUME_CALLS)
    a = ap.parse_args()

    out = Path(a.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    secs = M.by_name_sections(a.ini)
    census = json.load(open(a.census))
    names = M.parked_iterating(census)
    _, recs = M.load_dump(a.dump)
    _, recs_post = M.load_dump(a.post_dump)
    obs = recs["Earth"]["new"]["ecl"]

    slices = {}
    for label, src in (("pre", a.pre_src), ("post", a.post_src)):
        exe, md5, steps = build_slice(out / ("slice_" + label), src)
        slices[label] = {"exe": exe, "md5": md5, "steps": steps, "src": src}

    L = ["# F107 replay SCORED on the sliced engine text",
         "# pre  slice: %s   (%s)" % (slices["pre"]["md5"], a.pre_src),
         "#   %s" % slices["pre"]["steps"],
         "# post slice: %s   (%s)" % (slices["post"]["md5"], a.post_src),
         "#   %s" % slices["post"]["steps"],
         "# observer (Earth `ecl`, landed dump) = %r" % (obs,), ""]

    scored = {}
    worst_model = (0.0, "")
    for arm, steps, ctor in ARMS:
        # pre and mut run on the PRE slice (1 step/call), post on the post one
        label = "post" if arm == "post" else "pre"
        rows, detail = rows_for(secs, names, a.jd, obs, steps, ctor, a.calls)
        tsv = out / ("replay_%s.tsv" % arm)
        tsv.write_text("\n".join(rows) + "\n")
        txt = run_slice(slices[label]["exe"], tsv)
        (out / ("replay_%s.txt" % arm)).write_text(txt)
        got = parse_replay(txt)
        scored[arm] = (detail, got)
        L.append("== ARM %s (slice %s, %s, ctor %s)"
                 % (arm, label, slices[label]["steps"].split("steps_per_call=")[-1],
                    "at JD 0" if ctor else "SKIPPED"))
        L.append("%-12s %14s %14s %14s %14s" %
                 ("body", "res5 slice", "res5 model", "|dE| slice-model",
                  "|dpos| AU"))
        for n in names:
            el, r = detail[n]
            key = (n, "ctor" if ctor else "mut")
            rows_got = got.get(key, [])
            last = rows_got[-1] if rows_got else None
            if last is None:
                L.append("%-12s  (no slice output)" % n)
                continue
            mcall = r["calls"][-1]
            dE = abs(last["iterate"] - mcall["E"]) if el["family"] == "ell" \
                else abs(last["iterate"] - M.wrapE(mcall["E"]))
            dpos = (M.norm(M.sub(last["pos"], mcall["pos"]))
                    if el["family"] == "ell" else float("nan"))
            if el["family"] == "ell" and dpos == dpos:
                worst_model = max(worst_model, (dpos, n + "/" + arm))
            L.append("%-12s %14.6g %14.6g %14.6g %14.6g"
                     % (n, last["res"], mcall["res"], dE, dpos))
        L.append("")

    # (2) the landed bytes
    L.append("== THE LANDED DUMP, AGAINST THE SLICE (float32, the dump's width)")
    for arm, dumprecs, tag in (("pre", recs, "F104 leg_pre"),
                               ("post", recs_post, "F104 leg_post")):
        detail, got = scored[arm]
        for n in names:
            el, r = detail[n]
            if el["family"] != "ell":
                continue                       # perifocal basis: no component check
            rows_got = got.get((n, "ctor"), [])
            if not rows_got:
                continue
            p = [M.f32(x) for x in rows_got[-1]["pos"]]
            landed = (dumprecs.get(n, {}).get("new") or {}).get("ecl")
            if landed is None or secs[n].get("parent") != "Sun":
                continue                       # rotated frame: |.| only, below
            # BOTH SIDES ROUNDED THE SAME WAY.  The dump prints a float32 with
            # %.9g and JSON parses that decimal back as a DOUBLE, which is not
            # the float32; comparing the slice's float32 against that double
            # reports "differs" on values that are visibly identical (it did,
            # on this driver's first run).  Round the landed value to float32
            # too -- %.9g of a float32 round-trips exactly.
            same = all(p[i] == M.f32(landed[i]) for i in range(3))
            L.append("%-10s %-4s slice32 [%.9g, %.9g, %.9g]  landed [%.9g, %.9g, %.9g]"
                     "  %s (%s)" % (n, arm, p[0], p[1], p[2], landed[0],
                                    landed[1], landed[2],
                                    "IDENTICAL" if same else "DIFFERS", tag))
    L.append("")
    # (3) the displacement
    dpre = scored["pre"][1].get(("Eris", "ctor"))[-1]["pos"]
    dpost = scored["post"][1].get(("Eris", "ctor"))[-1]["pos"]
    dmut = scored["mut"][1].get(("Eris", "mut"))[-1]["pos"]
    L.append("== ERIS DISPLACEMENT from the sliced positionAtE")
    L.append("   |pre - post| = %.9f AU   (F104 landed eye-frame 2.029337245 AU)"
             % M.norm(M.sub(dpre, dpost)))
    L.append("   |mut - post| = %.9f AU   (0 = the mutant lands where the fix does)"
             % M.norm(M.sub(dmut, dpost)))
    L.append("")
    # (4) P4 on the SLICE: what the orbit-line sampler leaves behind.
    L.append("== P4, THE SAMPLER'S ONE-FRAME TRANSIENT, ON THE SLICED TEXT")
    L.append("# a `seed` row hands the slice the CONVERGED anomaly at the last")
    L.append("# sampled date (date + 89*visPeriod/180) and then ONE call at the")
    L.append("# body's own date -- 1 step on the pre slice, 2 on the post one.")
    L.append("# poserr is against the model's converged position (itself scored")
    L.append("# against the landed dump to <= 5.2e-08 relative in model_validate).")
    L.append("%-12s %-9s %6s %13s %13s %12s" %
             ("body", "branch", "steps", "res_rad", "poserr_AU", "ang_arcsec"))
    walked = M.walked_iterating(census)
    prows, pdet = [], {}
    for n in walked:
        sec = secs.get(n, {})
        el = M.elements(sec)
        if el is None or el["family"] != "ell":
            continue
        vis = M.stod(sec.get("orbit_visualization_period"), 0.0) or 0.0
        rec = recs.get(n)
        if vis <= 0.0 or rec is None:
            continue
        jd = rec["new"]["lastJD"]
        stale = jd + 89.0 * vis / M.ORBIT_POINTS
        E0 = M.kepler_exact(el["e"], M.mean_anomaly(el, stale))
        Mnow = M.mean_anomaly(el, jd)
        exact = M.kepler_exact(el["e"], Mnow)
        pdet[n] = (el, M.position_at_E(el, exact), rec["new"].get("dist") or 0.0,
                   census["rows"][n]["branch"], vis)
        prows.append("\t".join([n, "seed", "%.17g" % el["e"], "%.17g" % el["q"],
                                "%.17g" % el["Om"], "%.17g" % el["inc"],
                                "%.17g" % el["w"], "%.17g" % E0,
                                "%.17g" % Mnow]))
    tsv = out / "replay_sampler.tsv"
    tsv.write_text("\n".join(prows) + "\n")
    table = []
    for label in ("pre", "post"):
        txt = run_slice(slices[label]["exe"], tsv)
        (out / ("replay_sampler_%s.txt" % label)).write_text(txt)
        got = parse_replay(txt)
        steps = 1 if label == "pre" else 2
        for n, (el, epos, dist, branch, vis) in pdet.items():
            rr = got.get((n, "seed"), [])
            if len(rr) < 2:
                continue
            last = rr[-1]
            poserr = M.norm(M.sub(last["pos"], epos))
            import math as _m
            ang = (_m.degrees(_m.atan2(poserr, dist)) * 3600.0) if dist else float("nan")
            table.append((ang, n, branch, steps, last["res"], poserr))
    for ang, n, branch, steps, res, poserr in sorted(table, reverse=True)[:14]:
        L.append("%-12s %-9s %6d %13.6g %13.6g %12.4f"
                 % (n, branch, steps, res, poserr, ang))
    L.append("")
    L.append("worst slice-vs-model position disagreement: %.6g AU on %s"
             % worst_model)
    txt = "\n".join(L) + "\n"
    (out / "replay_score.txt").write_text(txt)
    print(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
