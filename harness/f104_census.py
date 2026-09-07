#!/usr/bin/env python3
"""F104 -- THE SOLVER CLASS OF EVERY DUMP RECORD, FROM THE FIELD FILE ALONE.

INTENT 5.145 / 11.223(b).  The fix doubles the Newton steps taken per call by
the ITERATIVE orbit solvers (EllipticalOrbit::eccentricAnomaly's four advancing
branches, IterativeEll/IterativeHyp::operator()).  Which bodies that reaches is
a property of the DATA, not of any measurement, so this module reads it out of
the installed ~/.spacecrafter/ssystem.ini and never opens a result file.

Three keys, composed:

  SOLVER   from `coord_func` + `orbit_eccentricity` in the body's own section:
             ell_orbit,  e == 0     -> `return M`, NO iteration      (orbit.cpp:517)
             ell_orbit,  0 < e < .2 -> fixed-point step              (orbit.cpp:524)
             ell_orbit, .2 <= e <.9 -> Newton step (Eris's)          (orbit.cpp:531)
             ell_orbit, .9 <= e < 1 -> Laguerre-Conway elliptic      (orbit.cpp:543)
             ell_orbit,  e == 1     -> `return M`, NO iteration      (orbit.cpp:547)
             ell_orbit,  e > 1      -> Laguerre-Conway hyperbolic    (orbit.cpp:559)
             comet_orbit, e < 1     -> IterativeEll::operator()      (iterative_orbits.hpp:97)
             comet_orbit, e > 1     -> IterativeHyp::operator()      (iterative_orbits.hpp:41)
             comet_orbit, e == 1    -> ParCometOrbit, closed form
             anything else          -> *_special / *_custom, closed form
           A record with no section in the field file has no orbit of these
           families and is classed NON-ITERATIVE by construction.
  REACH    a body's EYE-FRAME position composes its ancestors' positions, so a
           record is touched by the fix if its OWN solver iterates OR any
           ancestor's does (the field file's `parent =` chain).  On this corpus
           that adds exactly the non-iterating children of an iterating body --
           Hiiaka and Namaka, both `e == 0` under the `comet_orbit` Haumea.
           The camera's own chain (Earth -> Sun -> SolarSystem -> MilkyWay ->
           Universe, all closed-form) carries no iterating solver, so the view
           matrix itself is not perturbed.
  PARKED   class P/I of f100_partition.py, taken from F100's COMMITTED
           partition JSON (artifacts/f100/partition_f96_leg_pre.json) -- a
           freshness-blind key built by a delivered task, not re-derived here.
  RECORDS  the 120 record NAMES, an inventory (not a measurement) read from
           F100's committed launch-state dump, or from --names.

The section KEY is not the body NAME: `[Hi'iaka]` carries `name = Hiiaka` and
`[Hydra]` carries `name = Hydra_`.  Keying on the section header loses exactly
those two, which is why this module keys on the `name =` value.

The file is ISO-8859-ish and untracked: the Bash grep wrapper (ugrep -I) skips
it SILENTLY, so it is read as BYTES here and decoded greedy-UTF-8 with a
per-byte Latin-1 fallback (CLAUDE.md's encoding rule, F70's reference method).

  usage
    ./f104_census.py [--ini PATH] [--partition JSON] [--dump DUMP] [--json OUT]
"""
import argparse
import gzip
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_INI = Path.home() / ".spacecrafter" / "ssystem.ini"
DEFAULT_PART = HERE / "artifacts" / "f100" / "partition_f96_leg_pre.json"
DEFAULT_DUMP = HERE / "artifacts" / "f100" / "leg_pre" / "pinned_p0_launch.json.gz"

# the four advancing statements of eccentricAnomaly + the two operator() steps
ITER_BRANCHES = {
    "ell<0.2": "orbit.cpp:524 fixed-point",
    "ell0.2-0.9": "orbit.cpp:531 Newton (Eris's branch)",
    "ell0.9-1": "orbit.cpp:543 Laguerre-Conway elliptic",
    "ell>1": "orbit.cpp:559 Laguerre-Conway hyperbolic",
    "cometEll": "iterative_orbits.hpp:97 IterativeEll",
    "cometHyp": "iterative_orbits.hpp:41 IterativeHyp",
}


def decode(raw):
    """Greedy UTF-8 with a per-byte fallback (F70's reference decoder)."""
    out, i = [], 0
    while i < len(raw):
        for length in (4, 3, 2, 1):
            try:
                out.append(raw[i:i + length].decode("utf-8"))
                i += length
                break
            except Exception:
                pass
        else:
            out.append(chr(raw[i]))
            i += 1
    return "".join(out)


def read_ini(path):
    """section key -> {lowercased key: value}, plus '__sec__'."""
    text = decode(open(path, "rb").read())
    secs, cur = {}, None
    for line in text.split("\n"):
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
            secs[cur] = {"__sec__": cur}
        elif cur and "=" in s and not s.startswith("#"):
            k, v = s.split("=", 1)
            secs[cur][k.strip().lower()] = v.strip()
    return secs


