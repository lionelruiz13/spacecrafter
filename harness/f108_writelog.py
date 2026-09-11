#!/usr/bin/env python3
"""F108 -- `write_log = false` MEASURED ON BOTH BINARIES, NOT ASSUMED.

    cd claude/harness
    DISPLAY=:2 python3 f108_writelog.py <absOutdir> --pre <binA> --post <binB>

Sec.5.115's only mitigation in the field is `write_log = false`, which kills
the channel; the F108 mandate is that the key KEEPS ITS MEANING.  "Keeps" is a
comparison, so this runs ONE launch per binary, each on its own fresh farm with
that one key flipped, and puts the two log directories side by side.

It runs on a FARM and not on the real home ON PURPOSE: the leg has to change
`config.ini`, and the real one's md5 (03fbee59) is a precondition every later
task asserts.  The farm's config.ini is a real copy (b3_farm.sh), the key is
rewritten by f96_offset.ini_set (which fails loudly if the key is absent), and
the md5 is asserted in == out around the launch so an app-side rewrite of the
file would be visible.

What the comparison must show, and what it cannot: the gate is at log.cpp:128
and is set from the config at main.cpp:269, so every line written BEFORE that
point lands in the file on both binaries -- that is today's behaviour and the
reason the startup preamble is in the file even with the key off.  The lines
F108 adds are flushed at :220, i.e. in that same pre-config window.
"""
import argparse
import hashlib
import json
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, build_farm, ini_set, no_instance           # noqa: E402
import logread                                                        # noqa: E402

FAILS = []
D12_KEY = "Log retention ("
FROZEN = ("config.ini", "ssystem.ini", "scripts/fscripts/startup.sts")


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def leg(name, binary, out):
    farm = out / ("farm_" + name)
    home = build_farm(farm)                      # <farm>/.spacecrafter
    old = ini_set(home / "config.ini", "debug", "write_log", "false")
    print("\n=== %s: %s" % (name, binary))
    print("    write_log %s -> false, config.ini md5 %s"
          % (old, md5(home / "config.ini")))
    before = {f: md5(home / f) for f in FROZEN if (home / f).is_file()}
    hits = no_instance()
    if hits:
        raise RuntimeError("another spacecrafter is running: %s" % hits)
    app = App(binary, farm, out, name)
    t = app.start()
    rc = app.stop()
    time.sleep(0.5)
    after = {f: md5(home / f) for f in FROZEN if (home / f).is_file()}
    for f in before:
        if before[f] != after.get(f):
            FAILS.append("%s moved %s" % (name, f))
    return read_leg(name, binary, out, home, rc, t)


def read_leg(name, binary, out, home, rc, t):
    logdir = home / "log"
    files = sorted(p.name for p in logdir.iterdir() if p.name.endswith(".log"))
    rec = {"bin": str(binary), "md5": md5(binary), "rc": rc, "port_s": t,
           "files": [(p, (logdir / p).stat().st_size) for p in files],
           "channels": {}}
    for c in logread.CHANNELS:
        p = logread.live(logdir, c)
        txt = p.read_bytes().decode("latin-1", errors="replace") if p else ""
        lines = txt.splitlines()
        rec["channels"][c] = {
            "live": p.name if p else None,
            "bytes": len(txt),
            "lines": len(lines),
            "d12": sum(1 for l in lines if D12_KEY in l),
            "last": lines[-1][:70] if lines else "",
            # The last line cLog writes BEFORE the key is applied is
            # checkConfigIni's (main.cpp:258, ten lines above :269).  After it,
            # a dead channel may carry exactly ONE line: the `EOF` that close()
            # writes STRAIGHT to the stream (log.cpp:104) without going through
            # write() and therefore without meeting the gate at all.
            "after_gate": (len(lines) - 1 - max(
                (i for i, l in enumerate(lines) if "config.ini is up to date" in l),
                default=len(lines) - 1)),
            "eof_last": bool(lines) and lines[-1].endswith("EOF"),
            "has_anchor": any("config.ini is up to date" in l for l in lines),
        }
    applog = (out / (name + ".applog")).read_text(encoding="latin-1",
                                                  errors="replace")
    rec["applog_d12"] = sum(1 for l in applog.splitlines() if D12_KEY in l)
    print("    rc=%s  files: %s" % (rc, rec["files"]))
    for c, v in rec["channels"].items():
        print("    %-13s live=%-24s %6d B  %4d lines  D12 %d  after-the-gate %d"
              " EOF-last %s"
              % (c, v["live"], v["bytes"], v["lines"], v["d12"],
                 v["after_gate"], v["eof_last"]))
    print("    D12 lines on the console: %d" % rec["applog_d12"])
    return rec


