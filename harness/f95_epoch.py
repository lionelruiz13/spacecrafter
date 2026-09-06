#!/usr/bin/env python3
"""F95 - what moved at a PINNED simulation clock, and how far (Sec.5.62's class).

    ./f95_epoch.py <absOutdir> [--ref 2]

The soak's cycle-boundary interlude pins the clock to ONE Julian day (J0) and
takes a dual dump there, every cycle.  Anything in the expected-constant set
that differs between two of those dumps is a same-binary, same-session,
same-simulation-instant change - which is the shape Sec.5.62 records at a much
larger magnitude and which nothing has ever measured at this one.

This reads the dumps back and answers four questions with numbers:

  1. WHICH bodies move, and on WHICH half (old / new)?  The halves are read
     separately because the two paths compute the same quantity by different
     code, and an asymmetry between them IS the attribution.
  2. HOW FAR, in absolute AU and in units of the last place of the larger
     value, so the number can be compared with Sec.11.87(c)'s measured
     cross-launch Kepler float floor (<= 2e-13 AU, up to 1e-11 AU on moons).
  3. HOW MANY DISTINCT VALUES does each body take?  Two stable values that
     alternate is a different fact from a drift, and only counting says which.
  4. WHAT DISTINGUISHES the bodies that move?  The candidate is orbital
     ECCENTRICITY, and it is read from the INSTALLED data file - a cited
     fetch, never recall (Sec.11.51(d)).
"""

import argparse
import json
import math
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import dumpread                                       # noqa: E402

REAL_SSYSTEM = Path.home() / ".spacecrafter" / "ssystem.ini"


def ulps(a, b):
    """Distance in representable doubles between a and b.  A float difference
    is only meaningful next to the spacing of the numbers it sits between."""
    if a == b:
        return 0
    if math.isnan(a) or math.isnan(b) or math.isinf(a) or math.isinf(b):
        return None
    import struct
    ia = struct.unpack("<q", struct.pack("<d", a))[0]
    ib = struct.unpack("<q", struct.pack("<d", b))[0]
    if (ia < 0) != (ib < 0):
        return abs(ia) + abs(ib)
    return abs(ia - ib)


def load_bodies(path):
    """-> {name: record} over every `type=body` line, both halves kept."""
    out = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        if not line.strip():
            continue
        try:
            rec = dumpread.loads(line)
        except Exception:                                         # noqa: BLE001
            continue
        if rec.get("type") == "body" and "name" in rec:
            out[rec["name"]] = rec
    return out


def eccentricities():
    """The INSTALLED ssystem.ini, read.  ISO-8859 and untracked: the Bash grep
    wrapper skips it silently (CLAUDE.md's encoding hazard), so it is read
    here as bytes and decoded per line."""
    ecc, cur = {}, None
    if not REAL_SSYSTEM.is_file():
        return ecc
    for raw in REAL_SSYSTEM.read_bytes().split(b"\n"):
        s = raw.decode("latin-1").strip()
        m = re.match(r"^\[(.+)\]$", s)
        if m:
            cur = m.group(1)
            continue
        m = re.match(r"^\s*name\s*=\s*(.+?)\s*$", s)
        if m and cur is not None:
            cur = m.group(1)
            continue
        m = re.match(r"^\s*orbit_[Ee]ccentricity\s*=\s*([0-9.eE+-]+)\s*$", s)
        if m and cur is not None:
            try:
                ecc[cur] = float(m.group(1))
            except ValueError:
                pass
    return ecc


def flatten(v, prefix=""):
    """-> {path: float} over a nested list/dict of numbers."""
    out = {}
    if isinstance(v, (int, float)) and not isinstance(v, bool):
        out[prefix] = float(v)
    elif isinstance(v, list):
        for i, x in enumerate(v):
            out.update(flatten(x, "%s[%d]" % (prefix, i)))
    elif isinstance(v, dict):
        for k, x in v.items():
            out.update(flatten(x, "%s.%s" % (prefix, k) if prefix else k))
    return out


# the same set prediction.txt names, split by half
OLD_FIELDS = ["ecl", "rotLocalToParent", "rotLocalToParentUnprecessed",
              "matLocalToParent"]
