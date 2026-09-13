#!/usr/bin/env python3
"""F118 -- THE TRAIL WALKER'S TRANSIENT AS A FUNCTION OF THE DUMP'S FRAME
LATENCY k.  INTENT Sec.11.239(h) / Sec.5.150's rider / Sec.11.241.

    python3 claude/harness/f118_predict.py trail  [--jd J] [--stale S] [--frames K]
    python3 claude/harness/f118_predict.py jump   [--jd J] [--days N]  [--frames K]
    python3 claude/harness/f118_predict.py score  <legdir> [--threshold T]
    python3 claude/harness/f118_predict.py all

WHAT THE WALKER DOES, and why the observable is a DECAYING one.

`ModularBody::show()` calls `useNow()` FIRST (ModularBody.cpp:812), and for a
body whose renderHidden is still set that is the D8 barrier: 1 +
RESUME_EXTRA_ITERATIONS = 5 calls at the current date, i.e. 10 solver steps, so
the body's seed `iterativeLastE` (orbit.hpp:120) ends CONVERGED at the frame's
own date.  THEN `resumeModulesAfterHidden()` (:821) reaches
`TrailModule::resumeAfterHidden` (TrailModule.cpp:169), whose reconstruction
loop (:227-234) calls `orbit->positionAtTimevInVSOP87Coordinates(date,
sampleJD, tmp)` -- which is `positionAtTime` (orbit.cpp:466-468) -- and that
walks THE SAME SEED (orbit.cpp:582) to the LAST reconstructed sample's date,
`lastJD + missed*deltaTrail`.  So the walk does not perturb a converged
position; it perturbs the SEED the next evaluation starts from, and it drags
it BACKWARDS in time by

    staleness = (date - trail.lastJD) - missed*deltaTrail   in [0, deltaTrail)

which the dump reports directly as `new.lastJD - new.trail[0].headJD` (the
module writes the last reconstructed sample's date as its head, :235-238 and
TrailModule.cpp:331).

The body's NEXT evaluation is one call = ITERATIVE_STEPS_PER_CALL = 2 steps
(iterative_orbits.hpp:31) from that seed; the frame after it, 2 more from
wherever the first left off; and so on.  At a PINNED clock every one of those
calls asks for the same mean anomaly, so the error contracts by the branch's
own factor per step -- on the `e < 0.2` fixed-point branch (orbit.cpp:519-525)
that factor is `e*cos(E)`, i.e. the residual falls by about e^2 PER FRAME.
For Elara (e = 0.196) that is 0.0385 per frame: three frames take 8.9e-05 AU
below 5.6e-07.  THE WHOLE QUESTION OF THIS TASK IS THEREFORE k, the number of
frames between the unhide and the dump, and this model prints residual(k).

THE SIGN OF THE OFFSET.  Sec.11.239(h)'s slice (f111_predict.py:308) used
`off = +deltaTrail*n` and took a MAX over 360 anomalies, where the max is
sign-blind.  The seed the walker leaves is at an EARLIER date, so at a NAMED
date the offset is NEGATIVE: `M_seed = M_date - n*staleness`.  This model uses
the negative sign and says so; it is a correction of a convention, not of a
number (the maxima agree).

NOTHING HERE LAUNCHES ANYTHING.  The solvers are imported from f107_model (I2:
one home for the tree's own iteration text), never re-derived.
"""
import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f107_model import (                                          # noqa: E402
    ecc_anomaly, kepler_exact, position_at_E, mean_anomaly, norm, sub,
    position_comet as M_position_comet, iter_ell_call as M_iter_ell_call,
    by_name_sections as M_by_name_sections, elements as M_elements,
    DEFAULT_INI as M_DEFAULT_INI,
)
import f111_predict as P                                          # noqa: E402

STEPS = P.STEPS                    # ITERATIVE_STEPS_PER_CALL = 2
DELTA_TRAIL = P.DELTA_TRAIL        # TrailModule.hpp:193
RESUME_CALLS = P.RESUME_CALLS      # 1 + RESUME_EXTRA_ITERATIONS = 5
JD = 2461233.5                     # f96_offset.JD, F91's own frame
THRESHOLD = 5.6e-7                 # F111's pre-registered PERTURBED threshold, AU
AU_ARCSEC = 180.0 * 3600.0 / math.pi

TARGETS = ["Elara", "Leda", "Sinope", "Himalia", "Lysithea", "Thebe", "Neried"]


