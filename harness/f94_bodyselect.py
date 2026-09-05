#!/usr/bin/env python3
"""F94 - DOES `struct if body_selected equal 600` FIRE FOR SATURN?  (INTENT 11.214)

    cd claude/harness && DISPLAY=:2 ./f94_run.sh <absOutdir> --bin <binary> --tag pre
    cd claude/harness && ./f94_bodyselect.py --plan          # the predictions, no launch

WHAT THIS MEASURES, and on which surface.  `Core::setSelectedBodyName`
(`core.cpp:2226-2276`) turns the selected object's ENGLISH name into the number a
script reads as `$body_selected`.  Two of its literals are misspelled - `"Satun"`
(:2255) and `"Ganymed"` (:2251) - so no loaded body matches them and Saturn and
Ganymede fall to the table's 999 default (`core.hpp:752`), which is INTENT
§5.98.  This drives the SHIPPED command surface only: `script action play` on a
scratch script that does `select planet <body>` then `struct if body_selected
equal <n>`, and reads the answer back through the shipped `get status position`.

THE OBSERVABLE, and why it is the clock.  Each branch sets a distinct UTC date;
`get status position` returns `lat;lon;alt;JDay;heading;`
[observed: coreLink.cpp:616-629, sprintf "%2.2f;%3.2f;%10.2f;%10.6f;%10.6f;"],
so field 4 says WHICH branch ran, exactly, as a number, with no locale and no
pixel in the path.  `timerate rate 0` first, so the reading does not drift
between the branch and the read.  Three outcomes are distinguishable, not two:
the if-date, the else-date, and the NEUTRAL date the script sets before the test
- which is what "the play never reached the test" looks like, and it must not be
confused with "the branch did not fire".

THE FOUR CONTROLS, each of which can fail:
  D  literal   `struct if 1 equal 1` - no body, no table.  Fires on BOTH
               binaries or the instrument is broken and nothing here counts.
  C  body      Titan = 604, same chain, literal never misspelled.  Fires on
               BOTH binaries: this is what makes A's and B's zeros attributable
               to the two typos rather than to a selection that never happened.
  `get status object` after every play: a NON-EMPTY readout is the proof that
               the body really is selected, so a pre-fix else-branch cannot be
               explained by `select planet Saturn` having done nothing.
  E  consumer  the ONE shipped consumer's own test (`equal 0`) replayed on
               Saturn: the same branch on both binaries is a MEASUREMENT of
               "the shipped script's behaviour does not change", not an
               argument about 999 and 600 both being non-zero.

FUNCTIONAL, not photometric: no pixel is read, no frame compared.  The runner
runs `f56_canary.sh --no-scene` before the launch and asserts md5 in == out on
every real-home file this run could reach.
"""

import argparse
import glob
import hashlib
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REAL_HOME = Path.home() / ".spacecrafter"
PORT = 7805
SCRATCH_DIR = "f94"                       # farm-relative, under scripts/
CONSUMER = "internal/fly_to_selected.sts"  # the one shipped consumer of the var

# ---------------------------------------------------------------- the plan
# jday = SpaceDate::JulianDayFromDateTime(y,1,1,0,0,0)  [space_date.cpp:206-250],
# transcribed and evaluated OFF-LINE; `date utc` feeds StringToJday straight into
# setJDay (app_command_interface.cpp:3999, coreLink.cpp:507) and `get status
# position` prints timeMgr->getJDay(), so these are exact, not approximate.
JD = {2030: 2462502.500000, 2031: 2462867.500000, 2032: 2463232.500000,
      2033: 2463598.500000, 2034: 2463963.500000, 2035: 2464328.500000,
      2036: 2464693.500000, 2037: 2465059.500000, 2038: 2465424.500000,
      2039: 2465789.500000, 2040: 2466154.500000}
NEUTRAL = 2030

# id, script, body, test, if-year, else-year, expected pre, expected post
TESTS = [
    ("D", "d_literal.sts",      None,       "1 equal 1",                2037, 2038, "if",   "if"),
    ("A", "a_saturn.sts",       "Saturn",   "body_selected equal 600",  2031, 2032, "else", "if"),
    ("B", "b_ganymede.sts",     "Ganymede", "body_selected equal 503",  2033, 2034, "else", "if"),
    ("C", "c_titan.sts",        "Titan",    "body_selected equal 604",  2035, 2036, "if",   "if"),
    ("E", "e_consumer_cond.sts", "Saturn",  "body_selected equal 0",    2039, 2040, "else", "else"),
]

