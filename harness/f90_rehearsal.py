#!/usr/bin/env python3
"""F90 - THE TESTER'S DAY IN THE APP, REHEARSED, AND THE DEVELOPER'S SMOKE SUITE.

    cd claude/harness && DISPLAY=:2 ./f90_rehearsal_run.sh <absOutdir> [-- <args>]
    cd claude/harness && DISPLAY=:2 ./f90_rehearsal.py <absOutdir> [--bin B]
                                                       [--show basis/<name>.sts]
                                                       [--inject-fault]

WHAT THIS IS.  ONE launch, on a private farm, in the field's own French locale,
driven ONLY through the shipped command surface, doing the nine things a
planetarium operator does in a sitting:

    launch . author a body . run a shipped show . search . select and read out
    . save . reload . one keyboard ramp . quit

Every other instrument in `claude/harness/` is a proxy built from OUR model of
operation.  This one is the operation.  It is also the smoke suite a newcomer
runs after a build: one command, one exit code.

WHAT IT IS NOT.  It is FUNCTIONAL.  It makes no photometric claim of any kind -
it reads no pixel, it compares no frame, and `f56_canary.sh` is NOT run by it
(the runner runs `--no-scene` before the first launch; the full canary is the
photometric gate and this suite has no photometric member to gate).  A green
here says the shipped commands did what they say, not that the picture is right.

--------------------------------------------------------------------------
THE THREE STEP STATES, and why there are three rather than two
--------------------------------------------------------------------------
    PASS        the shipped command delivered its function.
    DIVERGENCE  the function was delivered AND something the ledger already
                records was observed on the way.  The row - or the dated §11
                clause, where the fact was recorded without a register row - is
                cited AT the step.  This does NOT fail the run: a smoke suite
                that reds on every known-open row is a suite nobody can read.
    FAIL        the function was not delivered.  ANY failed step makes the
                process exit non-zero.

A divergence with NO citation is a FAIL, deliberately: an unattributed
divergence is either a new defect (mint a row) or a broken expectation (fix the
criterion), and both need a human.  Nothing in this file fixes anything.

--------------------------------------------------------------------------
THE FARM, and the three write hazards it closes
--------------------------------------------------------------------------
`f55_farm.sh` (which is `b3_farm.sh` plus a real `scripts/` + `scripts/fscripts/`)
is the base.  This suite adds two more real directories, and the reason for each
is a WRITER in the shipped engine that would otherwise reach the owner's files:

  1. `sessions/`      - `session action save` writes `sessions/<name>.ini`
                        [observed: SessionFile.cpp:57-58, DIRECTORY = "sessions"
                        at SessionFile.hpp:211].  b3 symlinks the directory, so
                        a save in a b3 farm writes the REAL tree (the hazard is
                        already recorded at harness/README.md, F61 section).
  2. `scripts/basis/` - THE ONE THIS TASK FOUND.  When a played script carries a
                        faulty line, `ScriptAnnotator::flush` REWRITES THE
                        SCRIPT FILE ITSELF - sibling `<path>.tmp` + `rename`, in
                        the target's own directory [observed:
                        script_annotator.cpp:163-179].  `f55_farm.sh` makes
                        `scripts/` real but leaves `scripts/basis/` a SYMLINK to
                        the owner's directory, so both the temp and the rename
                        land on the owner's file.  So `scripts/basis/` is rebuilt
                        real, the show under test is COPIED into it, and the
                        owner's copy's md5 is asserted in == out by the runner.
  3. `config.ini`     - `b3_farm.sh:22` COPIES `config.ini` and `ssystem.ini`
                        (`case config.ini|ssystem.ini) cp`) and symlinks the
                        rest at `:24`.  Asserted here anyway (`is_file and not
                        is_symlink`), because two shipped writers rewrite it
                        (Sec.5.42 explicit, Sec.5.112 at a version bump) and a
                        symlink would put them on the field's own file.

`scripts/fscripts/startup.sts` is copied too (same annotator hazard, and it is
the file the app plays at launch, `script_mgr.cpp:384-388` / `app.cpp:689`).

--------------------------------------------------------------------------
LOCALE
--------------------------------------------------------------------------
The farm inherits the field's `config.ini`, which is `app_locale = fr` and
`sky_locale = fr`.  Nothing here changes either.  `app_locale` alone would be
INERT (Sec.5.136: one static catalogue map, the sky locale loads last), so an
English control - which this suite does not need - would have to set BOTH keys.

--------------------------------------------------------------------------
THE REPLY CHANNEL
--------------------------------------------------------------------------
`get`'s answer is queued and drained to the socket SET that sent `$LOGON`, not
to the issuer (Sec.5.47 / Sec.11.135: `checkDataToSend` -> `broadcast`, and
`clientBroadcastTab` is set true in exactly one place, io.cpp:614).  So the one
socket this suite opens sends `$LOGON` first and reads everything that comes
back, replies and log lines together; a reply is recognised by shape.

--------------------------------------------------------------------------
THE NINE STEPS - OBSERVABLE / CHANNEL / CRITERION
--------------------------------------------------------------------------
Printed by `--plan` and by every run BEFORE the launch, so the criteria are on
the record before the evidence is.  `PLAN` below is the single source of that
text (I2: the header does not restate it).
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
sys.path.insert(0, str(HERE))
import b24_select                                    # noqa: E402  (shape matcher)
import dumpread                                      # noqa: E402  (dump grammar)

DEFAULT_BIN = HERE / ".." / ".." / "build-claude" / "src" / "spacecrafter"
REAL_HOME = Path.home() / ".spacecrafter"
PORT = 7805

# The body this rehearsal authors.  Shape and keys are the field's own - the
# `body action load` lines of ~/.spacecrafter/scripts/fscripts/06old.sts, an
# authored satellite of Earth - shortened to stay well inside the 1024-byte
# `tcp_buffer_in_size` the field config sets (Sec.11.43's constraint).
# `coord_func ell_orbit`, never `surface_point`: Sec.5.50 records that
# `body action load ... coord_func surface_point` KILLS THE APP, and a smoke
# suite must not drive a known killer.
PROBE_NAME = "F90ProbeSat"
PROBE_TYPO = "F90PrbeSat"          # --inject-fault: one letter, the tester's typo


def probe_cmd(name):
    return (
        "body action load name %s type Moon parent Earth radius 0.2 "
        "oblateness 0.0 albedo 0.9 lighting false color 0.8,0.9,0.8 "
        "tex_map earth_sats/sattext001.png halo true "
        "tex_halo earth_sats/NULLw.png big_halo false big_halo_size 8 "
        "rot_pole_ra 0.0 rot_pole_de 45.0 rot_periode 0.002 "
        "coord_func ell_orbit orbit_Epoch 2454804.56951084 "
        "orbit_Period 0.0683531232257665 "
        "orbit_visualization_period 0.0683531232257665 "
        "orbit_SemiMajorAxis 7056.81219499864 orbit_Eccentricity 0.0010418 "
        "orbit_Inclination 97.969 orbit_AscendingNode 214.3671 "
        "orbit_LongOfPericenter 336.8124 orbit_MeanLongitude 574.4404 "
        "orbit_color 0.8,0.9,0.8" % name)


PLAN = [
    ("S1", "launch",
     "a TCP connection accepted on 127.0.0.1:%d; the process still alive; the "
     "farm's log/spacecrafter.log; the process's own stdout+stderr" % PORT,
     "TCP connect + the farm's log/ + the launched process's stdout/stderr",
     "PASS iff a connection is accepted within 180 s and the process has not "
     "exited.  RECORDED beside it: every ERROR/WARNING line the app log carries, "
     "and the Sec.5.77 SILENCES - failure-shaped lines written to a RAW stream "
     "(no cLog console tag, log.cpp:32-35) and ABSENT from spacecrafter.log.  A "
     "non-empty silence set is a DIVERGENCE citing Sec.5.77/Sec.11.146, never a "
     "FAIL: the app is up, and the row is open by the owner's own routing.  The "
     "presence of a STALE /tmp/spacecrafter.lock is recorded before the launch "
     "and never removed - it adds one silence of its own (main.cpp:236-245 shells "
     "out `kill -0`), and it is the axis that explains a silence count differing "
     "between two runs of this suite."),

    ("S2", "author a body",
     "the authored body's english name among the dual dump's `type=body` records",
     "`body action load ...` over TCP, then `body action dual_dump <file>`",
     "PASS iff the name is absent from a dump taken BEFORE the load and present "
     "in a dump taken after it.  The pre-dump is the control: without it the "
     "check cannot go red.  Under --inject-fault the load line carries a "
     "one-letter typo in the body's name, so the step MUST fail."),

    ("S3", "run a shipped show",
     "the farm script log's `ScriptMgr: load <path>` and `ScriptMgr: script "
     "end`; the count of `script action pause` the show stops at; the number of "
     "`#!` annotation tails in the played file after the run; the wall time",
     "`script action play filename <show>` over TCP + the farm's "
     "log/script-*.log + the farm's OWN COPY of the .sts",
     "PASS iff the script log records the load AND reaches the end within the "
     "show's own wait-duration total + 60 s, with each authored pause resumed by "
     "`script action resume`.  `#!` count RECORDED: 0 on a show that parses; any "
     "other value is a DIVERGENCE, not a FAIL, and the file it is in is the "
     "farm's copy so the owner's is untouched by construction."),

    ("S4", "search  [DEPRECATED - R22]",
     "the reply to `search name <a shipped body>` on the $LOGON socket",
     "TCP",
     "NO GATE.  The result is RECORDED and the step is reported DEPRECATED "
     "whatever it says: the owner retired the search family (R22), so a red here "
     "would be a claim about a surface nobody intends to keep.  The step stays IN "
     "because a rehearsal that skips what the tester still has on his keyboard is "
     "not a rehearsal."),

    ("S5", "select + read out",
     "the reply to `get status object` and to `get status position`; the NEW "
     "path's info block for the selected body in the dual dump's `.navstr` "
     "sidecar - its SHAPE, and, recorded separately, the LANGUAGE of its labels",
     "TCP + the `.navstr` sidecar written beside every dual dump "
     "[ssystem_factory.cpp:1179-1180]",
     "PASS iff `select planet <body>` is followed by a `.navstr` block for that "
     "body whose NEW info readout satisfies b24_select.info_block_shape() - five "
     "lines, a numeric magnitude, two sexagesimal pairs, a number plus a unit "
     "token.  SPELLING IS NEVER COMPARED.  The labels are RECORDED verbatim and "
     "matched against the INSTALLED catalogue `language/fr.txt` to name the "
     "language mechanically (a cited fetch, never recall).  The two `get` replies "
     "are recorded; a missing reply is a DIVERGENCE citing Sec.5.47/Sec.11.135."),

    ("S6", "save",
     "`sessions/<name>.ini` and `modularSystem/<name>.ini` under the FARM",
     "`session action save filename ...` + `body action save filename ...` over "
     "TCP; the farm's own directories",
     "PASS iff both files exist under the FARM, are non-empty, and the composed "
     "system file names the authored body.  Also asserted: NOTHING was written "
     "under the real ~/.spacecrafter (the runner's md5 in == out covers "
     "config.ini/ssystem.ini/the show/startup.sts; this step additionally "
     "compares the real sessions/ and modularSystem/ listings before and after)."),

    ("S7", "reload",
     "the dual dump after `session action load`, and after `body action reload` - "
     "the NAMES gained and lost, not only the count; the session file written FROM "
     "the restored state, byte for byte; and, on either side of the reload, whether "
     "the SHIPPED anchor surface still resolves the anchor.ini names",
     "TCP + the farm's sessions/ + `body action dual_dump` + `camera action "
     "follow_rotation name <anchor>` (the NAME registry; its refusal is the "
     "anchor layer's own, CameraAnchors.cpp:442-449) + `camera action switch "
     "name <anchor>` (the anchor USE, whose `ensureBody` is Sec.11.111(g)'s "
     "rebuild-from-the-declaration, CameraAnchors.cpp:392-428)",
     "PASS iff (i) the session written from the restored state is BYTE-IDENTICAL "
     "to the one that was loaded - Sec.11.129's T4 fixed point, which needs the "
     "clock frozen (`timerate rate 0`) because the file carries `jday`; and (ii) "
     "`body action reload` returns without an error line in the app log.  "
     "PREDICTION, committed here before the run: the authored body LEAVES THE "
     "DRAWN (new) path, because reload re-reads THIS system's own data file "
     "[ssystem_factory.cpp:952 -> ModularSystem::reloadSystem] and `ssystem.ini` "
     "never held it - while it may well SURVIVE ON THE OLD PATH, which the "
     "command does not touch at all (Sec.11.55(j)), so the per-path halves are "
     "read separately and a body-count alone is refused as evidence.  An "
     "old-only survivor is a DIVERGENCE citing Sec.11.55(i)(j), and the `search` "
     "and `get status object` answers taken right after say what the control "
     "surface still claims about a body the drawn universe no longer has.  Any "
     "body the reload LOSES is a DIVERGENCE citing Sec.11.117(k)(3); the two "
     "anchor probes say whether an operator would notice - the NAME probe on "
     "either side, then the USE probe, which is where Sec.11.111(g)'s rebuild "
     "must fire if the design holds.  A lost anchor that does NOT come back on "
     "use is a Sec.5.79 mint, and this suite says so rather than deciding it."),

    ("S8", "one keyboard ramp",
     "the OLD projector fov and the NEW drawn half-fov in the dual dump, before "
     "and after a held zoom-in and a held zoom-out",
     "F25's key channel - `xkey.c` (XTEST) on the app's own 1024x1024 window; "
     "the drag channel is DEAD on this host (README, F25 section) so keys only",
     "PASS iff the fov MOVES under a held `Prior` (zoom in) and moves back under "
     "a held `Next` (zoom out), each by more than 1e-3 deg.  This is the "
     "arrival test for the interactive channel, not a parity claim."),

    ("S9", "quit",
     "the process's exit code and the wall time from `shutdown action now` to "
     "exit; the `Frame stall detected` count in the farm's vulkan log; any "
     "teardown fault",
     "`shutdown action now` [app_command_interface.cpp:2397-2405, the verb is "
     "`shutdown`, base_command_interface.hpp:387, with `action now`] + the "
     "process's own exit status + the farm's log/",
     "PASS iff the process exits 0 within 180 s of the command.  A non-exit is "
     "the Sec.5.59/A40 class (an untimed frame wait upstream of teardown) and is "
     "recorded as a DIVERGENCE against that row, WITH the kill that had to "
     "follow it - never masked.  A negative exit code (a signal) is a FAIL and "
     "gets its own row check."),
]

# ------------------------------------------------------------------ plumbing

STEPS = []          # [{id, name, state, why, rows[], detail{}}]
_T0 = time.time()


def log(msg):
    print("[%7.1fs] %s" % (time.time() - _T0, msg), flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def no_instance():
    """The Sec.11.134(b) probe: /proc/<pid>/comm, every account, no self-match."""
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def screensaver_active():
    try:
        out = subprocess.run(
            ["gdbus", "call", "--session", "--dest", "org.gnome.ScreenSaver",
             "--object-path", "/org/gnome/ScreenSaver",
             "--method", "org.gnome.ScreenSaver.GetActive"],
            capture_output=True, text=True, timeout=20,
            env={**os.environ,
                 "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/%d/bus" % os.getuid()})
        return out.stdout.strip()
    except Exception as e:                                        # noqa: BLE001
        return "probe-error %s" % e


def listing(d):
    d = Path(d)
    return sorted((p.name, p.stat().st_size) for p in d.iterdir()) if d.is_dir() else []


# ------------------------------------------------------------------- the farm

def build_farm(farm, show_rel):
    """f55_farm.sh + the two extra real directories this suite needs.

    Returns a dict of everything asserted, so the record says what the farm IS
    rather than what it was meant to be."""
    farm = Path(farm)
    if farm.exists():
        shutil.rmtree(farm)
    r = subprocess.run([str(HERE / "f55_farm.sh"), str(farm)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("f55_farm.sh failed: %s%s" % (r.stdout, r.stderr))
    dst = farm / ".spacecrafter"

    # (1) sessions/ real - `session action save` writes into it
    src_sessions = REAL_HOME / "sessions"
    (dst / "sessions").unlink(missing_ok=True)
    (dst / "sessions").mkdir(parents=True, exist_ok=True)
    if src_sessions.is_dir():
        for e in src_sessions.iterdir():
            (dst / "sessions" / e.name).symlink_to(e)

    # (2) scripts/<dir>/ of the show real, the show itself a COPY - the
    #     annotator rewrites the file it played.
    show_dir, show_name = show_rel.split("/", 1)
    src_dir = REAL_HOME / "scripts" / show_dir
    (dst / "scripts" / show_dir).unlink(missing_ok=True)
    (dst / "scripts" / show_dir).mkdir(parents=True, exist_ok=True)
    for e in src_dir.iterdir():
        if e.name == show_name:
            shutil.copy2(e, dst / "scripts" / show_dir / e.name)
        else:
            (dst / "scripts" / show_dir / e.name).symlink_to(e)

    # (3) startup.sts a COPY - same annotator hazard, and the app plays it at
    #     launch, so a bad line in it is written back before anything else runs.
    real_startup = REAL_HOME / "scripts" / "fscripts" / "startup.sts"
    if real_startup.is_file():
        shutil.copy2(real_startup, dst / "scripts" / "fscripts" / "startup.sts")

    cfg = dst / "config.ini"
    asserts = {
        "config.ini.is_regular_file": cfg.is_file() and not cfg.is_symlink(),
        "config.ini.md5": md5(cfg),
        "ssystem.ini.is_regular_file": (dst / "ssystem.ini").is_file()
                                       and not (dst / "ssystem.ini").is_symlink(),
        "sessions.is_real_dir": (dst / "sessions").is_dir()
                                and not (dst / "sessions").is_symlink(),
        "scripts.is_real_dir": (dst / "scripts").is_dir()
                               and not (dst / "scripts").is_symlink(),
        "show_dir.is_real_dir": (dst / "scripts" / show_dir).is_dir()
                                and not (dst / "scripts" / show_dir).is_symlink(),
        "show.is_regular_file": (dst / "scripts" / show_rel).is_file()
                                and not (dst / "scripts" / show_rel).is_symlink(),
        "startup.is_regular_file": (dst / "scripts" / "fscripts" / "startup.sts").is_file()
                                   and not (dst / "scripts" / "fscripts" / "startup.sts").is_symlink(),
        "modularSystem.is_real_dir": (dst / "modularSystem").is_dir()
                                     and not (dst / "modularSystem").is_symlink(),
        "log.is_real_dir": (dst / "log").is_dir() and not (dst / "log").is_symlink(),
    }
    bad = [k for k, v in asserts.items() if v is False]
    if bad:
        raise RuntimeError("farm shape assert failed: %s" % bad)
    return asserts


# ------------------------------------------------------------------ the app

class App:
    """One launch, one $LOGON socket, everything recorded."""

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
        self.dump_n = 0

    # ---- lifecycle
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

    # ---- wire
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
        """Send a `get`/`search` and wait for a line the caller recognises.

        `ServerSocket::send` writes strlen+1 bytes, i.e. the NUL terminator goes
        on the wire (io.cpp:832-834), so the blob is NUL-separated and the NULs
        are stripped here rather than in every caller."""
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

    # ---- dump
    def dump(self, name, wait=25.0):
        self.dump_n += 1
        f = self.out / ("%s_%02d_%s.json" % (self.tag, self.dump_n, name))
        for p in (f, Path(str(f) + ".navstr")):
            if p.exists():
                p.unlink()
        self.send("body action dual_dump filename %s" % f, 1.5)
        end = time.time() + wait
        while time.time() < end:
            if f.exists() and f.stat().st_size > 0 \
               and Path(str(f) + ".navstr").exists():
                time.sleep(0.6)          # the writer is a stream; let it close
                return f
            time.sleep(0.3)
        return f if f.exists() else None

    def bodies(self, dumpfile):
        """Every `type=body` record by name, including ones with no `new` side.

        dumpread.loads is the ONE grammar (sanitize_nonfinite + unquote);
        dumpread.load_dump drops records with no `new` half, which is right for a
        comparison and wrong for an inventory (F84's own note)."""
        out = {}
        if dumpfile is None or not Path(dumpfile).exists():
            return out
        for line in open(dumpfile, encoding="utf-8", errors="replace"):
            if not line.strip():
                continue
            try:
                rec = dumpread.loads(line)
            except Exception:                                     # noqa: BLE001
                continue
            if rec.get("type") == "body" and "name" in rec:
                out[rec["name"]] = rec
        return out

    def header(self, dumpfile):
        if dumpfile is None or not Path(dumpfile).exists():
            return {}
        for line in open(dumpfile, encoding="utf-8", errors="replace"):
            if line.strip():
                try:
                    return dumpread.loads(line)
                except Exception:                                 # noqa: BLE001
                    return {}
        return {}

    # ---- logs
    def applog_text(self):
        return self.applog.read_bytes().decode("latin-1") if self.applog.exists() else ""

    def _logfile(self, prefix):
        g = sorted(glob.glob(str(self.home / "log" / (prefix + "*.log"))))
        return Path(g[-1]) if g else None

    def app_log_text(self):
        p = self._logfile("spacecrafter")
        return p.read_bytes().decode("latin-1") if p else ""

    def script_log_text(self):
        p = self._logfile("script")
        return p.read_bytes().decode("latin-1") if p else ""

    def vulkan_log_text(self):
        p = self._logfile("vulkan")
        return p.read_bytes().decode("latin-1") if p else ""


# --------------------------------------------------------------- the recorder

def record(sid, state, why, rows=None, **detail):
    name = next(p[1] for p in PLAN if p[0] == sid)
    STEPS.append({"id": sid, "step": name, "state": state, "why": why,
                  "rows": rows or [], "detail": detail})
    log("%s %-22s %-10s %s" % (sid, name, state, why))


def print_plan():
    print("=" * 78)
    print("F90 REHEARSAL - the nine steps, their observables and their pass criteria")
    print("STATES: PASS = the function was delivered | DIVERGENCE = delivered, with a")
    print("        ledger row observed on the way (cited, NOT a failure) | FAIL = not")
    print("        delivered.  Any FAIL exits non-zero.  A divergence with no row is a FAIL.")
    print("=" * 78)
    for sid, name, obs, chan, crit in PLAN:
        print("\n%s  %s" % (sid, name.upper()))
        print("    OBSERVABLE  %s" % obs)
        print("    CHANNEL     %s" % chan)
        print("    CRITERION   %s" % crit)
    print("\n" + "=" * 78, flush=True)


# ------------------------------------------------------------------ the steps

SEXA_PAIR = re.compile(r"[+-]?\d+\D+\d+\D+\d+")


def catalogue(lang):
    """The INSTALLED catalogue, read - never recalled.  -> {msgid: msgstr}."""
    p = REAL_HOME / "language" / ("%s.txt" % lang)
    out = {}
    if not p.is_file():
        return out
    for line in p.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r'^"(.*)";"(.*)"\s*$', line)
        if m:
            out[m.group(1)] = m.group(2)
    return out


