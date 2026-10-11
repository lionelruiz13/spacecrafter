#!/usr/bin/env python3
"""F147 census helper (read-only): search the ledger set for a regex and print
path:line plus a context snippet around each match.

usage: lf.py [-i] [-c N] [-m MAX] [-r] [-s SET] PATTERN [PATTERN2 ...]
  every PATTERN must match the same line (AND); the snippet is centred on the first.
  -r   also require a ruling marker on the line (vixy|owner|RULED|ANSWERED|RATIFIED|his word|his words|your word)
  -s   file set: L (ledger, default) | A (ledger + dispatch archive + channels) | Q (queue only)
"""
import glob, os, re, sys

ROOT = "/home/claude/spacecrafter/claude"
SH = "/home/claude/shared"
LEDGER = ([f"{ROOT}/INTENT.md"] + sorted(glob.glob(f"{ROOT}/INTENT/*.md"))
          + sorted(glob.glob(f"{ROOT}/INTENT/archive/*.md"))
          + [f"{ROOT}/DECISIONS_PENDING.md"] + sorted(glob.glob(f"{ROOT}/DECISIONS_PENDING/archive/*.md"))
          + sorted(glob.glob(f"{SH}/QUEUE/Q-*.md")) + sorted(glob.glob(f"{SH}/QUEUE/archive/Q-*.md")))
CHANNELS = [f"{ROOT}/{n}" for n in ("HOST-EVENTS.md", "FEATURE_REQUESTS.md", "SCRIPT_SURFACE.md",
                                    "USER_QUESTIONS.md", "DEPLOYMENT-MAP.md", "SHAPE.md", "README.md",
                                    "harness/README.md", "fable-dispatch.md")]
CHANNELS += sorted(glob.glob(f"{ROOT}/USER_QUESTIONS*.md")) + sorted(glob.glob(f"{ROOT}/fable-dispatch/archive/*.md"))
RULING = re.compile(r"vixy|owner|RULED|ANSWERED|RATIFIED|his word|your word|CONFIRMED", re.I)


def main(argv):
    ci, ctx, mx, need, fset = False, 160, 40, False, "L"
    pats = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "-i": ci = True
        elif a == "-r": need = True
        elif a == "-c": ctx = int(argv[i + 1]); i += 1
        elif a == "-m": mx = int(argv[i + 1]); i += 1
        elif a == "-s": fset = argv[i + 1]; i += 1
        else: pats.append(a)
        i += 1
    flags = re.I if ci else 0
    rx = [re.compile(p, flags) for p in pats]
    files = {"L": LEDGER, "A": LEDGER + CHANNELS, "Q": [f for f in LEDGER if "/QUEUE/" in f]}[fset]
    n = 0
    seen = set()
    for f in files:
        if f in seen or not os.path.exists(f):
            continue
        seen.add(f)
        with open(f, "rb") as fh:
            for ln, raw in enumerate(fh, 1):
                line = raw.decode("utf-8", "replace").rstrip("\n")
                m = rx[0].search(line)
                if not m or not all(r.search(line) for r in rx[1:]):
                    continue
                if need and not RULING.search(line):
                    continue
                a, b = max(0, m.start() - ctx), min(len(line), m.end() + ctx)
                rel = f.replace(ROOT + "/", "").replace(SH + "/", "~/shared/")
                print(f"{rel}:{ln}: ...{line[a:b]}...")
                n += 1
                if n >= mx:
                    print(f"[stopped at {mx}]")
                    return 0
    print(f"[{n} hit(s)]")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
