#!/usr/bin/env python3
"""F116 -- THE LOG SIZE BUDGET, MEASURED WHILE THE APPLICATION WRITES.

    cd claude/harness
    DISPLAY=:2 python3 f116_budget.py <absOutdir> --bin <binary> --iters 60000
    DISPLAY=:2 python3 f116_budget.py <absOutdir> --bin <mutant> --iters 60000 \
        --budget 4194304 --seed shader:6291456

WHAT IT MEASURES.  Sec.11.237 gives the five log channels a TOTAL size budget
(LOG_RETENTION_BYTES, 1 GiB as shipped): when the sum of their live files and
numbered archives crosses it, the channel holding the most bytes rotates as it
would at open and one D12 line says so.  A 1 GiB log is not a test, so the proof
runs a SCRATCH build whose comparison constant is a few MiB -- `--budget` is
what this driver was told that number is, so it can score the crossing point;
it never sets anything in the application.

THE SHOW.  A generated flat script of `--iters` identical-length lines, played
from an absolute path, preceded by the script surface's own `comment` command:
AppCommandInterface::executeCommand echoes every line to the SCRIPT channel
(app_command_interface.cpp:346) and then, with swapCommand set, writes "this
command has not been executed" and returns (:361-363).  So each line costs
exactly two log lines and has NO side effect -- no refusal, so the script
annotator never rewrites the file it played (F90's farm hazard), and no state
of the engine is touched.  ScriptMgr::update runs commands in a while loop
bounded by a 400 ms deadline (script_mgr.cpp:304), so the channel grows at
megabytes per second and the bound is crossed in seconds.

WHAT IS ASSERTED, not assumed:
  * the pre-launch gate, through f116_assert.sh (comm + /proc/<pid>/exe + port
    7805 + GPU headroom), because the comm probe alone is blind to a staging
    binary (Sec.11.231(j2));
  * the FIELD's config.ini and ssystem.ini md5 in == out, although the launch
    runs on a farm -- the field pair is a precondition of every later task;
  * the show file's md5 in == out (the annotator rewrites what it annotates);
  * the window, POLLED while the app writes: no channel ever holds more than
    LOG_RETENTION_LAUNCHES files and no <channel>.8.log is ever created;
  * every in-session D12 line, read from the process's own stdout (which no
    rotation can move) AND from the log files (where a rotation of the internal
    channel does move them -- that is why both are read).
"""
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, build_farm                                # noqa: E402
import logread                                                        # noqa: E402

FIELD = Path.home() / ".spacecrafter"
D12_KEY = "Log retention ("
#: the in-session line's own marker (the open-time lines do not carry it)
BUDGET_KEY = "-byte budget for all log files"
LINE_LEN = 100          # every generated command line, in characters
ECHO_COST = 14 + 9 + 16 + LINE_LEN + 1     # ticks + "(Info ): " + "Execute_command " + line + NL
SKIP_COST = 14 + 9 + 35 + LINE_LEN + 1     # ticks + "(Info ): " + "this command ..." + line + NL

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


def make_show(path, iters):
    """A flat show whose byte cost is arithmetic, not a measurement."""
    out = ["comment"]
    for i in range(iters):
        head = "f116_filler_%07d" % i
        out.append(head + " " + "p" * (LINE_LEN - len(head) - 1))
    for ln in out[1:]:
        assert len(ln) == LINE_LEN, len(ln)
    Path(path).write_text("\n".join(out) + "\n")
    return ECHO_COST + SKIP_COST


def snapshot(logdir):
    """Names and sizes of the five channels' files -- the observable."""
    logdir = Path(logdir)
    snap, total = {}, 0
    for c in logread.CHANNELS:
        files = []
        live = logdir / (c + ".log")
        for p in ([live] if live.is_file() else []) + logread.archives(logdir, c):
            try:
                files.append((p.name, p.stat().st_size))
            except OSError:
                pass
        snap[c] = files
        total += sum(s for _, s in files)
    return {"channels": snap, "total": total}