def show_own_duration(path):
    """The show's own wait total, parsed from the file - mechanical, not recalled."""
    total, pauses, lines = 0.0, 0, 0
    for line in Path(path).read_text(encoding="latin-1").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        lines += 1
        m = re.match(r"^wait\s+duration\s+([0-9.]+)", s)
        if m:
            total += float(m.group(1))
        if re.match(r"^script\s+action\s+pause\b", s):
            pauses += 1
    return round(total, 2), pauses, lines


ANSI = re.compile(r"\x1b\[[0-9;]*m")
CLOG_TAG = re.compile(r"\((?:Error|Warn\.|Info |Debug)\): ")
FAILWORD = re.compile(r"(?i)\b(invalid|failed|failure|can'?t open|cannot|unable|"
                      r"error|no such|abort)\b")


def startup_silences(stderr_txt, applog_txt):
    """Sec.5.77's class, DETECTED BY CHANNEL rather than by wording.

    A line the app wrote through `cLog` carries a tag on the console -
    `(Error): ` `(Warn.): ` `(Info ): ` `(Debug): ` [observed: log.cpp:32-35 and
    writeConsole, :165-208] - and is in `spacecrafter.log` by construction
    (`print_log = true` in the field config puts cLog on the console too, which
    is exactly what makes a naive stderr-vs-log diff report false positives).
    A line with NO complete tag came from a RAW stream write - std::cerr,
    printf, fprintf - and that is the channel Sec.5.77/Sec.11.146 is about.

    -> (silences, clog_console_echo_count)."""
    silences, echo = [], 0
    flat = ANSI.sub("", applog_txt)
    for raw in stderr_txt.splitlines():
        s = ANSI.sub("", raw).strip()
        if not s:
            continue
        if CLOG_TAG.search(s):
            echo += 1
            continue
        if not FAILWORD.search(s):
            continue
        if s not in flat:
            silences.append(s)
    return silences, echo


