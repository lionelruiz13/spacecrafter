#!/usr/bin/env python3
"""F59 -- the enumerator demonstrated ABLE TO FAIL, both directions, before it is
trusted.  A green that cannot discriminate is not evidence.

Four checks, each with its prediction committed in the assertion itself:

  P1 POSITIVE   an injected in-window uppercase-keyword line MUST produce its pair.
  P2 NEGATIVE   the same sentence with the keyword in LOWERCASE must NOT (the frozen
                lexicon is uppercase-exact -- `intent_backmarker_scan.py:30`).
  P3 NEGATIVE   the same sentence with the citation pushed BEYOND 160 characters of the
                keyword must NOT (the window is the boundary, not the sentence).
  P4 HOMES      `homes("5.31")` must return the ARCHIVED entry file and NOT a §11 stub,
                while the frozen instrument's `stub("5.31")` returns §11.31's stub line
                -- the stub-collision defect (§11.165 fourth-property note) shown firing
                on this audit's own axis, which is why half 2 was re-implemented.

Usage: python3 f59_enum_check.py <root>          (root required, §11.168(m))
"""
import contextlib, io, os, re, shutil, subprocess, sys, tempfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else sys.exit("usage: f59_enum_check.py <root>")
HERE = os.path.dirname(os.path.abspath(__file__))
ENUM = os.path.join(HERE, "f59_enum.py")


def pairs_of(root):
    out = subprocess.run([sys.executable, ENUM, root], capture_output=True, text=True)
    if out.returncode:
        sys.exit("enumerator failed on %s:\n%s" % (root, out.stderr))
    rows = out.stdout.splitlines()[1:]
    return {tuple(r.split("\t")[:2]) for r in rows if r.strip()}


base = pairs_of(ROOT)
tmp = tempfile.mkdtemp(prefix="f59chk-")
fails = []


def variant(name, sentence, expect):
    d = os.path.join(tmp, name)
    shutil.copytree(ROOT, d, ignore=shutil.ignore_patterns(".git", "build*", "artifacts"))
    p = os.path.join(d, "INTENT", "11.99.md")
    with open(p, "a", encoding="utf-8") as f:
        f.write("\n" + sentence + "\n")
    got = pairs_of(d)
    present = ("11.99", "5.79") in got
    ok = present == expect
    print("  %-4s %-9s pair(11.99,5.79) present=%s expected=%s  delta_pairs=%+d  %s"
          % (name, "POSITIVE" if expect else "NEGATIVE", present, expect,
             len(got) - len(base), "OK" if ok else "*** FAIL ***"))
    if not ok:
        fails.append(name)


print("BASE: %d pairs with a §5 target on %s" % (len(base), ROOT))
variant("P1", "    - **(z) the claim is REFUTED**: see §5.79 for the criterion.", True)
variant("P2", "    - **(z) the claim is refuted**: see §5.79 for the criterion.", False)
variant("P3", "    - **(z) the claim is REFUTED**" + (" filler" * 40) + " see §5.79.", False)

# P4 -- home resolution vs the frozen instrument's stub()
sys.path.insert(0, ROOT)
import importlib.util
spec = importlib.util.spec_from_file_location(
    "scan_probe", os.path.join(ROOT, "intent_backmarker_scan.py"))
mod = importlib.util.module_from_spec(spec)
sys.argv = ["scan_probe", ROOT]
with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
    spec.loader.exec_module(mod)      # runs the scan; we only want its stub()
    spec2 = importlib.util.spec_from_file_location("f59enum", ENUM)
    sys.argv = ["f59enum", ROOT]
    enum = importlib.util.module_from_spec(spec2)
    spec2.loader.exec_module(enum)

frozen = mod.stub("5.31")
mine = enum.homes("5.31")
kinds = [k for k, _, _ in mine]
print("\n  P4   HOMES    homes('5.31') = %s" % kinds)
print("       frozen stub('5.31') -> %r" % frozen[:90])
p4_ok = kinds == ["archived-entry"] and frozen.startswith("31. ") and "§5.31" not in frozen
print("       %s (this audit re-implements half 2 because of exactly this)"
      % ("OK -- the frozen lookup returns §11.31's stub, mine returns the real home"
         if p4_ok else "*** FAIL ***"))
if not p4_ok:
    fails.append("P4")

shutil.rmtree(tmp, ignore_errors=True)
print("\n%s" % ("ALL CHECKS PASS" if not fails else "FAILED: " + ",".join(fails)))
sys.exit(1 if fails else 0)