NEW_FIELDS = ["ecl"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--ref", type=int, default=2,
                    help="reference cycle (default 2; cycle 1 is first-touch)")
    a = ap.parse_args()
    out = Path(a.out)
    files = sorted(f for f in out.glob("cycle_*.json") if f.stat().st_size > 0)
    if len(files) < 3:
        print("need at least three non-empty cycle dumps, have %d" % len(files))
        return 1
    byn = {int(re.search(r"cycle_(\d+)", f.name).group(1)): f for f in files}
    ref_n = a.ref if a.ref in byn else min(byn)
    ref = load_bodies(byn[ref_n])

    print("=" * 96)
    print("F95 - THE PINNED-CLOCK DUMP SERIES (Sec.5.62's class, at ulp scale)")
    print("dumps %d   reference cycle %d   bodies in reference %d"
          % (len(files), ref_n, len(ref)))
    print("=" * 96)

    # per body per half: the set of distinct value-tuples seen, and the worst
    # absolute / ulp distance from the reference.
    seen = {}          # (name, half, field) -> {tuple: [cycles]}
    worst = {}         # (name, half) -> (abs, ulp, field, index)
    sizes = {}
    for n in sorted(byn):
        f = byn[n]
        sizes.setdefault(f.stat().st_size, []).append(n)
        bodies = load_bodies(f)
        for name, rec in bodies.items():
            if name not in ref:
                continue
            for half, fields in (("old", OLD_FIELDS), ("new", NEW_FIELDS)):
                h, rh = rec.get(half) or {}, ref[name].get(half) or {}
                for fld in fields:
                    if fld not in h or fld not in rh:
                        continue
                    fa, fb = flatten(rh[fld], fld), flatten(h[fld], fld)
                    key = (name, half, fld)
                    seen.setdefault(key, {}).setdefault(
                        tuple(sorted(fb.items())), []).append(n)
                    for k in fa:
                        if k not in fb or fa[k] == fb[k]:
                            continue
                        d = abs(fa[k] - fb[k])
                        u = ulps(fa[k], fb[k])
                        cur = worst.get((name, half))
                        if cur is None or d > cur[0]:
                            worst[(name, half)] = (d, u, k, fa[k], fb[k])

    print("\n--- DUMP FILE SIZES (a changed printed digit changes the size) ---")
    for s in sorted(sizes):
        print("  %d B : %d dump(s)  cycles %s"
              % (s, len(sizes[s]), sizes[s][:14]))

    print("\n--- WHICH BODIES MOVE, ON WHICH HALF, AND HOW FAR ---")
    print("  %-14s %-5s %-11s %-11s %-28s %s"
          % ("body", "half", "max |delta|", "ulps", "field[index]", "distinct values"))
    movers = sorted(worst, key=lambda k: -worst[k][0])
    for (name, half) in movers:
        d, u, k, va, vb = worst[(name, half)]
        nvals = max(len(v) for (n2, h2, _f), v in seen.items()
                    if n2 == name and h2 == half)
        print("  %-14s %-5s %-11.3e %-11s %-28s %d"
              % (name, half, d, u, k, nvals))
    if not movers:
        print("  NONE - every body's expected-constant fields are byte-identical "
              "across every dump, on both halves.")

    old_movers = sorted({n for (n, h) in movers if h == "old"})
    new_movers = sorted({n for (n, h) in movers if h == "new"})
    print("\n  OLD half moves for %d body/ies: %s" % (len(old_movers), old_movers))
    print("  NEW half moves for %d body/ies: %s"
          % (len(new_movers), new_movers or "NONE"))
    print("  bodies compared: %d   so the NEW path is bit-stable on %d of %d"
          % (len(ref), len(ref) - len(new_movers), len(ref)))

    print("\n--- HOW MANY DISTINCT VALUES (a coin flip is not a drift) ---")
    for name in old_movers:
        for (n2, h2, fld), vals in sorted(seen.items()):
            if n2 != name or h2 != "old" or len(vals) < 2:
                continue
            occ = sorted(((len(cs), min(cs)) for cs in vals.values()),
                         reverse=True)
            print("  %-14s old.%-26s %d distinct value(s), occurrences %s"
                  % (name, fld, len(vals), [o[0] for o in occ]))
            if fld == "ecl":
                for j, (tup, cs) in enumerate(
                        sorted(vals.items(), key=lambda kv: -len(kv[1]))):
                    print("      value %d on cycles %s" % (j + 1, sorted(cs)))

    print("\n--- THE TWO VALUES, VERBATIM, FOR THE WIDEST MOVER ---")
    if movers:
        name, half = movers[0]
        for (n2, h2, fld), vals in sorted(seen.items()):
            if n2 == name and h2 == half and fld == "ecl" and len(vals) == 2:
                for j, (tup, cs) in enumerate(
                        sorted(vals.items(), key=lambda kv: -len(kv[1]))):
                    print("  %s.%s.%s value %d (%d cycles): %s"
                          % (name, half, fld, j + 1, len(cs),
                             [v for _k, v in tup]))
                break

    print("\n--- WHAT DISTINGUISHES THEM: ORBITAL ECCENTRICITY ---")
    print("  read from %s - a cited fetch, never recall (Sec.11.51(d))" % REAL_SSYSTEM)
    ecc = eccentricities()
    print("  eccentricities parsed from the installed file: %d bodies" % len(ecc))
    have = {n: ecc[n] for n in ref if n in ecc}
    if have:
        mv = {n: have[n] for n in old_movers if n in have}
        st = {n: have[n] for n in have if n not in old_movers}
        print("  MOVERS   (%d): %s"
              % (len(mv), sorted(mv.items(), key=lambda x: -x[1])))
        if st:
            top = sorted(st.items(), key=lambda x: -x[1])[:8]
            print("  the eight most eccentric bodies that DID NOT move: %s" % top)
            print("  -> movers' min eccentricity %.4f ; non-movers' max %.4f"
                  % (min(mv.values()) if mv else float("nan"),
                     max(st.values())))
        missing = [n for n in old_movers if n not in have]
        if missing:
            print("  NOT FOUND in the data file under that name: %s" % missing)
    print("\n" + "=" * 96)
    return 0


if __name__ == "__main__":
    sys.exit(main())
