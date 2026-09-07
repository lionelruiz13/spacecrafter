#!/usr/bin/env python3
"""F100 -- THE PARTITION KEY, and it is deliberately blind to freshness.

INTENT 5.139 / 11.216(j1).  F96 measured that 48 of 120 dump records keep a
byte-identical new-path `mat` translation across a camera move.  A partition
built from that measurement cannot then be used to explain it; this module
builds the partition from fields the dump carries for its OWN reasons and that
no freshness computation touches:

  class P (PARKED)     -- effective renderHidden: the record's own `relation`
                          is a BodyRelation::HIDDEN_* value (< 3, see
                          ModularBody.hpp:323-330) or any ancestor's is,
                          walking the `parent` chain.  `relation` is the
                          DECLARED value (11.117(b)), so the chain walk is what
                          reproduces `renderHidden`'s OR-with-ancestors.
  class I (UNREACHED)  -- eye-frame position exactly (0,0,0), i.e. `dist` == 0:
                          a `mat` that no update walk and no useNow() ever
                          wrote.  On the shipped corpus these are the other
                          systems, the Universe subtree and the anchor bodies;
                          the dump's second loop (ssystem_factory.cpp:1223-1245)
                          does not call useNow() for the new-only records, and
                          the isolation stop (ModularSystem.cpp:196) keeps the
                          walk out of them.

THE KEY IS UNCHANGED SINCE F105 AND ITS MEMBERSHIP IS NOT (INTENT 11.226).  That
second loop DOES call useNow() as of code 318c0c8b, so a dump is a use for 120
of 120 records.  Two of the anchor bodies -- baryEarthMoon (under the walked
Earth) and orbit_autour_lune (under the walked Moon) -- are served by it and
LEAVE class I: on a post-F105 launch dump |I| is 27 where 11.220(c) recorded 29,
and |P n I| is 8 where it recorded 10.  That is the instrument reporting a real
change, not drifting: `dist == 0` still means exactly "a position no walk and no
use ever wrote", and it still means it for the eight that remain, because the
barrier REFUSES a use it has no frame for (a parent the loaded system's walk
never visits has never published one) instead of refreshing them into the
identity frame.  Had the call landed without that precondition, six of the eight
would have left class I with a plausible-looking wrong position and the key
would have stopped meaning what it says -- which is why the two changes are one
delivery.  Scoring an F96-era report against a post-F105 dump therefore compares
29 against 27 BY CONSTRUCTION; use the dump that belongs to the run.

Neither key reads a second dump.  `python3 f100_partition.py <dump.json[.gz]>`
prints the two classes; `--against <report>` scores them against an F96-shaped
"frozen: a, b, c" line, which is the only place the freshness measurement is
allowed to meet them.
"""
import argparse
import gzip
import json
import re
import sys


def load_bodies(path):
    """Every `body` record of a dual dump, by name.  `hops` lines also carry a
    "name" key (F97's gotcha) -- key on "type"."""
    op = gzip.open if str(path).endswith(".gz") else open
    out = {}
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
                out[rec["name"]] = rec
    return out


def new_half(rec):
    return rec.get("new") or {}


def parked(bodies):
    """class P: effective renderHidden, by relation + the parent chain."""
    def eff(name, seen=None):
        seen = seen or set()
        if name in seen:              # a cycle would be a data defect; say no
            return False
        seen.add(name)
        nh = new_half(bodies.get(name, {}))
        rel = nh.get("relation")
        if rel is not None and rel < 3:
            return True
        parent = nh.get("parent")
        if parent and parent in bodies and parent != name:
            return eff(parent, seen)
        return False
    return set(n for n in bodies if eff(n))


def unreached(bodies):
    """class I: an eye-frame position that was never written."""
    return set(n for n in bodies if new_half(bodies[n]).get("dist") == 0)


def partition(bodies):
    P, I = parked(bodies), unreached(bodies)
    return {"P": sorted(P), "I": sorted(I), "P_only": sorted(P - I),
            "I_only": sorted(I - P), "both": sorted(P & I),
            "union": sorted(P | I), "n": len(bodies)}


def frozen_from_report(path):
    """The 'frozen: a, b, c' line an F96-shaped report writes."""
    text = open(path, errors="replace").read()
    m = re.search(r"frozen: (.*)", text)
    if not m:
        raise ValueError("no 'frozen:' line in %s" % path)
    return set(x.strip() for x in m.group(1).split(",") if x.strip())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dump")
    ap.add_argument("--against", help="an F96-shaped report with a 'frozen:' line")
    ap.add_argument("--json", help="write the partition here")
    a = ap.parse_args()
    b = load_bodies(a.dump)
    p = partition(b)
    print("records            %d" % p["n"])
    print("class P (parked)   %d" % len(p["P"]))
    print("class I (unreached)%d" % len(p["I"]))
    print("P n I              %d" % len(p["both"]))
    print("P u I              %d" % len(p["union"]))
    print("P \\ I  (%2d): %s" % (len(p["P_only"]), " ".join(p["P_only"])))
    print("I \\ P  (%2d): %s" % (len(p["I_only"]), " ".join(p["I_only"])))
    rc = 0
    if a.against:
        fr = frozen_from_report(a.against)
        u = set(p["union"])
        print("frozen in report   %d" % len(fr))
        print("predicted \\ frozen : %s" % (" ".join(sorted(u - fr)) or "(none)"))
        print("frozen \\ predicted : %s" % (" ".join(sorted(fr - u)) or "(none)"))
        if u == fr:
            print("MATCH: the partition reproduces the frozen set exactly")
        else:
            print("MISMATCH")
            rc = 1
    if a.json:
        json.dump(p, open(a.json, "w"), indent=1)
    return rc


if __name__ == "__main__":
    sys.exit(main())
