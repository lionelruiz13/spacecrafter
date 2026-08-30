#!/usr/bin/env python3
"""F56 — the CACHE MANIFEST: a recursive md5+size+mtime snapshot of a directory,
and a diff of two snapshots with per-file deltas.

WHY IT EXISTS (§11.172(i), the instrument gap it closes).  `b3_farm.sh` builds a
temp-HOME farm by symlinking every `~/.spacecrafter` entry it does not explicitly
name, and `cache/` is one of them — so EVERY farm run in this corpus reads and
writes the REAL `~/.spacecrafter/cache`.  §11.172(i) measured one instance
(`t-bodies-moon_normal.dat` rewritten INSIDE an F55 run) and named the class: a
mutable, cross-run, cross-task state variable sitting under every photometric
measurement the corpus has ever taken, covered by no assert (the standing md5
in==out assert covers `config.ini` / `ssystem.ini` only).

WHAT IT DOES NOT DO, and why that is deliberate.  It REPORTS mutations; it never
prevents them.  Isolating the cache (a private cache dir per run) would make every
run a COLD-CACHE run, which is a different measurement condition from the one this
corpus's entire baseline was taken under — a change of the thing being measured,
not an improvement of the instrument.  Isolation is therefore out of scope here and
is a decision for the ledger, not for a preflight script.

usage:
  f56_manifest.py snapshot <dir> <out.json>     # walk + hash
  f56_manifest.py diff <pre.json> <post.json> [--out <diff.json>] [--quiet]

`snapshot` follows the top-level directory but records symlinks AS symlinks (target
recorded, never followed) — a farm's `cache` symlink and a real `cache` directory
are different facts and are kept different.  Exit codes: 0 on success (INCLUDING a
diff that found mutations — a cache write is expected behaviour, not an error), 2 on
usage error, 3 on an unreadable path.
"""
import hashlib
import json
import os
import sys
import time
from pathlib import Path


