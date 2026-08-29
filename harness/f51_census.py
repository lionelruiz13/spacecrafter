#!/usr/bin/env python3
"""F51 (C) — the photometric-baseline census (§11.164(l)(3)).

Part 1, `--driver`: partition every committed run by the `Driver Version` line
its applog carries.  This is F48's grep, re-derived rather than quoted, and
checked against F48's committed `artifacts/f48/f48_driver_census.txt.gz` —
the check is that the two agree line-for-line on the runs F48 saw, so the
denominator this task reports is the same instrument's.

Part 2, `--gates`: the mechanical half of the baseline enumeration — which
tracked, non-artifact harness files read pixel VALUES at all, and which of
those carry a comparison against a number.  The verdict per file is NOT
mechanical and is not made here; it is recorded in INTENT §11.167 with the
reason per member.  What this part guarantees is the DENOMINATOR: no file that
reads pixels is silently absent from the list.

usage: f51_census.py --driver [<outfile>]
       f51_census.py --gates  [<outfile>]
"""
import gzip, json, os, re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ART = HERE / "artifacts"
REPO = HERE.parent

DRIVER = re.compile(r"Driver Version\s*:\s*([0-9.]+)")
PIXEL = re.compile(r"from PIL import|Image\.open|convert\(.L.\)|np\.asarray")
PHOTO = re.compile(r"(luma|lit\b|lit_|bright|dark|_px|px_|pixels|\.mean\(|max\(axis=2\)|changed|count)", re.I)
CMPN = re.compile(r"(<=|>=|<|>|==|!=)\s*-?\d")


def read_lines(p):
    if p.suffix == ".gz":
        with gzip.open(p, "rt", errors="replace") as f:
            return f.read().split("\n")
    return p.read_text(errors="replace").split("\n")


def driver_census():
    rows = []
    for p in sorted(ART.rglob("*")):
        if not p.is_file():
            continue
        if not (p.name.endswith(".log") or p.name.endswith(".applog")
                or p.name.endswith(".log.gz") or p.name.endswith(".applog.gz")):
            continue
        try:
            for line in read_lines(p):
                m = DRIVER.search(line)
                if m:
                    rows.append((os.path.getmtime(p), m.group(1),
                                 str(p.relative_to(ART))))
                    break
        except OSError:
            continue
    rows.sort()
    import datetime
    by = {}
    for t, v, path in rows:
        by.setdefault(v, []).append((t, path))
    out = {"total_runs_with_driver_line": len(rows), "versions": {}}
    for v, items in sorted(by.items()):
        out["versions"][v] = {
            "n": len(items),
            "first": datetime.datetime.fromtimestamp(items[0][0]).strftime("%Y-%m-%d %H:%M"),
            "last": datetime.datetime.fromtimestamp(items[-1][0]).strftime("%Y-%m-%d %H:%M"),
            "first_path": items[0][1], "last_path": items[-1][1]}
    # the boundary, as an open-closed bracket between the two adjacent runs
    vs = sorted(by, key=lambda v: by[v][0][0])
    if len(vs) == 2:
        a, b = vs
        out["boundary"] = {
            "old": a, "new": b,
            "last_old_run": out["versions"][a]["last"],
            "first_new_run": out["versions"][b]["first"],
            "bracket": f"({out['versions'][a]['last']}, {out['versions'][b]['first']}]"}
    # per-artifact-directory epoch label: this is what lets a FLAGGED harness's
    # committed baseline be dated by MEASUREMENT rather than by guess.
    dirs = {}
    for t, v, path in rows:
        d = path.split("/")[0]
        dirs.setdefault(d, {}).setdefault(v, 0)
        dirs[d][v] += 1
    out["by_artifact_dir"] = {d: {"versions": vv,
                                  "epoch": ("pre-2026-08-26" if set(vv) == {"580.568.0"}
                                            else "post-2026-08-26" if set(vv) == {"580.636.192"}
                                            else "mixed/other:" + ",".join(sorted(vv)))}
                              for d, vv in sorted(dirs.items())}
    out["rows"] = [f"{t}|{v}|{p}" for t, v, p in rows]
    # cross-check against F48's committed census
    f48 = ART / "f48/f48_driver_census.txt.gz"
    if f48.exists():
        prev = {}
        for line in read_lines(f48):
            if "|" not in line:
                continue
            _, ver, path = line.split("|", 2)
            prev[path.strip().lstrip("./")] = ver.split(":")[-1].strip()
        mine = {path: v for _, v, path in rows}
        common = set(prev) & set(mine)
        out["f48_crosscheck"] = {
            "f48_rows": len(prev), "this_rows": len(mine),
            "common": len(common),
            "disagreeing_version": sorted(p for p in common if prev[p] != mine[p]),
            "new_since_f48": sorted(set(mine) - set(prev)),
            "missing_vs_f48": sorted(set(prev) - set(mine))[:10]}
    return out


def gate_census():
    files = subprocess.run(["git", "-C", str(REPO), "ls-files", "harness"],
                           capture_output=True, text=True).stdout.split()
    files = [f for f in files if not f.startswith("harness/artifacts/")]
    out = {"tracked_non_artifact_harness_files": len(files),
           "read_pixels": [], "read_pixels_and_compare_to_a_number": {}}
    for f in files:
        p = REPO / f
        try:
            text = p.read_text(errors="replace")
        except (OSError, UnicodeDecodeError):
            continue
        if not PIXEL.search(text):
            continue
        out["read_pixels"].append(f)
        hits = []
        for i, l in enumerate(text.split("\n")):
            s = l.strip()
            if not s or s.startswith("#"):
                continue
            if PHOTO.search(s) and CMPN.search(s):
                hits.append(f"{i + 1}: {s[:160]}")
        if hits:
            out["read_pixels_and_compare_to_a_number"][f] = hits
    out["n_read_pixels"] = len(out["read_pixels"])
    out["n_with_numeric_comparison"] = len(out["read_pixels_and_compare_to_a_number"])
    return out


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ("--driver", "--gates"):
        print(__doc__); return 2
    res = driver_census() if sys.argv[1] == "--driver" else gate_census()
    if len(sys.argv) > 2:
        out = Path(sys.argv[2])
        rows = res.pop("rows", None)
        if rows is not None:
            # the 1727 per-run rows go next to the summary, gzipped, in F48's
            # own `mtime|version|path` format so the two censuses are diffable
            with gzip.open(str(out.with_suffix("")) + "_rows.txt.gz", "wt") as f:
                f.write("\n".join(rows) + "\n")
        out.write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
