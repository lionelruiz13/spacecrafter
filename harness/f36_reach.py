#!/usr/bin/env python3
"""F36 / INTENT §5.77 — which console-report sites are on the STARTUP path.

§5.77's owed item is *"an enumeration of the other startup paths that report only
on stderr"*. `f36_enum.py` produces the site table; this driver supplies the word
"startup" with a measurement instead of a reading of the call graph.

ONE FRESH LAUNCH, UNDER gdb, TWO SURFACES READ ON THE SAME RUN:

  - **could it report?** — one self-disabling breakpoint on the enclosing
    function of every project console-output site, each hit stamped STARTUP or
    POST against a breakpoint on `App::startMainLoop` (see `f36_probe.py`).
  - **did it report?** — the app's own stdout+stderr, which `f27.Session` already
    captures into `<tag>.applog` with both streams merged in order. The console
    output of a healthy launch is a SUBSET of the enumerated sites, so it is the
    positive control for the probe: every line the app printed must be
    attributable to a site the enumeration knows about, and every site that
    printed must be one the probe marked reachable. A site marked unreachable
    that nevertheless printed would falsify the probe.

The launch itself is F27's `Session` (I2 — same farm, same `/proc/<pid>/comm`
concurrent-instance assert, same frozen config/ssystem md5 in == out pair), with
F31's `launch_prefix` kwarg so the argv runs under gdb: ptrace_scope=1 blocks
attaching, so a probe has to be present from the first instruction.

Usage:
    cd claude/harness && DISPLAY=:2 ./f36_reach.py artifacts/f36 \
        --bin ../../build-claude/src/spacecrafter
"""
import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import f27_reply as f27                                            # noqa: E402

GDB_SCRIPT = HERE / "f36_reach.gdb"
ENUM = HERE / "f36_enum.py"


def build_gdb_script(path: Path):
    path.write_text(
        "# F36 / INTENT §5.77 — generated wrapper; the probe sets its own\n"
        "# breakpoints through the gdb Python API (f36_probe.py), so this file\n"
        "# only fixes the gdb settings the probe depends on.\n"
        "set pagination off\n"
        "set confirm off\n"
        "set breakpoint pending on\n"
        "set print elements 200\n"
        "handle SIGUSR1 nostop noprint pass\n"
        "source %s\n"
        "run\n" % (HERE / "f36_probe.py"))


def parse_probe(probe: Path):
    manifest, reach, boundary = [], [], False
    for line in probe.read_text(errors="replace").splitlines():
        if line.startswith("MANIFEST-SUMMARY"):
            manifest.append(("SUMMARY", line))
        elif line.startswith("MANIFEST "):
            m = re.match(r"MANIFEST spec=(\S+) locations=(\S+)(?: label=(.*))?", line)
            if m:
                manifest.append((m.group(1), m.group(2), m.group(3) or ""))
        elif line.startswith("REACH "):
            m = re.match(r"REACH\s+(\S+)\s+(.*)", line)
            if m:
                reach.append({"phase": m.group(1), "label": m.group(2).strip()})
        elif line.startswith("BOUNDARY"):
            boundary = True
    return manifest, reach, boundary


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default="../../build-claude/src/spacecrafter")
    a = ap.parse_args()

    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    # RESOLVED, not as typed: `Session` launches with cwd = the farm it builds,
    # so a path relative to the harness directory would be resolved against the
    # farm and gdb would start with no executable at all (measured 2026-08-09:
    # 77 pending breakpoints and `run` refusing with "No executable file
    # specified" — an instrument that produces a full-looking manifest of zeros).
    binary = Path(a.bin).resolve()
    if not binary.exists():
        f27.fail(f"binary not found: {binary}")

    sites = out / "f36_sites.json"
    subprocess.run([sys.executable, str(ENUM), "-o", str(sites)], check=True)
    rows = json.load(open(sites))
    proj = [r for r in rows if not r["entitycore"]]

    probe = out / "f36_probe.txt"
    if probe.exists():
        probe.unlink()
    os.environ["F36_PROBE"] = str(probe)
    os.environ["F36_SITES"] = str(sites)
    build_gdb_script(GDB_SCRIPT)

    f27.note(f"sites: {len(rows)} ({len(proj)} project) -> {sites}")
    sess = f27.Session(out, "f36", binary,
                       launch_prefix=("gdb", "-q", "-batch", "-x",
                                      str(GDB_SCRIPT), "--args"),
                       port_wait=300)
    drv = sess.client("drv")
    rc = sess.stop(drv, exit_wait=120)

    manifest, reach, boundary = parse_probe(probe)
    summary = [m for m in manifest if m[0] == "SUMMARY"]
    unresolved = [m for m in manifest if m[0] != "SUMMARY" and m[1] in ("0", "-1")
                  or (m[0] != "SUMMARY" and str(m[1]).startswith("SPEC-ERROR"))]
    startup = [r for r in reach if r["phase"] == "STARTUP"]
    post = [r for r in reach if r["phase"] == "POST"]

    res = {
        "binary": str(binary),
        "binary_md5": f27.md5(binary),
        "returncode": rc,
        "boundary_seen": boundary,
        "sites_total": len(rows),
        "sites_project": len(proj),
        "breakpoints": len([m for m in manifest if m[0] != "SUMMARY"]),
        "manifest_summary": [m[1] for m in summary],
        "unresolved": [list(m) for m in unresolved],
        "startup_reached": sorted(r["label"] for r in startup),
        "post_reached": sorted(r["label"] for r in post),
    }
    (out / "f36_result.json").write_text(json.dumps(res, indent=1))

    f27.note(f"app exit {rc} | boundary seen: {boundary}")
    f27.note(f"breakpoints {res['breakpoints']} | unresolved {len(unresolved)}")
    f27.note(f"STARTUP-reached functions: {len(startup)}")
    f27.note(f"POST-startup-reached functions: {len(post)}")
    for r in sorted(startup, key=lambda r: r["label"]):
        print("  STARTUP  " + r["label"])
    for r in sorted(post, key=lambda r: r["label"]):
        print("  POST     " + r["label"])
    if unresolved:
        f27.note("UNRESOLVED (measurement holes):")
        for m in unresolved:
            print("  " + " ".join(str(x) for x in m))
    print(f"-> {out/'f36_result.json'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
