#!/usr/bin/env python3
"""premise_check.py -- re-run every checkable premise of a dispatch section.

Owner ruling 2026-09-05 (session 23 close, on the Q-67 prevention: "The prevention
proposed is nice, you can implement it"): a premise written into a task section
must be the OUTPUT OF A COMMAND, pasted, never a number or a structure recalled
from a listing or from convention.  Seventeen dispatcher defects in one round were
that class (numbers from memory; a `.po` that does not exist; a hook role inferred
from a filename).  Prose does not fire under load; a process step bound to an event
does.  This is the step, bound to THREE events:

  * the MINT   -- the supervisor runs `premise_check.py <ID>`; every line must PASS
                  before the section is committed (a premise typed from memory
                  fails here, one hop before it can reach an executor);
  * the DISPATCH -- run again with the per-round variables refreshed;
  * the GATE   -- the executor runs it FIRST at Sec.0.7; any FAIL is observed-vs-
                  stated in checkable form, and an input-side FAIL is an abort.

Grammar, inside the task section (between its `### <ID>` header and the next
`### `/`## ` header), a fenced block whose first line is exactly `PREMISES`:

    ```
    PREMISES
    # comment lines and blank lines are skipped
    <shell command>  =>  <expected stdout, stripped>
    ```

Each command runs under `bash -c` with cwd = the CODE root (the parent of
`claude/`), inherits the environment, 120 s timeout; stdout is stripped and
compared byte-for-byte to the expected text.  A line whose expected text is the
literal `REFRESH-AT-DISPATCH` FAILS by construction: it marks a per-round
variable the dispatcher has not yet filled, so an unrefreshed prompt is visible
instead of silent.  Commands are READ-ONLY by convention; a small denylist refuses
the obvious mutators so a typo'd premise cannot move the tree (a guard against
accidents, not a security boundary -- the file is under the same write control as
every other harness script).

Exit 0 iff every premise line PASSes.  `--list` reports which live sections carry a
block (a live section without one is the coverage gap the rule forbids).
`--self-test` runs an in-memory block with one deliberate mismatch and one refused
line and asserts both are reported -- the instrument shown able to fail.
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
HARNESS_ROOT = os.path.dirname(HERE)              # .../claude
CODE_ROOT = os.path.dirname(HARNESS_ROOT)         # .../spacecrafter
DEFAULT_FILE = os.path.join(HARNESS_ROOT, "fable-dispatch.md")
TIMEOUT_S = 120
REFRESH = "REFRESH-AT-DISPATCH"
DENY = re.compile(r"(^|[\s;&|(])(rm|mv|cp|dd|tee|truncate|chmod|chown|sed\s+-i)\b"
                  r"|git\s+(commit|push|reset|checkout|switch|rebase|merge|branch\s+-[mdD]|apply(?!\s+--check)|worktree\s+(add|remove)|clean|stash)\b"
                  r"|(?<![0-9&])>(?!>?\s*/dev/null)")


def sections(text):
    """Yield (id, start_line, end_line, body) for every `### <ID> ` section."""
    lines = text.split("\n")
    heads = [(i, l) for i, l in enumerate(lines) if l.startswith("### ") or l.startswith("## ")]
    for k, (i, l) in enumerate(heads):
        if not l.startswith("### "):
            continue
        m = re.match(r"### (\S+)", l)
        if not m:
            continue
        end = heads[k + 1][0] if k + 1 < len(heads) else len(lines)
        yield m.group(1), i + 1, end, "\n".join(lines[i:end])


def block_of(body):
    """Return the PREMISES lines of a section body, or None if it has no block."""
    m = re.search(r"^```[^\n]*\nPREMISES[ \t]*\n(.*?)^```", body, re.S | re.M)
    if not m:
        return None
    out = []
    for raw in m.group(1).split("\n"):
        s = raw.strip()
        if not s or s.startswith("#"):
            continue
        if "=>" not in s:
            out.append((s, None))          # malformed: reported as FAIL
            continue
        cmd, exp = s.rsplit("=>", 1)
        out.append((cmd.strip(), exp.strip()))
    return out


def run_line(cmd, exp):
    if exp is None:
        return "FAIL", "(malformed: no `=>`)", ""
    if exp == REFRESH:
        return "FAIL", exp, "(unrefreshed per-round variable)"
    if DENY.search(cmd):
        return "FAIL", exp, "(refused: mutating token in a premise command)"
    try:
        p = subprocess.run(["bash", "-c", cmd], cwd=CODE_ROOT, capture_output=True,
                           text=True, timeout=TIMEOUT_S)
        obs = p.stdout.strip()
    except subprocess.TimeoutExpired:
        return "FAIL", exp, "(timeout %ds)" % TIMEOUT_S
    return ("PASS" if obs == exp else "FAIL"), exp, obs


def check(lines, label):
    fails = 0
    print("== PREMISES %s: %d line(s), cwd=%s ==" % (label, len(lines), CODE_ROOT))
    for cmd, exp in lines:
        state, e, o = run_line(cmd, exp)
        if state != "PASS":
            fails += 1
        print("%-4s expected=%r observed=%r :: %s" % (state, e, o if len(o) < 200 else o[:197] + "...", cmd))
    print("== %s: %d PASS, %d FAIL ==" % (label, len(lines) - fails, fails))
    return fails


def self_test():
    lines = [("echo alpha", "alpha"),
             ("echo beta", "gamma"),                       # must FAIL: mismatch
             ("rm -rf /tmp/never-run-me", "whatever"),     # must FAIL: refused, never executed
             ("printf x", REFRESH)]                        # must FAIL: unrefreshed marker
    fails = check(lines, "self-test")
    ok = fails == 3
    print("self-test:", "PASS (3 expected failures reported)" if ok else "FAIL (%d failures reported, 3 expected)" % fails)
    return 0 if ok else 2


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("section", nargs="?", help="task section id, e.g. F91")
    ap.add_argument("--file", default=DEFAULT_FILE, help="the dispatch file (default: fable-dispatch.md)")
    ap.add_argument("--list", action="store_true", help="report which sections carry a PREMISES block")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        return self_test()
    text = open(a.file, encoding="utf-8").read()
    secs = list(sections(text))
    if a.list:
        for sid, s, e, body in secs:
            b = block_of(body)
            print("%-6s lines %5d-%-5d %s" % (sid, s, e, "PREMISES %d line(s)" % len(b) if b is not None else "NO PREMISES BLOCK"))
        missing = sum(1 for _, _, _, body in secs if block_of(body) is None)
        print("== %d section(s), %d without a block ==" % (len(secs), missing))
        return 0
    if not a.section:
        ap.error("a section id, --list or --self-test is required")
    for sid, s, e, body in secs:
        if sid == a.section:
            b = block_of(body)
            if b is None:
                print("FAIL: section %s (lines %d-%d) has no PREMISES block" % (sid, s, e))
                return 1
            return 1 if check(b, sid) else 0
    print("FAIL: no section `### %s` in %s" % (a.section, a.file))
    return 1


if __name__ == "__main__":
    sys.exit(main())