LOCK_FILE = Path("/tmp/spacecrafter.lock")     # spacecrafter.hpp:48 + main.cpp:237


def step_launch(app, tag):
    t0 = time.time()
    # RECORDED, NEVER REMOVED.  `main.cpp:236-245` keeps a single-instance lock
    # in the system temp dir and validates it by shelling out `kill -0 <pid>`;
    # when a previous launch was killed rather than quit, `sh` prints "kill: No
    # such process" ON THE PROCESS'S OWN STDERR and nothing reaches the app log
    # - a Sec.5.77-class line whose presence depends on RUN HISTORY, not on the
    # build.  Removing it here would erase the one axis that explains a varying
    # silence count between two runs of this suite.
    stale_lock = LOCK_FILE.is_file()
    stale_lock_pid = LOCK_FILE.read_text().strip()[:20] if stale_lock else None
    wait = app.start()
    if wait is None:
        record("S1", "FAIL", "the app never accepted a connection on port %d "
               "(exit %s)" % (PORT, app.proc.poll() if app.proc else "?"),
               tcp_wait_s=None)
        return False
    stderr_txt = app.applog_text()
    applog_txt = app.app_log_text()
    silences, clog_echo = startup_silences(stderr_txt, applog_txt)
    errs = [l.strip() for l in applog_txt.splitlines()
            if "(Error)" in l or "(Warn.)" in l]
    detail = dict(tcp_wait_s=wait, launch_wall_s=round(time.time() - t0, 1),
                  pid=app.proc.pid, screensaver_GetActive=screensaver_active(),
                  stale_lock_file=stale_lock, stale_lock_pid=stale_lock_pid,
                  applog_lines=len(applog_txt.splitlines()),
                  app_log_error_lines=len(errs),
                  app_log_error_sample=errs[:12],
                  clog_console_echo_lines=clog_echo,
                  stderr_silence_count=len(silences),
                  stderr_silences=silences[:20],
                  proc_comm_hits_after=len(no_instance()))
    if silences:
        record("S1", "DIVERGENCE",
               "the app is up (TCP after %.1f s) and %d raw-stream failure line(s) "
               "never reach spacecrafter.log%s: %s"
               % (wait, len(silences),
                  " [a STALE /tmp/spacecrafter.lock was present, so one of them is "
                  "this suite's own run history]" if stale_lock else "",
                  "; ".join(s[:60] for s in silences)),
               rows=["Sec.5.77", "Sec.11.146"], **detail)
    else:
        record("S1", "PASS", "TCP accepted after %.1f s; no unlogged startup failure "
               "on stderr" % wait, **detail)
    return True