def chain(el, M, offset_M, frames):
    """The frame chain at a PINNED date: a seed converged at `M + offset_M`,
    then one solver CALL per frame at `M`, each of STEPS steps, the seed
    carried across calls exactly as `iterativeLastE` is (orbit.hpp:120).

    Returns [(k, err3d, errR)] for k = 1..frames, plus the exact position.
    """
    e = el["e"]
    Ee = kepler_exact(e, M)
    pe = position_at_E(el, Ee)
    re_ = norm(pe)
    lastE = kepler_exact(e, M + offset_M)
    rows = []
    for k in range(1, frames + 1):
        _, lastE = ecc_anomaly(e, M, lastE, STEPS)
        p = position_at_E(el, lastE)
        rows.append((k, norm(sub(p, pe)), abs(norm(p) - re_)))
    return rows, pe, re_


def rows_for(names=None):
    all_rows = {r[0]: r for r in P.sampling_bodies()}
    out = []
    for n in (names or TARGETS):
        r = all_rows.get(n)
        if r is None:
            out.append((n, None, None, "ABSENT", None))
            continue
        out.append(r)
    return out


def table(jd, offset_days, frames, title, note):
    """offset_days > 0 means the seed sits that many days IN THE PAST."""
    lines = [title, "", note, "",
             "jd = %.9f   seed is %.6f day(s) behind   frames k = 1..%d"
             % (jd, offset_days, frames), "",
             "%-11s %-10s %8s %11s %11s %s"
             % ("body", "branch", "e", "n_rad/day", "offset_rad",
                "  ".join("err3d(k=%d)" % k for k in range(1, frames + 1)))]
    data = {}
    for r in rows_for():
        n = r[0]
        if r[1] is None:
            lines.append("%-11s ABSENT from the census" % n)
            continue
        el, branch, e = r[1], r[3], r[4]
        M = mean_anomaly(el, jd)
        off = -offset_days * el["n"]
        rows, _pe, _re = chain(el, M, off, frames)
        data[n] = {"branch": branch, "e": e, "n": el["n"], "offset_rad": off,
                   "err3d": [x[1] for x in rows], "errR": [x[2] for x in rows]}
        lines.append("%-11s %-10s %8.4f %11.6f %11.5g %s"
                     % (n, branch, e, el["n"], off,
                        "  ".join("%11.5g" % x[1] for x in rows)))
    lines.append("")
    lines.append("%-11s %-10s %8s %11s %11s %s"
                 % ("body", "branch", "e", "n_rad/day", "offset_rad",
                    "  ".join(" errR(k=%d)" % k for k in range(1, frames + 1))))
    for n, d in data.items():
        lines.append("%-11s %-10s %8.4f %11.6f %11.5g %s"
                     % (n, d["branch"], d["e"], d["n"], d["offset_rad"],
                        "  ".join("%11.5g" % x for x in d["errR"])))
    lines.append("")
    lines.append("PERTURBED (>= %.1e AU) by k, on the RADIAL statistic F111 scores:"
                 % THRESHOLD)
    for n, d in data.items():
        ks = [str(k + 1) for k, v in enumerate(d["errR"]) if v >= THRESHOLD]
        lines.append("  %-11s %s" % (n, ",".join(ks) if ks else "(none)"))
    lines.append("PERTURBED (>= %.1e AU) by k, on the 3-D statistic:" % THRESHOLD)
    for n, d in data.items():
        ks = [str(k + 1) for k, v in enumerate(d["err3d"]) if v >= THRESHOLD]
        lines.append("  %-11s %s" % (n, ",".join(ks) if ks else "(none)"))
    return "\n".join(lines) + "\n", data


def fold(x):
    y = math.fmod(x, 2 * math.pi)
    return y + 2 * math.pi if y < 0.0 else y


def chain_comet(el, dt_target, dt_seed, frames):
    """The same frame chain for the COMET family: `EllCometOrbit::positionAtTime`
    (orbit.cpp:344-347) hands `JD - t0` to the persistent `IterativeEll orbit`
    member (orbit.hpp:219), whose (H, c, s) the trail walk leaves converged at
    the last reconstructed sample's date.  The update is Newton's
    (iterative_orbits.hpp:128-132), so the error is SQUARED per step, not
    multiplied by e -- which is why this family is predicted invisible and the
    `ell < 0.2` fixed-point one is not."""
    e = el["e"]
    Hs = kepler_exact(e, fold(el["n"] * dt_seed))
    state = (Hs, math.cos(Hs), math.sin(Hs))
    Ht = kepler_exact(e, fold(el["n"] * dt_target))
    pe = M_position_comet(el, Ht)
    re_ = norm(pe)
    rows = []
    for k in range(1, frames + 1):
        state, _ = M_iter_ell_call(el, state, dt_target, STEPS)
        p = M_position_comet(el, state[0])
        rows.append((k, norm(sub(p, pe)), abs(norm(p) - re_)))
    return rows, pe, re_


