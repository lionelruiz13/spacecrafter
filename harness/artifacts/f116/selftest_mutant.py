#!/usr/bin/env python3
"""F116 -- THE NEW READER CHECKS, SHOWN ABLE TO FAIL (F108's shape, F101's rule).

    python3 artifacts/f116/selftest_mutant.py

Sec.11.237 widened what an archive can be: `.1.log` is the channel's previous
file, which is the previous LAUNCH until the size budget rotates a channel in
session, and an earlier slice of the RUNNING launch afterwards.  `logread.py`
answers that with `window_text`, and this puts the pre-Sec.11.237 answer back --
"the whole session is the live file" -- and runs the SAME suite against it.

The mutant must fail, and it must fail ONLY on the in-session checks: every
check that describes the launch window (F108's 25) must still pass, because this
change did not touch them.  That is the discrimination: exit 0 means the new
checks carry the new claim and nothing else.
"""
import re
import sys
import types
from pathlib import Path

HERE = Path(__file__).resolve().parent
HARNESS = HERE.parent.parent

MUTANT = '''
def window_text(logdir, channel="script", encoding="latin-1"):
    """THE PRE-Sec.11.237 ANSWER: a launch writes one file, so the session IS
    the live file.  True until a size-budget rotation moves part of it away."""
    return text(logdir, channel, encoding)
'''


def main():
    src = (HARNESS / "logread.py").read_text()
    m = re.search(r'\ndef window_text\(logdir, channel="script".*?(?=\ndef budget)',
                  src, re.S)
    if not m:
        print("FAIL: could not find window_text() to mutate - the mutant is stale")
        return 1
    mutated = src[:m.start()] + "\n" + MUTANT + src[m.end():]

    mod = types.ModuleType("logread_f116_mutant")
    mod.__file__ = str(HARNESS / "logread.py")       # so window()/budget() resolve
    exec(compile(mutated, "logread_f116_mutant", "exec"), mod.__dict__)

    print("--- the mutant's own run of the suite ---")
    rc = mod.selftest()
    if rc == 0:
        print("\nFAIL: the mutant PASSED the suite - the new checks prove nothing.")
        return 1
    print("\nok: the mutant fails the suite, so the in-session checks discriminate.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
