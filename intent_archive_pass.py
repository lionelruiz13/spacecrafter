#!/usr/bin/env python3
"""INTENT.md archival pass — pure relocation, reconstruction-verified.

Convention (INTENT.md "Archival" note, 2026-07-31): closed units leave the live
surface by PURE MOVE — bytes are relocated, never edited. Two modes:
  - move:  the unit's line(s) leave INTENT.md and become INTENT/archive/<id>.md
           (units that had no split file).
  - stub:  the unit's index line leaves INTENT.md and is APPENDED verbatim to
           its existing INTENT/<id>.md (append-only-compatible snapshot).

Verification: the removed payloads re-inserted at their recorded line indices
must reproduce the pre-pass INTENT.md byte-exact (md5 asserted). A manifest
(JSON) with per-unit indices + payload md5s lands in INTENT/archive/.

Run from claude/: python3 intent_archive_pass.py [--dry-run]
"""
import hashlib, json, re, sys, os

DRY = "--dry-run" in sys.argv
DATE = "2026-07-31"
PASS_NAME = "pass1"

INTENT = "INTENT.md"
ARCH = "INTENT/archive"

# ---- unit lists (classification: supervising session 2026-07-31; criterion:
# ---- closed + read by no open row/decision/scheduled work) -----------------
S5_MOVE = [3, 6, 10, 11, 12, 13, 14, 15, 28, 29, 30, 31, 33, 37, 38, 39, 40, 45]
S5_STUB = [4, 16, 17, 22, 23, 24]          # body already in INTENT/5.<n>.md
S6_MOVE = [8]                               # multi-line entry (P5, resolved+verified)
S11_STUB = [101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
            113, 114, 115, 117, 118, 119, 120, 121]
S13_MOVE = ["B6", "B9", "B11", "B13", "B16", "B19", "B20", "B22", "B23",
            "B25", "B26", "B28", "B29", "B32", "B40"]
# kept live on doubt or open residual: 5.2 (live class name), 5.5, 5.43 (rider),
# all OPEN §5; §11.112 (D15 pending), 11.116 (live scheduling), 11.122/11.123
# (current working set); every §13.B row carrying an open residual.

CLOSED_MARKERS = re.compile(r"(FIXED|CLOSED|RESOLVED|DONE|COMPLETE|RECLASSIFIED)")

def md5(b): return hashlib.md5(b).hexdigest()

def section_range(lines, start_pat, end_pat):
    s = e = None
    for i, l in enumerate(lines):
        if s is None and re.match(start_pat, l): s = i
        elif s is not None and re.match(end_pat, l): e = i; break
    assert s is not None and e is not None, (start_pat, end_pat)
    return s, e

def find_unit(lines, lo, hi, head_pat, multiline=False):
    idxs = [i for i in range(lo, hi) if re.match(head_pat, lines[i])]
    assert len(idxs) == 1, f"anchor not unique: {head_pat} -> {idxs}"
    start = idxs[0]
    end = start + 1
    if multiline:
        while end < hi and not re.match(r"^(\d+\.\s|\||#|---)", lines[end]):
            end += 1
        while end > start + 1 and lines[end - 1].strip() == "":
            end -= 1
    return start, end