def solver_of(sec):
    """(class, branch, eccentricity) for one field section."""
    cf = (sec.get("coord_func") or "").strip().lower()
    try:
        e = float(sec.get("orbit_eccentricity", "nan"))
    except ValueError:
        e = float("nan")
    if cf == "ell_orbit":
        if e == 0.0:
            return "NONITER", "ell e==0 (orbit.cpp:517 return M)", e
        if e < 0.2:
            return "ITER", "ell<0.2", e
        if e < 0.9:
            return "ITER", "ell0.2-0.9", e
        if e < 1.0:
            return "ITER", "ell0.9-1", e
        if e == 1.0:
            return "NONITER", "ell e==1 (orbit.cpp:547 return M)", e
        return "ITER", "ell>1", e
    if cf == "comet_orbit":
        if e < 1.0:
            return "ITER", "cometEll", e
        if e > 1.0:
            return "ITER", "cometHyp", e
        return "NONITER", "comet e==1 (ParCometOrbit)", e
    return "NONITER", cf or "(no coord_func)", e


def record_names(path):
    op = gzip.open if str(path).endswith(".gz") else open
    names = []
    with op(path, "rt", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except ValueError:
                continue
            if rec.get("type") == "body":
                names.append(rec["name"])
    return names


def census(ini_path, part_path, names):
    secs = read_ini(ini_path)
    by_name = {}
    for key, sec in secs.items():
        nm = sec.get("name")
        if nm:
            by_name[nm] = sec
    part = json.load(open(part_path))
    P, I = set(part["P"]), set(part["I"])
    rows = {}
    for n in names:
        sec = by_name.get(n)
        if sec is None:
            cls, branch, e = "NONITER", "(no ssystem.ini section)", None
            sk = None
        else:
            cls, branch, e = solver_of(sec)
            sk = sec["__sec__"]
        if n in I:
            place = "unreached"
        elif n in P:
            place = "parked"
        else:
            place = "walked"
        rows[n] = {"solver": cls, "branch": branch, "e": e, "place": place,
                   "section": sk, "parent": (sec or {}).get("parent")}
    # REACH: own solver, or any ancestor's (the eye-frame position composes the
    # chain).  Walk the field file's own `parent =` values by body NAME.
    def chain(n):
        out, x, seen = [], n, set()
        while x and x in rows and x not in seen:
            seen.add(x)
            out.append(x)
            x = rows[x]["parent"]
        return out
    for n in rows:
        anc = [a for a in chain(n)[1:] if rows.get(a, {}).get("solver") == "ITER"]
        rows[n]["iter_ancestors"] = anc
        touched = rows[n]["solver"] == "ITER" or bool(anc)
        rows[n]["touched"] = touched
        klass = ("NON-ITERATIVE" if not touched else
                 ("PARKED-ITERATIVE" if rows[n]["place"] in ("parked", "unreached")
                  else "WALKED-ITERATIVE"))
        rows[n]["class"] = klass
    return rows, by_name


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ini", default=str(DEFAULT_INI))
    ap.add_argument("--partition", default=str(DEFAULT_PART))
    ap.add_argument("--dump", default=str(DEFAULT_DUMP))
    ap.add_argument("--names", help="one record name per line, instead of --dump")
    ap.add_argument("--json", help="write the census here")
    a = ap.parse_args()
    if a.names:
        names = [l.strip() for l in open(a.names) if l.strip()]
    else:
        names = record_names(a.dump)
    rows, by_name = census(a.ini, a.partition, names)
    print("field sections with a name : %d" % len(by_name))
    print("dump records               : %d" % len(names))
    missing = sorted(n for n in names if rows[n]["section"] is None)
    print("records with NO section    : %d" % len(missing))
    for klass in ("NON-ITERATIVE", "WALKED-ITERATIVE", "PARKED-ITERATIVE"):
        sel = sorted(n for n in rows if rows[n]["class"] == klass)
        print("\n== %s  (%d)" % (klass, len(sel)))
        for n in sel:
            r = rows[n]
            print("   %-20s %-10s %-34s e=%-24s %s"
                  % (n, r["place"], r["branch"],
                     "n/a" if r["e"] is None else repr(r["e"]),
                     ("via " + ",".join(r["iter_ancestors"]))
                     if r["solver"] != "ITER" and r["iter_ancestors"] else ""))
    print("\n== iterative branch histogram (all records)")
    hist = {}
    for n, r in rows.items():
        if r["solver"] == "ITER":
            hist.setdefault(r["branch"], []).append(n)
    for b in sorted(hist):
        print("   %-12s %3d   %s" % (b, len(hist[b]), ITER_BRANCHES.get(b, "")))
    if a.json:
        json.dump({"rows": rows, "names": names}, open(a.json, "w"), indent=1)
    return 0


if __name__ == "__main__":
    sys.exit(main())
