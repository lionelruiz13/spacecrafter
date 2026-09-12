#!/usr/bin/env python3
"""F111 -- THE PREDICTION INSTRUMENT for Sec.5.150's fix: what changes when
`OrbitModule::sampleOrbit` stops walking the POSITION solver's seed and calls
the old path's batch pair instead.  INTENT Sec.5.150 / Sec.11.229(g)(h2) / Sec.11.239.

WHAT IS MODELLED, AND WHERE EACH FACT COMES FROM.

  PRE-FIX (the tree at code f3316fea), `OrbitModule.cpp:105-110`:
      for d = 0..179:  calc_date = date + (d-90)*vis/180
                       orbit->positionAtTimevInVSOP87Coordinates(date, calc_date, p[d])
  and `EllipticalOrbit::positionAtTimevInVSOP87Coordinates` (orbit.cpp:466-473)
  runs `positionAtTime` (:577-585) -> `eccentricAnomaly(M, iterativeLastE)`
  (:582) -- the POSITION solver's own `mutable` seed (orbit.hpp:120), advancing
  ITERATIVE_STEPS_PER_CALL = 2 Newton steps per call (iterative_orbits.hpp:31).
  So the 180 points walk `iterativeLastE` from d=0 (date - 90*incr) up to
  d=179 (date + 89*incr) and LEAVE it there.

  POST-FIX, the old plot's own shape (`orbit_plot.cpp:142/:174` + `:153/:167/:182`):
      orbit->prepairFastPositionAtTimevInVSOP87Coordinates(date, incr)   # batchLastE = 0
      for d = 0..179:  orbit->fastPositionAtTimevInVSOP87Coordinates(date, calc_date, p[d])
  and `EllipticalOrbit::fastPositionAtTimevInVSOP87Coordinates` (orbit.cpp:447-463)
  runs TEN warm-up calls when `batchLastE == 0` (:453-456) then one scoring call
  (:458), every one of them through `batchLastE` (orbit.hpp:122).  Each call is
  ITERATIVE_STEPS_PER_CALL steps, so the FIRST point gets 11*2 = 22 Newton steps
  and every later point 2, and `iterativeLastE` is never touched.

  THE OBSERVABLE THE ENGINE CAN SHOW.  The transient lives in the ONE frame
  after a resample (Sec.11.229(g)).  At a time rate where every frame advances at
  least `vis/180` of simulation time, EVERY frame resamples, so the perturbation
  is the steady state and any dump carries it.  The seed the body's own next
  evaluation then starts from is E(date_n + 89*incr) and the date it is asked
  for is date_n+1 = date_n + dJD, i.e. an offset of

      offset_M = (89*incr - dJD) * n          [rad of mean anomaly]

  which is 88*2pi/180 = 3.0717 rad at dJD = incr (the threshold) and
  89*2pi/180 = 3.1067 rad at dJD = 0 (the flag-on resample at rate 1).

  WHAT THE DUMP CAN COMPARE.  `ecl` is the body's parent-relative position and
  for a moon the frame carries the parent's rotation (Sec.11.229(i3)), so the
  available comparison is |pos| -- rotation invariant.  The model's own floor on
  this corpus is <= 5.2e-08 RELATIVE / 5.6e-09 AU on every walked record
  (f107_model.py validate against F104's landed dump).

usage
  ./f111_predict.py transient   # P1/P3: the steady-state perturbation, per body
  ./f111_predict.py rate        # P2: the every-frame-resample rate, per body
  ./f111_predict.py line        # P5: max |delta orbitPoint| pre vs post, per body
  ./f111_predict.py trail       # P6: the trail walker's own perturbation
  ./f111_predict.py score DUMP  # score a landed dump against the model
  ./f111_predict.py all         # every table above, one file
"""
import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from f107_model import (                                   # noqa: E402
    by_name_sections, elements, ecc_anomaly, kepler_exact, position_at_E,
    mean_anomaly, norm, sub, load_dump, walked_iterating, DEFAULT_INI,
    DEFAULT_CENSUS, DEFAULT_DUMP, ORBIT_POINTS,
)

