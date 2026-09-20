#!/usr/bin/env python3
"""Insert the staged sections before the section-2 boundary, then fill each PREMISES
block by RUNNING its commands (cwd = code root, as premise_check.py does).
Every expected value is a command's stripped stdout; none is typed."""
import subprocess, sys

CODE = "/home/claude/spacecrafter"
DISPATCH = CODE + "/claude/fable-dispatch.md"
STAGED = "/tmp/s34_sections.md"
IDS = ["F124", "F125", "F126"]
ANCHOR = "\n---\n\n## 2. Blocked "          # ASCII part of the boundary; must be unique


def run(cmd):
    p = subprocess.run(["bash", "-c", cmd], cwd=CODE, capture_output=True, text=True, timeout=120)
    return p.stdout.strip(), p.returncode, p.stderr.strip()


def block(tid):
    out = ["```", "PREMISES"]
    empties = []
    for line in open(f"/tmp/s34/{tid}.cmds").read().split("\n"):
        if not line.strip():
            continue
        if line.startswith("#"):
            out.append(line)
            continue
        if " => " in line:
            sys.exit(f"{tid}: command contains the separator: {line}")
        got, rc, err = run(line)
        if got == "":
            empties.append((line, rc, err))
        out.append(f"{line} => {got}")
    out.append("```")
    return "\n".join(out), empties


def main():
    text = open(DISPATCH, encoding="utf-8").read()
    if text.count(ANCHOR) != 1:
        sys.exit(f"anchor count {text.count(ANCHOR)} != 1 - nothing written")
    for tid in IDS:
        if f"### {tid} " in text:
            sys.exit(f"{tid} already present - nothing written")
    staged = open(STAGED, encoding="utf-8").read()
    for tid in IDS:
        if staged.count(f"@@PREMISES_{tid}@@") != 1:
            sys.exit(f"placeholder for {tid} not unique in the staged file")
    before = text.count("\n### F")
    # phase 1: sections in, placeholders still there (the live count must be final
    # on disk before the commands that read it are run)
    phase1 = text.replace(ANCHOR, "\n" + staged.rstrip("\n") + "\n" + ANCHOR, 1)
    open(DISPATCH, "w", encoding="utf-8").write(phase1)
    # phase 2: fill, everything built in memory before the file is reopened
    filled, report = phase1, []
    for tid in IDS:
        b, empties = block(tid)
        filled = filled.replace(f"@@PREMISES_{tid}@@", b, 1)
        report.append((tid, b.count(" => "), empties))
    if "@@PREMISES_" in filled:
        sys.exit("a placeholder survived - phase 1 is on disk, phase 2 NOT written")
    open(DISPATCH, "w", encoding="utf-8").write(filled)
    print(f"### F count: {before} -> {filled.count(chr(10) + '### F')}")
    for tid, n, empties in report:
        print(f"{tid}: {n} premise line(s), {len(empties)} with EMPTY stdout")
        for line, rc, err in empties:
            print(f"   EMPTY rc={rc} :: {line}\n      stderr: {err[:160]}")


main()
