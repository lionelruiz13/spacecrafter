#!/usr/bin/env python3
"""F35 / §5.81 — the distance-0 guard branch OBSERVED, both ways, two launches.

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f35_branch.py <absOutdir> [--bin <binary>]

`f35_degenerate.py` shows the VALUE flip (NaN -> (0,0)) on the dumped member.
This shows the BRANCH: a gdb breakpoint on the guard's only statement
(ModularBody.hpp:508) counts how many times a body entered
`ModularBody::update` with `distance == 0`.

  G1  a non-degenerate scene only (the default launch, clock stepped, one dump):
      the count must be ZERO. Note this also settles a second question the dump
      cannot: `MilkyWay` and `Universe` carry `"dist":0` in every dump, and G1
      says whether they REACH the guard (they do not — they are never
      `update()`d, so their `screen` is the member's default, not a computed
      centre).
  G2  the same scene plus `camera action transition_to target point`: the count
      must be > 0, and it must be zero BEFORE that command in the same launch —
      which G1 is, driven identically up to that point.

Two launches rather than one because the counterpart branch is far too hot to
instrument (64 visible bodies x every frame), so the discrimination is carried
by the scene.
"""

import argparse, json, os, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27

HERE = Path(__file__).resolve().parent
GDB = HERE / "f35_branch.gdb"
JD0 = 2461233.5


def leg(out, binary, tag, transition):
    sess = f27.Session(out, tag, binary,
                       launch_prefix=("gdb", "-q", "-batch", "-x", str(GDB), "--args"),
                       port_wait=180)
    drv = sess.client("drv")
    try:
        for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
                  "timerate rate 0"):
            drv.send(c, 0.6)
        drv.send(f"date jday {JD0:.9f}", 1.5)
        time.sleep(6.0)                      # let frames accumulate on the scene
        if transition:
            drv.send("camera action transition_to target point name Space", 1.5)
            time.sleep(6.0)                  # the same dwell, on the other side
        drv.send(f"body action dual_dump filename /tmp/f35_branch_{tag}.json", 1.5)
        sess.stop(drv, exit_wait=90)
    except Exception:
        try:
            sess.proc.kill()
        except Exception:
            pass
        raise
    text = (out / f"{tag}.applog").read_text(errors="replace")
    return text.count("F35-GUARD-TAKEN"), len(text.splitlines())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    out = Path(a.outdir); out.mkdir(parents=True, exist_ok=True)

    res = {"binary": a.bin, "binary_md5": f27.md5(a.bin)}
    res["G1_no_transition"], res["G1_loglines"] = leg(out, a.bin, "f35g1", False)
    res["G2_transition"], res["G2_loglines"] = leg(out, a.bin, "f35g2", True)
    res["ok"] = (res["G1_no_transition"] == 0 and res["G2_transition"] > 0)
    (out / "f35_branch.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1), flush=True)
    print(("OK   " if res["ok"] else "FAIL ")
          + f"branch both ways: G1(no distance-0 body) = {res['G1_no_transition']}, "
            f"G2(after transition_to point) = {res['G2_transition']}", flush=True)
    return 0 if res["ok"] else 1


if __name__ == "__main__":
    sys.exit(main())
