#!/usr/bin/env python3
"""F108 -- THE RETENTION WINDOW, MEASURED LAUNCH BY LAUNCH.

    cd claude/harness
    DISPLAY=:2 python3 f108_rotate.py <absOutdir> --home real --launches 5
    DISPLAY=:2 python3 f108_rotate.py <absOutdir> --home <farmdir> --launches 10
    python3 f108_rotate.py <absOutdir> --report            # score, no launch

WHAT IT MEASURES.  Every channel now keeps the last LOG_RETENTION_LAUNCHES
launches by numbered rotation at open (INTENT Sec.11.230 on Sec.5.115,
Sec.11.173(b), the tester's R20 "8 launches").  This driver launches the
application N times against ONE log directory and, after each launch, asserts
the whole claim rather than the file count alone:

  count     min(pre + k, W) files per channel, contiguous indices 1..W-1
  identity  the INODE chain -- rename preserves it, so `X.1.log` after launch k
            IS the file `X.log` was after launch k-1.  No log line carries a
            wall-clock date (the only prefix any line gets is SDL_GetTicks,
            log.cpp:124-126), so the dispatch's "first-line stamp" does not
            exist; the inode is the key that does, and it covers the three
            channels no command ever writes to.
  content   `My getpid() is <pid>` (main.cpp:239) for the INTERNAL channel and
            a per-launch token echoed by app_command_interface.cpp:210 for the
            SCRIPT channel -- two independent confirmations of the inode chain.
  D12       the `Log retention (` lines: one per channel plus one per legacy
            pile, in the live spacecrafter.log AND (under print_log) in the
            process's own stdout.
  legacy    the pre-rotation `script-YY.MM.DD.log` pile: count and bytes
            unchanged by every launch -- the app must never touch it (D9).

Each launch is a fresh process, preceded by the /proc/<pid>/comm
concurrent-instance probe, with the real home's config.ini and ssystem.ini md5
asserted in == out.  State accumulates in <out>/launches.jsonl, so a run can be
taken five launches at a time and scored afterwards with --report.
"""
import argparse
import hashlib
import json
import os
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, no_instance                              # noqa: E402
import logread                                                       # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
#: md5'd in == out around every launch.  `startup.sts` is in the list because
#: the app PLAYS it at launch and `ScriptAnnotator::flush` rewrites the file it
#: played whenever a line earns a diagnostic (F90's farm hazard) -- and in the
#: real-home legs that file is the field's own.
FROZEN = ("config.ini", "ssystem.ini", "scripts/fscripts/startup.sts")
LOCK = Path("/tmp/spacecrafter.lock")
D12_KEY = "Log retention ("

FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def note(m):
    NOTES.append(m)
    print("NOTE: " + m, flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def snapshot(logdir):
    """Everything the assertions need, from the file system only."""
    logdir = Path(logdir)
    snap = {"channels": {}, "dirents": len(list(logdir.iterdir()))}
    for c in logread.CHANNELS:
        live = logdir / (c + ".log")
        ent = []
        for p in ([live] if live.is_file() else []) + logread.archives(logdir, c):
            st = p.stat()
            ent.append({"name": p.name, "size": st.st_size, "inode": st.st_ino,
                        "mtime": st.st_mtime})
        leg = logread.legacy(logdir, c)
        snap["channels"][c] = {
            "files": ent,
            "legacy_count": len(leg),
            "legacy_bytes": sum(p.stat().st_size for p in leg),
        }
    return snap


def d12_lines(logdir):
    txt = logread.text(logdir, "spacecrafter")
    return [l for l in txt.splitlines() if D12_KEY in l]


def run_launch(binary, home_root, out, k, token, window):
    """One launch, then one snapshot.  Returns the record for launches.jsonl."""
    home = Path(home_root) / ".spacecrafter"
    logdir = home / "log"
    real = home.resolve() == REAL_HOME.resolve()
    before = {f: md5(home / f) for f in FROZEN if (home / f).is_file()}
    hits = no_instance()
    if hits:
        raise RuntimeError("another spacecrafter is running: %s" % hits)
    lock_before = LOCK.exists()

    app = App(binary, home_root, out, "L%02d" % k)
    t_port = app.start()
    # The token goes through the command surface, so the SCRIPT channel gets
    # `Execute_command <token>` (app_command_interface.cpp:210) whether or not
    # the command exists -- it does not, which is the point: no side effect.
    app.send(token, 1.0)
    rc = app.stop()
    time.sleep(0.5)

    after = {f: md5(home / f) for f in FROZEN if (home / f).is_file()}
    rec = {"k": k, "token": token, "pid": app.proc.pid, "rc": rc,
           "port_s": t_port, "real_home": real,
           "md5_in": before, "md5_out": after,
           "lock_before": lock_before, "lock_after": LOCK.exists(),
           "snap": snapshot(logdir),
           "d12": d12_lines(logdir),
           "applog_d12": sum(1 for l in
                             (out / ("L%02d.applog" % k)).read_text(
                                 encoding="latin-1", errors="replace").splitlines()
                             if D12_KEY in l),
           "window": window}
    for f in before:
        if before[f] != after.get(f):
            fail("launch %d moved %s: %s -> %s" % (k, f, before[f], after.get(f)))
    if rc != 0:
        fail("launch %d exited rc=%s" % (k, rc))
    return rec


def score(recs, window, pre, out):
    """Score every claim over the accumulated launches."""
    if not recs:
        print("no launches recorded")
        return
    print("\n--- file count per channel, after each launch (window %d) ---" % window)
    print("  k   " + "  ".join("%-13s" % c for c in logread.CHANNELS) + "  expected")
    for r in recs:
        k = r["k"]
        exp = min(pre + k, window)
        got = [len(r["snap"]["channels"][c]["files"]) for c in logread.CHANNELS]
        print("  %-3d " % k + "  ".join("%-13d" % g for g in got) + "  %d" % exp)
        if all(g == exp for g in got):
            ok("launch %d: every channel holds %d file(s)" % (k, exp))
        else:
            fail("launch %d: counts %s, expected %d everywhere" % (k, got, exp))
        for c in logread.CHANNELS:
            names = [e["name"] for e in r["snap"]["channels"][c]["files"]]
            want = [c + ".log"] + ["%s.%d.log" % (c, j) for j in range(1, exp)]
            if names != want:
                fail("launch %d channel %s: %s, expected %s" % (k, c, names, want))
            idx = [int(n.split(".")[1]) for n in names[1:]]
            if idx and max(idx) > window - 1:
                fail("launch %d channel %s: index %d beyond the window"
                     % (k, c, max(idx)))

    print("\n--- inode chain: X.j.log after launch k IS X.(j-1).log after k-1 ---")
    chain_ok = chain_bad = 0
    for a, b in zip(recs, recs[1:]):
        if b["k"] != a["k"] + 1:
            note("launches %d and %d are not consecutive; chain check skipped"
                 % (a["k"], b["k"]))
            continue
        for c in logread.CHANNELS:
            prev = a["snap"]["channels"][c]["files"]          # [live, .1, .2, ...]
            cur = b["snap"]["channels"][c]["files"]
            for j in range(1, len(cur)):
                if j - 1 >= len(prev):
                    continue
                if cur[j]["inode"] == prev[j - 1]["inode"]:
                    chain_ok += 1
                else:
                    chain_bad += 1
                    fail("chain broken: %s after launch %d (inode %d) is not %s "
                         "after launch %d (inode %d)"
                         % (cur[j]["name"], b["k"], cur[j]["inode"],
                            prev[j - 1]["name"], a["k"], prev[j - 1]["inode"]))
    (ok if chain_bad == 0 else fail)(
        "%d inode identities hold, %d broken" % (chain_ok, chain_bad))

    print("\n--- content keys: the pid line and the per-launch token ---")
    # Read off the LIVE directory as it stands after the last recorded launch,
    # so the keys are checked against the files themselves and not against the
    # snapshot that claimed them.
    pids = {r["k"]: r["pid"] for r in recs}
    tokens = {r["k"]: r["token"] for r in recs}
    hit = miss = 0
    last = recs[-1]
    ld = Path(last["logdir"])
    for j, p in enumerate(logread.history(ld, "spacecrafter")):
        want_k = last["k"] - j
        if want_k not in pids:
            continue
        txt = p.read_bytes().decode("latin-1", errors="replace")
        if ("My getpid() is %d" % pids[want_k]) in txt:
            hit += 1
        else:
            miss += 1
            fail("%s does not carry launch %d's pid %d"
                 % (p.name, want_k, pids[want_k]))
    for j, p in enumerate(logread.history(ld, "script")):
        want_k = last["k"] - j
        if want_k not in tokens:
            continue
        txt = p.read_bytes().decode("latin-1", errors="replace")
        if tokens[want_k] in txt:
            hit += 1
        else:
            miss += 1
            fail("%s does not carry launch %d's token %s"
                 % (p.name, want_k, tokens[want_k]))
    (ok if miss == 0 else fail)("%d content keys hit, %d missed" % (hit, miss))

    print("\n--- D12 lines ---")
    # One rotation line per channel, plus one line for each channel that has a
    # legacy dated pile -- which is the script channel in the field home and
    # NOTHING in a fresh farm, so the expected count is read off the pre-state
    # rather than assumed to be 6.
    exp_lines = len(logread.CHANNELS) + sum(
        1 for c in logread.CHANNELS
        if pre_snap["channels"][c]["legacy_count"] > 0)
    print("  expecting %d line(s) per launch (%d channels + %d legacy pile(s))"
          % (exp_lines, len(logread.CHANNELS), exp_lines - len(logread.CHANNELS)))
    for r in recs:
        lines = r["d12"]
        deleted = [l for l in lines if "deleted nothing" not in l
                   and ", deleted " in l]
        exp_del = 5 if pre + r["k"] > window else 0
        print("  k=%-3d %d line(s) in spacecrafter.log, %d in the applog, "
              "%d with a deletion (expected %d)"
              % (r["k"], len(lines), r["applog_d12"], len(deleted), exp_del))
        if len(lines) != exp_lines:
            fail("launch %d: %d `%s` lines, expected %d"
                 % (r["k"], len(lines), D12_KEY, exp_lines))
        if r["applog_d12"] != exp_lines:
            fail("launch %d: %d D12 lines on the console, expected %d"
                 % (r["k"], r["applog_d12"], exp_lines))
        if len(deleted) != exp_del:
            fail("launch %d: %d deletion clauses, expected %d"
                 % (r["k"], len(deleted), exp_del))
        for line in deleted:
            print("      " + line[line.index(", deleted "):].strip()[:150])

    print("\n--- the legacy pile, and the frozen pair ---")
    for r in recs:
        sc = r["snap"]["channels"]["script"]
        if (sc["legacy_count"], sc["legacy_bytes"]) != (pre_legacy[0], pre_legacy[1]):
            fail("launch %d moved the legacy pile: %d files/%d B, expected %d/%d"
                 % (r["k"], sc["legacy_count"], sc["legacy_bytes"],
                    pre_legacy[0], pre_legacy[1]))
    ok("legacy pile %d files / %d bytes across %d launches"
       % (pre_legacy[0], pre_legacy[1], len(recs)))
    for r in recs:
        for f, v in r["md5_in"].items():
            if r["md5_out"].get(f) != v:
                fail("launch %d: %s moved" % (r["k"], f))
    ok("config.ini and ssystem.ini md5 in == out on every launch")

    res = {"window": window, "pre": pre, "launches": len(recs),
           "fails": FAILS, "notes": NOTES,
           "legacy": {"count": pre_legacy[0], "bytes": pre_legacy[1]}}
    (out / "result.json").write_text(json.dumps(res, indent=1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--home", default="real",
                    help="'real' for $HOME, or a farm root holding .spacecrafter")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    ap.add_argument("--launches", type=int, default=0)
    ap.add_argument("--tag", default="f108")
    ap.add_argument("--report", action="store_true")
    a = ap.parse_args()

    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    home_root = Path.home() if a.home == "real" else Path(a.home)
    logdir = home_root / ".spacecrafter" / "log"
    window = logread.window()
    state = out / "launches.jsonl"
    recs = [json.loads(l) for l in state.read_text().splitlines()] if state.is_file() else []

    global pre_legacy, pre_snap
    pre_file = out / "pre.json"
    if not pre_file.is_file():
        snap = snapshot(logdir)
        pre = max(len(snap["channels"][c]["files"]) for c in logread.CHANNELS)
        pre_file.write_text(json.dumps(
            {"pre": pre, "snap": snap,
             "legacy": [snap["channels"]["script"]["legacy_count"],
                        snap["channels"]["script"]["legacy_bytes"]]}, indent=1))
    pre_state = json.loads(pre_file.read_text())
    pre = pre_state["pre"]
    pre_snap = pre_state["snap"]
    pre_legacy = tuple(pre_state["legacy"])

    print("binary  = %s  md5 %s" % (a.bin, md5(a.bin)))
    print("home    = %s  (real=%s)" % (home_root, home_root == Path.home()))
    print("logdir  = %s  window = %d  pre-existing files per channel = %d"
          % (logdir, window, pre))
    print("legacy  = %d file(s), %d bytes" % pre_legacy)

    if a.launches:
        done = max([r["k"] for r in recs], default=0)
        for k in range(done + 1, done + a.launches + 1):
            print("\n=== launch %d ===" % k, flush=True)
            rec = run_launch(a.bin, home_root, out, k, "%s_launch_%d" % (a.tag, k),
                             window)
            rec["logdir"] = str(logdir)
            with state.open("a") as fh:
                fh.write(json.dumps(rec) + "\n")
            recs.append(rec)
            print("  rc=%s port=%.1fs files/channel=%s"
                  % (rec["rc"], rec["port_s"],
                     [len(rec["snap"]["channels"][c]["files"])
                      for c in logread.CHANNELS]), flush=True)

    if a.report or a.launches:
        score(recs, window, pre, out)
    print("\n%d FAIL, %d NOTE" % (len(FAILS), len(NOTES)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