# --- the constants this model reads from the tree, each with its site ---------
STEPS = 2            # ITERATIVE_STEPS_PER_CALL, iterative_orbits.hpp:31
WARMUP_CALLS = 10    # orbit.cpp:454 `for (size_t i = 0; i < 10; ++i)`
FPS = 144.0          # config.ini maximum_fps, measured [144.0,146.4] Sec.11.159(k7)
SEC_PER_DAY = 86400.0
LAST_SAMPLE = ORBIT_POINTS // 2 - 1   # d = 179 - 90 = +89 increments
DELTA_TRAIL = 1.0    # TrailModule.hpp:193 `double deltaTrail = 1;` (sim days)
RESUME_CALLS = 5     # 1 + RESUME_EXTRA_ITERATIONS, ModularBody.hpp:338


def sampling_bodies(ini=None, census=None):
    """The walked ITERATING records that actually resample: vis > 0.

    `re.sidereal_period` is `orbit_visualization_period` (ModularSystem.cpp:1209);
    absent => 0 => OrbitModule::update takes its `still orbit` branch (:134-136).
    """
    secs = by_name_sections(ini or DEFAULT_INI)
    cen = json.load(open(census or DEFAULT_CENSUS))
    out = []
    for n in walked_iterating(cen):
        sec = secs.get(n)
        el = elements(sec) if sec else None
        if el is None:
            continue
        vis = 0.0
        raw = sec.get("orbit_visualization_period")
        if raw:
            try:
                vis = float(raw.replace(",", "."))
            except ValueError:
                vis = 0.0
        out.append((n, el, vis, cen["rows"][n]["branch"], cen["rows"][n]["e"]))
    return sorted(out)


def perturbed_E(el, M, offset_M, steps=STEPS):
    """One call of the solver at mean anomaly M, seeded at the converged E of
    (M + offset_M) -- the seed the sampler leaves behind."""
    seed = kepler_exact(el["e"], M + offset_M)
    _, E = ecc_anomaly(el["e"], M, seed, steps)
    return E


def radial(el, E):
    p = position_at_E(el, E)
    return norm(p), p


NOMINAL_OFFSET = LAST_SAMPLE * 2 * math.pi / ORBIT_POINTS   # dJD = 0, i.e. rate 1


def stage_offset(vis, rate, fps):
    """The seed offset in mean anomaly at a running clock.

    The sampler leaves the seed at E(date_n + 89*incr); the next frame asks for
    date_n+1 = date_n + dJD.  offset = (89*incr - dJD) * n with n = 2pi/vis and
    incr = vis/180, i.e. 2pi*(89/180 - dJD/vis), and dJD = rate/(86400*fps).
    """
    if rate is None:
        return NOMINAL_OFFSET, 0.0
    dJD = rate / (SEC_PER_DAY * fps)
    return 2.0 * math.pi * (LAST_SAMPLE / ORBIT_POINTS - dJD / vis), dJD


def transient_rows(samples=360, steps=STEPS, rate=None, fps=FPS):
    """P1/P3: the perturbation over a whole orbit, because the magnitude is a
    function of WHERE the body is and a leg at a running clock sweeps all of it.

    Returns per body both the 3D position error and the RADIAL error -- the
    radial one is what a |ecl| comparison against the model can see -- plus
    whether the stage's rate makes this body resample EVERY frame.
    """
    rows = []
    for n, el, vis, branch, e in sampling_bodies():
        if vis <= 0.0:
            continue
        off, dJD = stage_offset(vis, rate, fps)
        every = (rate is None) or (dJD >= vis / ORBIT_POINTS)
        d3, dr = [], []
        for k in range(samples):
            M = 2.0 * math.pi * k / samples
            Ee = kepler_exact(el["e"], M)
            Ep = perturbed_E(el, M, off, steps)
            re_, pe = radial(el, Ee)
            rp_, pp = radial(el, Ep)
            d3.append(norm(sub(pp, pe)))
            dr.append(abs(rp_ - re_))
        d3.sort()
        dr.sort()
        q = lambda v, f: v[min(len(v) - 1, int(f * len(v)))]        # noqa: E731
        rows.append((n, branch, e, vis, off, every,
                     q(d3, 0.50), d3[-1],
                     q(dr, 0.10), q(dr, 0.50), q(dr, 0.90), dr[-1]))
    rows.sort(key=lambda r: -r[9])
    return rows


