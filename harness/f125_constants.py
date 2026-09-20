#!/usr/bin/env python3
"""F125 - the owner's declared constants, read from his header: the one place a gate takes
an expected value from (INTENT G12 (S5): his declared values are the ruling).

    from f125_constants import owner_constant
    owner_constant("BODY_EARLY_VISIBILITY_BOUNDING_SIZE")   -> 2.0

A gate that pastes a number observed in a run certifies whatever is; one that reads the
constant goes red when the code stops honouring it. A name that is no longer declared is
an error, never a default. As a command it prints the constants the gates use.
"""
import os, re, sys

HEADER = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                      "..", "..", "src", "experimentalModule", "ModularBody.hpp")
USED = ("BODY_EARLY_VISIBILITY_BOUNDING_SIZE", "BODY_FULL_VISIBILITY_BOUNDING_SIZE",
        "BODY_BIG_TEXTURE_BOUNDING_SIZE", "MIN_MOVEMENT_SPEED")


def owner_constant(name, header=HEADER):
    pat = re.compile(r"^\s*constexpr\s+(?:int|float|double)\s+" + re.escape(name)
                     + r"\s*=\s*([-+0-9.eE]+)f?\s*;")
    hits = [m.group(1) for m in map(pat.match, open(header, encoding="ascii")) if m]
    if len(hits) != 1:
        raise SystemExit("%s: %d declarations of %s (expected exactly 1)"
                         % (os.path.normpath(header), len(hits), name))
    return float(hits[0])


if __name__ == "__main__":
    for n in sys.argv[1:] or USED:
        print("%-40s = %g" % (n, owner_constant(n)))
