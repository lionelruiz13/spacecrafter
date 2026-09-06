#!/usr/bin/env python3
"""F98 - THE `14.sts` ABORT, REPRODUCED ON ITS OWN LAUNCH.

    cd claude/harness && DISPLAY=:2 python3 artifacts/f98/f98_repro14.py \
        <absOutdir> [--shows fscripts/14.sts] [--budget 240]

WHAT THE SOAK'S SHAKEDOWN SAW (2026-09-06, `/home/claude/sc-f98/shake`, binary
`46849f69`, code `master-beta @ 22499f04`): playing the tester's own
`fscripts/14.sts` - 528 uncommented `body action load` lines, most of them
`mode in_galaxy ... filename Star_*` - the application logged
`(Error): Can't allocate buffer in 'uniform BufferMgr' !` ONCE PER BODY while
reporting `Succesfull loading ojm <name>` for the same body, then
`Frame stall detected` with the engine's own stack trace parked in
`App::draw` (app.cpp:831) inside `__platform_wait`, then `This frame stall is
very long`, then `CRITICAL : Device lost while waiting frame completion`, then
`terminate called without an active exception` - SIGABRT, exit -6.

THIS SCRIPT ISOLATES IT.  One fresh farm, one fresh launch, one playlist given
on the command line, and three things counted from the application's own
stream: the per-body buffer error, the device-lost line, and the exit status.
Run it with `--shows fscripts/panorama1.sts` for the NEGATIVE arm: a show that
authors nothing must produce zero of the first two and exit 0 on
`shutdown action now`.

It reuses the soak driver's own farm builder and wire, so the farm shape and
the boundary (every played `.sts` a COPY) are the same ones the campaign runs
under; nothing here writes into the real HOME.
"""

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(HERE))
import f95_soak as S                                          # noqa: E402

BUFFER_ERR = "Can't allocate buffer in 'uniform BufferMgr'"
DEVICE_LOST = "Device lost while waiting frame completion"
TERMINATE = "terminate called"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--shows", default="fscripts/14.sts",
                    help="comma-separated, played in order")
    ap.add_argument("--budget", type=float, default=240.0,
                    help="seconds to watch after the last play command")
    ap.add_argument("--gap", type=float, default=1.0,
                    help="seconds between two `script action play` commands. "
                         "The soak plays shows SEQUENTIALLY, so a gap long "
                         "enough for the first show to finish is the arm that "
                         "matches it; the 1 s default overlaps them, which is "
                         "a different condition and must not be confused with "
                         "it")
    ap.add_argument("--bin", default=str(S.DEFAULT_BIN))
    ap.add_argument("--dump-after-each", action="store_true",
                    help="take a dual dump after every show and count the two "
                         "halves - the bisect that attributes a dual-path "
                         "desync to ONE show instead of to a cycle")
    a = ap.parse_args()

    out = Path(a.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    shows = [s.strip() for s in a.shows.split(",") if s.strip()]

    hits = S.no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)

    frozen = S.frozen_md5s(dirs=["fscripts"])
    farm = out / "farm"
    asserts = S.build_farm(farm, shows)
    home = farm / ".spacecrafter"
    applog = out / "repro.applog"
    print("farm    : %s (%d asserts)" % (farm, len(asserts)))
    print("shows   : %s" % shows)
    print("frozen  : %d files, digest %s" % (len(frozen),
                                             S.frozen_digest(frozen)[:8]))

    env = {**os.environ, "HOME": str(farm),
           "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([a.bin], cwd=str(home), stdout=open(applog, "w"),
                            stderr=subprocess.STDOUT, env=env)
    print("pid     : %d at %s" % (proc.pid, time.strftime("%H:%M:%S")))
    wire = S.Wire(logon=True)
    if not wire.connect(time.time() + 300):
        proc.kill()
        raise SystemExit("the application never accepted a connection")
    print("TCP up  : %s" % time.strftime("%H:%M:%S"))

    t0 = time.time()
    events = []
    import dumpread

    def snapshot(i, rel):
        """A dual dump taken AFTER show <rel> has had its gap to run."""
        if not a.dump_after_each or proc.poll() is not None:
            return
        f = out / ("dump_%02d_%s.json"
                   % (i, rel.split("/")[-1].replace(".sts", "")))
        if 1:
            wire.send("body action dual_dump filename %s" % f, 1.5)
            end = time.time() + 60
            while time.time() < end and not (
                    f.exists() and f.stat().st_size > 0
                    and Path(str(f) + ".navstr").exists()):
                time.sleep(0.4)
            time.sleep(0.6)
            try:
                # require_old=False (added F101): this bisect EXISTS to report
                # an old half that goes to 1 and then to 0 - Sec.5.143's own
                # evidence - so the guard at the reader must not raise here.
                _h, pairs, missing_new, missing_old = dumpread.load_dump(
                    f, require_old=False)
                row = {"after": rel, "both": len(pairs),
                       "old_only": len(missing_new),
                       "new_only": len(missing_old),
                       "bodies_old": len(pairs) + len(missing_new),
                       "bodies_new": len(pairs) + len(missing_old)}
            except Exception as e:                                # noqa: BLE001
                row = {"after": rel, "error": repr(e)}
            events.append(row)
            print("   dump : %s" % row)

    for i, rel in enumerate(shows):
        if i:
            end = time.time() + a.gap
            while time.time() < end and proc.poll() is None:
                time.sleep(0.5)
            if proc.poll() is not None:
                break
            snapshot(i - 1, shows[i - 1])
        wire.send("script action play filename %s" % rel, 1.0)
        print("play    : %s at %+.1f s" % (rel, time.time() - t0))
    if shows and proc.poll() is None:
        end = time.time() + a.gap
        while time.time() < end and proc.poll() is None:
            time.sleep(0.5)
        snapshot(len(shows) - 1, shows[-1])

    dead_at = None
    while time.time() - t0 < a.budget:
        if proc.poll() is not None:
            dead_at = time.time() - t0
            break
        time.sleep(1.0)
    txt = applog.read_text(encoding="latin-1", errors="replace")
    n_buf = txt.count(BUFFER_ERR)
    n_lost = txt.count(DEVICE_LOST)
    n_term = txt.count(TERMINATE)
    n_load = txt.count("Execute_command body action load")

    quit_rc, quit_wall = None, None
    if proc.poll() is None:
        q0 = time.time()
        wire.send("shutdown action now", 0.4)
        try:
            proc.wait(timeout=120)
        except subprocess.TimeoutExpired:
            proc.kill()
        quit_rc, quit_wall = proc.returncode, round(time.time() - q0, 2)

    res = {
        "shows": shows, "bin": a.bin, "bin_md5": S.md5(a.bin)[:8],
        "body_action_load_executed": n_load,
        "buffer_allocation_errors": n_buf,
        "device_lost_lines": n_lost,
        "terminate_lines": n_term,
        "died_unprompted_at_s": dead_at,
        "exit_status": proc.returncode,
        "quit_exit_code": quit_rc, "quit_wall_s": quit_wall,
        "frozen_moved": [f for f, m in frozen.items()
                         if S.md5(f) != m] if all(Path(f).exists()
                                                  for f in frozen) else "?",
        "applog": str(applog), "proc_after": S.no_instance(),
        "dumps": events,
    }
    (out / "result.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