def parse_budget_lines(text):
    """The in-session D12 lines, in order, with their numbers pulled out."""
    out = []
    for ln in text.splitlines():
        if D12_KEY not in ln or BUDGET_KEY not in ln:
            continue
        rec = {"raw": ln.strip()}
        m = re.search(r"Log retention \(([a-z]+)\.log\)", ln)
        rec["channel"] = m.group(1) if m else None
        m = re.search(r"the (\d+)-byte budget", ln)
        rec["budget"] = int(m.group(1)) if m else None
        m = re.search(r"was reached \((\d+) bytes in use\)", ln)
        rec["total_before"] = int(m.group(1)) if m else None
        m = re.search(r"Deleted (\S+) \((\d+) bytes", ln)
        rec["deleted"] = m.group(1) if m else None
        rec["deleted_size"] = int(m.group(2)) if m else 0
        m = re.search(r"it now uses (\d+) of the last (\d+) slots", ln)
        rec["kept"] = int(m.group(1)) if m else None
        m = re.search(r", (\d+) bytes in use now", ln)
        rec["total_after"] = int(m.group(1)) if m else None
        out.append(rec)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    ap.add_argument("--farm", default="/home/claude/sc-f116/farm")
    ap.add_argument("--iters", type=int, default=60000)
    ap.add_argument("--budget", type=int, default=1024 * 1024 * 1024,
                    help="the budget the BINARY was built with (scoring only)")
    ap.add_argument("--seed", default="",
                    help="<channel>:<bytes> -- seed the farm's log dir with one "
                         "archive of that size, to put the excess on a channel "
                         "the show never writes")
    ap.add_argument("--label", default="run")
    ap.add_argument("--play-max", type=float, default=180.0)
    a = ap.parse_args()

    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    window = logread.window()

    r = subprocess.run([str(HERE / "f116_assert.sh"), a.label], capture_output=True, text=True)
    print(r.stdout.strip())
    if r.returncode != 0:
        print(r.stderr.strip())
        raise SystemExit("pre-launch assert failed rc=%d" % r.returncode)

    field_in = {f: md5(FIELD / f) for f in ("config.ini", "ssystem.ini")}
    home = build_farm(a.farm)
    logdir = home / "log"
    if a.seed:
        chan, size = a.seed.split(":")
        seeded = logdir / ("%s.1.log" % chan)
        seeded.write_bytes(b"S" * int(size))
        print("seeded %s with %d bytes" % (seeded, int(size)))

    show = out / "f116_chatty.sts"
    per_line = make_show(show, a.iters)
    show_md5 = md5(show)
    print("binary  = %s  md5 %s" % (a.bin, md5(a.bin)))
    print("farm    = %s" % home)
    print("show    = %s  %d lines, %d B per line into the script channel"
          % (show, a.iters, per_line))
    print("budget the binary was built with (scoring) = %d B" % a.budget)

    before_launch = snapshot(logdir)
    print("total the app will FIND in the directory = %d B" % before_launch["total"])
    app = App(a.bin, a.farm, out, a.label)
    t_port = app.start()
    pre = snapshot(logdir)
    print("port after %.1f s; total at rest = %d B" % (t_port, pre["total"]))

    polls = [dict(pre, t=0.0)]
    app.sock.sendall(("script action play filename %s\n" % show).encode())
    t0 = time.time()
    quiet = 0
    while time.time() - t0 < a.play_max:
        time.sleep(0.02)
        s = snapshot(logdir)
        s["t"] = round(time.time() - t0, 3)
        if s["total"] == polls[-1]["total"]:
            quiet += 1
            if quiet >= 60 and s["t"] > 2.0:
                break
        else:
            quiet = 0
        polls.append(s)
    print("play watched for %.1f s, %d polls" % (time.time() - t0, len(polls)))

    app.send("f116_end_marker", 1.0)
    rc = app.stop()
    time.sleep(0.5)
    post = snapshot(logdir)
    applog = (out / ("%s.applog" % a.label)).read_text(encoding="latin-1", errors="replace")

    # --- the D12 lines, from the console (never rotated) and from the files
    console = parse_budget_lines(applog)
    infile_text = ""
    for p in reversed(logread.history(logdir, "spacecrafter")):
        infile_text += p.read_bytes().decode("latin-1", errors="replace")
    infile = parse_budget_lines(infile_text)
    open_lines = [l for l in applog.splitlines() if D12_KEY in l and BUDGET_KEY not in l]

    # WHICH show line crossed the bound: the console stream carries the echoes
    # and the rotation lines in one order, so the last filler seen before a
    # rotation line is the line whose bytes crossed it.  This is the number the
    # pre-registration predicted from arithmetic, so it is computed here rather
    # than by hand afterwards.
    crossings, last_filler = [], None
    for ln in applog.splitlines():
        m = re.search(r"Execute_command f116_filler_(\d+)", ln)
        if m:
            last_filler = int(m.group(1))
        elif D12_KEY in ln and BUDGET_KEY in ln:
            crossings.append(last_filler)
    bursts = [c for i, c in enumerate(crossings)
              if i == 0 or c is None or crossings[i - 1] is None
              or c - crossings[i - 1] > 100]
    print("\n--- show line at each rotation ---")
    print("  bursts start at show line(s): %s" % bursts)

    print("\n--- in-session rotations ---")
    print("  %d on the console, %d still in the log files, %d open-time lines"
          % (len(console), len(infile), len(open_lines)))
    for rec in console[:12] + (console[-4:] if len(console) > 16 else []):
        print("   %-12s before=%-12s deleted=%-22s %-10s after=%s"
              % (rec["channel"], rec["total_before"], rec["deleted"],
                 rec["deleted_size"], rec["total_after"]))

    # --- the claims
    if rc != 0:
        fail("the app exited rc=%s" % rc)
    field_out = {f: md5(FIELD / f) for f in ("config.ini", "ssystem.ini")}
    (ok if field_in == field_out else fail)(
        "field config.ini/ssystem.ini md5 in == out (%s)" % field_in["config.ini"][:8])
    (ok if md5(show) == show_md5 else fail)("the played show was not rewritten")

    worst = max(polls, key=lambda s: s["total"])
    bad_count = [(s["t"], c, len(f)) for s in polls for c, f in s["channels"].items()
                 if len(f) > window]
    bad_index = sorted({n for s in polls + [post] for f in s["channels"].values()
                        for n, _ in f
                        if re.match(r"^[a-z]+\.([0-9]+)\.log$", n)
                        and int(re.match(r"^[a-z]+\.([0-9]+)\.log$", n).group(1)) >= window})
    (ok if not bad_count else fail)(
        "no channel ever held more than %d files (%d polls)" % (window, len(polls)))
    if bad_count:
        print("   " + str(bad_count[:5]))
    (ok if not bad_index else fail)(
        "no archive index reached %d (%s)" % (window, bad_index or "none"))
    print("  peak total seen while running = %d B (%.2f MiB), budget %d B"
          % (worst["total"], worst["total"] / 1048576.0, a.budget))
    print("  total at exit                 = %d B (%.2f MiB)"
          % (post["total"], post["total"] / 1048576.0))

    if console:
        first = console[0]
        over = first["total_before"] - a.budget if first["total_before"] else None
        (ok if first["budget"] == a.budget else fail)(
            "the line names the budget the binary carries (%s)" % first["budget"])
        # `pre` is taken once the app is up, i.e. AFTER any startup rotation has
        # already shed what the directory held; the state the app FOUND is the
        # one measured before the launch.
        if max(pre["total"], before_launch["total"]) > a.budget:
            # The directory was ALREADY over the budget when the app opened it
            # (the seeded arm, and the field's own upgrade case): the first
            # crossing is then the state found, not a line that crossed it.
            note("the directory opened %d B above the budget, so the first "
                 "line's total (+%s B) measures the state found, not a crossing"
                 % (max(pre["total"], before_launch["total"]) - a.budget, over))
        else:
            (ok if over is not None and 0 < over <= 600 else fail)(
                "the first crossing is at most one line above the budget (+%s B)" % over)
        for rec in console:
            if rec["kept"] is None or rec["kept"] > window:
                fail("a line claims %s slots of %d" % (rec["kept"], window))
        seq_ok = all(c["total_after"] is not None for c in console)
        (ok if seq_ok else fail)("every line carries the total it left behind")
    else:
        ok("NO in-session rotation happened (the delivered-budget arm)")

    # --- the instrumented build's own dump, if this binary carries one
    instr = [l for l in applog.splitlines() if l.startswith("[f116-instr]")]
    counters = {}
    for l in instr:
        print("  " + l.strip())
        m = re.search(r"total=(\d+)", l)
        if m:
            counters["total"] = int(m.group(1))
    if "total" in counters:
        # cLog::close writes one "(Info ): EOF" line per channel straight to the
        # stream, outside write(), so the file system holds exactly that much
        # more than the counter -- 13 bytes per channel, and nothing else.
        disk = post["total"]
        expect = counters["total"] + 13 * len(logread.CHANNELS)
        (ok if disk == expect else fail)(
            "the counter is exact: %d on disk == %d counted + 5x13 EOF (delta %d)"
            % (disk, counters["total"], disk - expect))

    res = {"before_launch": before_launch, "label": a.label, "bin": a.bin, "bin_md5": md5(a.bin), "rc": rc,
           "budget_scored": a.budget, "iters": a.iters, "per_line": per_line,
           "window": window, "seed": a.seed,
           "pre": pre, "post": post, "peak": worst,
           "crossing_lines": crossings, "burst_lines": bursts,
           "console_rotations": console, "infile_rotations": len(infile),
           "open_lines": len(open_lines), "instr": instr,
           "polls": len(polls), "fails": FAILS, "notes": NOTES}
    (out / ("result_%s.json" % a.label)).write_text(json.dumps(res, indent=1))
    (out / ("polls_%s.json" % a.label)).write_text(json.dumps(polls))
    print("\n%d FAIL, %d NOTE" % (len(FAILS), len(NOTES)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