def step_author(app, name_sent, name_expected):
    pre = app.dump("pre_author")
    pre_names = set(app.bodies(pre))
    control_ok = name_expected not in pre_names
    reply = app.send(probe_cmd(name_sent), 2.5)
    post = app.dump("post_author")
    post_names = set(app.bodies(post))
    present = name_expected in post_names
    detail = dict(name_sent=name_sent, name_expected=name_expected,
                  bodies_before=len(pre_names), bodies_after=len(post_names),
                  control_absent_before=control_ok, present_after=present,
                  wire_echo=reply[-300:] if reply else "")
    if not control_ok:
        record("S2", "FAIL", "the control failed: '%s' was ALREADY in the dump before "
               "the load, so this step could not have gone red" % name_expected, **detail)
        return False
    if present:
        record("S2", "PASS", "'%s' absent from %d bodies before the load, present among "
               "%d after" % (name_expected, len(pre_names), len(post_names)), **detail)
        return True
    record("S2", "FAIL", "'%s' is not in the dump after `body action load` (sent as "
           "'%s')" % (name_expected, name_sent), **detail)
    return False


def step_show(app, show_rel, farm):
    show_path = farm / ".spacecrafter" / "scripts" / show_rel
    own, pauses_declared, cmd_lines = show_own_duration(show_path)
    before = app.script_log_text()
    t0 = time.time()
    app.send("script action play filename %s" % show_rel, 1.5)
    budget = own + 60.0
    resumed, seen_end, seen_load = 0, False, False
    last_pause_seen = before.count("script action pause")
    while time.time() - t0 < budget:
        txt = app.script_log_text()
        new = txt[len(before):]
        if "ScriptMgr: load" in new:
            seen_load = True
        if "ScriptMgr: script end" in new or "End of script" in new:
            seen_end = True
            break
        n = txt.count("ScriptMgr::script action pause")
        if n > last_pause_seen:
            last_pause_seen = n
            app.send("script action resume", 0.8)
            resumed += 1
        time.sleep(0.5)
    wall = round(time.time() - t0, 1)
    new = app.script_log_text()[len(before):]
    body = show_path.read_text(encoding="latin-1")
    hashbang = sum(1 for ln in body.splitlines()
                   if b24_hasann(ln))
    detail = dict(show=show_rel, own_wait_total_s=own,
                  pauses_declared=pauses_declared, pauses_resumed=resumed,
                  command_lines=cmd_lines, wall_s=wall,
                  script_log_load=seen_load, script_log_end=seen_end,
                  annotation_tails=hashbang,
                  show_md5_after=md5(show_path),
                  script_log_new_lines=len(new.splitlines()))
    if not seen_load:
        record("S3", "FAIL", "the script log never recorded `ScriptMgr: load` for %s"
               % show_rel, **detail)
        return False
    if not seen_end:
        record("S3", "FAIL", "the show did not reach its end within %.1f s "
               "(own wait total %.1f s, %d pause(s) resumed)" % (budget, own, resumed),
               **detail)
        return False
    if hashbang:
        record("S3", "DIVERGENCE",
               "the show ran (%.1f s wall against %.1f s of authored waits, %d pause(s) "
               "resumed) and the engine wrote %d `#!` annotation tail(s) into the played "
               "file" % (wall, own, resumed, hashbang),
               rows=["Sec.11.184"], **detail)
        return True
    record("S3", "PASS", "loaded and ended: %.1f s wall against %.1f s of authored "
           "waits + %d operator pause(s) this suite resumed" % (wall, own, resumed),
           **detail)
    return True


