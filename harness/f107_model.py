#!/usr/bin/env python3
"""F107 -- THE PREDICTION INSTRUMENT: where a parked body's Newton seed comes
from, modelled in Python from the field file and the tree's READING, before any
engine text is compiled or launched.  INTENT 11.225(j2)(j3) / 11.229.

WHY A SECOND IMPLEMENTATION AT ALL.  F104's `f104_solver.py` SLICES
`EllipticalOrbit::eccentricAnomaly` out of `orbit.cpp` and compiles the tree's
own text (README F104: "slice the function out of the tree, do not mirror it").
That is the SCORING instrument and it stays so.  This module is the PREDICTING
one: it is deliberately an independent re-derivation, so that agreement between
the two is evidence about the READING (which dates, which seed, which branch,
which coupling) and not merely about arithmetic.  Nothing here is authoritative
-- every formula cites the site it models, and on any disagreement the tree's
text and the landed dump win and the disagreement is a finding.

THE MECHANISM MODELLED (each line cites the site):

  1. `ModularBody::ModularBody` evaluates EVERY body once at construction, at
     `lastJD = parent->lastJD` (ModularBody.cpp:121) through
     `orbit->positionAtTimevInVSOP87Coordinates(lastJD,lastJD,tmp)` (:126).
     Before the first frame that date is **JD 0** for every node, so the first
     thing a body's solver seed ever sees is M(JD 0).
  2. `EllipticalOrbit::positionAtTime` (orbit.cpp:577-585) computes the mean
     anomaly UNWRAPPED (`meanAnomalyAtEpoch + (JD-epoch)*2pi/period`, :581) and
     hands it to `eccentricAnomaly(meanAnomaly, iterativeLastE)` (:582) -- the
     `mutable` member seed (orbit.hpp:120).  `if (lastE == 0)` seeds it at M
     (:521/:527/:538/:557) and each branch then advances
     ITERATIVE_STEPS_PER_CALL steps (1 pre-F104, 2 at HEAD).
  3. `IterativeEll::operator()` (iterative_orbits.hpp:124-134) WRAPS its mean
     anomaly into [0,2pi) (`fmod`, :125-127) and its state (H,c,s) starts at
     (0,1,0) -- so a comet-family body's seed staleness is bounded by one
     revolution BY CONSTRUCTION, which is why the family matters.
  4. At the first use `ModularBody::useNow` runs `1 + RESUME_EXTRA_ITERATIONS`
     = 5 `recursiveTranslationUpdate`s (ModularBody.cpp:562-564), each one
     call of the solver (ModularBody.hpp:707, `++evalCount` :709).
  5. Each of those 5 calls RETARDS the date by the light trip of the CURRENT
     `distance` (ModularBody.hpp:698-702) and then REWRITES `distance` from the
     position it just computed (:961).  So the five calls are a COUPLED
     iteration: position -> distance -> retarded date -> position.  The dump's
     `lastJD` is the fifth call's retarded date and its `dist` the fifth
     distance, which is what makes this model checkable against landed bytes.

  usage
    ./f107_model.py replay  [--calls 5] [--jd 2461233.5] [--out FILE]
    ./f107_model.py sampler [--out FILE]      # the (j2) resample transient
"""
import argparse
import json
import math
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from f104_census import decode  # noqa: E402  (F70's reference byte decoder)

DEFAULT_INI = Path.home() / ".spacecrafter" / "ssystem.ini"
DEFAULT_CENSUS = HERE / "artifacts" / "f104" / "census.json"
DEFAULT_DUMP = HERE / "artifacts" / "f104" / "leg_pre" / "pinned_p0_launch.json.gz"
LAUNCH_JD = 2461233.5                      # 11.226(g), F100's pinned clock

AU_KM = 149597870.691                      # sc_const.hpp:45
# ModularBody.hpp:700 -- days of light travel per AU
LIGHT_DAYS_PER_AU = 149597870000.0 / (299792458.0 * 86400)
MAX_RETARDATION_DAYS = 32                  # ModularBody.hpp:699
RESUME_CALLS = 5                           # 1 + RESUME_EXTRA_ITERATIONS (=4)
ORBIT_POINTS = 180                         # OrbitModule.cpp:14