def trail_census(dump):
    """WHICH BODIES CAN RUN THE WALKER AT ALL -- read from the ENGINE's own
    dump, not deduced: `trailComponents` is dumped as an array and an empty one
    IS the finding "this body has no trail module" (ModularBody.cpp:1093-1106).

    The gate that produces it is `deduceBodyModuleList` (ModularBody.cpp:940-946):
    orbit_visualization_period > 0 AND **not isSatellite()** AND type != Arti.
    """
    import f105_dump
    _h, b = f105_dump.parse(dump)
    out = {}
    for n, v in b.items():
        new = v.get("new") or {}
        t = new.get("trail")
        if t:
            out[n] = {"trail": t[0], "relation": new.get("relation"),
                      "dist": new.get("dist"), "evalCount": new.get("evalCount")}
    return out


def cmd_corpus(a):
    """The corrected corpus, and residual(k) on it."""
    secs = M_by_name_sections(M_DEFAULT_INI)
    cen = trail_census(a.dump)
    lines = [cmd_corpus.__doc__.strip(), "",
             "census read from %s : %d bodies carry a TrailModule"
             % (a.dump, len(cen)), "",
             "%-11s %-16s %-7s %9s %11s %9s %s"
             % ("body", "coord_func", "family", "e", "n_rad/day", "relation",
                "  ".join("err3d(k=%d)" % k for k in range(1, a.frames + 1)))]
    rows = []
    for n in sorted(cen):
        sec = secs.get(n) or {}
        cf = (sec.get("coord_func") or "").strip()
        el = None
        try:
            el = M_elements(sec)
        except Exception:                                        # noqa: BLE001
            el = None
        if el is None:
            lines.append("%-11s %-16s %-7s %9s %11s %9s  no iterative seed "
                         "(SpecialOrbit / custom: no lastE, no (H,c,s))"
                         % (n, cf, "-", "-", "-", cen[n]["relation"]))
            continue
        fam = el["family"]
        if fam == "ell":
            Mt = mean_anomaly(el, a.jd)
            r, _pe, _re = chain(el, Mt, -a.stale * el["n"], a.frames)
        else:
            dt = a.jd - el["t0"]
            r, _pe, _re = chain_comet(el, dt, dt - a.stale, a.frames)
        rows.append((n, cf, fam, el["e"], el["n"], cen[n]["relation"], r))
    for n, cf, fam, e, nn, rel, r in sorted(rows, key=lambda x: -x[6][0][1]):
        lines.append("%-11s %-16s %-7s %9.4f %11.6g %9s %s"
                     % (n, cf, fam, e, nn, rel,
                        "  ".join("%11.5g" % x[1] for x in r)))
    lines.append("")
    lines.append("%-11s %-16s %-7s %9s %11s %9s %s"
                 % ("body", "coord_func", "family", "e", "n_rad/day", "relation",
                    "  ".join(" errR(k=%d)" % k for k in range(1, a.frames + 1))))
    for n, cf, fam, e, nn, rel, r in sorted(rows, key=lambda x: -x[6][0][2]):
        lines.append("%-11s %-16s %-7s %9.4f %11.6g %9s %s"
                     % (n, cf, fam, e, nn, rel,
                        "  ".join("%11.5g" % x[2] for x in r)))
    lines.append("")
    lines.append("MAX OVER 360 ANOMALIES at k=1 -- the number directly comparable")
    lines.append("to Sec.11.239(h)'s 8.9167e-05 AU, which is a max of the same shape.")
    lines.append("THE COMET FAMILY IS SPLIT IN TWO, and the split is the tree's own:")
    lines.append("`IterativeEll::operator()` FOLDS the mean anomaly into [0,2pi)")
    lines.append("(iterative_orbits.hpp:125-127) while `H` accumulates unfolded, so on")
    lines.append("the one cell per orbit where the fold WRAPS between the seed's date and")
    lines.append("the evaluation's, the seed is a whole turn away and two Newton steps")
    lines.append("do not recover.  `EllipticalOrbit::eccentricAnomaly` has NO fold")
    lines.append("(orbit.cpp:579-581), so the `ell` family cannot show this.  The wrap")
    lines.append("window is staleness/period of the orbit, printed per body.")
    lines.append("%-11s %-7s %13s %13s %13s %11s"
                 % ("body", "family", "err3d_max", "errR_max", "wrap_err3d",
                    "wrap_window"))
    for n, cf, fam, e, nn, rel, r in rows:
        el = M_elements(secs[n])
        d3m = drm = wrapm = 0.0
        for i in range(360):
            wrapped = False
            if fam == "ell":
                rr, _p, _q = chain(el, 2 * math.pi * i / 360.0,
                                   -a.stale * el["n"], 1)
            else:
                dt = (2 * math.pi * i / 360.0) / el["n"]
                # the fold wraps iff the target's folded M is below n*staleness
                wrapped = fold(el["n"] * dt) < abs(el["n"]) * a.stale
                rr, _p, _q = chain_comet(el, dt, dt - a.stale, 1)
            if wrapped:
                wrapm = max(wrapm, rr[0][1])
                continue
            d3m = max(d3m, rr[0][1])
            drm = max(drm, rr[0][2])
        win = (abs(el["n"]) * a.stale / (2 * math.pi)) if fam != "ell" else 0.0
        lines.append("%-11s %-7s %13.5g %13.5g %13.5g %11.3g"
                     % (n, fam, d3m, drm, wrapm, win))
    lines.append("")
    lines.append("PERTURBED (>= %.1e AU) by k, RADIAL:" % THRESHOLD)
    for n, cf, fam, e, nn, rel, r in rows:
        ks = [str(x[0]) for x in r if x[2] >= THRESHOLD]
        lines.append("  %-11s %s" % (n, ",".join(ks) if ks else "(none)"))
    lines.append("PERTURBED (>= %.1e AU) by k, 3-D:" % THRESHOLD)
    for n, cf, fam, e, nn, rel, r in rows:
        ks = [str(x[0]) for x in r if x[1] >= THRESHOLD]
        lines.append("  %-11s %s" % (n, ",".join(ks) if ks else "(none)"))
    return "\n".join(lines) + "\n"


