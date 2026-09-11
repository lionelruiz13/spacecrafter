#!/usr/bin/env python3
"""F108 -- `logread.py selftest` SHOWN ABLE TO FAIL (the f101 shape).

    python3 artifacts/f108/selftest_mutant.py

A suite that only ever passes is not evidence.  This takes `logread.py`,
replaces `live()` with the rule the six drivers actually had before F108 --
`sorted(glob("script-*.log"))[-1]`, the dash-requiring glob -- and runs the
SAME 25 checks against it.  The mutant must FAIL, and it must fail on the
checks that describe the new layout, which is what says those checks are the
ones carrying the change.

Exit 0 means the suite discriminates (mutant failed).  Exit 1 means it does
not (mutant passed) -- which would make every green run of the real suite
meaningless.
"""
import re
import sys
import types
from pathlib import Path

HERE = Path(__file__).resolve().parent
HARNESS = HERE.parent.parent

MUTANT_LIVE = '''
def live(logdir, channel="script"):
    """THE PRE-F108 RULE, verbatim in shape: the last of the dated files.
    (f61/f62/f63/f67/f68/f69: `sorted((sc/"log").glob("script-*.log"))[-1]`.)"""
    d = Path(logdir)
    if not d.is_dir():
        return None
    logs = sorted(d.glob(channel + "-*.log"))
    return logs[-1] if logs else None
'''


def main():
    src = (HARNESS / "logread.py").read_text()
    # Cut the real live() out (from its def to the next top-level def) and put
    # the pre-F108 rule in its place.
    m = re.search(r'\ndef live\(logdir, channel="script"\):.*?(?=\ndef history)',
                  src, re.S)
    if not m:
        print("FAIL: could not find live() to mutate - the mutant is stale")
        return 1
    mutated = src[:m.start()] + "\n" + MUTANT_LIVE + src[m.end():]

    mod = types.ModuleType("logread_mutant")
    mod.__file__ = str(HARNESS / "logread.py")       # so window() still resolves
    exec(compile(mutated, "logread_mutant", "exec"), mod.__dict__)

    print("--- the mutant's own run of the 25 checks ---")
    rc = mod.selftest()
    if rc == 0:
        print("\nFAIL: the mutant PASSED the suite - the suite proves nothing.")
        return 1
    print("\nok: the mutant fails the suite, so the suite discriminates.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