def read_ini_cased(path):
    """The field file through the ENGINE'S OWN line grammar, keys CASE-PRESERVED.

    `IniLine::read` (tools/ini_line.hpp) splits at the FIRST '=', trims blanks
    (space/tab/CR/LF) on both sides, treats '#' as a comment to end of line,
    '[x]' as a section and any other non-empty line as MALFORMED (ignored, with
    a warning -- the shipped `[Sedna] orbit_LongOfPericenter 95.58754` is the
    named instance).  It does NOT lower-case the key, and the loaders look keys
    up by exact spelling (`params["orbit_period"]`), so `orbit_Period` binds a
    DIFFERENT key and is invisible to them.  f104_census.py's reader lower-cases
    (it only needs `coord_func`/`orbit_eccentricity`, both lower-case in every
    shipped section) -- this one may not, because it feeds an ARITHMETIC model.
    """
    text = decode(open(path, "rb").read())
    secs, cur = {}, None
    for raw in text.split("\n"):
        line = raw.split("#", 1)[0].strip(" \t\r\n")
        if not line:
            continue
        if line.startswith("[") and line.endswith("]"):
            cur = line[1:-1]
            secs[cur] = {"__sec__": cur}
        elif cur is not None and "=" in line:
            k, v = line.split("=", 1)
            secs[cur][k.strip(" \t\r\n")] = v.strip(" \t\r\n")
    return secs


def stod(s, default=None):
    """Utility::strToDouble = std::stod in a try/catch (utility.cpp:399-406):
    it parses the LONGEST NUMERIC PREFIX and ignores the rest, so the shipped
    `orbit_pericenterdistance = 76,0616` is **76.0**, not 76.0616 (the field
    file's comma decimal separator is not the C locale's)."""
    if s is None:
        return default
    t = s.strip()
    best = None
    for i in range(len(t), 0, -1):
        try:
            best = float(t[:i])
            break
        except ValueError:
            continue
    return best if best is not None else default


def f(sec, key, default=None):
    return stod(sec.get(key), default)


def elements(sec):
    """The orbit an ORBIT LOADER would build from this section.

    ell_orbit  -> ElipticOrbitLoader.hpp:28-49 (semi-major axis /AU, :30;
                  arg = longofpericenter - ascendingnode, :37; anomaly at
                  epoch = meanlongitude - longofpericenter, :38).
    comet_orbit -> CometOrbitLoader.hpp:37-95 (semimajoraxis ALREADY in AU
                  here, no /AU; mean motion from Gauss's constant when the
                  parent is the system centre, :69; time at pericenter from
                  epoch and mean anomaly, :88).
    """
    cf = (sec.get("coord_func") or "").strip().lower()
    e = f(sec, "orbit_eccentricity", 0.0)
    inc = math.radians(f(sec, "orbit_inclination", 0.0))
    Om = math.radians(f(sec, "orbit_ascendingnode", 0.0))
    if cf == "ell_orbit":
        a = f(sec, "orbit_semimajoraxis") / AU_KM
        period = f(sec, "orbit_period")
        epoch = f(sec, "orbit_epoch", 2451545.0)
        lp = math.radians(f(sec, "orbit_longofpericenter", 0.0))
        ml = math.radians(f(sec, "orbit_meanlongitude", 0.0))
        w = lp - Om
        m_epoch = ml - (w + Om)
        return {"family": "ell", "e": e, "a": a, "q": a * (1.0 - e), "inc": inc,
                "Om": Om, "w": w, "period": period, "epoch": epoch,
                "m_epoch": m_epoch, "n": 2.0 * math.pi / period}
    if cf == "comet_orbit":
        a = f(sec, "orbit_semimajoraxis")
        q = f(sec, "orbit_pericenterdistance")
        if q is None or q <= 0.0:
            q = a * (1.0 - e)
        else:
            a = q / (1.0 - e)
        w = math.radians(f(sec, "orbit_argofpericenter", 0.0))
        mm = f(sec, "orbit_meanmotion")
        period = f(sec, "orbit_period")
        if mm is not None:
            n = mm * math.pi / 180.0
        elif period is not None:
            n = 2.0 * math.pi / period
        else:                               # system-centred parent: Gauss
            n = 0.01720209895 / (abs(a) * math.sqrt(abs(a)))
        t0 = f(sec, "orbit_timeatpericenter")
        if t0 is None:
            epoch = f(sec, "orbit_epoch")
            ma = math.radians(f(sec, "orbit_meananomaly"))
            t0 = epoch - ma / n
        return {"family": "comet", "e": e, "a": a, "q": q, "inc": inc,
                "Om": Om, "w": w, "n": n, "t0": t0, "period": 2 * math.pi / n}
    return None


