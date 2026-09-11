#!/usr/bin/env python3
"""F108 -- THE CAP AT ITS BOUNDARY: one launch onto a SEEDED steady state.

    cd claude/harness
    DISPLAY=:2 python3 f108_mutant.py <absOutdir> --tag A --bin <path>
    python3 f108_mutant.py <absOutdir> --tag A --bin <path> --seed-only

Counting to ten proves the cap only if the run reaches the boundary; seeding
the log directory with the steady state puts the boundary at launch ONE, and
then the arms differ by their first launch instead of their ninth.

The seed is 8 files per channel -- `<c>.log` plus `<c>.1.log` .. `<c>.7.log` --
each carrying a line that names its slot and each with a DISTINCT byte size
(5000 + 100*k), so two independent things can be read off one launch: which
seeded file now sits at which index (the slot line) and which one the rotation
deleted (the byte count in its own D12 line).

Arms, all on their own seeded farm, one launch each:
  A  the delivered binary
  B  the delete step disabled       -- predicted IDENTICAL to A (rename
                                       replaces its destination), with a D12
                                       line that then says "deleted nothing"
  C  the shift made unbounded       -- predicted 9 files, `<c>.8.log` created
"""
import argparse
import hashlib
import json
import re
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, build_farm, no_instance                    # noqa: E402
import logread                                                         # noqa: E402

D12_KEY = "Log retention ("
SEED_RE = re.compile(r"F108SEED slot=(\S+)")
FAILS = []


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def seed(logdir, window):
    """8 files per channel, tagged by slot, each a different size."""
    logdir = Path(logdir)
    logdir.mkdir(parents=True, exist_ok=True)
    made = {}
    for c in logread.CHANNELS:
        for k in ["live"] + list(range(1, window)):
            name = "%s.log" % c if k == "live" else "%s.%s.log" % (c, k)
            n = 5000 + 100 * (0 if k == "live" else k)
            body = "F108SEED slot=%s\n" % k
            body += "x" * (n - len(body) - 1) + "\n"
            (logdir / name).write_text(body)
            made[name] = (n, k)
    return made


def slots(logdir, c, window):
    """What slot line each file of this channel's family carries now."""
    out = {}
    for p in [logdir / ("%s.log" % c)] + [logdir / ("%s.%d.log" % (c, k))
                                          for k in range(1, window + 3)]:
        if not p.is_file():
            continue
        head = p.read_bytes()[:200].decode("latin-1", errors="replace")
        m = SEED_RE.search(head)
        out[p.name] = m.group(1) if m else "(written by the launch)"
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--seed-only", action="store_true")
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    window = logread.window()

    farm = out / ("farm_" + a.tag)
    home = build_farm(farm)
    made = seed(home / "log", window)
    print("arm %s: binary %s md5 %s" % (a.tag, a.bin, md5(a.bin)))
    print("  seeded %d files (%d per channel), window %d"
          % (len(made), len(made) // len(logread.CHANNELS), window))
    if a.seed_only:
        return 0

    hits = no_instance()
    if hits:
        raise RuntimeError("another spacecrafter is running: %s" % hits)
    app = App(a.bin, farm, out, "arm" + a.tag)
    t = app.start()
    rc = app.stop()
    time.sleep(0.5)

    logdir = home / "log"
    rec = {"arm": a.tag, "bin": a.bin, "md5": md5(a.bin), "rc": rc,
           "port_s": t, "window": window, "channels": {}}
    print("  rc=%s" % rc)
    for c in logread.CHANNELS:
        fam = [logdir / ("%s.log" % c)] + logread.archives(logdir, c)
        fam = [p for p in fam if p.is_file()]
        sl = slots(logdir, c, window)
        rec["channels"][c] = {
            "count": len(fam),
            "names": [p.name for p in fam],
            "slots": sl,
            "sizes": {p.name: p.stat().st_size for p in fam},
        }
        print("  %-13s %d file(s): %s" % (c, len(fam), " ".join(p.name for p in fam)))
        print("  %-13s slots: %s" % ("", ", ".join("%s=%s" % (k, v)
                                                   for k, v in sorted(sl.items()))))
    rec["d12"] = [l for l in logread.text(logdir, "spacecrafter").splitlines()
                  if D12_KEY in l]
    print("  D12 deletion clauses:")
    for l in rec["d12"]:
        i = l.find(", deleted ")
        print("    " + (l[i + 2:i + 80] if i > 0 else l[:80]))
    # Did any seeded slot leave the directory entirely?
    alive = set()
    for c in logread.CHANNELS:
        alive |= {v for v in rec["channels"][c]["slots"].values()}
    rec["seed_slots_alive"] = sorted(alive)
    print("  seed slots still present: %s" % rec["seed_slots_alive"])
    (out / ("arm_%s.json" % a.tag)).write_text(json.dumps(rec, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
