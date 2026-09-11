#!/usr/bin/env python3
"""F108 -- A LAUNCH REFUSED AS A SECOND INSTANCE HAS ALREADY ROTATED THE LOGS.

    cd claude/harness && python3 artifacts/f108/lockrefusal.py <absOutdir> <bin>

This is the experiment behind the choice of flush point.  main.cpp opens the
five channels at :215-219 and the single-instance check is 30 lines lower, at
:241-250, where a second instance writes one warning and `return 0`.  So the
rotation - including the DELETE - has already happened when a launch is
refused.  The report is flushed at :220, above that return, which is why the
deletion is still on record for a launch that did nothing else.

The lock is made VALID the only way the engine accepts (`is_lock_file` shells
out `kill -0 <pid>`, main.cpp:160-162): a live process of our own, whose pid
goes into /tmp/spacecrafter.lock.  No second instance of spacecrafter is ever
started, so the concurrent-instance rule is not bent.

Predicted, before the run: 8 files per channel (the seeded steady state,
rotated once), the seeded slot-7 file GONE, five D12 lines carrying the
deletion clause with 5700 bytes, the `New instance aborded` warning, exit 0 in
about a second, and the lock file left alone (the refusing path does not call
remove_lock_file).
"""
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent.parent))

import logread                                                        # noqa: E402
import f108_mutant as mut                                             # noqa: E402
from f96_offset import build_farm, no_instance                        # noqa: E402

LOCK = Path("/tmp/spacecrafter.lock")


def main():
    out, binary = Path(sys.argv[1]), sys.argv[2]
    out.mkdir(parents=True, exist_ok=True)
    if LOCK.exists():
        print("FAIL: %s already exists - not touching it" % LOCK)
        return 1
    hits = no_instance()
    if hits:
        print("FAIL: another spacecrafter is running: %s" % hits)
        return 1

    farm = out / "farm_lock"
    home = build_farm(farm)
    window = logread.window()
    mut.seed(home / "log", window)
    before = {c: mut.slots(home / "log", c, window) for c in logread.CHANNELS}

    holder = subprocess.Popen(["sleep", "45"])
    LOCK.write_text("%d\n" % holder.pid)
    print("lock holder pid %d, /tmp/spacecrafter.lock written" % holder.pid)
    try:
        t0 = time.time()
        with open(out / "lock.applog", "w") as fh:
            p = subprocess.run([binary], cwd=str(home), stdout=fh,
                               stderr=subprocess.STDOUT,
                               env={**__import__("os").environ,
                                    "HOME": str(farm)}, timeout=120)
        dt = time.time() - t0
        print("refused launch: exit %s after %.2f s" % (p.returncode, dt))
    finally:
        holder.kill()
        lock_survived = LOCK.exists()
        LOCK.unlink(missing_ok=True)

    logdir = home / "log"
    applog = (out / "lock.applog").read_text(encoding="latin-1", errors="replace")
    d12 = [l for l in logread.text(logdir, "spacecrafter").splitlines()
           if "Log retention (" in l]
    rc = 0
    for c in logread.CHANNELS:
        fam = logread.history(logdir, c)
        after = mut.slots(logdir, c, window)
        alive = set(after.values())
        print("  %-13s %d file(s); slot 7 present: %s"
              % (c, len(fam), "7" in alive))
        if len(fam) != window:
            print("FAIL: %s has %d file(s), expected %d" % (c, len(fam), window))
            rc = 1
        if "7" in alive:
            print("FAIL: %s still carries the seeded slot 7" % c)
            rc = 1
    print("  D12 lines in the refused launch's own log: %d" % len(d12))
    deleted = [l for l in d12 if "deleted nothing" not in l and ", deleted " in l]
    print("  of which carry a deletion clause: %d" % len(deleted))
    for l in deleted[:1]:
        i = l.find(", deleted ")
        print("    " + l[i + 2:i + 90])
    # The refusal warning itself: main.cpp:249 writes it through cLog, so it
    # reaches the LOG FILE, and it reaches the console only if isDebug is set -
    # which happens at :268, nineteen lines BELOW the return.  Measured on both
    # binaries, because this is the pre-existing shape and not F108's.
    log_refusal = [l for l in logread.text(logdir, "spacecrafter").splitlines()
                   if "New instance aborded" in l]
    con_refusal = [l for l in applog.splitlines() if "New instance aborded" in l]
    print("  refusal line in the log file: %d ; on the console: %d"
          % (len(log_refusal), len(con_refusal)))
    print("  last console line: %r"
          % (applog.splitlines()[-1][:70] if applog.splitlines() else ""))
    print("  lock file survived the refusal: %s" % lock_survived)
    if len(deleted) != len(logread.CHANNELS) or not log_refusal or not lock_survived:
        print("FAIL: the refused launch did not record what it deleted")
        rc = 1
    print("\n%s" % ("0 FAIL" if rc == 0 else "FAILED"))
    return rc


if __name__ == "__main__":
    sys.exit(main())