# ---------------------------------------------------------------- the solvers


def sign(x):
    return -1.0 if x < 0. else (1.0 if x > 0. else 0.0)


def ecc_anomaly(e, M, lastE, steps):
    """EllipticalOrbit::eccentricAnomaly, modelled (orbit.cpp:515-573).

    Returns the new lastE.  `steps` = ITERATIVE_STEPS_PER_CALL.
    """
    if e == 0.0:
        return lastE, M                     # :517 return M, seed untouched
    if e < 0.2:
        if lastE == 0:
            lastE = M                       # :521-522
        for _ in range(steps):              # :524-525
            lastE = M + e * math.sin(lastE)
    elif e < 0.9:
        if lastE == 0:
            lastE = M                       # :527-528
        for _ in range(steps):              # :532-533
            lastE += (M + e * math.sin(lastE) - lastE) / (1 - e * math.cos(lastE))
    elif e < 1.0:
        if lastE == 0:
            lastE = M + 0.85 * e * sign(math.sin(M))   # :538-539
        for _ in range(steps):              # :544-550
            s = e * math.sin(lastE)
            c = e * math.cos(lastE)
            fv = lastE - s - M
            f1 = 1 - c
            lastE += -5 * fv / (f1 + sign(f1) * math.sqrt(abs(16 * f1 * f1 - 20 * fv * s)))
    elif e == 1.0:
        return lastE, M                     # :551-554
    else:
        if lastE == 0:
            lastE = math.log(2 * M / e + 1.85)          # :557-558
        for _ in range(steps):              # :564-570
            s = e * math.sinh(lastE)
            c = e * math.cosh(lastE)
            fv = s - lastE - M
            f1 = c - 1
            lastE += -5 * fv / (f1 + sign(f1) * math.sqrt(abs(16 * f1 * f1 - 20 * fv * s)))
    return lastE, lastE


def iter_ell_call(el, state, dt, steps):
    """IterativeEll::operator(), modelled (iterative_orbits.hpp:124-134).

    state = (H, c, s), initialised (0, 1, 0) by the member declaration
    (:140-142) -- the EllCometOrbit constructor does NOT warp (orbit.cpp:319).
    """
    H, c, s = state
    e = el["e"]
    M = math.fmod(el["n"] * dt, 2 * math.pi)
    if M < 0.0:
        M += 2.0 * math.pi
    for _ in range(steps):
        H -= (M - H + e * s) / (e * c - 1)
        c = math.cos(H)
        s = math.sin(H)
    return (H, c, s), M


def kepler_exact(e, M):
    """The reference answer by a DIFFERENT method (bisection-guarded Newton run
    to a fixed point) -- f104_solver.cpp:57-73's shape, so a shared bug cannot
    cancel.  E - e sin E = M is monotone in E, so the root is unique in R."""
    lo, hi = M - 1.0 - e, M + 1.0 + e
    x = M
    for _ in range(400):
        fv = x - e * math.sin(x) - M
        if fv > 0:
            hi = x
        else:
            lo = x
        d = 1.0 - e * math.cos(x)
        nx = x - fv / d if d != 0.0 else 0.5 * (lo + hi)
        if not (lo < nx < hi):
            nx = 0.5 * (lo + hi)
        if nx == x:
            break
        x = nx
    return x


