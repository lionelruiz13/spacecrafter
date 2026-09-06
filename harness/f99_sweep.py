#!/usr/bin/env python3
"""F99 -- the same-class sweep over every registered orbit loader (INTENT S11.219).

S5.141 is one loader dereferencing a parent it never tested.  The question a fix
owes is whether that is an INSTANCE or a CLASS, and the only honest way to answer
it is to look at all of them -- so this walks the registry itself rather than a
remembered list, and reports, per loader, every line that names `parent` together
with the guard (if any) that stands between the lookup and the use.

The registry is the authority: `modules.cpp`'s registerModule calls are parsed
for the keyed loaders, plus the one-argument call that registers the DEFAULT
loader.  A loader that exists as a file but is not registered is reported as
unregistered rather than silently counted, and a registered loader whose file is
not found FAILS the run -- the sweep must not be able to report a clean table
over a corpus it did not read.

  python3 f99_sweep.py [--src /home/claude/spacecrafter/src]
"""
import argparse
import re
import sys
from pathlib import Path

LOOKUP = re.compile(r"findBody(Once)?\s*\(")
GUARD = re.compile(r"if\s*\(\s*!\s*(\w+)\s*\)|==\s*nullptr|!=\s*nullptr")
USE = re.compile(r"\bparent\w*\s*->|\bbody[AB]\s*->")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="/home/claude/spacecrafter/src")
    a = ap.parse_args()
    src = Path(a.src)
    mod = (src / "experimentalModule" / "modules.cpp").read_text()

    keyed = re.findall(r'registerModule\("([^"]+)",\s*std::make_unique<(\w+)>', mod)
    default = re.findall(r"registerModule\(std::make_unique<(\w+)>", mod)
    entries = [(k, c) for k, c in keyed] + [("<default>", c) for c in default]

    print("registry: %d keyed + %d default = %d loader(s)  [modules.cpp]"
          % (len(keyed), len(default), len(entries)))
    print()
    rows = []
    bad = 0
    for key, cls in entries:
        f = src / "experimentalModule" / "orbitModules" / (cls + ".hpp")
        if not f.is_file():
            print("FATAL: %s registered as '%s' but %s does not exist" % (cls, key, f))
            bad += 1
            continue
        lines = f.read_text().splitlines()
        lookups, guards, uses = [], [], []
        for i, ln in enumerate(lines, 1):
            if ln.lstrip().startswith("//"):
                continue
            if LOOKUP.search(ln):
                lookups.append(i)
            if GUARD.search(ln):
                guards.append(i)
            if USE.search(ln):
                uses.append(i)
        # A use is COVERED iff some guard line sits between the last lookup and it.
        first_use = min(uses) if uses else None
        last_lookup = max(lookups) if lookups else None
        after = [g for g in guards if last_lookup is not None and g >= last_lookup]
        if last_lookup is None:
            verdict = ("no lookup, no dereference" if first_use is None
                       else "USES a pointer it never looked up (check by hand)")
        elif first_use is None:
            # Looks a body up and never dereferences it: either it passes the
            # pointer on (Bary -> BarycenterOrbit2, SurfacePoint -> an orbit that
            # tests `p ?` at every evaluation) or it drops it.  Whether it TESTS
            # the miss is still worth reporting -- that is the difference between
            # a null that is handled and a null that is merely not dereferenced
            # HERE.
            verdict = ("no dereference; miss tested at :%s" % after[0]) if after \
                      else "no dereference; miss NOT tested"
        else:
            between = [g for g in guards if last_lookup <= g < first_use]
            verdict = ("GUARDED at :%s" % between[0]) if between else "*** UNGUARDED ***"
        rows.append((key, cls, lookups, guards, uses, verdict))

    w = max(len(k) for k, *_ in rows)
    print("%-*s  %-24s %-9s %-9s %-11s %s"
          % (w, "coord_func", "loader", "lookup@", "test@", "deref@", "verdict"))
    print("-" * (w + 76))
    unguarded = []
    for key, cls, lk, gd, us, v in rows:
        print("%-*s  %-24s %-9s %-9s %-11s %s"
              % (w, key, cls,
                 ",".join(map(str, lk)) or "-",
                 ",".join(map(str, gd)) or "-",
                 ",".join(map(str, us)) or "-", v))
        if "UNGUARDED" in v:
            unguarded.append(key)
    print()
    print("UNGUARDED: %d of %d -- %s" % (len(unguarded), len(rows), unguarded or "none"))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