def main():
    with open(INTENT, "rb") as f: pre = f.read()
    pre_md5 = md5(pre)
    lines = pre.decode("utf-8").split("\n")

    s5 = section_range(lines, r"^## 5\. Defects", r"^## 6\. Decisions")
    s6 = section_range(lines, r"^## 6\. Decisions", r"^## 7\.")
    s11 = section_range(lines, r"^## 11\. Open investigation log", r"^## 12\.")
    s13b = section_range(lines, r"^### 13\.B", r"^### 13\.C")

    units = []  # (id, mode, start, end, dest)
    for n in S5_MOVE:
        a, b = find_unit(lines, *s5, rf"^{n}\. \*\*")
        units.append((f"5.{n}", "move", a, b, f"{ARCH}/5.{n}.md"))
    for n in S5_STUB:
        a, b = find_unit(lines, *s5, rf"^{n}\. \*\*")
        assert os.path.exists(f"INTENT/5.{n}.md"), f"INTENT/5.{n}.md missing"
        units.append((f"5.{n}", "stub", a, b, f"INTENT/5.{n}.md"))
    for n in S6_MOVE:
        a, b = find_unit(lines, *s6, rf"^{n}\. \*\*", multiline=True)
        units.append((f"6.{n}", "move", a, b, f"{ARCH}/6.{n}.md"))
    for n in S11_STUB:
        a, b = find_unit(lines, *s11, rf"^{n}\. \*\*")
        assert os.path.exists(f"INTENT/11.{n}.md"), f"INTENT/11.{n}.md missing"
        units.append((f"11.{n}", "stub", a, b, f"INTENT/11.{n}.md"))
    for r in S13_MOVE:
        a, b = find_unit(lines, *s13b, rf"^\| {r} \|")
        units.append((r, "move", a, b, f"{ARCH}/{r}.md"))

    # overlap + closed-marker guard (typo catcher; the list is the authority)
    seen = set()
    for (uid, mode, a, b, dest) in units:
        for i in range(a, b):
            assert i not in seen, f"overlap at line {i+1} ({uid})"
            seen.add(i)
        payload = "\n".join(lines[a:b])
        # closure is lexically marked in §5/§6/§13; §11 index lines are journal
        # records (closure structural, verified by reading — file must exist).
        if not uid.startswith("11."):
            assert CLOSED_MARKERS.search(payload), f"{uid}: no closed marker found"

    # build outputs
    manifest = {"pass": PASS_NAME, "date": DATE, "pre_md5": pre_md5, "units": []}
    removed = sorted(seen)
    for (uid, mode, a, b, dest) in sorted(units, key=lambda u: u[2]):
        payload = "\n".join(lines[a:b])
        manifest["units"].append({
            "id": uid, "mode": mode, "dest": dest,
            "line_start": a + 1, "line_count": b - a,
            "payload_md5": md5(payload.encode()),
        })
        if DRY: continue
        if mode == "move":
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            kind = "§13.B row" if uid.startswith("B") else f"§{uid.split('.')[0]} entry"
            note = (" A one-line pointer remains in §13.C."
                    if uid.startswith("B") else "")
            with open(dest, "w", encoding="utf-8") as f:
                f.write(f"# {uid} — archived {DATE} (INTENT.md archival {PASS_NAME})\n\n"
                        f"*Moved verbatim from INTENT.md ({kind}; closed, read by no "
                        f"open row/decision). This file is the entry.{note}*\n\n"
                        f"{payload}\n")
        else:
            with open(dest, "a", encoding="utf-8") as f:
                f.write(f"\n\n---\n\n*INTENT.md index line retired to this file {DATE} "
                        f"(archival {PASS_NAME}; the line was a derived view — this "
                        f"file is the entry's authority):*\n\n{payload}\n")

    new_lines = [l for i, l in enumerate(lines) if i not in seen]

    # reconstruction verification (inverse insert -> byte-exact pre-pass file)
    recon = list(new_lines)
    for (uid, mode, a, b, dest) in sorted(units, key=lambda u: u[2]):
        recon[a:a] = lines[a:b]
    assert "\n".join(recon).encode() == pre, "RECONSTRUCTION FAILED"
    print(f"reconstruction OK ({len(units)} units, {len(removed)} lines, "
          f"pre md5 {pre_md5})")

    if DRY:
        for u in manifest["units"]:
            print(f"  {u['mode']:4} {u['id']:7} L{u['line_start']}"
                  f"+{u['line_count']} -> {u['dest']}")
        return
    with open(INTENT, "w", encoding="utf-8") as f:
        f.write("\n".join(new_lines))
    with open(f"{ARCH}/{DATE}-{PASS_NAME}.manifest.json", "w") as f:
        json.dump(manifest, f, indent=1)
    with open(INTENT, "rb") as f:
        print("post md5", md5(f.read()))

if __name__ == "__main__":
    main()