def rot_matrix(Om, inc, w):
    """R = zrotation(ascendingNode) * xrotation(inclination) *
    zrotation(argOfPeriapsis), orbit.cpp:497-499."""
    def zr(t):
        c, s = math.cos(t), math.sin(t)
        return [[c, -s, 0], [s, c, 0], [0, 0, 1]]

    def xr(t):
        c, s = math.cos(t), math.sin(t)
        return [[1, 0, 0], [0, c, -s], [0, s, c]]

    def mul(A, B):
        return [[sum(A[i][k] * B[k][j] for k in range(3)) for j in range(3)]
                for i in range(3)]
    return mul(mul(zr(Om), xr(inc)), zr(w))


def position_at_E(el, E):
    """EllipticalOrbit::positionAtE, modelled (orbit.cpp:477-503), e < 1 branch.
    `rotate_to_vsop87` is the IDENTITY for a body whose parent is the system
    centre (all three parent_rot_* stay 0 -- ElipticOrbitLoader.hpp:12), so this
    IS the heliocentric VSOP87 vector the dump's `ecl` carries."""
    e = el["e"]
    a = el["q"] / (1.0 - e)
    x = a * (math.cos(E) - e)
    y = a * math.sqrt(1 - e * e) * math.sin(E)
    R = rot_matrix(el["Om"], el["inc"], el["w"])
    return [R[0][0] * x + R[0][1] * y, R[1][0] * x + R[1][1] * y,
            R[2][0] * x + R[2][1] * y]


def position_comet(el, H):
    """EllCometOrbit::positionAtTime -> IterativeEll's return value
    (iterative_orbits.hpp:133) with d1/d2 from CometOrbit's constructor
    (orbit.cpp:218-219); `rotate_to_vsop87` identity as above."""
    e, q = el["e"], el["q"]
    a = q / (1.0 - e)
    h1 = q * math.sqrt((1.0 + e) / (1.0 - e))
    co, so = math.cos(el["w"]), math.sin(el["w"])
    cOm, sOm = math.cos(el["Om"]), math.sin(el["Om"])
    ci, si = math.cos(el["inc"]), math.sin(el["inc"])
    d1 = [-so * sOm * ci + co * cOm, so * cOm * ci + co * sOm, so * si]
    d2 = [-co * sOm * ci - so * cOm, co * cOm * ci - so * sOm, co * si]
    k1, k2 = a * (math.cos(H) - e), h1 * math.sin(H)
    return [d1[i] * k1 + d2[i] * k2 for i in range(3)]


def mean_anomaly(el, jd):
    if el["family"] == "ell":
        return el["m_epoch"] + (jd - el["epoch"]) * el["n"]   # orbit.cpp:579-581
    return el["n"] * (jd - el["t0"])                          # orbit.cpp:346


def wrapE(x):
    """The eccentric anomaly folded into (-pi, pi] the way `atan2(s, c)` folds
    it -- the comet family's state is (H, c, s) and only (c, s) survives into
    the returned point, so a recovered H can only ever be the folded one."""
    return math.atan2(math.sin(x), math.cos(x))


def norm(v):
    return math.sqrt(sum(x * x for x in v))


def sub(a, b):
    return [a[i] - b[i] for i in range(3)]


# ---------------------------------------------------------------- the replay