def b24_hasann(line):
    """`#!` tail detection, the engine's own rule (a `#` inside a quoted run is
    text) - ScriptAnnotator::annotationBegin, script_annotator.cpp:41-54."""
    in_quote = False
    for i, c in enumerate(line):
        if c == '"':
            in_quote = not in_quote
        elif c == "#" and not in_quote:
            return line.find("#!", i) != -1
    return False


def step_search(app, needle):
    hit, blob, lat = app.ask("search name %s" % needle,
                             lambda b: b.strip() or None, timeout=8.0)
    payload = None
    for ln in (blob or "").replace("\r", "\n").split("\n"):
        s = ln.strip()
        if s and not s.startswith("$"):
            payload = s
            break
    record("S4", "DEPRECATED",
           "R22 retires the search family; the answer is a datum, never a gate. "
           "`search name %s` -> %r" % (needle, (payload or "")[:120]),
           needle=needle, reply=payload, latency_s=lat, raw=(blob or "")[:400])
    return True


def step_select(app, body):
    app.send("select planet %s" % body, 1.5)
    obj, obj_raw, obj_lat = app.ask(
        "get status object",
        lambda b: (b.strip() or None) if len(b.strip()) > 3 else None, timeout=10.0)
    pos, pos_raw, pos_lat = app.ask(
        "get status position",
        lambda b: (b.strip() or None) if len(b.strip()) > 3 else None, timeout=10.0)
    d = app.dump("selected")
    names = set(app.bodies(d))
    blocks = b24_select.navstr_blocks(str(d) + ".navstr", names) if d else {}
    shape = b24_select.info_block_shape(blocks.get(body, ""))
    fr = catalogue("fr")
    en = catalogue("en")
    labels = shape.get("labels", [])
    lang = []
    for lab in labels:
        got = None
        for msgid, msgstr in fr.items():
            if msgstr == lab or msgstr.rstrip() == lab.rstrip():
                got = ("fr", msgid)
                break
        if got is None and (lab in en.values() or lab.rstrip() in [v.rstrip() for v in en.values()]):
            got = ("en-catalogue", lab)
        if got is None:
            got = ("untranslated-or-absent", lab)
        lang.append({"label": lab, "label_escaped": lab.encode("unicode_escape").decode(),
                     "verdict": got[0], "catalogue_msgid": got[1]})
    nbsp = any("\xa0" in l for l in labels)
    detail = dict(body=body, get_status_object=(obj_raw or "")[:600],
                  get_status_object_latency_s=obj_lat,
                  get_status_position=(pos_raw or "")[:300],
                  get_status_position_latency_s=pos_lat,
                  navstr_blocks=len(blocks), shape=
                  {k: v for k, v in shape.items() if k != "raw"},
                  shape_raw=shape.get("raw"), labels=lang, has_U00A0=nbsp)
    if "error" in shape:
        record("S5", "FAIL", "the NEW info readout for %s does not have the writer's "
               "shape: %s" % (body, shape["error"]), **detail)
        return False
    verdicts = {l["verdict"] for l in lang}
    langword = ("french (every label is a fr.txt msgstr)" if verdicts == {"fr"}
                else "mixed: %s" % sorted(verdicts))
    rows, state = [], "PASS"
    if not obj_raw or not obj_raw.strip():
        rows.append("Sec.5.47")
        state = "DIVERGENCE"
    if verdicts != {"fr"}:
        rows.append("Sec.5.111")
        state = "DIVERGENCE"
    record("S5", state,
           "shape OK (5 lines, magnitude %r, %s / %s, distance %r %s); labels are %s"
           % (shape.get("magnitude"), shape.get("radec"), shape.get("altaz"),
              shape.get("distance"), shape.get("unit"), langword),
           rows=rows, **detail)
    return True