def md5_of(path, chunk=1 << 20):
    h = hashlib.md5()
    with open(path, "rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                return h.hexdigest()
            h.update(b)


def snapshot(root):
    root = Path(root)
    rec = {"root": str(root), "taken_wall": round(time.time(), 3),
           "taken_iso": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
           "root_exists": root.exists(),
           "root_is_symlink": root.is_symlink(),
           "root_symlink_target": os.readlink(root) if root.is_symlink() else None,
           "files": {}, "dirs": [], "symlinks": {}, "errors": []}
    if not root.exists():
        return rec
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        dirnames.sort()
        d = Path(dirpath)
        rel_d = str(d.relative_to(root))
        if rel_d != ".":
            rec["dirs"].append(rel_d)
        for name in sorted(filenames):
            p = d / name
            rel = str(p.relative_to(root))
            try:
                if p.is_symlink():
                    rec["symlinks"][rel] = os.readlink(p)
                    continue
                st = p.stat()
                rec["files"][rel] = {"size": st.st_size,
                                     "mtime_ns": st.st_mtime_ns,
                                     "mtime_iso": time.strftime(
                                         "%Y-%m-%dT%H:%M:%S",
                                         time.localtime(st.st_mtime)),
                                     "md5": md5_of(p)}
            except OSError as e:
                rec["errors"].append(f"{rel}: {e}")
    rec["n_files"] = len(rec["files"])
    rec["n_bytes"] = sum(v["size"] for v in rec["files"].values())
    rec["aggregate_md5"] = hashlib.md5(
        "".join(f"{k}:{v['md5']}\n" for k, v in sorted(rec["files"].items()))
        .encode()).hexdigest()
    return rec


def diff(pre, post):
    a, b = pre["files"], post["files"]
    added = sorted(set(b) - set(a))
    removed = sorted(set(a) - set(b))
    content = []
    touched = []          # same md5, mtime moved: an open/rewrite with equal bytes
    for k in sorted(set(a) & set(b)):
        if a[k]["md5"] != b[k]["md5"]:
            content.append({"path": k,
                            "md5": [a[k]["md5"], b[k]["md5"]],
                            "size": [a[k]["size"], b[k]["size"]],
                            "size_delta": b[k]["size"] - a[k]["size"],
                            "mtime": [a[k]["mtime_iso"], b[k]["mtime_iso"]],
                            "mtime_delta_s": round(
                                (b[k]["mtime_ns"] - a[k]["mtime_ns"]) / 1e9, 3)})
        elif a[k]["mtime_ns"] != b[k]["mtime_ns"]:
            touched.append({"path": k,
                            "mtime": [a[k]["mtime_iso"], b[k]["mtime_iso"]],
                            "mtime_delta_s": round(
                                (b[k]["mtime_ns"] - a[k]["mtime_ns"]) / 1e9, 3)})
    return {"root": [pre["root"], post["root"]],
            "window_s": round(post["taken_wall"] - pre["taken_wall"], 3),
            "pre_iso": pre["taken_iso"], "post_iso": post["taken_iso"],
            "aggregate_md5": [pre.get("aggregate_md5"), post.get("aggregate_md5")],
            "aggregate_changed": pre.get("aggregate_md5") != post.get("aggregate_md5"),
            "n_files": [pre.get("n_files"), post.get("n_files")],
            "added": added, "removed": removed,
            "content_changed": content, "mtime_only": touched,
            "n_mutations": len(added) + len(removed) + len(content) + len(touched),
            "symlinks_changed": pre.get("symlinks") != post.get("symlinks")}


def report(d, out=sys.stdout):
    p = lambda *a: print(*a, file=out)
    p(f"CACHE MANIFEST DIFF  {d['root'][0]}")
    p(f"  window            : {d['window_s']} s  ({d['pre_iso']} -> {d['post_iso']})")
    p(f"  files             : {d['n_files'][0]} -> {d['n_files'][1]}")
    p(f"  aggregate md5     : {d['aggregate_md5'][0]} -> {d['aggregate_md5'][1]}"
      f"  CHANGED={d['aggregate_changed']}")
    p(f"  mutations         : {d['n_mutations']}")
    if not d["n_mutations"]:
        p("  (no file added, removed, rewritten or re-stamped inside the window)")
    for k in d["added"]:
        p(f"  ADDED     {k}")
    for k in d["removed"]:
        p(f"  REMOVED   {k}")
    for c in d["content_changed"]:
        p(f"  REWRITTEN {c['path']}")
        p(f"            md5   {c['md5'][0]} -> {c['md5'][1]}")
        p(f"            size  {c['size'][0]} -> {c['size'][1]}  ({c['size_delta']:+d} B)")
        p(f"            mtime {c['mtime'][0]} -> {c['mtime'][1]}")
    for t in d["mtime_only"]:
        p(f"  RESTAMPED {t['path']}  {t['mtime'][0]} -> {t['mtime'][1]}"
          f"  (bytes identical)")
    if d["symlinks_changed"]:
        p("  SYMLINK SET CHANGED")


def main():
    argv = sys.argv[1:]
    if not argv:
        print(__doc__)
        return 2
    if argv[0] == "snapshot":
        if len(argv) < 3:
            print(__doc__)
            return 2
        rec = snapshot(argv[1])
        Path(argv[2]).parent.mkdir(parents=True, exist_ok=True)
        Path(argv[2]).write_text(json.dumps(rec, indent=1))
        print(f"manifest: {rec.get('n_files', 0)} files, "
              f"{rec.get('n_bytes', 0)} B, aggregate md5 "
              f"{rec.get('aggregate_md5')}  -> {argv[2]}")
        return 0 if rec["root_exists"] else 3
    if argv[0] == "diff":
        if len(argv) < 3:
            print(__doc__)
            return 2
        pre = json.loads(Path(argv[1]).read_text())
        post = json.loads(Path(argv[2]).read_text())
        d = diff(pre, post)
        if "--out" in argv:
            Path(argv[argv.index("--out") + 1]).write_text(json.dumps(d, indent=1))
        if "--quiet" not in argv:
            report(d)
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