def replay(el, jd_use, obs, steps, calls=RESUME_CALLS, ctor=True,
           retard=True, ctor_jd=0.0):
    """The first use of a parked body, from construction.

    Returns a list of per-call dicts.  `ctor=False` models the MUTATION (the
    constructor's evaluation skipped when parent->lastJD == 0), i.e. a seed
    still at its declared zero when the first real use arrives.
    """
    out = {"ctor": None, "calls": []}
    if el["family"] == "ell":
        lastE = 0.0
        if ctor:
            lastE, _ = ecc_anomaly(el["e"], mean_anomaly(el, ctor_jd), lastE, steps)
            out["ctor"] = {"jd": ctor_jd, "M": mean_anomaly(el, ctor_jd),
                           "lastE": lastE}
        dist = 0.0                          # never evaluated: ModularBody.hpp:2180
        for k in range(calls):
            jd = jd_use
            if retard and dist == dist:     # :698-702
                r = dist * LIGHT_DAYS_PER_AU
                jd -= r if r < MAX_RETARDATION_DAYS else MAX_RETARDATION_DAYS
            M = mean_anomaly(el, jd)
            lastE, E = ecc_anomaly(el["e"], M, lastE, steps)
            pos = position_at_E(el, E)
            exact = kepler_exact(el["e"], M)
            dist = norm(sub(pos, obs))      # :961 (frame translation length)
            out["calls"].append({"call": k + 1, "jd": jd, "M": M, "E": E,
                                 "exact": exact, "res": abs(E - exact),
                                 "pos": pos, "dist": dist,
                                 "poserr": norm(sub(pos, position_at_E(el, exact)))})
        return out
    # comet family
    state = (0.0, 1.0, 0.0)
    if ctor:
        state, _ = iter_ell_call(el, state, ctor_jd - el["t0"], steps)
        out["ctor"] = {"jd": ctor_jd, "M": math.fmod(el["n"] * (ctor_jd - el["t0"]),
                                                     2 * math.pi), "lastE": state[0]}
    dist = 0.0
    for k in range(calls):
        jd = jd_use
        if retard:
            r = dist * LIGHT_DAYS_PER_AU
            jd -= r if r < MAX_RETARDATION_DAYS else MAX_RETARDATION_DAYS
        state, M = iter_ell_call(el, state, jd - el["t0"], steps)
        H = state[0]
        pos = position_comet(el, H)
        exact = kepler_exact(el["e"], M)
        dist = norm(sub(pos, obs))
        out["calls"].append({"call": k + 1, "jd": jd, "M": M, "E": H,
                             "exact": exact, "res": abs(H - exact),
                             "pos": pos, "dist": dist,
                             "poserr": norm(sub(pos, position_comet(el, exact)))})
    return out


