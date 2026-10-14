#!/usr/bin/env python3
"""List every reference this delivery ADDS to the six contract files, resolved
the way anchor_gate.run() resolves it (own pin, FILE_OVERRIDES, file token,
content disambiguation), with the text of its first and last line at its pin
and its state against the working tree. A reference is NEW when its
(json path, file, line list, pin) tuple is absent from the HEAD version of the
same file. Read-only: imports the gate, writes nothing but stdout.
usage: python3 -B new_refs.py"""
import json, subprocess, sys
sys.path.insert(0, "/home/claude/spacecrafter/util/scedit/tests")
import anchor_gate as ag

def refs(g, path, pin):
    out = []
    cmds = g["families"]["commands"] if path == ag.MERGED else g.get("commands", {})
    for jp, s in ag.walk(g):
        if jp.startswith("._meta"):
            continue
        import re
        m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == ag.MERGED
                     else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
        handler = cmds.get(m.group(1), {}).get("handler") if m else None
        for ti, t in enumerate(ag.tokenize(s)):
            tpin = t.pin or pin
            if (jp, ti) in ag.FILE_OVERRIDES:
                rp, how = ag.FILE_OVERRIDES[(jp, ti)], "override"
            elif t.kind == "FILE":
                rp, how = ag.resolve_file(t.file)
            else:
                rp, how, _b, _e = ag.disambiguate(t, tpin, s, handler)
            for a, b, _, _ in t.elems:
                out.append((jp, rp, a, b, tpin, how))
    return out

total = 0
for path in ag.TARGETS:
    pin = ag.file_pin(path)
    old = json.loads(subprocess.run(["git", "-C", ag.REPO, "show", "HEAD:" + path],
                                    capture_output=True).stdout)
    new = ag.load(path)
    before = {}
    for r in refs(old, path, pin):
        before[r] = before.get(r, 0) + 1
    for r in refs(new, path, pin):
        if before.get(r, 0):
            before[r] -= 1
            continue
        jp, rp, a, b, tpin, how = r
        total += 1
        ls = ag.lines_at(tpin, rp) or []
        v = ag.classify_any(tpin, rp, a, how, head=ag.WORKTREE)[0]
        if b is not None:
            v2 = ag.classify_any(tpin, rp, b, how, head=ag.WORKTREE)[0]
            v = v if v == v2 else "%s/%s" % (v, v2)
        first = ls[a - 1].strip() if 0 < a <= len(ls) else "<out of range>"
        last = (ls[b - 1].strip() if 0 < b <= len(ls) else "<out of range>") if b else ""
        print("%s | %s | %s:%s @ %s | %s\n    first: %s%s" % (
            path.split("/")[-1], jp, rp, "%d-%d" % (a, b) if b else a, tpin[:8], v,
            first[:110], ("\n    last:  " + last[:110]) if b else ""))
print("NEW references: %d" % total)