def step_save(app, farm, session_name, system_name, probe):
    real_sessions_before = listing(REAL_HOME / "sessions")
    real_modular_before = listing(REAL_HOME / "modularSystem")
    app.send("timerate rate 0", 1.2)               # T4 needs the clock frozen
    app.send("session action save filename %s" % session_name, 2.5)
    app.send("body action save filename %s" % system_name, 3.0)
    sess = farm / ".spacecrafter" / "sessions" / ("%s.ini" % session_name)
    sysf = farm / ".spacecrafter" / "modularSystem" / ("%s.ini" % system_name)
    sys_txt = sysf.read_text(encoding="latin-1") if sysf.is_file() else ""
    detail = dict(
        session_file=str(sess), session_exists=sess.is_file(),
        session_bytes=sess.stat().st_size if sess.is_file() else 0,
        system_file=str(sysf), system_exists=sysf.is_file(),
        system_bytes=sysf.stat().st_size if sysf.is_file() else 0,
        system_names_probe=(probe in sys_txt),
        real_sessions_unchanged=(real_sessions_before == listing(REAL_HOME / "sessions")),
        real_modular_unchanged=(real_modular_before == listing(REAL_HOME / "modularSystem")))
    bad = []
    if not detail["session_exists"] or detail["session_bytes"] == 0:
        bad.append("the session file was not written under the farm")
    if not detail["system_exists"] or detail["system_bytes"] == 0:
        bad.append("the composed system file was not written under the farm")
    if detail["system_exists"] and not detail["system_names_probe"]:
        bad.append("the composed system file does not name the authored body '%s'" % probe)
    if not detail["real_sessions_unchanged"]:
        bad.append("the REAL ~/.spacecrafter/sessions changed")
    if not detail["real_modular_unchanged"]:
        bad.append("the REAL ~/.spacecrafter/modularSystem changed")
    if bad:
        record("S6", "FAIL", "; ".join(bad), **detail)
        return False
    record("S6", "PASS", "sessions/%s.ini (%d B) and modularSystem/%s.ini (%d B) written "
           "under the FARM, the system file names '%s', and the real home's two "
           "directories are untouched"
           % (session_name, detail["session_bytes"], system_name,
              detail["system_bytes"], probe), **detail)
    return True


ANCHOR_PROBES = ["baryEarthMoon", "orbit_autour_lune"]
NOT_DECLARED = "is not a declared anchor"


def anchor_probe(app, names):
    """Ask the SHIPPED anchor surface whether each name still RESOLVES.

    `camera action follow_rotation name <n> value true` ->
    CameraAnchors::setFollowRotation, which logs "'<n>' is not a declared anchor"
    and returns false when the name is gone [observed: CameraAnchors.cpp:442-449,
    wired at app_command_interface.cpp:4605-4613].  A body-count delta in a dump
    cannot say whether an operator would notice; this can.

    NOTE ON WHAT THIS DOES *NOT* SHOW, because the distinction is the whole
    point: `find(name)` asks the NAME REGISTRY only.  Whether the anchor's owned
    BODY was rebuilt is a different question, and `anchor_use_probe` below is the
    one that asks it."""
    out = {}
    for n in names:
        mark = len(app.app_log_text())
        app.send("camera action follow_rotation name %s value true" % n, 1.2)
        new = app.app_log_text()[mark:]
        out[n] = "REFUSED" if (NOT_DECLARED in new and n in new) else "resolves"
    return out


def anchor_use_probe(app, names, restore="Earth"):
    """USE each anchor, which is where Sec.11.111(g)'s rebuild is supposed to fire.

    `camera action switch name <n>` -> CameraAnchors::switchTo, which calls
    `ensureBody(*anchor)` - the rebuild-from-the-declaration path - before it
    warps [observed: CameraAnchors.cpp:392-428].  So this is exactly the "next
    anchor use" Sec.11.117(k)(3) says the anchor half is designed around, and
    the dump taken right after says whether the owned body came back.
    Restores the camera with a switch to an ordinary body name (an implicit body
    anchor, CameraAnchors.cpp:404-411) so the rest of the run starts level."""
    out = {}
    for n in names:
        mark = len(app.app_log_text())
        app.send("camera action switch name %s" % n, 2.5)
        d = app.dump("after_switch_%s" % n)
        present = n in app.bodies(d)
        errs = [l.strip() for l in app.app_log_text()[mark:].splitlines()
                if "(Error)" in l]
        out[n] = {"body_back_in_dump": present, "errors": errs[:4]}
    app.send("camera action switch name %s" % restore, 2.5)
    return out