def rescore(name, binary, out):
    """The same reading, on a leg that already ran (no launch)."""
    print("\n=== %s (re-scored from disk): %s" % (name, binary))
    return read_leg(name, binary, out, Path(out) / ("farm_" + name) / ".spacecrafter",
                    0, 0.0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--pre", required=True)
    ap.add_argument("--post", required=True)
    ap.add_argument("--score-only", action="store_true",
                    help="re-score the farms already in <out>, no launch")
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)

    if a.score_only:
        res = {"pre": rescore("pre", a.pre, out), "post": rescore("post", a.post, out)}
    else:
        res = {"pre": leg("pre", a.pre, out), "post": leg("post", a.post, out)}

    print("\n--- what write_log=false does on each binary ---")
    for c in logread.CHANNELS:
        p, q = res["pre"]["channels"][c], res["post"]["channels"][c]
        print("  %-13s pre %-26s %6d B / %d lines   post %-16s %6d B / %d lines"
              % (c, p["live"], p["bytes"], p["lines"],
                 q["live"], q["bytes"], q["lines"]))
        for arm, v in (("pre", p), ("post", q)):
            if not v["eof_last"]:
                FAILS.append("%s: %s does not end with close()'s EOF line"
                             % (arm, c))
            if not v["has_anchor"]:
                # No checkConfigIni line on this channel, so the only thing it
                # can legitimately hold with the key off is that EOF line.
                if v["lines"] != 1:
                    FAILS.append("%s: %s holds %d line(s) with the key off, "
                                 "expected 1 (the EOF)" % (arm, c, v["lines"]))
            elif v["after_gate"] != 1:
                FAILS.append("%s: %s carries %d line(s) after the gate - with "
                             "the key off the only line past main.cpp:269 may "
                             "be close()'s EOF" % (arm, c, v["after_gate"]))
        if c != "spacecrafter" and (p["bytes"], q["bytes"]) != (13, 13):
            FAILS.append("%s: %d B pre / %d B post, expected 13 B (EOF only) "
                         "on both" % (c, p["bytes"], q["bytes"]))
    if res["post"]["channels"]["spacecrafter"]["d12"] != 5:
        FAILS.append("post: %d D12 lines in the log with the key off, expected 5"
                     % res["post"]["channels"]["spacecrafter"]["d12"])
    if res["post"]["applog_d12"] != 5:
        FAILS.append("post: %d D12 lines on the console, expected 5"
                     % res["post"]["applog_d12"])
    if res["pre"]["channels"]["spacecrafter"]["d12"] != 0:
        FAILS.append("pre: the baseline binary printed a retention line")
    # THE CRITERION THAT CAN FAIL: the two arms' internal logs, normalised for
    # what MUST differ between two processes (ticks prefix, pid, ram figures,
    # the farm's own path), must differ by EXACTLY the five retention lines.
    import re as _re
    VOLATILE = ("ram ", "getpid", "CONFIG DIR", "Lock file is", "farm_",
                "Total ram", "Shared ram", "Buffer ram", "Total high",
                "Free high", "Mem unit")

    def norm(path):
        out = []
        for l in Path(path).read_bytes().decode("latin-1").splitlines():
            l = _re.sub(r"^[0-9]{6,}: ", "", l)
            if any(v in l for v in VOLATILE):
                continue
            out.append(l)
        return out

    import difflib
    a = norm(out / "farm_pre/.spacecrafter/log/spacecrafter.log")
    b = norm(out / "farm_post/.spacecrafter/log/spacecrafter.log")
    added = [l[2:] for l in difflib.unified_diff(a, b, n=0, lineterm="")
             if l.startswith("+") and not l.startswith("+++")]
    removed = [l[2:] for l in difflib.unified_diff(a, b, n=0, lineterm="")
               if l.startswith("-") and not l.startswith("---")]
    print("\n--- pre -> post, normalised: %d added, %d removed ---"
          % (len(added), len(removed)))
    for l in added:
        print("  + " + l[:100])
    for l in removed:
        print("  - " + l[:100])
    if len(removed) != 0:
        FAILS.append("the key-off log LOST %d line(s) on the new binary"
                     % len(removed))
    if len(added) != 5 or any(D12_KEY not in l for l in added):
        FAILS.append("the key-off log gained %d line(s), of which %d are not "
                     "retention lines" % (len(added),
                                          sum(1 for l in added if D12_KEY not in l)))
    res["diff"] = {"added": added, "removed": removed}
    (out / "result.json").write_text(json.dumps(res, indent=1))
    for f in FAILS:
        print("FAIL: " + f)
    print("\n%d FAIL" % len(FAILS))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