def cmd_transient(a):
    rate = getattr(a, "rate", None)
    fps = getattr(a, "fps", FPS) or FPS
    lines = [
        "P1/P3  THE STEADY-STATE PERTURBATION AT AN EVERY-FRAME-RESAMPLE RATE",
        "       seed = converged E at M + offset; then ONE call of %d Newton" % STEPS,
        "       steps at the body's own M.  offset = 2pi*(89/180 - dJD/vis),",
        "       dJD = rate/(86400*fps); at rate 1 (dJD ~ 0) it is %.6f rad."
        % NOMINAL_OFFSET,
        "       stage: rate = %s, fps = %g" % (rate if rate else "1 (nominal)", fps),
        "       Distribution over 360 mean anomalies -- a leg at a running clock",
        "       sweeps the orbit, so a single number would not be a prediction.",
        "       err3d = |pos - pos_exact| AU;  errR = ||pos| - |pos_exact|| AU,",
        "       and errR is what a |ecl| comparison against the model measures.",
        "       MODEL FLOOR on this corpus: 5.6e-09 AU / 5.2e-08 relative.",
        "       every = does this rate resample this body EVERY frame?",
        "",
        "%-12s %-10s %7s %10s %9s %5s %11s %11s %11s %11s" %
        ("body", "branch", "e", "vis_d", "offs_rad", "every",
         "err3d_p50", "errR_p10", "errR_p50", "errR_max"),
    ]
    for r in transient_rows(rate=rate, fps=fps):
        lines.append("%-12s %-10s %7.4f %10.3f %9.4f %5s %11.4g %11.4g %11.4g %11.4g"
                     % (r[0], r[1], r[2], r[3], r[4], "yes" if r[5] else "no",
                        r[6], r[8], r[9], r[11]))
    return "\n".join(lines) + "\n"


def cmd_rate(a):
    """P2: the rate at which every frame resamples."""
    head = (
        "P2  THE EVERY-FRAME-RESAMPLE RATE, PER BODY\n"
        "    `timerate rate R` sets time_speed = R * JD_SECOND\n"
        "    (app_command_interface.cpp:3479) and TimeMgr::update adds\n"
        "    time_speed * dt_seconds to JDay (time_mgr.cpp:42).\n"
        "    `TimeMgr::setTimeSpeed` (time_mgr.hpp:53-55) stores the value with\n"
        "    NO clamp, so nothing in the engine bounds the rate from above.\n"
        "    Per frame dJD = R / (86400 * fps), and the resample gate\n"
        "    (OrbitModule.cpp:132, |date - lastSampleJD| >= vis/180) fires every\n"
        "    frame at\n\n"
        "        R >= 86400 * fps * vis / 180 = %d * vis   [sim seconds per wall second]\n\n"
        "    at the config cap fps = 144 (config.ini:28).  A SLOWER frame rate\n"
        "    makes dJD LARGER, so this is the conservative (upper) bound."
        % int(SEC_PER_DAY * FPS / ORBIT_POINTS))
    k = SEC_PER_DAY * FPS / ORBIT_POINTS
    lines = [head, "",
             "%-12s %10s %12s %14s %14s" %
             ("body", "vis_d", "incr_d", "rate_needed", "sim_days/wall_s")]
    rows = [(n, vis) for n, el, vis, br, e in sampling_bodies() if vis > 0]
    for n, vis in sorted(rows, key=lambda x: x[1]):
        lines.append("%-12s %10.3f %12.5f %14.4g %14.4g"
                     % (n, vis, vis / ORBIT_POINTS, k * vis, k * vis / SEC_PER_DAY))
    return "\n".join(lines) + "\n"