def step_reload(app, farm, session_name, probe):
    sessdir = farm / ".spacecrafter" / "sessions"
    first = (sessdir / ("%s.ini" % session_name)).read_bytes()
    app.send("session action load filename %s" % session_name, 6.0)
    d_rest = app.dump("restored")
    app.send("session action save filename %s2" % session_name, 2.5)
    second_p = sessdir / ("%s2.ini" % session_name)
    second = second_p.read_bytes() if second_p.is_file() else b""
    t4 = (second == first)

    bodies_before = set(app.bodies(d_rest))
    anchors_before = anchor_probe(app, ANCHOR_PROBES)
    log_before = len(app.app_log_text())
    app.send("body action reload", 6.0)
    d_after = app.dump("post_reload")
    bodies_after = set(app.bodies(d_after))
    recs_after = app.bodies(d_after)
    new_log = app.app_log_text()[log_before:]
    # `(Error): ` is the log file's own tag (log.cpp:32) - `(Err.)` matches
    # NOTHING and would make this criterion structurally unable to fire.
    reload_errs = [l.strip() for l in new_log.splitlines() if "(Error)" in l]
    anchors_after = anchor_probe(app, ANCHOR_PROBES)
    probe_survived = probe in bodies_after
    lost = sorted(bodies_before - bodies_after)
    gained = sorted(bodies_after - bodies_before)
    rebuilt = anchor_use_probe(app, [n for n in ANCHOR_PROBES if n in lost]) \
        if lost else {}

    # WHICH PATH still holds the authored body, and what the control surface
    # then answers about it.  `body action load` reaches BOTH paths
    # (Core::addSolarSystemBody -> SSystemFactory::addBody), while
    # `body action reload` is NEW-PATH-ONLY by construction
    # (ModularSystem::reloadSystem has no old-path counterpart, Sec.11.55(j)) -
    # so the pair can leave a body on the old path and off the DRAWN one.  A
    # body count cannot see that; the per-path halves can.
    prec = recs_after.get(probe)
    paths = {"in_dump": prec is not None,
             "old_half": (prec is not None and prec.get("old") is not None),
             "new_half": (prec is not None and prec.get("new") is not None)}
    _, srch, _ = app.ask("search name %s" % probe,
                         lambda b: (b.strip() or None) if b.strip() else None, 6.0)
    app.send("select planet %s" % probe, 1.5)
    _, sel, _ = app.ask("get status object",
                        lambda b: (b.strip() or None) if len(b.strip()) > 3 else None, 8.0)
    paths["search_answers"] = (srch or "").strip()[:120]
    paths["get_status_object_answers"] = (sel or "").strip()[:200]

    detail = dict(t4_byte_identical=t4, first_bytes=len(first),
                  second_bytes=len(second),
                  bodies_after_session_load=len(bodies_before),
                  bodies_after_body_reload=len(bodies_after),
                  bodies_lost=lost, bodies_gained=gained,
                  probe_present_before_reload=(probe in bodies_before),
                  probe_survived_body_reload=probe_survived,
                  probe_after_reload_by_path=paths,
                  prediction_probe_survives=False,
                  prediction_held=(paths["new_half"] is False),
                  anchor_surface_before=anchors_before,
                  anchor_surface_after=anchors_after,
                  anchor_rebuilt_on_use=rebuilt,
                  reload_error_lines=reload_errs[:10])
    bad = []
    if not t4:
        bad.append("Sec.11.129's T4 fixed point FAILED: the session written from the "
                   "restored state is %d B against the loaded file's %d B"
                   % (len(second), len(first)))
    if reload_errs:
        bad.append("`body action reload` logged %d error line(s): %s"
                   % (len(reload_errs), reload_errs[0][:160]))
    if bad:
        record("S7", "FAIL", "; ".join(bad), **detail)
        return False
    tail = ("T4 byte-identical (%d B); `body action reload` logged no error; %d -> %d "
            "dump records; the authored body after the reload: old-half %s / NEW(drawn)-"
            "half %s, and the plan's prediction (it leaves the drawn path) %s"
            % (len(first), len(bodies_before), len(bodies_after),
               paths["old_half"], paths["new_half"],
               "HELD" if not paths["new_half"] else "FAILED"))
    rows = []
    if lost:
        rows += ["Sec.11.117(k)(3)", "Sec.11.111(g)"]
    if paths["old_half"] and not paths["new_half"]:
        rows += ["Sec.11.55(i)", "Sec.11.55(j)"]
    if rows:
        back = {n: v["body_back_in_dump"] for n, v in rebuilt.items()}
        record("S7", "DIVERGENCE",
               "%s; the reload SILENTLY dropped %s; the anchor NAME surface went %s -> "
               "%s and the owned body came back on USE: %s; with the authored body off "
               "the drawn path, `search` answers %r and `get status object` answers %r"
               % (tail, lost or "no body", anchors_before, anchors_after, back or "n/a",
                  paths["search_answers"], paths["get_status_object_answers"]),
               rows=rows, **detail)
        return True
    record("S7", "PASS", tail, **detail)
    return True


def step_keys(app, xkey, win):
    def fovs():
        d = app.dump("fov")
        h = app.header(d)
        return (h.get("oldView", {}).get("projector", {}).get("fov"),
                h.get("camera", {}).get("halfFov"))

    def hold(key, ms):
        r = subprocess.run([str(xkey), "spacecrafter", key, str(ms), win],
                           capture_output=True, text=True)
        return {"key": key, "ms": ms, "rc": r.returncode,
                "out": (r.stdout + r.stderr).strip()}

    f0, h0 = fovs()
    ev_in = hold("Prior", 1500)
    time.sleep(1.2)
    f1, h1 = fovs()
    ev_out = hold("Next", 1500)
    time.sleep(1.2)
    f2, h2 = fovs()
    detail = dict(fov_start=f0, fov_after_zoom_in=f1, fov_after_zoom_out=f2,
                  halfFov_start=h0, halfFov_after_zoom_in=h1,
                  halfFov_after_zoom_out=h2, xkey_in=ev_in, xkey_out=ev_out)
    if ev_in["rc"] != 0 or ev_out["rc"] != 0:
        record("S8", "FAIL", "xkey failed - the keystroke never reached the app "
               "(%s / %s)" % (ev_in["out"], ev_out["out"]), **detail)
        return False
    if None in (f0, f1, f2):
        record("S8", "FAIL", "the dump did not carry the old projector's fov "
               "(%s -> %s -> %s)" % (f0, f1, f2), **detail)
        return False
    din, dout = f1 - f0, f2 - f1
    if abs(din) <= 1e-3 or abs(dout) <= 1e-3:
        record("S8", "FAIL", "a held zoom key did not move the fov: %.6f -> %.6f -> "
               "%.6f deg (d_in %+.6f, d_out %+.6f)" % (f0, f1, f2, din, dout), **detail)
        return False
    record("S8", "PASS", "held Prior then Next: fov %.5f -> %.5f -> %.5f deg "
           "(%+.5f then %+.5f); drawn halfFov %.7f -> %.7f -> %.7f rad"
           % (f0, f1, f2, din, dout, h0 or -1, h1 or -1, h2 or -1), **detail)
    return True