_T0 = time.time()


def log(msg):
    print("[%7.1fs] %s" % (time.time() - _T0, msg), flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def no_instance():
    """The §11.134(b) probe: /proc/<pid>/comm, every account, no self-match."""
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def print_plan():
    print("F94 - PREDICTIONS, committed before the first launch (INTENT 11.214)")
    print()
    print("observable = field 4 of `get status position` (JDay); "
          "neutral = %d-01-01 = %.6f" % (NEUTRAL, JD[NEUTRAL]))
    print()
    hdr = ("%-3s %-22s %-9s %-26s %-16s %-16s %-9s %-9s"
           % ("id", "script", "body", "test", "if -> jday", "else -> jday",
              "PRE-fix", "POST-fix"))
    print(hdr)
    print("-" * len(hdr))
    for tid, scr, body, test, yif, yel, pre, post in TESTS:
        print("%-3s %-22s %-9s %-26s %-16s %-16s %-9s %-9s"
              % (tid, scr, body or "-", test,
                 "%d %.1f" % (yif, JD[yif]), "%d %.1f" % (yel, JD[yel]),
                 pre, post))
    print()
    print("PRE-fix  = binary built from code at 5a1e5749 (md5 f7112cb9), the two typos present")
    print("POST-fix = the same tree with `Satun`->`Saturn` and `Ganymed`->`Ganymede`")
    print()
    print("A run in which D or C answers anything but its if-date REFUTES the")
    print("instrument, not the code, and the A/B cells of that run mean nothing.")
    print("A cell that answers the NEUTRAL date means the play never reached the")
    print("test and is a FAIL, never a 'branch did not fire'.")


# ------------------------------------------------------------------- the farm

def build_farm(farm):
    """f55_farm.sh + the real directories every WRITER in this run can reach.

    Returns the asserted shape, so the record says what the farm IS."""
    farm = Path(farm)
    if farm.exists():
        shutil.rmtree(farm)
    r = subprocess.run([str(HERE / "f55_farm.sh"), str(farm)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("f55_farm.sh failed: %s%s" % (r.stdout, r.stderr))
    dst = farm / ".spacecrafter"

    # (1) sessions/ real - nothing here saves, but `session action save` exists on
    #     the surface and b3 symlinks the directory (F90's hazard 1).
    (dst / "sessions").unlink(missing_ok=True)
    (dst / "sessions").mkdir(parents=True, exist_ok=True)
    if (REAL_HOME / "sessions").is_dir():
        for e in (REAL_HOME / "sessions").iterdir():
            (dst / "sessions" / e.name).symlink_to(e)

    # (2) scripts/internal/ real, the CONSUMER a COPY - `ScriptAnnotator::flush`
    #     rewrites the file it played, sibling .tmp + rename IN THE TARGET'S OWN
    #     DIRECTORY (script_annotator.cpp:163-179), so a symlinked directory
    #     sends both writes into the owner's tree (F90's hazard 2).
    cons_dir, cons_name = CONSUMER.split("/", 1)
    (dst / "scripts" / cons_dir).unlink(missing_ok=True)
    (dst / "scripts" / cons_dir).mkdir(parents=True, exist_ok=True)
    for e in (REAL_HOME / "scripts" / cons_dir).iterdir():
        if e.name == cons_name:
            shutil.copy2(e, dst / "scripts" / cons_dir / e.name)
        else:
            (dst / "scripts" / cons_dir / e.name).symlink_to(e)

    # (3) scripts/f94/ real, the scratch scripts COPIED from the tracked source -
    #     same annotator hazard, and the played bytes must be the committed bytes.
    (dst / "scripts" / SCRATCH_DIR).mkdir(parents=True, exist_ok=True)
    for e in sorted((HERE / "f94_scripts").iterdir()):
        shutil.copy2(e, dst / "scripts" / SCRATCH_DIR / e.name)

    # (4) startup.sts a COPY - `f55_farm.sh` deliberately leaves this one to the
    #     caller (it is the file F55 authored), and the app PLAYS it at launch,
    #     so the annotator reaches it before anything else runs.
    real_startup = REAL_HOME / "scripts" / "fscripts" / "startup.sts"
    if real_startup.is_file():
        shutil.copy2(real_startup, dst / "scripts" / "fscripts" / "startup.sts")

    asserts = {
        "config.ini.is_regular_file": (dst / "config.ini").is_file()
                                      and not (dst / "config.ini").is_symlink(),
        "config.ini.md5": md5(dst / "config.ini"),
        "ssystem.ini.is_regular_file": (dst / "ssystem.ini").is_file()
                                       and not (dst / "ssystem.ini").is_symlink(),
        "sessions.is_real_dir": (dst / "sessions").is_dir()
                                and not (dst / "sessions").is_symlink(),
        "scripts.is_real_dir": (dst / "scripts").is_dir()
                               and not (dst / "scripts").is_symlink(),
        "scripts_internal.is_real_dir": (dst / "scripts" / cons_dir).is_dir()
                                        and not (dst / "scripts" / cons_dir).is_symlink(),
        "consumer.is_regular_file": (dst / "scripts" / CONSUMER).is_file()
                                    and not (dst / "scripts" / CONSUMER).is_symlink(),
        "consumer.md5_in": md5(dst / "scripts" / CONSUMER),
        "scratch.is_real_dir": (dst / "scripts" / SCRATCH_DIR).is_dir()
                               and not (dst / "scripts" / SCRATCH_DIR).is_symlink(),
        "scratch.count": len(list((dst / "scripts" / SCRATCH_DIR).iterdir())),
        "startup.is_regular_file": (dst / "scripts" / "fscripts" / "startup.sts").is_file()
                                   and not (dst / "scripts" / "fscripts" / "startup.sts").is_symlink(),
        "log.is_real_dir": (dst / "log").is_dir() and not (dst / "log").is_symlink(),
    }
    bad = [k for k, v in asserts.items() if v is False]
    if bad:
        raise RuntimeError("farm shape assert failed: %s" % bad)
    return asserts


# -------------------------------------------------------------------- the app

class App:
    """One launch, one $LOGON socket, everything recorded (F90's shape)."""

    def __init__(self, binary, farm, outdir, tag):
        self.binary = str(binary)
        self.farm = Path(farm)
        self.home = self.farm / ".spacecrafter"
        self.out = Path(outdir)
        self.tag = tag
        self.applog = self.out / ("%s.applog" % tag)
        self.proc = None
        self.sock = None
        self.rx = b""

    def start(self):
        hits = no_instance()
        if hits:
            raise RuntimeError("another spacecrafter is running: %s" % hits)
        env = {**os.environ, "HOME": str(self.farm),
               "DISPLAY": os.environ.get("DISPLAY", ":2")}
        t0 = time.time()
        self.proc = subprocess.Popen([self.binary], cwd=str(self.home),
                                     stdout=open(self.applog, "w"),
                                     stderr=subprocess.STDOUT, env=env)
        while time.time() - t0 < 180:
            try:
                self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=2)
                break
            except OSError:
                if self.proc.poll() is not None:
                    return None
                time.sleep(1)
        if self.sock is None:
            return None
        self.sock.settimeout(0.2)
        self.send("$LOGON", 1.5)
        return round(time.time() - t0, 1)

    def _drain(self, seconds):
        end = time.time() + seconds
        while time.time() < end:
            try:
                b = self.sock.recv(65536)
                if not b:
                    break
                self.rx += b
            except socket.timeout:
                pass
            except OSError:
                break

    def send(self, cmd, pause=0.8):
        mark = len(self.rx)
        self.sock.sendall((cmd + "\n").encode("latin-1"))
        self._drain(pause)
        return self.rx[mark:].decode("latin-1", "replace")

    def ask(self, cmd, want, timeout=8.0):
        """`ServerSocket::send` puts the NUL terminator on the wire
        (io.cpp:832-834), so the blob is NUL-separated; stripped here once."""
        mark = len(self.rx)
        t0 = time.time()
        self.sock.sendall((cmd + "\n").encode("latin-1"))
        while time.time() - t0 < timeout:
            self._drain(0.4)
            blob = self.rx[mark:].decode("latin-1", "replace").replace("\x00", "")
            hit = want(blob)
            if hit is not None:
                return hit, blob, round(time.time() - t0, 2)
        return None, self.rx[mark:].decode("latin-1", "replace").replace("\x00", ""), \
            timeout

    def _logfile(self, prefix):
        g = sorted(glob.glob(str(self.home / "log" / (prefix + "*.log"))))
        return Path(g[-1]) if g else None

    def script_log_text(self):
        p = self._logfile("script")
        return p.read_bytes().decode("latin-1") if p else ""

    def app_log_text(self):
        p = self._logfile("spacecrafter")
        return p.read_bytes().decode("latin-1") if p else ""

    def applog_text(self):
        return self.applog.read_bytes().decode("latin-1") if self.applog.exists() else ""

    # ---- the two readouts this task uses
    def position(self):
        """`get status position` -> lat;lon;alt;jday;heading; (coreLink.cpp:616)."""
        hit, blob, lat = self.ask(
            "get status position",
            lambda b: (b.strip() or None) if b.count(";") >= 5 else None, timeout=10.0)
        raw = (hit or blob or "").strip()
        m = re.search(r"(-?[\d.]+);\s*(-?[\d.]+);\s*(-?[\d.]+);\s*(-?[\d.]+);\s*(-?[\d.]+);", raw)
        fields = [float(x) for x in m.groups()] if m else None
        return dict(raw=raw, fields=fields, latency_s=lat)

    def obj(self):
        hit, blob, lat = self.ask(
            "get status object",
            lambda b: (b.strip() or None) if len(b.strip()) > 3 else None, timeout=10.0)
        return dict(raw=(blob or "").replace("\x00", "").strip()[:600], latency_s=lat)

    def play(self, rel, budget):
        """One `script action play`, polled to `ScriptMgr: script end`.

        Resumes any `script action pause` the same way F90 does - the field
        config has flag_skip_pause = false, so a show with nobody at the console
        STOPS (harness/README.md, F90 section)."""
        before = self.script_log_text()
        n_load0 = before.count("ScriptMgr: load")
        n_end0 = before.count("ScriptMgr: script end")
        n_pause0 = before.count("ScriptMgr::script action pause")
        t0 = time.time()
        self.send("script action play filename %s" % rel, 1.0)
        seen_load = seen_end = False
        resumed = 0
        while time.time() - t0 < budget:
            txt = self.script_log_text()
            if txt.count("ScriptMgr: load") > n_load0:
                seen_load = True
            if txt.count("ScriptMgr::script action pause") > n_pause0:
                n_pause0 = txt.count("ScriptMgr::script action pause")
                self.send("script action resume", 0.8)
                resumed += 1
            if txt.count("ScriptMgr: script end") > n_end0:
                seen_end = True
                break
            time.sleep(0.4)
        wall = round(time.time() - t0, 1)
        new = self.script_log_text()[len(before):]
        return dict(script=rel, loaded=seen_load, ended=seen_end, wall_s=wall,
                    pauses_resumed=resumed, log_new=new[-4000:])


def hasann(line):
    """`#!` tail detection, the engine's own rule (a `#` inside a quoted run is
    text) - ScriptAnnotator::annotationBegin, script_annotator.cpp:41-54."""
    in_quote = False
    for i, c in enumerate(line):
        if c == '"':
            in_quote = not in_quote
        elif c == "#" and not in_quote:
            return line.find("#!", i) != -1
    return False


def classify(jday, yif, yel):
    if jday is None:
        return "no-reading"
    for tag, y in (("if", yif), ("else", yel), ("neutral", NEUTRAL)):
        if abs(jday - JD[y]) < 0.5:
            return tag
    return "unexpected(%.6f)" % jday


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir", nargs="?")
    ap.add_argument("--bin", default=str(HERE / ".." / ".." / "build-claude" / "src" / "spacecrafter"))
    ap.add_argument("--farm", default=None)
    ap.add_argument("--tag", default="run")
    ap.add_argument("--plan", action="store_true")
    a = ap.parse_args()
    if a.plan:
        print_plan()
        return 0
    if not a.outdir:
        ap.error("outdir required")

    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    farm = Path(a.farm or ("/home/claude/sc-f94/farm-%s" % a.tag))
    binary = Path(a.bin).resolve()

    result = dict(tag=a.tag, when=time.strftime("%Y-%m-%d %H:%M:%S %Z"),
                  binary=str(binary), binary_md5=md5(binary),
                  display=os.environ.get("DISPLAY"), farm=str(farm),
                  predictions={t[0]: dict(script=t[1], body=t[2], test=t[3],
                                          if_year=t[4], else_year=t[5],
                                          expect_pre=t[6], expect_post=t[7])
                               for t in TESTS},
                  steps=[], fails=[])

    log("binary %s  md5 %s" % (binary, result["binary_md5"]))
    result["farm_shape"] = build_farm(farm)
    log("farm %s  (%d scratch scripts)" % (farm, result["farm_shape"]["scratch.count"]))

    app = App(binary, farm, out, a.tag)
    up = app.start()
    if up is None:
        result["fails"].append("the app never answered on port %d" % PORT)
        (out / ("%s_result.json" % a.tag)).write_text(json.dumps(result, indent=1))
        return 1
    log("TCP up in %.1f s" % up)
    result["tcp_up_s"] = up

    try:
        for tid, scr, body, test, yif, yel, exp_pre, exp_post in TESTS:
            rel = "%s/%s" % (SCRATCH_DIR, scr)
            play = app.play(rel, budget=45.0)
            pos = app.position()
            obj = app.obj() if body else dict(raw="(no body in this test)", latency_s=0)
            jday = pos["fields"][3] if pos["fields"] else None
            verdict = classify(jday, yif, yel)
            step = dict(id=tid, body=body, test=test, script=rel,
                        if_year=yif, else_year=yel, expect_pre=exp_pre,
                        expect_post=exp_post, jday=jday, branch=verdict,
                        position_raw=pos["raw"], object_raw=obj["raw"],
                        play=play)
            result["steps"].append(step)
            log("%s %-9s %-26s -> jday %s  branch=%s  (%.1fs, loaded=%s ended=%s)"
                % (tid, body or "-", test, jday, verdict, play["wall_s"],
                   play["loaded"], play["ended"]))
            if not play["ended"]:
                result["fails"].append("%s: the play never reached `script end`" % tid)
            if verdict in ("neutral", "no-reading") or verdict.startswith("unexpected"):
                result["fails"].append("%s: unusable reading (%s)" % (tid, verdict))
            if body and not obj["raw"]:
                result["fails"].append("%s: `get status object` is EMPTY - the "
                                       "selection cannot be shown to have happened" % tid)

        # ---- the ONE shipped consumer, played once, on the same surface
        cons_path = farm / ".spacecrafter" / "scripts" / CONSUMER
        md5_in = md5(cons_path)
        app.send("select planet Saturn", 1.5)
        obj_before = app.obj()
        pos_before = app.position()
        play = app.play(CONSUMER, budget=120.0)
        pos_after = app.position()
        body_txt = cons_path.read_text(encoding="latin-1")
        ann = sum(1 for ln in body_txt.splitlines() if hasann(ln))
        cons = dict(script=CONSUMER, md5_in=md5_in, md5_out=md5(cons_path),
                    annotation_tails=ann, play=play,
                    object_before=obj_before["raw"],
                    position_before=pos_before["raw"],
                    position_after=pos_after["raw"],
                    fields_after=pos_after["fields"])
        result["consumer"] = cons
        log("consumer %s: ended=%s wall=%.1fs md5 %s -> %s  #!=%d"
            % (CONSUMER, play["ended"], play["wall_s"], md5_in[:8],
               cons["md5_out"][:8], ann))
        if not play["ended"]:
            result["fails"].append("consumer: the play never reached `script end`")
    finally:
        # the shipped quit (F90 S9's command), then a bounded wait, then force
        try:
            app.send("shutdown action now", 0.5)
        except OSError:
            pass
        try:
            app.proc.wait(timeout=120)
        except subprocess.TimeoutExpired:
            app.proc.kill()
            app.proc.wait(timeout=30)
        result["exit_code"] = app.proc.returncode
        time.sleep(2)
        result["applog_tail"] = app.applog_text()[-3000:]
        result["script_log_tail"] = app.script_log_text()[-6000:]
        (out / ("%s_result.json" % a.tag)).write_text(json.dumps(result, indent=1))
        (out / ("%s_scriptlog.txt" % a.tag)).write_text(app.script_log_text())

    log("FAILS: %d %s" % (len(result["fails"]), result["fails"]))
    return 1 if result["fails"] else 0


if __name__ == "__main__":
    sys.exit(main())
