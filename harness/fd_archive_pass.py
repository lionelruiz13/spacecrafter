#!/usr/bin/env python3
"""fable-dispatch.md archival pass -- pure byte-exact relocation, reconstruction-verified.

Convention (fable-dispatch.md "Archival" note, 2026-08-01, pass 1): a unit that no
longer contributes to the live surface (a DELIVERED+accepted task section `F<n>`; a
session update note `update-s<n>` superseded by a later one) leaves the file as a
pure move to `fable-dispatch/archive/<id>.md`; references are never rewritten
(resolution-by-insertion). Passes 1..19 did this in-process and left only their
manifests; this file is that in-process instrument written down (session 28), same
manifest schema, so a successor replays the act instead of re-deriving it.

Ordering (QUEUE Q-56, destructive step LAST): archive files are written first, the
reconstruction is verified in-process against the pre-move bytes, the manifest is
written, and only then is the live surface rewritten -- and re-verified from disk.

usage:
  fd_archive_pass.py PASS DATE PRE_COMMIT UNIT... [--seam-after ID:START:COUNT]...
      UNIT = ID:LINE_START:LINE_COUNT   (1-based, inclusive; a unit is contiguous)
      --seam-after ID:START:COUNT   lines removed right after unit ID (a doubled
                                    separator); recorded in the manifest as seam_tidy
  --dry-run   verify and print the plan, write nothing
Run from the harness repo root (the directory holding fable-dispatch.md).
"""
import hashlib
import json
import os
import sys

SRC = "fable-dispatch.md"
ARCH = "fable-dispatch/archive"


def md5(b: bytes) -> str:
    return hashlib.md5(b).hexdigest()


def parse_spec(s):
    ident, start, count = s.split(":")
    return ident, int(start), int(count)


def main(argv):
    dry = "--dry-run" in argv
    argv = [a for a in argv if a != "--dry-run"]
    if len(argv) < 4:
        print(__doc__)
        return 2
    pass_id, date, pre_commit = argv[0], argv[1], argv[2]
    units, seams = [], []
    i = 3
    while i < len(argv):
        if argv[i] == "--seam-after":
            seams.append(parse_spec(argv[i + 1]))
            i += 2
        else:
            units.append(parse_spec(argv[i]))
            i += 1

    pre_bytes = open(SRC, "rb").read()
    pre_md5 = md5(pre_bytes)
    lines = pre_bytes.split(b"\n")
    # a trailing newline yields an empty last element; keep it so join() restores it
    n = len(lines)

    # every removal as (start, count, kind, id); all 1-based, contiguous, non-overlapping
    removals = [(s, c, "unit", ident) for ident, s, c in units]
    removals += [(s, c, "seam", ident) for ident, s, c in seams]
    removals.sort()
    last_end = 0
    for s, c, kind, ident in removals:
        if s <= last_end:
            print(f"OVERLAP at {ident} ({s})")
            return 1
        if s + c - 1 > n:
            print(f"OUT OF RANGE at {ident}: {s}+{c}-1 > {n}")
            return 1
        last_end = s + c - 1

    # the unit ids must be the section/note headers they claim
    for ident, s, c in units:
        head = lines[s - 1].decode("utf-8", "replace")
        ok = (ident.startswith("F") and head.startswith(f"### {ident} ")) or \
             (ident.startswith("update-s") and head.startswith("**Update ["))
        if not ok:
            print(f"HEADER MISMATCH for {ident} at line {s}: {head[:80]!r}")
            return 1

    payloads = {}
    manifest_units, seam_tidy = [], []
    for s, c, kind, ident in removals:
        chunk = lines[s - 1:s - 1 + c]
        if kind == "unit":
            payload = b"\n".join(chunk) + b"\n"
            payloads[ident] = payload
            manifest_units.append({
                "id": ident, "mode": "move", "dest": f"{ARCH}/{ident}.md",
                "line_start": s, "line_count": c, "payload_md5": md5(payload)})
        else:
            seam_tidy.append({
                "removed_after_unit": ident,
                "lines": [x.decode("utf-8") for x in chunk],
                "positions_pre": list(range(s, s + c)),
                "reason": "the moved unit sat between two --- separators; removing it "
                          "leaves them doubled (pass-13..19 precedent)."})

    # the live surface after the pass
    keep = []
    cursor = 1
    for s, c, kind, ident in removals:
        keep.extend(lines[cursor - 1:s - 1])
        cursor = s + c
    keep.extend(lines[cursor - 1:])
    post_bytes = b"\n".join(keep)

    # reconstruction in-process: re-insert every removal at its recorded position,
    # ascending, and demand the pre-move md5
    recon = list(keep)
    for s, c, kind, ident in removals:  # ascending order restores original indices
        recon[s - 1:s - 1] = lines[s - 1:s - 1 + c]
    if md5(b"\n".join(recon)) != pre_md5:
        print("RECONSTRUCTION FAILED in-process")
        return 1

    manifest = {
        "pass": pass_id, "date": date, "source": SRC, "pre_md5": pre_md5,
        "pre_commit": pre_commit,
        "reconstruction": "verified byte-exact in-process against the pre-move bytes "
                          "(payloads and seam-tidy lines re-inserted at recorded "
                          "positions in ascending order reproduce pre_md5); "
                          "re-verified from disk after the write",
        "seam_tidy": seam_tidy, "kept_live": [], "units": manifest_units,
        "instrument": "harness/fd_archive_pass.py",
    }
    print(f"pre_md5 {pre_md5}  lines {n - 1} -> {len(keep) - 1}  "
          f"removed {sum(c for _, c, _, _ in removals)}")
    for u in manifest_units:
        print(f"  {u['id']:<12} {u['line_start']:>5} +{u['line_count']:<4} -> {u['dest']}  {u['payload_md5'][:8]}")
    for t in seam_tidy:
        print(f"  seam after {t['removed_after_unit']}: positions {t['positions_pre']} {t['lines']}")
    if dry:
        print("dry run: nothing written")
        return 0

    os.makedirs(ARCH, exist_ok=True)
    for ident, payload in payloads.items():
        dest = f"{ARCH}/{ident}.md"
        if os.path.exists(dest):
            print(f"REFUSED: {dest} exists")
            return 1
        with open(dest, "wb") as f:
            f.write(payload)
    mpath = f"{ARCH}/{date}-{pass_id}.manifest.json"
    with open(mpath, "w") as f:
        json.dump(manifest, f, indent=1, ensure_ascii=False)
        f.write("\n")
    # destructive step last
    with open(SRC, "wb") as f:
        f.write(post_bytes)

    # re-verify from disk
    disk = open(SRC, "rb").read().split(b"\n")
    for s, c, kind, ident in removals:
        if kind == "unit":
            chunk = open(f"{ARCH}/{ident}.md", "rb").read()
            assert chunk.endswith(b"\n")
            ins = chunk[:-1].split(b"\n")
        else:
            ins = [x.encode("utf-8") for x in next(t for t in seam_tidy if t["positions_pre"][0] == s)["lines"]]
        disk[s - 1:s - 1] = ins
    ok = md5(b"\n".join(disk)) == pre_md5
    print("reconstruction from disk:", "OK" if ok else "FAILED")
    print("manifest:", mpath)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