def step_quit(app):
    stalls_before = app.vulkan_log_text().count("Frame stall detected")
    t0 = time.time()
    try:
        app.send("shutdown action now", 0.5)
    except OSError as e:                                          # noqa: BLE001
        log("shutdown send raised %s (the socket may already be closing)" % e)
    hung = False
    try:
        app.proc.wait(timeout=180)
    except subprocess.TimeoutExpired:
        hung = True
        app.proc.kill()
        app.proc.wait(timeout=30)
    wall = round(time.time() - t0, 1)
    rc = app.proc.returncode
    time.sleep(2)
    vulk = app.vulkan_log_text()
    detail = dict(exit_code=rc, quit_wall_s=wall, hung=hung,
                  frame_stall_detected=vulk.count("Frame stall detected"),
                  frame_stall_very_long=vulk.count("This frame stall is very long"),
                  frame_stall_before_quit=stalls_before,
                  proc_comm_hits_after=len(no_instance()))
    if hung:
        record("S9", "DIVERGENCE",
               "the process did not exit within 180 s of `shutdown action now` and had "
               "to be killed (rc %s) - the teardown-vs-incomplete-frame class" % rc,
               rows=["Sec.5.59", "A40"], **detail)
        return True
    if rc == 0:
        record("S9", "PASS", "`shutdown action now` -> exit 0 after %.1f s; %d frame "
               "stall(s) logged over the whole run"
               % (wall, detail["frame_stall_detected"]), **detail)
        return True
    record("S9", "FAIL", "the process exited %s (%s) after %.1f s"
           % (rc, "signal %d" % -rc if rc and rc < 0 else "non-zero", wall), **detail)
    return False


# ------------------------------------------------------------------- driver

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", default=str(DEFAULT_BIN))
    ap.add_argument("--farm", default=None,
                    help="farm root (default <out>/farm); wiped and rebuilt")
    ap.add_argument("--show", default="basis/zodiacal_light.sts")
    ap.add_argument("--select", default="Mars")
    ap.add_argument("--search", default="Mars")
    ap.add_argument("--tag", default="run")
    ap.add_argument("--inject-fault", action="store_true",
                    help="one-letter typo in the authored body's NAME (step S2)")
    ap.add_argument("--plan", action="store_true", help="print the plan and exit 0")
    a = ap.parse_args()

    if a.plan:
        print_plan()
        return 0

    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    farm = Path(a.farm) if a.farm else out / "farm"

    print_plan()
    log("binary  = %s  (md5 %s)" % (a.bin, md5(a.bin)))
    log("display = %s" % os.environ.get("DISPLAY"))
    log("show    = %s" % a.show)
    log("mode    = %s" % ("INJECTED FAULT (S2 must fail)" if a.inject_fault
                          else "clean"))

    farm_asserts = build_farm(farm, a.show)
    log("farm    = %s  (config.ini regular file: %s, md5 %s)"
        % (farm, farm_asserts["config.ini.is_regular_file"],
           farm_asserts["config.ini.md5"]))

    xkey = out / "xkey"
    subprocess.run(["gcc", "-O1", "-o", str(xkey), str(HERE / "xkey.c"), "-lX11",
                    "/usr/lib/x86_64-linux-gnu/libXtst.so.6"], check=True)

    name_sent = PROBE_TYPO if a.inject_fault else PROBE_NAME
    app = App(a.bin, farm, out, a.tag)
    t_run = time.time()
    try:
        if step_launch(app, a.tag):
            step_author(app, name_sent, PROBE_NAME)
            step_show(app, a.show, farm)
            step_search(app, a.search)
            step_select(app, a.select)
            step_save(app, farm, "f90", "f90sys", PROBE_NAME)
            step_reload(app, farm, "f90", PROBE_NAME)
            step_keys(app, xkey, "1024x1024")
            step_quit(app)
        else:
            if app.proc and app.proc.poll() is None:
                app.proc.kill()
    finally:
        if app.proc and app.proc.poll() is None:
            app.proc.kill()

    wall = round(time.time() - t_run, 1)
    print("\n" + "=" * 78)
    print("F90 REHEARSAL - PER-STEP TABLE   tag=%s   %.1f s" % (a.tag, wall))
    print("%-4s %-24s %-11s %-14s %s" % ("ID", "STEP", "STATE", "ROWS", "WHY"))
    print("-" * 78)
    for s in STEPS:
        print("%-4s %-24s %-11s %-14s %s"
              % (s["id"], s["step"], s["state"], ",".join(s["rows"]) or "-", s["why"]))
    missing = [p[0] for p in PLAN if p[0] not in {s["id"] for s in STEPS}]
    for m in missing:
        print("%-4s %-24s %-11s %-14s %s"
              % (m, next(p[1] for p in PLAN if p[0] == m), "NOT RUN", "-",
                 "an earlier step ended the run"))
    # A DIVERGENCE with no citation is a FAIL by this suite's own rule (see the
    # module docstring): an unattributed divergence needs a human, not a green.
    fails = [s["id"] for s in STEPS
             if s["state"] == "FAIL"
             or (s["state"] == "DIVERGENCE" and not s["rows"])] + missing
    print("-" * 78)
    print("FAIL: %s" % (", ".join(fails) if fails else "none"))
    print("DIVERGENCE: %s" % (", ".join("%s(%s)" % (s["id"], ",".join(s["rows"]))
                                        for s in STEPS
                                        if s["state"] == "DIVERGENCE") or "none"))
    print("=" * 78, flush=True)

    (out / ("f90_%s_result.json" % a.tag)).write_text(json.dumps(
        {"tag": a.tag, "binary": a.bin, "binary_md5": md5(a.bin),
         "display": os.environ.get("DISPLAY"), "show": a.show,
         "inject_fault": a.inject_fault, "farm": str(farm),
         "farm_asserts": farm_asserts, "wall_s": wall,
         "steps": STEPS, "fails": fails}, indent=1, default=str))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