def load_dump(path):
    import gzip
    op = gzip.open if str(path).endswith(".gz") else open
    hdr, recs = None, {}
    with op(path, "rt", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            try:
                r = json.loads(line)
            except ValueError:
                continue
            if r.get("type") == "header":
                hdr = r
            elif r.get("type") == "body":
                recs[r["name"]] = r
    return hdr, recs


def f32(x):
    import struct
    return struct.unpack("f", struct.pack("f", x))[0]


def parked_iterating(census):
    rows = census["rows"]
    return sorted(n for n, r in rows.items()
                  if r["solver"] == "ITER" and r["place"] in ("parked", "unreached"))


def walked_iterating(census):
    rows = census["rows"]
    return sorted(n for n, r in rows.items()
                  if r["solver"] == "ITER" and r["place"] == "walked")


def by_name_sections(ini):
    """Sections by the body NAME (`name =`), not by the section header: the
    shipped `[Hi'iaka]` carries `name = Hiiaka` and `[Hydra]` carries
    `name = Hydra_` (F104's census note).  Keys keep their authored case."""
    secs = read_ini_cased(ini)
    out = {}
    for sec in secs.values():
        if sec.get("name"):
            out[sec["name"]] = sec
    return out


def cmd_replay(a):
    secs = by_name_sections(a.ini)
    census = json.load(open(a.census))
    _, recs = load_dump(a.dump)
    obs = recs["Earth"]["new"]["ecl"]       # the observer, from the landed dump
    lines = []
    lines.append("# F107 replay MODEL (python, independent of the sliced engine text)")
    lines.append("# obs (Earth new ecl, landed dump) = %r" % (obs,))
    lines.append("# use jd = %.17g ; ctor jd = 0 ; calls = %d" % (a.jd, a.calls))
    lines.append("")
    hdr = ("%-12s %-9s %-10s %14s %14s %12s %12s %12s" %
           ("body", "branch", "arm", "res_call5", "poserr5_AU", "res_ctor",
            "lastJD_5", "dist_5"))
    for who, names in (("PARKED", parked_iterating(census)),):
        lines.append("== %s ITERATING (%d) -- each arm: pre = 1 step/call, "
                     "post = 2, mut = pre with the ctor evaluation skipped"
                     % (who, len(names)))
        lines.append(hdr)
        for n in names:
            el = elements(secs[n])
            if el is None:
                lines.append("%-12s (no elements)" % n)
                continue
            for arm, steps, ctor in (("pre", 1, True), ("post", 2, True),
                                     ("mut", 1, False)):
                r = replay(el, a.jd, obs, steps, a.calls, ctor)
                c5 = r["calls"][-1]
                lines.append("%-12s %-9s %-10s %14.6g %14.6g %12s %12.9f %12.7f"
                             % (n, census["rows"][n]["branch"], arm,
                                c5["res"], c5["poserr"],
                                ("%.6g" % r["ctor"]["lastE"]) if r["ctor"] else "-",
                                c5["jd"], c5["dist"]))
        lines.append("")
    # WHICH RECORDS THE MUTATION CAN MOVE, at float32 (the dump's own width).
    # The mutation skips the constructor's evaluation for EVERY body whose
    # parent->lastJD is 0, so the prediction has to be made for every body it
    # touches -- and a WALKED body has been re-evaluated every frame since the
    # load, so only the PARKED ones (evalCount 5 at P0) can still carry it.
    lines.append("== FLOAT32 IDENTITY, per parked iterating body (dump width)")
    lines.append("%-12s %-26s %-26s %-26s %s" %
                 ("body", "pre ecl32", "post ecl32", "mut ecl32", "moves?"))
    for n in parked_iterating(census):
        el = elements(secs[n])
        arms = {}
        for arm, steps, ctor in (("pre", 1, True), ("post", 2, True),
                                 ("mut", 1, False)):
            c5 = replay(el, a.jd, obs, steps, a.calls, ctor)["calls"][-1]
            arms[arm] = tuple(f32(x) for x in c5["pos"])
        moves = []
        if arms["pre"] != arms["post"]:
            moves.append("pre!=post")
        if arms["pre"] != arms["mut"]:
            moves.append("pre!=mut")
        if arms["post"] != arms["mut"]:
            moves.append("post!=mut")
        fmt = lambda t: "[%.9g %.9g %.9g]" % t
        lines.append("%-12s %-26s %-26s %-26s %s"
                     % (n, fmt(arms["pre"]), fmt(arms["post"]), fmt(arms["mut"]),
                        ",".join(moves) if moves else "identical on all three"))
    lines.append("")
    # Eris in detail: every call, and the landed bytes beside it
    el = elements(secs["Eris"])
    lines.append("== ERIS, CALL BY CALL (the coupled position/distance/date iteration)")
    for arm, steps, ctor in (("pre", 1, True), ("post", 2, True), ("mut", 1, False)):
        r = replay(el, a.jd, obs, steps, a.calls, ctor)
        if r["ctor"]:
            lines.append("  %s ctor: jd 0  M %.17g  lastE %.17g"
                         % (arm, r["ctor"]["M"], r["ctor"]["lastE"]))
        else:
            lines.append("  %s ctor: SKIPPED (seed still 0 at the first use)" % arm)
        for c in r["calls"]:
            lines.append("  %s call %d jd %.10f M %.17g E %.17g res %.6g "
                         "dist %.7f ecl32 [%.9g, %.9g, %.9g]"
                         % (arm, c["call"], c["jd"], c["M"], c["E"], c["res"],
                            c["dist"], f32(c["pos"][0]), f32(c["pos"][1]),
                            f32(c["pos"][2])))
        c5 = r["calls"][-1]
        exact_pos = position_at_E(el, c5["exact"])
        lines.append("  %s  |pos5 - pos(exact)| = %.9f AU" % (arm, c5["poserr"]))
        lines.append("")
    # the displacement F104 measured: pre's 5th iterate against post's
    rpre = replay(el, a.jd, obs, 1, a.calls, True)["calls"][-1]
    rpost = replay(el, a.jd, obs, 2, a.calls, True)["calls"][-1]
    d = norm(sub(rpre["pos"], rpost["pos"]))
    lines.append("== ERIS pre vs post: |delta| = %.9f AU  (landed eye-frame "
                 "|delta| 2.029337245 AU)" % d)
    # the angular readout: the perpendicular component over the eye distance
    los = sub(rpre["pos"], obs)
    delta = sub(rpre["pos"], rpost["pos"])
    dl = norm(los)
    par = sum(delta[i] * los[i] for i in range(3)) / dl
    perp = math.sqrt(max(0.0, norm(delta) ** 2 - par * par))
    ang = math.degrees(math.atan2(perp, dl + par))
    lines.append("   angle at the observer = %.6f deg  (landed alt/az "
                 "separation 1.1987248926268401 deg)" % ang)
    txt = "\n".join(lines) + "\n"
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


def cmd_sampler(a):
    """(j2): what the orbit-line sampler leaves behind, per WALKED iterating
    body, and what the next frame's single call reads.

    OrbitModule::sampleOrbit (OrbitModule.cpp:94-114) walks d = 0..179 with
    calc_date = date + (d - 90)*period/180, in that order, through the SAME
    `iterativeLastE` seed (:110 -> orbit.cpp:466 -> :577 -> :582), so the seed
    is left at the LAST sample's date = date + 89*period/180.  The body's own
    next evaluation is ONE call at the frame's own date.
    """
    secs = by_name_sections(a.ini)
    census = json.load(open(a.census))
    _, recs = load_dump(a.dump)
    obs = recs["Earth"]["new"]["ecl"]
    lines = ["# F107 sampler MODEL: the one-frame transient a resample leaves",
             "# seed left at E(lastJD + 89*visPeriod/180) -- the LAST of the 180",
             "# samples (OrbitModule.cpp:105-110, ascending d); then ONE call at",
             "# the body's own date with `steps` steps (2 at HEAD, 1 pre-F104).",
             "# The sampler's increment is orbit_visualization_period/180",
             "# (ModularSystem.cpp:1209 -> re.sidereal_period; ABSENT => 0 =>",
             "# OrbitModule::update takes the `still orbit` branch, :134-136, and",
             "# NEVER samples).  poserr = |pos - pos(exact)| AU, rotation-invariant;",
             "# ang = poserr over the record's own dumped `dist`, arcsec.",
             ""]
    lines.append("%-12s %-9s %12s %6s %13s %13s %12s" %
                 ("body", "branch", "visPeriod_d", "steps", "res_rad",
                  "poserr_AU", "ang_arcsec"))
    rows = []
    for n in walked_iterating(census):
        sec = secs.get(n, {})
        el = elements(sec)
        if el is None:
            continue
        vis = stod(sec.get("orbit_visualization_period"), 0.0) or 0.0
        rec = recs.get(n)
        dist = (rec["new"].get("dist") or 0.0) if rec else 0.0
        if vis <= 0.0:
            rows.append((n, census["rows"][n]["branch"], vis, 0,
                         0.0, 0.0, 0.0))
            continue
        jd = rec["new"]["lastJD"] if rec else a.jd
        stale_jd = jd + 89.0 * vis / ORBIT_POINTS
        for steps in (1, 2):
            if el["family"] == "ell":
                seedE = kepler_exact(el["e"], mean_anomaly(el, stale_jd))
                _, E = ecc_anomaly(el["e"], mean_anomaly(el, jd), seedE, steps)
                exact = kepler_exact(el["e"], mean_anomaly(el, jd))
                pos, epos = position_at_E(el, E), position_at_E(el, exact)
            else:
                def wrap(x):
                    y = math.fmod(x, 2 * math.pi)
                    return y + 2 * math.pi if y < 0 else y
                Hs = kepler_exact(el["e"], wrap(el["n"] * (stale_jd - el["t0"])))
                state = (Hs, math.cos(Hs), math.sin(Hs))
                state, M = iter_ell_call(el, state, jd - el["t0"], steps)
                E = state[0]
                exact = kepler_exact(el["e"], M)
                pos, epos = position_comet(el, E), position_comet(el, exact)
            res = abs(E - exact)
            poserr = norm(sub(pos, epos))
            ang = (math.degrees(math.atan2(poserr, dist)) * 3600.0) if dist else float("nan")
            rows.append((n, census["rows"][n]["branch"], vis, steps, res, poserr, ang))
    for r in sorted(rows, key=lambda x: (-(x[6] if x[6] == x[6] else -1), x[0], x[3])):
        lines.append("%-12s %-9s %12.3f %6d %13.6g %13.6g %12.4f" % r)
    txt = "\n".join(lines) + "\n"
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


def cmd_validate(a):
    """THE MODEL'S OWN GATE, and it can fail.

    For every ITERATING record, compute the position this model says the engine
    would hold if its solver were CONVERGED at the record's own dumped `lastJD`,
    and compare with the landed `ecl` (float32).  A body whose readout is
    converged must match to float32; a body that does NOT match either is
    under-converged in the engine (the thing this task is about) or is modelled
    with the wrong elements (a defect in THIS file).  The two are told apart by
    the pre/post pair: only Eris moved between them (11.225(d)), so every other
    iterating record's dumped position IS its converged one.
    """
    secs = by_name_sections(a.ini)
    census = json.load(open(a.census))
    _, recs = load_dump(a.dump)
    lines = ["# F107 model gate: converged position vs the landed dump's ecl",
             "# dump = %s" % a.dump, ""]
    lines.append("%-14s %-10s %-9s %14s %14s %s" %
                 ("body", "place", "branch", "err_AU", "err_rel", "verdict"))
    bad = 0
    names = parked_iterating(census) + walked_iterating(census)
    for n in sorted(names):
        sec = secs.get(n)
        el = elements(sec) if sec else None
        rec = recs.get(n)
        if el is None or rec is None:
            lines.append("%-14s %-10s %-9s %14s %14s %s"
                         % (n, "?", "?", "-", "-", "NO ELEMENTS/RECORD"))
            bad += 1
            continue
        new = rec["new"]
        jd = new.get("lastJD")
        M = mean_anomaly(el, jd)
        if el["family"] == "ell":
            pos = position_at_E(el, kepler_exact(el["e"], M))
        else:
            Mw = math.fmod(M, 2 * math.pi)
            if Mw < 0:
                Mw += 2 * math.pi
            pos = position_comet(el, kepler_exact(el["e"], Mw))
        got = new.get("ecl")
        # WHICH CHECK IS AVAILABLE depends on the parent.  `rotate_to_vsop87`
        # (orbit.cpp:412-436) is the IDENTITY only when all three parent_rot_*
        # are zero, i.e. when the parent IS the system centre
        # (ElipticOrbitLoader.hpp:12 / CometOrbitLoader.hpp:14) -- then the
        # dump's parent-relative `ecl` is this model's vector COMPONENT for
        # component.  For a moon the matrix is a real rotation built from the
        # parent's own rotation elements, which this model does not carry; a
        # rotation preserves LENGTH, so the check there is |pos| vs |ecl| --
        # weaker (it cannot see a wrong node/inclination) but still a function
        # of a, e and M, which is what the replay uses.
        sun = (sec.get("parent") == "Sun")
        err = norm(sub(pos, got)) if sun else abs(norm(pos) - norm(got))
        rel = err / max(1e-12, norm(got))
        ok = rel < 3e-6                     # float32 print precision
        if not ok:
            bad += 1
        lines.append("%-14s %-10s %-9s %14.6g %14.6g %s"
                     % (n, census["rows"][n]["place"], census["rows"][n]["branch"],
                        err, rel, ("ok" if ok else "MISMATCH")
                        + ("" if sun else " [|.| only: rotated frame]")))
    lines.append("")
    lines.append("%d of %d iterating records mismatch (Eris pre-fix is the"
                 " EXPECTED one)" % (bad, len(names)))
    txt = "\n".join(lines) + "\n"
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["replay", "sampler", "validate"])
    ap.add_argument("--ini", default=str(DEFAULT_INI))
    ap.add_argument("--census", default=str(DEFAULT_CENSUS))
    ap.add_argument("--dump", default=str(DEFAULT_DUMP))
    ap.add_argument("--jd", type=float, default=LAUNCH_JD)
    ap.add_argument("--calls", type=int, default=RESUME_CALLS)
    ap.add_argument("--out")
    a = ap.parse_args()
    return {"replay": cmd_replay, "sampler": cmd_sampler,
            "validate": cmd_validate}[a.mode](a)


if __name__ == "__main__":
    sys.exit(main())