def cmd_trail(a):
    txt, _ = table(
        a.jd, a.stale, a.frames,
        "P1 -- THE WALKER'S OWN TRANSIENT, residual(k) at a pinned clock",
        "seed converged at date - staleness by the reconstruction loop's last\n"
        "sample (TrailModule.cpp:227-238), then one call per frame at `date`.")
    return txt


def cmd_jump(a):
    txt, _ = table(
        a.jd, a.days, a.frames,
        "P2 -- THE NO-HIDE CONTROL: a bare `date jday` jump of N days",
        "no hide, so no barrier and no walk: the walked body meets the jump\n"
        "with a seed left at the OLD date and gets 2 steps per frame.  This is\n"
        "NOT a floor -- see the prediction file's C2.")
    return txt


def load_leg(legdir):
    """Every dump of a leg, in file order, parsed by f105_dump."""
    import f105_dump
    legdir = Path(legdir)
    out = []
    for p in sorted((legdir / "dumps").glob("*.json")):
        h, b = f105_dump.parse(p)
        out.append((p.name, h, b))
    return out


def score_record(el, rec, trail_head=None):
    """The measured residual of ONE dumped record against the model's converged
    position at the record's OWN dumped lastJD -- F111's scoring, plus the 3-D
    statistic (a rotation preserves |a-b|, so `rotate_to_vsop87` cancels)."""
    new = rec.get("new") or {}
    jd, ecl = new.get("lastJD"), new.get("ecl")
    if jd is None or not ecl:
        return None
    M = mean_anomaly(el, jd)
    Ee = kepler_exact(el["e"], M)
    pe = position_at_E(el, Ee)
    return {"lastJD": jd, "ecl": ecl, "r": norm(ecl),
            "errR": abs(norm(ecl) - norm(pe)),
            "evalCount": new.get("evalCount"),
            "trail": (new.get("trail") or [None])[0]}


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    t = sub.add_parser("trail")
    t.add_argument("--jd", type=float, default=JD)
    t.add_argument("--stale", type=float, default=0.99)
    t.add_argument("--frames", type=int, default=6)
    j = sub.add_parser("jump")
    j.add_argument("--jd", type=float, default=JD)
    j.add_argument("--days", type=float, default=30.99)
    j.add_argument("--frames", type=int, default=6)
    c = sub.add_parser("corpus")
    c.add_argument("dump")
    c.add_argument("--jd", type=float, default=JD)
    c.add_argument("--stale", type=float, default=0.99)
    c.add_argument("--frames", type=int, default=6)
    s = sub.add_parser("all")
    s.add_argument("--jd", type=float, default=JD)
    s.add_argument("--stale", type=float, default=0.99)
    s.add_argument("--days", type=float, default=30.99)
    s.add_argument("--frames", type=int, default=6)
    a = ap.parse_args()
    if a.cmd == "trail":
        sys.stdout.write(cmd_trail(a))
    elif a.cmd == "corpus":
        sys.stdout.write(cmd_corpus(a))
    elif a.cmd == "jump":
        sys.stdout.write(cmd_jump(a))
    else:
        sys.stdout.write(cmd_trail(a))
        sys.stdout.write("\n")
        sys.stdout.write(cmd_jump(a))
    return 0


if __name__ == "__main__":
    sys.exit(main())