def line_rows(samples=24):
    """P5: |orbitPoint[d]_pre - orbitPoint[d]_post| for the 180 sampled points.

    PRE  : seed = iterativeLastE as the frame leaves it.  Two cases are modelled:
           (A) rate 1, the flag has just come on -- the seed is the CONVERGED E
               at the body's own date;
           (B) the every-frame-resample steady state -- the seed is the
               PERTURBED E the previous frame's sampler left (offset 89 incr).
           Then 180 calls of 2 steps, ascending d.
    POST : batchLastE = 0 at the prepair (orbit.cpp:442), then d=0 gets
           10 warm-up calls + 1 scoring call = 22 steps, d>0 get 2 steps.
    EXACT: the converged line (what both should approximate).
    """
    off = LAST_SAMPLE * 2 * math.pi / ORBIT_POINTS
    out = []
    for n, el, vis, branch, e in sampling_bodies():
        if vis <= 0.0:
            continue
        incrM = 2.0 * math.pi / ORBIT_POINTS       # one increment in mean anomaly
        worstA = worstB = worstPostErr = worstPreErrA = 0.0
        argA = argB = -1
        for k in range(samples):
            M0 = 2.0 * math.pi * k / samples       # the body's own mean anomaly
            for case in ("A", "B"):
                seed = (kepler_exact(el["e"], M0) if case == "A"
                        else perturbed_E(el, M0, off))
                pre, post, exact = [], [], []
                bE = 0.0
                for d in range(ORBIT_POINTS):
                    Md = M0 + (d - ORBIT_POINTS // 2) * incrM
                    seed, Epre = ecc_anomaly(el["e"], Md, seed, STEPS)
                    if bE == 0.0:
                        for _ in range(WARMUP_CALLS):
                            bE, _x = ecc_anomaly(el["e"], Md, bE, STEPS)
                    bE, Epost = ecc_anomaly(el["e"], Md, bE, STEPS)
                    Eex = kepler_exact(el["e"], Md)
                    pre.append(position_at_E(el, Epre))
                    post.append(position_at_E(el, Epost))
                    exact.append(position_at_E(el, Eex))
                dd = [norm(sub(pre[d], post[d])) for d in range(ORBIT_POINTS)]
                mx = max(dd)
                if case == "A":
                    if mx > worstA:
                        worstA, argA = mx, dd.index(mx)
                    worstPreErrA = max(worstPreErrA,
                                       max(norm(sub(pre[d], exact[d]))
                                           for d in range(ORBIT_POINTS)))
                    worstPostErr = max(worstPostErr,
                                       max(norm(sub(post[d], exact[d]))
                                           for d in range(ORBIT_POINTS)))
                elif mx > worstB:
                    worstB, argB = mx, dd.index(mx)
        out.append((n, branch, e, vis, worstA, argA, worstB, argB,
                    worstPreErrA, worstPostErr))
    out.sort(key=lambda r: -r[4])
    return out


def cmd_line(a):
    lines = [line_rows.__doc__.strip(), "",
             "%-12s %-10s %7s %10s %12s %5s %12s %5s %12s %12s" %
             ("body", "branch", "e", "vis_d", "max|pre-post|", "at_d",
              "steadymax", "at_d", "pre_vs_exact", "post_vs_exact")]
    for r in line_rows():
        lines.append("%-12s %-10s %7.4f %10.3f %12.5g %5d %12.5g %5d %12.5g %12.5g"
                     % r)
    return "\n".join(lines) + "\n"


def cmd_trail(a):
    """P6: TrailModule::resumeAfterHidden's own walk (TrailModule.cpp:227-234).

    `missed = (date - lastJD)/deltaTrail` samples are re-evaluated at
    `lastJD + k*deltaTrail`, each 1+RESUME_EXTRA_ITERATIONS = 5 times at ITS OWN
    date, so the seed ends CONVERGED at the LAST reconstructed sample's date --
    which is within ONE deltaTrail of the frame's own date by construction
    (`missed` is the floor of the quotient).  The body's next evaluation is one
    call at `date`, i.e. a seed at most deltaTrail*n rad of mean anomaly stale.

    This is the whole magnitude: the walker's staleness is bounded by the
    module's own cadence, where the sampler's is half a VISUALISATION PERIOD.
    """
    lines = [cmd_trail.__doc__.strip(), "",
             "deltaTrail = %g sim day(s) (TrailModule.hpp:193)" % DELTA_TRAIL, "",
             "%-12s %-10s %7s %12s %12s %12s %12s" %
             ("body", "branch", "e", "period_d", "offset_rad", "err3d_max",
              "errR_max")]
    rows = []
    for n, el, vis, branch, e in sampling_bodies():
        off = DELTA_TRAIL * el["n"]        # worst case: a full deltaTrail behind
        d3 = dr = 0.0
        for k in range(360):
            M = 2.0 * math.pi * k / 360.0
            Ee = kepler_exact(el["e"], M)
            Ep = perturbed_E(el, M, off)
            re_, pe = radial(el, Ee)
            rp_, pp = radial(el, Ep)
            d3 = max(d3, norm(sub(pp, pe)))
            dr = max(dr, abs(rp_ - re_))
        rows.append((n, branch, e, el["period"], off, d3, dr))
    for r in sorted(rows, key=lambda x: -x[5]):
        lines.append("%-12s %-10s %7.4f %12.4f %12.5g %12.5g %12.5g" % r)
    return "\n".join(lines) + "\n"


def chain(el, vis, dJD, frames=400, sampler=None, M0=0.0, steps=STEPS):
    """THE FRAME CHAIN -- written AFTER the pre-fix leg refuted the flat
    "flag off reads zero" prediction, and written at its cause rather than
    around it (artifacts/f111/prediction.txt P3 / R1).

    Each frame the walk evaluates the body ONCE (`recursiveTranslationUpdate`,
    one solver call = `steps` Newton steps, ModularBody.hpp:707) from the seed
    the previous frame left, at a date `dJD` later.  That alone leaves a
    residual when dJD is large, which is WHY a rate high enough to resample
    every frame has a floor of its own.  Then, if the orbit line is shown and
    the resample gate has fired (OrbitModule.cpp:132), the PRE-fix sampler walks
    the SAME seed through 180 dates and leaves it at +89 increments; the
    POST-fix one does not touch it at all, so `sampler=None` models BOTH the
    flag-off arm and the post-fix flag-on arm.

    Returns (resid_rad, err3d_AU, errR_AU) of the LAST walk evaluation -- the
    value a dump taken at that frame reads.
    """
    e = el["e"]
    n = el["n"]
    incrM = (vis / ORBIT_POINTS) * n        # one sample increment, in mean anomaly
    M = M0
    E = kepler_exact(e, M)
    since = 0.0
    last = (0.0, 0.0, 0.0)
    for _ in range(frames):
        M += dJD * n
        since += abs(dJD)
        _, E = ecc_anomaly(e, M, E, steps)  # the walk's own call
        Eex = kepler_exact(e, M)
        p, pe = position_at_E(el, E), position_at_E(el, Eex)
        last = (abs(E - Eex), norm(sub(p, pe)),
                abs(norm(p) - norm(pe)))
        if sampler == "pre" and since >= vis / ORBIT_POINTS:
            since = 0.0
            seed = E
            for d in range(ORBIT_POINTS):
                seed, _x = ecc_anomaly(e, M + (d - ORBIT_POINTS // 2) * incrM,
                                       seed, steps)
            E = seed
        elif sampler is None and since >= vis / ORBIT_POINTS:
            since = 0.0                      # the post-fix sampler runs, and
                                             # leaves `iterativeLastE` alone
    return last


def cmd_chain(a):
    """Score the chain model at the MEASURED per-frame date jump of each arm."""
    djd_off = getattr(a, "djd_off", 8.04)
    djd_on = getattr(a, "djd_on", 75.4)
    lines = [cmd_chain.__doc__.strip(),
             "dJD/frame measured from consecutive dumps' evalCount deltas",
             "  flag OFF arm: %.4f d/frame     flag ON arm: %.4f d/frame"
             % (djd_off, djd_on),
             "errR = ||pos| - |pos_exact||, the quantity the leg scores.",
             "PRE_on models the pre-fix sampler; POST_on = OFF_chain at the ON",
             "arm's own dJD, because the post-fix sampler leaves the seed alone.",
             "",
             "%-12s %10s %12s %12s %12s" %
             ("body", "vis_d", "OFF@off_dJD", "POST@on_dJD", "PRE@on_dJD")]
    for n, el, vis, branch, e in sampling_bodies():
        if vis <= 0.0:
            continue
        worst = [0.0, 0.0, 0.0]
        for k in range(8):                   # eight starting anomalies
            M0 = 2.0 * math.pi * k / 8.0
            worst[0] = max(worst[0], chain(el, vis, djd_off, 200, None, M0)[2])
            worst[1] = max(worst[1], chain(el, vis, djd_on, 200, None, M0)[2])
            worst[2] = max(worst[2], chain(el, vis, djd_on, 200, "pre", M0)[2])
        lines.append("%-12s %10.3f %12.5g %12.5g %12.5g"
                     % (n, vis, worst[0], worst[1], worst[2]))
    return "\n".join(lines) + "\n"


def cmd_score(a):
    """Score a landed dump: for every sampling walked-iterating record, the
    RADIAL residual between its dumped |ecl| and the model's CONVERGED position
    at the record's own dumped lastJD, beside the perturbation the model
    predicts for that same date.  A flag-off arm must read the first column at
    the model floor; a flag-on arm at an every-frame-resample rate must read it
    at the second."""
    rate = getattr(a, "rate", None)
    fps = getattr(a, "fps", FPS) or FPS
    thr = getattr(a, "threshold", 5.6e-7)
    _, recs = load_dump(a.dump)
    lines = ["# f111_predict score: %s" % a.dump,
             "# rate=%s fps=%g  PERTURBED threshold = %.3g AU (100x the model"
             " floor)" % (rate if rate else "1 (nominal)", fps, thr),
             "# resid = ||ecl| - |model converged at the record's own lastJD||",
             "# pred  = the perturbation the model predicts at that same date",
             "",
             "%-12s %18s %5s %14s %14s %14s %10s" %
             ("body", "lastJD", "every", "resid_AU", "pred_AU", "|ecl|",
              "verdict")]
    n_pert = n_conv = n_missing = 0
    for n, el, vis, branch, e in sampling_bodies():
        rec = recs.get(n)
        if not rec or not rec.get("new"):
            n_missing += 1
            continue
        new = rec["new"]
        jd, got = new.get("lastJD"), new.get("ecl")
        if jd is None or not got:
            n_missing += 1
            continue
        off, dJD = stage_offset(vis, rate, fps) if vis > 0 else (NOMINAL_OFFSET, 0.0)
        every = vis > 0 and ((rate is None) or (dJD >= vis / ORBIT_POINTS))
        M = mean_anomaly(el, jd)
        rex, _ = radial(el, kepler_exact(el["e"], M))
        rpe, _ = radial(el, perturbed_E(el, M, off))
        resid = abs(norm(got) - rex)
        pred = abs(rpe - rex)
        if resid >= thr:
            verdict = "PERTURBED"
            n_pert += 1
        else:
            verdict = "converged"
            n_conv += 1
        lines.append("%-12s %18.9f %5s %14.6g %14.6g %14.6g %10s"
                     % (n, jd, "yes" if every else "no", resid, pred,
                        norm(got), verdict))
    lines.append("")
    lines.append("PERTURBED %d | converged %d | missing %d"
                 % (n_pert, n_conv, n_missing))
    return "\n".join(lines) + "\n"


def cmd_cost(a):
    """P7: the arithmetic cost of a resample, pre and post, against D11."""
    pre_calls, pre_steps = ORBIT_POINTS, ORBIT_POINTS * STEPS
    post_calls = ORBIT_POINTS + WARMUP_CALLS
    post_steps = post_calls * STEPS
    return (
        "P7  COST PER RESAMPLE (D11: 1 ms/frame is the denominator)\n"
        "    PRE : %d solver calls, %d Newton steps (180 points x %d steps)\n"
        "    POST: %d solver calls, %d Newton steps (%d warm-up calls at d=0\n"
        "          + 180 scoring calls), plus ONE prepair call that writes a\n"
        "          double (orbit.cpp:442) -- +%.1f%% Newton steps per resample.\n"
        "    A resample fires once per vis/180 of SIMULATION time\n"
        "    (OrbitModule.cpp:132), i.e. at rate 1 once per %.2f days for the\n"
        "    shortest-period sampling body and once per %.2f days for the\n"
        "    longest -- not per frame.  Per-frame only at the rates of P2.\n"
        % (pre_calls, pre_steps, STEPS, post_calls, post_steps, WARMUP_CALLS,
           100.0 * (post_steps - pre_steps) / pre_steps,
           min(v for n, e, v, b, ec in sampling_bodies() if v > 0) / ORBIT_POINTS,
           max(v for n, e, v, b, ec in sampling_bodies() if v > 0) / ORBIT_POINTS))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["transient", "rate", "line", "trail",
                                     "cost", "score", "chain", "all"])
    ap.add_argument("dump", nargs="?", default=str(DEFAULT_DUMP))
    ap.add_argument("--out")
    ap.add_argument("--rate", type=float, default=None,
                    help="timerate rate of the stage (sim seconds per wall second)")
    ap.add_argument("--fps", type=float, default=FPS)
    ap.add_argument("--threshold", type=float, default=5.6e-7)
    ap.add_argument("--djd-off", type=float, default=8.04, dest="djd_off")
    ap.add_argument("--djd-on", type=float, default=75.4, dest="djd_on")
    a = ap.parse_args()
    if a.mode == "all":
        stages = []
        for r in (None, 1.0e8, 7.0e8):
            a.rate = r
            stages.append(cmd_transient(a))
        a.rate = None
        txt = "\n".join(stages + [cmd_rate(a), cmd_line(a), cmd_trail(a),
                                  cmd_cost(a)])
    else:
        txt = {"transient": cmd_transient, "rate": cmd_rate, "line": cmd_line,
               "trail": cmd_trail, "cost": cmd_cost, "score": cmd_score,
               "chain": cmd_chain}[a.mode](a)
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
