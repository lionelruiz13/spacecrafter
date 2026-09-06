#!/usr/bin/env python3
"""F95/F98 - THE MULTI-HOUR SOAK UNDER SHOW LOAD (DEPLOYMENT-MAP T5.2).

    ./f95_soak.py plan   [--playlist-dir D]... [--cap S]  # criteria+model, no launch
    ./f95_soak.py selftest                              # the verdict functions, both ways
    ./f95_soak.py start  <absOutdir> [--hours H] [--sample S] [--bin B]
                         [--playlist-dir D]... [--root R] [--cap S]
    ./f95_soak.py status <absOutdir>                    # instant, foreground
    ./f95_soak.py wait   <absOutdir> <sec<=540>         # the executor's poll unit
    ./f95_soak.py stop   <absOutdir>                    # ask the driver to end now

F98 GENERALISED THIS DRIVER RATHER THAN COPYING IT (I2).  The second soak runs
the tester's own `fscripts/` corpus - 136 shows that AUTHOR BODIES, which is
the load F95 could not put on the engine (Sec.11.215(m)) - and the only things
that differ are parameters: WHICH script directories are the playlist
(`--playlist-dir`, default F95's three), WHERE the farm lives (`--root`,
default F95's) and whether a show may be cut short (`--cap`, default none =
F95's behaviour exactly).  `startup.sts` is never a playlist member on either
run.  The FAIL criteria and the LEAK rule are the same string in the same
place, so `criteria_sha` in `config.json` is identical for an F95 run and an
F98 run - which is the mechanical form of "the criteria did not change".

WHAT THIS IS.  ONE launch of the application on a private farm, in the field's
own French locale, with the eight shipped shows of `basis/` + `custom/` +
`deepsky/` played round-robin through the shipped command surface for H hours,
every authored `script action pause` RESUMED the way an operator resumes it,
and a 30-second sampler on the process.  It is the instrument F90 said the
stability class needs: five launches of 91 s cannot see a lost wakeup
(Sec.5.61), a teardown that cannot be serviced (Sec.5.59/A40), an intermittent
shutdown segfault (Sec.11.15(d)), a frozen-scene micro-instability (B30) or an
unattributed mid-session epoch shift (Sec.5.62).  Every one of those is a
function of HOURS.

WHAT IT IS NOT.  FUNCTIONAL.  It reads no pixel and makes no photometric claim;
only the `--no-scene` canary runs, as its own preflight.  It FIXES nothing: a
finding is recorded against its row, or minted, by a person.

--------------------------------------------------------------------------
THE PROCESS SHAPE, and why it is the shape the protocol forces
--------------------------------------------------------------------------
`fable-dispatch.md` Sec.0.5 forbids the Bash tool's `run_in_background` for long
campaigns (three executor stalls root-caused to it).  So the long-running thing
is NOT a backgrounded tool call: `start` launches a DETACHED DRIVER with
`setsid` and returns at once, the driver owns the application for the whole
soak and writes its state to disk, and every later executor call is a
FOREGROUND read of that state (`status`, `wait`).  An executor abort does not
stop the soak; a successor resumes with `status`.

The driver's state is under the OUTDIR, never under `/tmp` (Sec.0.5: `/tmp` is
session-lifetime and nothing tells you when it will be cleared).

--------------------------------------------------------------------------
TWO SOCKETS, and why the probe has its own
--------------------------------------------------------------------------
The hang detector is the round-trip time of `get status position`, taken by the
SAMPLER thread while the MAIN thread is driving the playlist.  Both threads
would otherwise contend for one socket and the measured round trip would be my
own lock rather than the application's latency.  So:

  socket A  - `$LOGON` subscriber; every fire-and-forget command (play, resume,
              select, date, timerate, dual_dump, shutdown).  Main thread only.
  socket B  - every `get` round trip, from the sampler AND from the interlude,
              under one lock.  The lock WAIT is measured and recorded
              SEPARATELY from the round trip, so a slow sample can never be
              mistaken for a slow application.

Since Sec.11.135 an answer goes to the CONNECTION THAT ASKED when its slot
still matches (`io.cpp:765-791`), so socket B hears its own replies without
`$LOGON`; socket A still subscribes, and its copy of every reply is drained and
discarded.  `ServerSocket::send` puts the NUL terminator on the wire
(`io.cpp:830-834`), so the blob is NUL-separated - stripped in one place.

--------------------------------------------------------------------------
THE FARM - F90's shape, widened to three script directories
--------------------------------------------------------------------------
`ScriptAnnotator::flush` REWRITES THE FILE IT PLAYED whenever a line earns a
`#!` diagnostic or a stale tail must be cleared - sibling `<path>.tmp` then
`rename`, both in the target's own directory (`script_annotator.cpp:163-179`).
`f55_farm.sh` makes `scripts/` and `scripts/fscripts/` real and leaves
`scripts/basis/` a SYMLINK to the owner's directory, and it deliberately does
NOT copy `startup.sts` (F94's gotcha - the app plays that one at launch with
the annotator armed).  Every played directory is rebuilt real, every played
`.sts` is a COPY, the asset sub-directories and media entries beside them stay
symlinks (they are only read), and `startup.sts` is copied.  `sessions/` is
rebuilt real because `session action save` writes there
(`SessionFile.hpp:211`).  The farm-shape properties are asserted before the
launch and `start` aborts if one fails; the real HOME's md5s are asserted in
== out on every file of the frozen set - eleven at F95's defaults (config,
ssystem, `startup.sts` and the eight played shows), 139 on the `fscripts/`
corpus (config, ssystem and all 137 `.sts`).  The digest is PER FILE and never
over a concatenation: `cat *.sts | md5sum` is locale-dependent, because `ls`
and the shell glob collate differently under `fr_FR.UTF-8` (measured at F98's
mint: `68c9b4ba` vs `e2123d2b` over the same 137 files).

The texture cache is NOT isolated - `b3_farm.sh` symlinks it, which is a
recorded property of this recipe (Sec.11.172(i)) and watched, not prevented, by
`f56_manifest.py`.  The canary takes a manifest on every run.
"""

import argparse
import csv
import glob
import hashlib
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import threading
import time
import traceback
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from sts_duration import parse as show_model              # noqa: E402
from sts_duration import show_own_duration                # noqa: E402,F401

DEFAULT_BIN = HERE / ".." / ".." / "build-claude" / "src" / "spacecrafter"
REAL_HOME = Path.home() / ".spacecrafter"
PORT = 7805
FARM_ROOT = Path("/home/claude/sc-f95")     # F95's root; `--root` overrides it

# The pinned simulation clock of the cycle-boundary interlude.  ONE value for
# every cycle of a run, so the dump diff across cycles is taken at the same
# simulation instant - which is what makes it an instrument for Sec.5.62's
# class (a same-binary measurement moving between epochs of one session) rather
# than a reading of the clock advancing.
J0 = 2460000.5

# F19's bound, the one the whole B7 record is written against: "HUNG (no exit
# in 45 s)" (Sec.5.59).  Used for BOTH the live hang criterion and the quit.
HUNG_S = 45.0

# The probe's own budget must exceed HUNG_S or the criterion cannot be
# MEASURED - a probe that gives up at 45 s can only ever say ">= 45", never how
# far past it the application went.
PROBE_BUDGET_S = 90.0

PLAYLIST_DIRS = ["basis", "custom", "deepsky"]     # F95's default playlist

# NEVER a playlist member, in any directory: the application plays this one
# itself at launch (`ScriptMgr::playStartupScript`, script_mgr.cpp:384-388), so
# queueing it would race the driver against the app's own script engine.  It IS
# in the frozen set and it IS copied onto the farm (the annotator rewrites what
# it plays).
NEVER_PLAYED = {"startup.sts"}

# The show grace: a show is given its MODELLED duration plus this before it is
# called a SHOW-TIMEOUT.  F95's constant, kept so the label keeps its meaning
# across both campaigns.
SHOW_GRACE_S = 60.0

# ~~`fscripts/` is NOT played: 137 shows that author bodies (06old.sts authors
# 3000 satellites, Sec.5.137) and run for minutes each.  A second soak over
# them is a named follow-up, not this one.~~  [SUPERSEDED 2026-09-06, F98:
# the follow-up RAN - `fscripts/` is a playlist directory like any other now,
# named with `--playlist-dir fscripts`.  And the "3000" was never measured:
# `06old.sts` carries 170 `body action load` lines, all 170 uncommented
# (Sec.11.218).]  What is excluded is now DERIVED from what is played, so the
# plan can never claim an exclusion the run does not have.


def excluded_dirs(dirs, home=REAL_HOME):
    """-> the script directories this run does NOT play, mechanically."""
    root = Path(home) / "scripts"
    if not root.is_dir():
        return []
    return sorted(d.name for d in root.iterdir()
                  if d.is_dir() and d.name not in dirs
                  and any(d.glob("*.sts")))


# =========================================================================
# THE CRITERIA.  ONE source of this text (I2): `plan` prints it, and
# `artifacts/f95/prediction.txt` IS that output, committed before the first
# launch of the campaign.
# =========================================================================

CRITERIA = """\
==============================================================================
F95 - THE MULTI-HOUR SOAK: FAIL CRITERIA, THE LEAK RULE, AND WHAT IS ONLY
RECORDED.  Committed BEFORE the first launch.
==============================================================================

THE FOUR FAIL FLAGS.  Each names the NUMBER that fires it, so each can fail.

  F1 DEATH   the application's pid is gone before T+H and before the driver
             sent `shutdown action now`.  Observable: `/proc/<pid>` and the
             child's exit status, read by the sampler every S seconds.
             TERMINAL: the driver stops, records, and writes its verdict.
             CONTROL: `kill -9` the app during a `wait` -> F1 within one
             sample.

  F2 HANG    the round trip of `get status position` exceeds 45.0 s (F19's
             bound, Sec.5.59: every "HUNG" in the B7 record is "no exit in
             45 s"), OR the farm's vulkan log gains `This frame stall is very
             long` (fps.cpp:159, twenty consecutive same-frame ticks) while
             the session is UNLOCKED.  The lock state is read beside every
             sample precisely so a 1 Hz screen-lock throttle (F67) is
             ATTRIBUTED and never counted as the application's fault.
             NOT terminal: the driver keeps running so a recovery is visible.
             CONTROL: `kill -STOP` for 60 s -> F2; `kill -CONT` -> a recovered
             sample.

  F3 QUIT    at the end: exit code != 0, OR wall-from-`shutdown action now`-to
             -exit > 45.0 s, OR a teardown fault line in the process's own
             stream (`terminate called`, `Segmentation fault`, `double free`,
             `Assertion`, `core dumped`, `munmap_chunk`, `free(): `,
             `stack smashing`).  A non-exit is Sec.5.59/A40's class and is
             recorded WITH the kill that had to follow it, never masked.

  F4 FROZEN FILE MOVED   any md5 of the eleven real-HOME files (config.ini,
             ssystem.ini, the eight played shows, startup.sts) differs from
             the value recorded before the launch.  Sampled every 20 samples
             and at the end.  TERMINAL: the boundary is the task's, and
             continuing would write more.

THE LEAK RULE (sign-based; the magnitude is recorded either way).

  VmRSS is read at every CYCLE BOUNDARY, immediately after the interlude - the
  same playlist point every time, which is the only thing that makes two
  readings comparable.  Over the boundaries of cycle 2 to the last COMPLETE
  cycle:

      strictly increasing at every step  ==>  LEAK
          reported as MB/cycle and MB/h, with the whole series.
      otherwise                          ==>  NO LEAK BY THIS RULE
          and the report states the largest swing between two consecutive
          boundaries as the FLOOR, and last-vs-first against that floor.  A
          rise smaller than the floor is not evidence of anything.

  Cycle 1 is excluded by construction: it carries first-touch allocation, lazy
  texture upload and the first pass through every show, so it is not the same
  state as any later cycle.  A partial final cycle is excluded from the series
  for the same reason - it is not the same playlist point - and is recorded.

RECORDED, NEVER GATING (a number with no threshold is still evidence):

  - `Frame stall detected` and `This frame stall is very long` counts per
    hour, WITH the screensaver GetActive and logind LockedHint reading beside
    them.  BASELINE HANDED IN: on this binary the F90 smoke suite logged 2
    stalls per ~92 s launch tonight where its 2026-09-05 runs logged 0.  This
    campaign records the rate; it does not gate on it and does not chase it.
  - the per-cycle WALL time and its slope (a show slowing cycle after cycle).
  - log growth in bytes/hour on BOTH the script log (Sec.5.115's uncapped,
    keepHistory=true channel) and the app log - which prices the eight-launch
    retention window the tester chose at R20.
  - the GPU MiB trend per pid (`nvidia-smi --query-compute-apps`).
  - VmSize, thread count and open-fd count.
  - THE DUMP DIFF ACROSS CYCLES at the pinned simulation clock J0 =
    2460000.5 - the Sec.5.62 instrument.  EXPECTED BYTE-IDENTICAL (named here,
    before cycle 2, because a set named afterwards is not a prediction):

        the set of body names in the dump;
        for every body, `old.ecl` and `new.ecl` (position relative to the
            parent) and `old.rotLocalToParent`,
            `old.rotLocalToParentUnprecessed`, `old.matLocalToParent`;
        `old.parent`, `new.parent`, `new.bodyType`, `new.primary`;
        the header's `jd` (pinned by the interlude itself), `timeSpeed`,
            `timePaused`.

    Every one of those is a pure function of the simulation date and the
    loaded data, so at a pinned J0 it cannot legitimately move.  EXPECTED TO
    MOVE, and therefore excluded: everything observer- or view-dependent -
    `old.mat`/`eye`/`dist`/`screen`/`screenSz`/`visible`/`axisRot`,
    `new.mat`/`dist`/`screen`/`attitude`/`surfaceLocked`, `altaz_old`,
    `altaz_new`, and the header's `camera`, `helioToEye`, `oldLocalVision`,
    `oldView`, `anchors`, `ramp`, `control`, `bigTextures`, `gates` - the
    eight shows are image overlays and do not move the camera, but nothing in
    this instrument depends on that and it is not asserted.

    ANY difference inside the expected-constant set is reported WITH ITS
    VALUES against Sec.5.62.  Zero differences is the prediction.

WHAT THIS INSTRUMENT CANNOT SEE, stated here so a green is not over-read: it
reads no pixel, so B30's frozen-scene micro-instability is outside it; it plays
image-overlay shows, so it exercises the media path and the script engine but
not body authoring; and one quit cannot see an intermittent shutdown fault -
that is the ten-cycle secondary arm, run or owed.
==============================================================================
"""


# ------------------------------------------------------------------ plumbing

def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def now_iso():
    return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())


def no_instance():
    """The Sec.11.134(b) probe: /proc/<pid>/comm, every account, no self-match.

    The stock `pgrep -f <path>` pattern is blind to an out-of-tree binary and
    self-matches its own wrapper (measured: 3 reported with nothing running)."""
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def playlist_shows(home=REAL_HOME, dirs=None, skip=None):
    """Every `.sts` of every playlist directory, mechanically - never a
    recalled list, and never `startup.sts` (the app plays that one itself).

    `skip` names shows to leave out.  It exists for ONE reason and the reason
    is recorded with every run that uses it: a show that MEASURABLY takes the
    application down cannot also be the show that lets a multi-hour soak
    measure anything else.  F98 skipped `fscripts/06.sts` after reproducing,
    three ways, that `06.sts` followed by `14.sts` aborts the process in ~15 s
    (Sec.11.218).  A skip is a FINDING made visible, never a green bought
    quietly: it is echoed by `plan`, stored in `config.json` and printed in
    the report."""
    skip = set(skip or ())
    out = []
    for d in (dirs if dirs is not None else PLAYLIST_DIRS):
        for p in sorted((Path(home) / "scripts" / d).glob("*.sts")):
            rel = "%s/%s" % (d, p.name)
            if p.name in NEVER_PLAYED or rel in skip or p.name in skip:
                continue
            out.append(rel)
    return out


def frozen_set(home=REAL_HOME, dirs=None):
    """The real-HOME files whose md5 must be identical in and out.

    config + ssystem + `startup.sts` + EVERY `.sts` of every playlist
    directory - the played ones and the unplayed one alike, because the farm
    holds a copy of each and a write that escaped the farm would land here.
    Eleven files at F95's defaults; 139 on the `fscripts/` corpus."""
    home = Path(home)
    files = [home / "config.ini", home / "ssystem.ini",
             home / "scripts" / "fscripts" / "startup.sts"]
    for d in (dirs if dirs is not None else PLAYLIST_DIRS):
        files += sorted((home / "scripts" / d).glob("*.sts"))
    seen, uniq = set(), []
    for f in files:
        if str(f) not in seen and f.is_file():
            seen.add(str(f))
            uniq.append(f)
    return uniq


def frozen_md5s(home=REAL_HOME, dirs=None):
    return {str(f): md5(f) for f in frozen_set(home, dirs)}


def frozen_digest(md5s):
    """The aggregate fingerprint of a set of files: the PER-FILE md5s, sorted
    in C order, one per line, hashed.  Never `cat *.sts | md5sum` - a
    concatenation digest depends on the collation of the glob that built it
    (`fr_FR.UTF-8` vs `LC_ALL=C` gave two different answers over the same 137
    files, F98's mint).  This reproduces, in-process, exactly
        md5sum <files> | cut -c1-32 | LC_ALL=C sort | md5sum
    so the number is comparable with the one the dispatch measured."""
    body = "".join(h + "\n" for h in sorted(md5s.values())).encode()
    return hashlib.md5(body).hexdigest()


TEARDOWN_FAULT = re.compile(
    r"terminate called|Segmentation fault|double free|Assertion|core dumped|"
    r"munmap_chunk|free\(\): |stack smashing|SIGSEGV|SIGABRT")


_STATE_LOCK = threading.Lock()


def atomic_write_json(path, obj):
    """Atomic for a READER, and safe between WRITERS.

    The first version used ONE temp name, `<path>.tmp`, and the soak's own
    first leg is what found the cost: the sampler thread and the playlist
    thread both call `write_state()`, and when their windows overlapped one
    `os.replace` consumed the temp file the other was still about to rename -
    `FileNotFoundError`, 2 h 44 min into a 3 h run, which ended the leg 16
    minutes early.  A three-hour campaign is exactly the instrument that finds
    a race this narrow, and it found it in the harness rather than in the
    application.  The temp name is now unique per writer AND the whole
    read-modify-replace is serialised, because either alone still leaves two
    writers racing to be last."""
    tmp = Path("%s.%d.%d.tmp" % (path, os.getpid(), threading.get_ident()))
    with _STATE_LOCK:
        tmp.write_text(json.dumps(obj, indent=1, default=str))
        os.replace(str(tmp), str(path))


class LogTail:
    """An incremental reader with cumulative counters.

    The script log is APPENDED across launches and never capped (Sec.5.115), so
    re-reading it whole every half second would grow into megabytes of I/O per
    poll over three hours.  This keeps a byte offset and counts on the delta.
    If the resolved path changes (the channel writes a per-DAY file) the offset
    resets, which is the only correct thing to do with a different file."""

    def __init__(self, logdir, prefix, patterns):
        self.logdir = Path(logdir)
        self.prefix = prefix
        self.patterns = list(patterns)
        self.counts = {p: 0 for p in self.patterns}
        self.path = None
        self.pos = 0
        # the vulkan tail is polled by the SAMPLER thread and, once, by the
        # quit path on the main thread; an unguarded offset would double-count
        # or skip a chunk exactly when the numbers matter most.
        self.lock = threading.Lock()

    def _resolve(self):
        g = sorted(glob.glob(str(self.logdir / (self.prefix + "*.log"))))
        return Path(g[-1]) if g else None

    def poll(self):
        """-> the NEW text since the last poll; counters updated."""
        with self.lock:
            p = self._resolve()
            if p is None:
                return ""
            if p != self.path:
                self.path = p
                self.pos = 0
            try:
                with open(p, "rb") as fh:
                    fh.seek(self.pos)
                    chunk = fh.read()
                    self.pos = fh.tell()
            except OSError:
                return ""
            txt = chunk.decode("latin-1", "replace")
            for pat in self.patterns:
                self.counts[pat] += txt.count(pat)
            return txt

    def size(self):
        p = self._resolve()
        try:
            return p.stat().st_size if p else 0
        except OSError:
            return 0


# ------------------------------------------------------------------- the farm

def build_farm(farm, shows, copy_all_sts=True):
    """F90's `build_farm`, widened to every directory this playlist plays.

    `copy_all_sts` copies EVERY `.sts` of a played directory, not only the
    played ones: on the `fscripts/` corpus the unplayed member is
    `startup.sts`, which the application plays by itself with the annotator
    armed, and a symlink there would let `ScriptAnnotator::flush` rewrite the
    owner's file.  The media entries beside them stay symlinks - they are only
    read, and `fscripts/` holds 897 MB of them.

    Returns the assert dict, so the record says what the farm IS."""
    farm = Path(farm)
    if farm.exists():
        shutil.rmtree(farm)
    r = subprocess.run([str(HERE / "f55_farm.sh"), str(farm)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("f55_farm.sh failed: %s%s" % (r.stdout, r.stderr))
    dst = farm / ".spacecrafter"

    # (1) sessions/ real - `session action save` writes into it.
    (dst / "sessions").unlink(missing_ok=True)
    (dst / "sessions").mkdir(parents=True, exist_ok=True)
    if (REAL_HOME / "sessions").is_dir():
        for e in (REAL_HOME / "sessions").iterdir():
            (dst / "sessions" / e.name).symlink_to(e)

    # (2) every played directory real; every played .sts a COPY; the asset
    #     sub-directories beside them symlinks (read only).
    played = {}
    for rel in shows:
        d, n = rel.split("/", 1)
        played.setdefault(d, set()).add(n)
    for d, names in played.items():
        src_dir = REAL_HOME / "scripts" / d
        tgt_dir = dst / "scripts" / d
        # f55_farm.sh leaves `scripts/fscripts` a REAL directory of symlinks
        # and every other child of `scripts/` a symlink to the owner's
        # directory: unlink() on the real one raises IsADirectoryError, so the
        # two cases are handled apart instead of by exception.
        if tgt_dir.is_symlink() or (tgt_dir.exists() and not tgt_dir.is_dir()):
            tgt_dir.unlink()
        if tgt_dir.is_dir():
            shutil.rmtree(tgt_dir)
        tgt_dir.mkdir(parents=True, exist_ok=True)
        for e in src_dir.iterdir():
            tgt = tgt_dir / e.name
            if e.name in names or (copy_all_sts and e.is_file()
                                   and e.name.endswith(".sts")):
                shutil.copy2(e, tgt)
            else:
                tgt.symlink_to(e)

    # (3) startup.sts a COPY - f55_farm.sh deliberately leaves it to the
    #     caller and the app PLAYS it at launch with the annotator armed.
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
        "startup.is_regular_file":
            (dst / "scripts/fscripts/startup.sts").is_file()
            and not (dst / "scripts/fscripts/startup.sts").is_symlink(),
        "modularSystem.is_real_dir": (dst / "modularSystem").is_dir()
                                     and not (dst / "modularSystem").is_symlink(),
        "log.is_real_dir": (dst / "log").is_dir()
                           and not (dst / "log").is_symlink(),
    }
    for d in sorted(played):
        fd = dst / "scripts" / d
        asserts["scripts/%s.is_real_dir" % d] = fd.is_dir() and not fd.is_symlink()
        sts = [e for e in fd.iterdir() if e.name.endswith(".sts")]
        other = [e for e in fd.iterdir() if not e.name.endswith(".sts")]
        asserts["scripts/%s.sts_count" % d] = len(sts)
        asserts["scripts/%s.every_sts_is_a_copy" % d] = \
            all(e.is_file() and not e.is_symlink() for e in sts)
        asserts["scripts/%s.media_count" % d] = len(other)
        asserts["scripts/%s.every_media_is_a_symlink" % d] = \
            all(e.is_symlink() for e in other)
    for rel in shows:
        p = dst / "scripts" / rel
        asserts["%s.is_regular_file" % rel] = p.is_file() and not p.is_symlink()
    bad = [k for k, v in asserts.items() if v is False]
    if bad:
        raise RuntimeError("farm shape assert failed: %s" % bad)
    return asserts


def farm_sts_md5s(farm, dirs):
    """Every `.sts` COPY on the farm, by md5.

    Compared in == out at the end: `ScriptAnnotator::flush` rewrites the file
    it played when a line earns a `#!` diagnostic, so a farm copy that MOVED is
    the annotator's signature (a datum about the corpus, never a fault of the
    application), and a farm copy that moved on a REAL-HOME path would be the
    boundary breach F4 exists for."""
    dst = Path(farm) / ".spacecrafter"
    out = {}
    for d in dirs:
        p = dst / "scripts" / d
        if p.is_dir():
            for e in sorted(p.glob("*.sts")):
                out[str(e)] = md5(e)
    return out


# ------------------------------------------------------------------ the wire

class Wire:
    """One TCP connection to the application, with its own receive buffer."""

    def __init__(self, logon):
        self.sock = None
        self.rx = b""
        self.logon = logon
        self.errors = []

    def connect(self, deadline):
        while time.time() < deadline:
            try:
                self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=2)
                self.sock.settimeout(0.2)
                if self.logon:
                    self.send("$LOGON", 1.0)
                return True
            except OSError:
                time.sleep(1)
        return False

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

    def send(self, cmd, pause=0.4):
        """Fire and forget.  A send that raises is RECORDED, never fatal: when
        the application is stopped or dying, the socket is exactly where that
        shows up, and losing the driver at that moment would lose the run's
        most interesting samples."""
        try:
            self.sock.sendall((cmd + "\n").encode("latin-1"))
        except OSError as e:                                      # noqa: BLE001
            self.errors.append((time.time(), cmd[:40], repr(e)))
            return False
        self._drain(pause)
        # socket A's copies of every reply are not read by anyone; keep the
        # buffer bounded so a three-hour run cannot grow it without limit.
        if len(self.rx) > 1 << 20:
            self.rx = self.rx[-(1 << 16):]
        return True

    def ask(self, cmd, want, budget):
        """-> (hit, blob, rtt_s).  `rtt_s` is the APPLICATION's latency: the
        clock starts after the send and stops on the recognised reply."""
        mark = len(self.rx)
        t0 = time.time()
        self.sock.sendall((cmd + "\n").encode("latin-1"))
        while time.time() - t0 < budget:
            self._drain(0.2)
            blob = self.rx[mark:].decode("latin-1", "replace").replace("\x00", "")
            hit = want(blob)
            if hit is not None:
                return hit, blob, time.time() - t0
        return None, self.rx[mark:].decode("latin-1", "replace").replace("\x00", ""), \
            time.time() - t0


POSITION_RE = re.compile(
    r"(-?\d+\.\d+);\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);")


def want_position(blob):
    m = POSITION_RE.search(blob)
    return m.group(0) if m else None


# ------------------------------------------------------------- host channels

def read_status(pid):
    out = {"vmrss_kb": "", "vmsize_kb": "", "threads": ""}
    try:
        for line in open("/proc/%d/status" % pid):
            if line.startswith("VmRSS:"):
                out["vmrss_kb"] = int(line.split()[1])
            elif line.startswith("VmSize:"):
                out["vmsize_kb"] = int(line.split()[1])
            elif line.startswith("Threads:"):
                out["threads"] = int(line.split()[1])
    except OSError:
        pass
    return out


def read_fds(pid):
    try:
        return len(os.listdir("/proc/%d/fd" % pid))
    except OSError:
        return ""


def read_gpu_mib(pid):
    try:
        r = subprocess.run(
            ["nvidia-smi", "--query-compute-apps=pid,used_memory",
             "--format=csv,noheader,nounits"],
            capture_output=True, text=True, timeout=15)
        for line in r.stdout.splitlines():
            parts = [x.strip() for x in line.split(",")]
            if len(parts) == 2 and parts[0] == str(pid):
                return int(parts[1])
        return 0
    except Exception:                                             # noqa: BLE001
        return ""


def read_screensaver():
    """F67's channel (`f67_tcp_live.py:162`): the GNOME screensaver's own say."""
    try:
        r = subprocess.run(
            ["gdbus", "call", "--session", "--dest", "org.gnome.ScreenSaver",
             "--object-path", "/org/gnome/ScreenSaver",
             "--method", "org.gnome.ScreenSaver.GetActive"],
            capture_output=True, text=True, timeout=15,
            env={**os.environ,
                 "DBUS_SESSION_BUS_ADDRESS":
                     "unix:path=/run/user/%d/bus" % os.getuid()})
        s = r.stdout.strip()
        return "true" if "true" in s else ("false" if "false" in s else s[:20])
    except Exception:                                             # noqa: BLE001
        return "probe-error"


def resolve_session():
    try:
        r = subprocess.run(["loginctl", "list-sessions", "--no-legend"],
                           capture_output=True, text=True, timeout=15)
        for line in r.stdout.splitlines():
            sid = line.split()[0]
            q = subprocess.run(["loginctl", "show-session", sid,
                                "-p", "State", "-p", "Type"],
                               capture_output=True, text=True, timeout=15)
            if "State=active" in q.stdout:
                return sid
    except Exception:                                             # noqa: BLE001
        pass
    return ""


def read_locked_hint(sid):
    if not sid:
        return ""
    try:
        r = subprocess.run(["loginctl", "show-session", sid,
                            "-p", "LockedHint", "--value"],
                           capture_output=True, text=True, timeout=15)
        return r.stdout.strip()[:10]
    except Exception:                                             # noqa: BLE001
        return "probe-error"


# ---------------------------------------------------------------- the verdict

def leak_verdict(series):
    """THE LEAK RULE, as a pure function so it can be unit-tested both ways.

    `series` = the VmRSS reading at each COMPLETE cycle boundary, in order,
    cycle 1 first.  The rule is applied from cycle 2 onward."""
    tail = series[1:]
    if len(tail) < 3:
        return {"verdict": "INCONCLUSIVE",
                "why": "fewer than 3 comparable cycle boundaries (%d) - the "
                       "rule needs at least two consecutive steps to have a "
                       "sign and one more to have a floor" % len(tail),
                "series": series, "series_used": tail}
    steps = [tail[i + 1] - tail[i] for i in range(len(tail) - 1)]
    strictly_increasing = all(s > 0 for s in steps)
    floor = max(abs(s) for s in steps)
    span = tail[-1] - tail[0]
    if strictly_increasing:
        return {"verdict": "LEAK", "why": "strictly increasing at every one of "
                                          "the %d steps from cycle 2" % len(steps),
                "series": series, "series_used": tail, "steps": steps,
                "per_cycle": span / float(len(steps)), "span": span,
                "floor": floor}
    return {"verdict": "NO LEAK BY THIS RULE",
            "why": "%d of %d steps from cycle 2 are not positive" %
                   (sum(1 for s in steps if s <= 0), len(steps)),
            "series": series, "series_used": tail, "steps": steps,
            "span": span, "floor": floor,
            "span_vs_floor": "span %s vs floor %s" % (span, floor)}


def show_outcome(seen_end, elapsed, budget, will_cap, slack=0.25):
    """THE FOUR THINGS A SHOW-PLAY CAN BE, as a pure function so the labels can
    be tested both ways without an application.

      ended         `ScriptMgr: script end` was seen.  The show finished.
      CAPPED        it did not, its deadline arrived, and the MODEL had said
                    the show is longer than the cap.  By design; not a finding.
      SHOW-TIMEOUT  it did not, its deadline arrived, and the model had NOT
                    said so - the show overran its own modelled duration plus
                    the grace.  F95's meaning, and a FINDING.
      INTERRUPTED   it did not and the deadline had NOT arrived: the RUN ended
                    under it (stop file, terminal flag, or T+H mid-playlist).
                    Neither label that means something about the show may be
                    applied here - the DEATH control produced exactly this case
                    and the first implementation called it CAPPED."""
    if seen_end:
        return "ended"
    if elapsed < budget - slack:
        return "INTERRUPTED"
    return "CAPPED" if will_cap else "SHOW-TIMEOUT"


def very_long_arm(prev, now, screensaver, locked_hint):
    """F2's SECOND arm, as `prediction.txt` words it: the vulkan log GAINS a
    `This frame stall is very long` in an UNLOCKED session.

    The first implementation tested the CUMULATIVE count against zero, and the
    HANG control showed what that costs: one very-long stall (caused by the
    control's own SIGSTOP) latched the flag onto every later sample, so the
    channel could never say "and now it is quiet again".  GAINS means an
    increase since the previous sample, which is what was committed, and it is
    also the only form that can go back to green."""
    if now <= prev:
        return False
    return screensaver != "true" and locked_hint != "yes"


CONST_BODY_FIELDS_OLD = ["ecl", "rotLocalToParent", "rotLocalToParentUnprecessed",
                         "matLocalToParent", "parent"]
CONST_BODY_FIELDS_NEW = ["ecl", "parent", "bodyType", "primary"]
CONST_HEADER_FIELDS = ["jd", "timeSpeed", "timePaused"]


def dump_constant_view(path):
    """The EXPECTED-CONSTANT projection of a dual dump, named in CRITERIA."""
    import dumpread
    view = {"header": {}, "bodies": {}}
    first = True
    for line in open(path, encoding="utf-8", errors="replace"):
        if not line.strip():
            continue
        try:
            rec = dumpread.loads(line)
        except Exception:                                         # noqa: BLE001
            continue
        if first:
            first = False
            if rec.get("type") == "header":
                view["header"] = {k: rec.get(k) for k in CONST_HEADER_FIELDS}
                continue
        if rec.get("type") == "body" and "name" in rec:
            b = {}
            old, new = rec.get("old") or {}, rec.get("new") or {}
            for f in CONST_BODY_FIELDS_OLD:
                if f in old:
                    b["old." + f] = old[f]
            for f in CONST_BODY_FIELDS_NEW:
                if f in new:
                    b["new." + f] = new[f]
            view["bodies"][rec["name"]] = b
    return view


def dump_diff(a, b):
    """-> a list of differences inside the expected-constant set only."""
    diffs = []
    for k in CONST_HEADER_FIELDS:
        if a["header"].get(k) != b["header"].get(k):
            diffs.append({"where": "header." + k,
                          "a": a["header"].get(k), "b": b["header"].get(k)})
    na, nb = set(a["bodies"]), set(b["bodies"])
    for n in sorted(na - nb):
        diffs.append({"where": "body:%s" % n, "a": "present", "b": "ABSENT"})
    for n in sorted(nb - na):
        diffs.append({"where": "body:%s" % n, "a": "ABSENT", "b": "present"})
    for n in sorted(na & nb):
        fa, fb = a["bodies"][n], b["bodies"][n]
        for f in sorted(set(fa) | set(fb)):
            if fa.get(f) != fb.get(f):
                diffs.append({"where": "%s.%s" % (n, f),
                              "a": fa.get(f), "b": fb.get(f)})
    return diffs


# ==================================================================== driver

SAMPLE_COLS = [
    "i", "wall_iso", "t_rel_s", "phase", "cycle", "show",
    "pid_alive", "vmrss_kb", "vmsize_kb", "threads", "fds", "gpu_mib",
    "applog_b", "scapplog_b", "scriptlog_b", "vulkanlog_b",
    "stall_detected", "stall_very_long",
    "probe_ok", "probe_rtt_ms", "probe_lock_wait_ms", "sim_jd",
    "screensaver", "locked_hint", "loadavg1", "flags",
]

# The finer, SECONDARY series: one row per SHOW boundary.  The LEAK verdict
# stays on CYCLE boundaries (F95's rule, unchanged) because only there is the
# playlist at the same point; this is recorded so a step inside a cycle can be
# ATTRIBUTED to a show afterwards.
SHOW_COLS = [
    "cycle", "idx", "show", "t_start_iso", "wall_s", "own_s", "expanded_s",
    "planned_s", "deadline_s", "outcome", "resumed", "declared_pauses",
    "cmd_lines", "seen_load", "vmrss_kb", "vmsize_kb", "threads", "fds",
    "scriptlog_b",
]

CYCLE_COLS = [
    "cycle", "complete", "t_start_iso", "t_end_iso", "wall_s",
    "shows_played", "shows_timeout", "shows_capped", "pauses_resumed",
    "bodies_old", "bodies_new", "bodies_both", "old_only", "new_only",
    "vmrss_kb", "vmsize_kb", "threads", "fds", "gpu_mib",
    "applog_b", "scriptlog_b", "stall_detected", "stall_very_long",
    "dump_file", "dump_bytes", "dump_diff_vs_cycle2", "jd_after",
]


class Driver:

    def __init__(self, out):
        self.out = Path(out)
        self.cfg = json.loads((self.out / "config.json").read_text())
        self.farm = Path(self.cfg["farm"])
        self.home = self.farm / ".spacecrafter"
        self.binary = self.cfg["bin"]
        self.hours = float(self.cfg["hours"])
        self.sample_s = float(self.cfg["sample"])
        self.shows = list(self.cfg["shows"])
        self.frozen_in = dict(self.cfg["frozen_in"])
        self.dirs = list(self.cfg.get("playlist_dirs") or PLAYLIST_DIRS)
        self.cap = self.cfg.get("cap")          # None = F95's behaviour
        self.authored = list(self.cfg.get("authored_probes") or [])
        self.applog = self.out / "soak.applog"

        self.proc = None
        self.a = Wire(logon=True)
        self.b = Wire(logon=False)
        self.block = threading.Lock()

        self.t0 = None
        self.i = 0
        self.cycle = 0
        self.show = ""
        self.phase = "starting"
        self.flags = []
        self.flag_detail = []
        self.stop_evt = threading.Event()
        self.terminal = threading.Event()
        self.stall = {"detected": 0, "very_long": 0}
        self._prev_very_long = 0
        self.last_sample = {}
        self.session_id = ""
        self.vulk = None
        self.script_tail = None

        sp, cp = self.out / "samples.csv", self.out / "cycles.csv"
        shp = self.out / "shows.csv"
        sh_new = not (shp.exists() and shp.stat().st_size > 0)
        self.shows_fh = open(shp, "a", buffering=1)
        self.shows_w = csv.DictWriter(self.shows_fh, fieldnames=SHOW_COLS,
                                      extrasaction="ignore", restval="")
        if sh_new:
            self.shows_w.writeheader()
        sp_new = not (sp.exists() and sp.stat().st_size > 0)
        cp_new = not (cp.exists() and cp.stat().st_size > 0)
        self.samples_fh = open(sp, "a", buffering=1)
        self.samples_w = csv.DictWriter(self.samples_fh, fieldnames=SAMPLE_COLS,
                                        extrasaction="ignore", restval="")
        if sp_new:
            self.samples_w.writeheader()
        self.cycles_fh = open(cp, "a", buffering=1)
        self.cycles_w = csv.DictWriter(self.cycles_fh, fieldnames=CYCLE_COLS,
                                       extrasaction="ignore", restval="")
        if cp_new:
            self.cycles_w.writeheader()

    # ---- logging and state
    def log(self, msg):
        print("[%s +%7.1fs] %s"
              % (now_iso(), (time.time() - self.t0) if self.t0 else 0.0, msg),
              flush=True)

    def raise_flag(self, flag, why, terminal=False, **detail):
        if flag not in self.flags:
            self.flags.append(flag)
        self.flag_detail.append({"flag": flag, "at_iso": now_iso(),
                                 "sample": self.i, "cycle": self.cycle,
                                 "why": why, **detail})
        self.log("FLAG %s: %s" % (flag, why))
        if terminal:
            self.terminal.set()
            self.stop_evt.set()
        self.write_state()

    def write_state(self):
        atomic_write_json(self.out / "state.json", {
            "phase": self.phase, "pid": self.proc.pid if self.proc else None,
            "t0_iso": time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(self.t0))
                      if self.t0 else None,
            "t0_epoch": self.t0, "now_iso": now_iso(),
            "elapsed_s": round(time.time() - self.t0, 1) if self.t0 else 0,
            "target_s": self.hours * 3600.0,
            "hours": self.hours, "sample_s": self.sample_s,
            "samples": self.i, "cycle": self.cycle, "show": self.show,
            "flags": self.flags, "flag_detail": self.flag_detail,
            "stall": dict(self.stall), "last_sample": self.last_sample,
            "driver_pid": os.getpid(),
            "finished": self.phase in ("done", "aborted"),
        })

    # ---- the frozen-file assert
    def check_frozen(self):
        moved = []
        for f, want in self.frozen_in.items():
            try:
                got = md5(f)
            except OSError:
                got = "MISSING"
            if got != want:
                moved.append({"file": f, "in": want, "out": got})
        return moved

    # ---- the sampler thread
    def sampler(self):
        n = 0
        while not self.stop_evt.is_set():
            target = self.t0 + (n + 1) * self.sample_s
            while time.time() < target and not self.stop_evt.is_set():
                time.sleep(0.25)
            if self.stop_evt.is_set():
                break
            n += 1
            try:
                self.take_sample()
            except Exception as e:                                # noqa: BLE001
                self.log("sampler raised %r" % e)
            # a probe that overran its slot must not make every later sample
            # late: catch the schedule up and record nothing about the skip
            # beyond the row's own wall clock, which already says it.
            while self.t0 + (n + 1) * self.sample_s < time.time():
                n += 1

    def take_sample(self):
        self.i += 1
        pid = self.proc.pid
        alive = self.proc.poll() is None
        row = {"i": self.i, "wall_iso": now_iso(),
               "t_rel_s": round(time.time() - self.t0, 1),
               "phase": self.phase, "cycle": self.cycle, "show": self.show,
               "pid_alive": int(alive)}
        row.update(read_status(pid) if alive else
                   {"vmrss_kb": "", "vmsize_kb": "", "threads": ""})
        row["fds"] = read_fds(pid) if alive else ""
        row["gpu_mib"] = read_gpu_mib(pid) if alive else ""
        try:
            row["applog_b"] = self.applog.stat().st_size
        except OSError:
            row["applog_b"] = ""
        row["scapplog_b"] = self.appfile_size("spacecrafter")
        row["scriptlog_b"] = self.script_size()
        row["vulkanlog_b"] = self.vulk.size()
        self.vulk.poll()
        self.stall["detected"] = self.vulk.counts["Frame stall detected"]
        self.stall["very_long"] = self.vulk.counts["This frame stall is very long"]
        row["stall_detected"] = self.stall["detected"]
        row["stall_very_long"] = self.stall["very_long"]
        row["screensaver"] = read_screensaver()
        row["locked_hint"] = read_locked_hint(self.session_id)
        try:
            row["loadavg1"] = round(os.getloadavg()[0], 2)
        except OSError:
            row["loadavg1"] = ""

        # A DEAD PID IS NEVER PROBED.  Probing one costs the full 90 s budget
        # and would then read as a HANG - the detector would answer F2 for a
        # process that is not there.  The row is written and the sample ends
        # here whatever the phase; only the FLAG is phase-dependent, because a
        # pid gone AFTER `shutdown action now` is the quit, not a death.
        if not alive:
            row["probe_ok"], row["probe_rtt_ms"] = 0, ""
            row["probe_lock_wait_ms"], row["sim_jd"] = "", ""
            row["flags"] = "F1" if self.phase != "quitting" else ""
            self.samples_w.writerow(row)
            self.last_sample = row
            if self.phase != "quitting":
                self.raise_flag(
                    "F1", "the application's pid %d is gone at +%.1f s, before "
                          "T+H and before any shutdown was sent (exit status %s)"
                    % (pid, time.time() - self.t0, self.proc.returncode),
                    terminal=True, exit_status=self.proc.returncode)
            return

        lw0 = time.time()
        got = self.block.acquire(timeout=PROBE_BUDGET_S + 30.0)
        lock_wait = time.time() - lw0
        if not got:
            row["probe_ok"], row["probe_rtt_ms"] = 0, ""
            row["probe_lock_wait_ms"] = round(lock_wait * 1000, 1)
            row["sim_jd"], row["flags"] = "", ""
            self.samples_w.writerow(row)
            self.last_sample = row
            return
        try:
            hit, _blob, rtt = self.b.ask("get status position", want_position,
                                         PROBE_BUDGET_S)
        except OSError as e:                                      # noqa: BLE001
            hit, rtt = None, PROBE_BUDGET_S
            self.log("probe socket raised %r" % e)
        finally:
            self.block.release()
        row["probe_ok"] = 1 if hit else 0
        row["probe_rtt_ms"] = round(rtt * 1000, 1)
        row["probe_lock_wait_ms"] = round(lock_wait * 1000, 1)
        row["sim_jd"] = hit.split(";")[3].strip() if hit else ""

        fl, why = [], []
        if rtt > HUNG_S:
            fl.append("F2")
            why.append("round trip %.1f s exceeds the %.1f s bound" % (rtt, HUNG_S))
        if very_long_arm(self._prev_very_long, self.stall["very_long"],
                         row["screensaver"], row["locked_hint"]):
            fl.append("F2")
            why.append("`This frame stall is very long` gained %d (total %d) with "
                       "screensaver=%s lockedHint=%s"
                       % (self.stall["very_long"] - self._prev_very_long,
                          self.stall["very_long"], row["screensaver"],
                          row["locked_hint"]))
        self._prev_very_long = self.stall["very_long"]
        row["flags"] = ",".join(sorted(set(fl)))
        self.samples_w.writerow(row)
        self.last_sample = row

        if "F2" in fl and "F2" not in self.flags:
            self.raise_flag(
                "F2", "; ".join(why),
                rtt_s=round(rtt, 2), very_long=self.stall["very_long"],
                screensaver=row["screensaver"], locked_hint=row["locked_hint"])
        elif "F2" in fl:
            # every later firing is recorded too - a hang that recurs is a
            # different fact from a hang that happened once.
            self.flag_detail.append({"flag": "F2-again", "at_iso": now_iso(),
                                     "sample": self.i, "rtt_s": round(rtt, 2),
                                     "why": "; ".join(why)})

        if self.i % 20 == 0:
            moved = self.check_frozen()
            if moved:
                self.raise_flag("F4", "a frozen real-HOME file moved: %s"
                                % [m["file"] for m in moved],
                                terminal=True, moved=moved)
        self.write_state()

    def appfile_size(self, prefix):
        g = sorted(glob.glob(str(self.home / "log" / (prefix + "*.log"))))
        try:
            return Path(g[-1]).stat().st_size if g else 0
        except OSError:
            return 0

    def script_size(self):
        return self.appfile_size("script")

    # ---- the playlist
    def show_budget(self, path):
        """-> (own, expanded, planned, deadline).  ONE place decides how long a
        show is given, and the two labels that can come out of it:

          CAPPED       the MODEL says the show is longer than the cap, so the
                       driver ends it at the cap.  A DESIGN decision, declared
                       before the launch, never a finding.  `panorama0.sts`
                       loops 1 000 000 times - it is infinite by construction
                       and no run can wait for it.
          SHOW-TIMEOUT the show did not end within its OWN modelled duration
                       plus the grace - F95's meaning, unchanged.  A finding,
                       whether the fault is the application's or the model's;
                       the model's blindness is enumerated in `sts_duration`
                       and the predicted set is committed before the launch.
        """
        m = show_model(path)
        own, exp = m["own"], m["expanded"]
        will_cap = self.cap is not None and exp > self.cap
        planned = min(exp, self.cap) if self.cap is not None else exp
        # A show the cap will cut is ended AT the cap: the grace exists to let
        # a show finish its own authored content, and a capped show has no
        # "own end" to wait for.  Everything else keeps F95's deadline exactly.
        deadline = planned if will_cap else planned + SHOW_GRACE_S
        return own, exp, planned, deadline, will_cap, m

    def play_show(self, rel):
        path = self.home / "scripts" / rel
        own, expanded, planned, budget, will_cap, m = self.show_budget(path)
        declared, cmd_lines = m["pauses"], m["lines"]
        with self.block:
            self.script_tail.poll()
            before = self.script_tail.counts["ScriptMgr::script action pause"]
            end_before = self.script_tail.counts["ScriptMgr: script end"]
            load_before = self.script_tail.counts["ScriptMgr: load"]
            self.a.send("script action play filename %s" % rel, 1.0)
        t0 = time.time()
        resumed, seen_end, seen_load = 0, False, False
        while time.time() - t0 < budget and not self.stop_evt.is_set():
            with self.block:
                self.script_tail.poll()
                c = self.script_tail.counts
                if c["ScriptMgr: load"] > load_before:
                    seen_load = True
                if c["ScriptMgr: script end"] > end_before:
                    seen_end = True
                elif c["ScriptMgr::script action pause"] > before:
                    before = c["ScriptMgr::script action pause"]
                    # THE OPERATOR'S ANSWER.  Never `flag skip_pause on`:
                    # honour-vs-skip under unattended play is the tester's
                    # question (Sec.3, session 23), not this driver's.
                    self.a.send("script action resume", 0.6)
                    resumed += 1
            if seen_end:
                break
            time.sleep(0.5)
        elapsed = time.time() - t0
        wall = round(elapsed, 1)
        # A show can also end because the RUN ended - the stop file, a terminal
        # flag, or T+H arriving mid-playlist.  The DEATH control found this:
        # `01.sts` was cut short at 47 s of a 60 s deadline by the `kill -9`
        # and the first version labelled it CAPPED.  A show that never reached
        # its deadline is INTERRUPTED, and neither of the two labels that mean
        # something about the show itself may be applied to it - SHOW-TIMEOUT
        # is a FINDING and a finding manufactured by the run's own end is
        # exactly the kind of false positive this campaign must not produce.
        outcome = show_outcome(seen_end, elapsed, budget, will_cap)
        unfinished = outcome != "ended"
        interrupted = outcome == "INTERRUPTED"
        capped = outcome == "CAPPED"
        timed_out = outcome == "SHOW-TIMEOUT"
        if unfinished:
            self.log("%s %s after %.1f s (deadline %.1f, own %.1f, expanded "
                     "%.1f, planned %.1f, %d pause(s) resumed, load seen %s)"
                     % ("INTERRUPTED" if interrupted else
                        ("CAPPED" if capped else "SHOW-TIMEOUT"), rel, wall,
                        budget, own, expanded, planned, resumed, seen_load))
            with self.block:
                # leave the engine in a known state for the next show
                self.a.send("script action end", 1.0)
                time.sleep(1.0)
                self.script_tail.poll()
        return {"show": rel, "wall_s": wall, "own_s": own,
                "expanded_s": expanded, "planned_s": planned,
                "deadline_s": round(budget, 1),
                "declared_pauses": declared, "resumed": resumed,
                "cmd_lines": cmd_lines, "seen_load": seen_load,
                "outcome": outcome, "capped": capped, "timeout": timed_out,
                "interrupted": interrupted}

    def interlude(self, cycle):
        """select . read out . a dual dump at the PINNED clock . restore."""
        f = self.out / ("cycle_%03d.json" % cycle)
        for p in (f, Path(str(f) + ".navstr")):
            if p.exists():
                p.unlink()
        with self.block:
            self.a.send("select planet Mars", 1.0)
            obj, _, obj_rtt = self.b.ask(
                "get status object",
                lambda b: (b.strip() or None) if len(b.strip()) > 3 else None, 20.0)
            pos, _, pos_rtt = self.b.ask("get status position", want_position, 20.0)
            # freeze FIRST, then pin: the value dumped is then J0 itself and
            # not J0 plus whatever elapsed between the two commands.
            self.a.send("timerate rate 0", 0.8)
            self.a.send("date jday %.6f" % J0, 0.8)
            self.a.send("body action dual_dump filename %s" % f, 1.5)
        end = time.time() + 60.0
        while time.time() < end:
            if f.exists() and f.stat().st_size > 0 \
                    and Path(str(f) + ".navstr").exists():
                time.sleep(0.8)
                break
            time.sleep(0.4)
        authoring = self.authoring_probe(f)
        authoring["where"] = "boundary"
        with self.block:
            self.a.send("deselect", 0.6)
            self.a.send("timerate rate 1", 0.6)
        return {"dump": f, "dump_bytes": f.stat().st_size if f.exists() else 0,
                "get_object": (obj or "")[:400], "get_object_rtt_s": round(obj_rtt, 2),
                "get_position": (pos or ""), "get_position_rtt_s": round(pos_rtt, 2),
                "authoring": authoring}

    # ---- the authoring instrument (F98)
    def authoring_snapshot(self, cycle, rel):
        """The SAME probe, taken immediately after an authoring show, because
        at the cycle boundary the corpus has already undone its own work.

        The playlist's LAST show is `panorama5old2.sts` and it issues `body
        action clear` [observed: the corpus; 6 shows carry it], which reaches
        `Core` -> `removeSupplementalBodies` and drops every deleteable body in
        BOTH trees [observed: core.cpp:1029, protosystem.cpp:204-227, the B34
        note at core.cpp:1020-1028].  So a boundary-only instrument would
        measure an empty universe every cycle and could not see the authoring
        at all.  The boundary probe is kept (it is the mandated one, and "the
        corpus cleans up after itself" is itself a measurement); this one is
        taken where the authored bodies are supposed to exist.  Cost: one dual
        dump per authoring probe show per cycle, at a NON-pinned clock, named
        so it can never be picked up by the Sec.5.62 cycle-dump comparison."""
        tag = rel.split("/")[-1].replace(".sts", "")
        f = self.out / ("authoring_c%03d_%s.json" % (cycle, tag))
        with self.block:
            self.a.send("body action dual_dump filename %s" % f, 1.5)
        end = time.time() + 45.0
        while time.time() < end:
            if f.exists() and f.stat().st_size > 0 \
                    and Path(str(f) + ".navstr").exists():
                time.sleep(0.6)
                break
            time.sleep(0.4)
        snap = self.authoring_probe(f, only_from=rel)
        snap.update({"where": "after:%s" % rel, "cycle": cycle,
                     "dump": str(f),
                     "dump_bytes": f.stat().st_size if f.exists() else 0})
        self.log("authoring after %s: bodies old/new %s/%s, probes %s"
                 % (rel, snap.get("bodies_old"), snap.get("bodies_new"),
                    [(p["name"], p["present_old"], p["present_new"])
                     for p in snap["probes"]]))
        return snap

    def authoring_probe(self, dump_path, only_from=None):
        """WHAT THE CORPUS AUTHORED, read on BOTH halves of the dump.

        The soak's own reason: `fscripts/` authors bodies and F95's playlist
        did not.  Two questions per cycle boundary.

        (1) HOW MANY bodies does each path hold?  `dumpread.load_dump` returns
            the records present on both halves and the two one-sided lists, so
            a body the OLD path kept and the NEW path lost - Sec.5.137's exact
            signature, and the new path is the one that DRAWS
            (Sec.11.206(c)) - is counted rather than inferred.

        (2) For one named body per authoring show: is it in the old half, the
            new half, and what does the CONTROL SURFACE say?  The readout is
            recorded but it is NOT the authority: Sec.5.139 (measured this same
            day) says a body outside the view cone keeps a frozen eye-frame
            position, so `get status object` can answer in a camera state that
            no longer exists.  THE DUMP HALVES DECIDE; the readout is beside
            them as the false-success channel Sec.5.137 is about."""
        res = {"probes": []}
        try:
            import dumpread
            # require_old=False: MEASURING the emptiness is this probe's job -
            # `bodies_old` below is the number Sec.11.218(h) reported as 246 ->
            # 1 -> 0 and Sec.5.143 was minted from, so the F101 guard must not
            # turn this campaign's own finding into an exception.
            _hdr, pairs, missing_new, missing_old = dumpread.load_dump(
                dump_path, require_old=False)
            names = {r["name"] for r in pairs} | set(missing_new) | set(missing_old)
            res.update({
                "bodies_both": len(pairs),
                "old_only": len(missing_new), "new_only": len(missing_old),
                "bodies_old": len(pairs) + len(missing_new),
                "bodies_new": len(pairs) + len(missing_old),
                "old_only_names": sorted(missing_new)[:40],
                "new_only_names": sorted(missing_old)[:40],
            })
        except Exception as e:                                    # noqa: BLE001
            res["error"] = repr(e)
            names, pairs, missing_new, missing_old = set(), [], [], []
        by_name = {r["name"]: r for r in pairs}
        for probe in self.authored:
            if only_from is not None and probe["from"] != only_from:
                continue
            name, src = probe["name"], probe["from"]
            # BOTH SPELLINGS OF THE SELECT, because the shakedown measured the
            # readout channel answering NOTHING for `select planet ALSAT 1`
            # while `search name ALSAT 1` answered `ALSAT 1(P);` - a body whose
            # authored name contains a space.  Which spelling the command
            # surface takes is a measurement, not a thing to assume, and the
            # false-success channel Sec.5.137 is about is exactly this readout,
            # so it has to be reached before anything can be said about it.
            with self.block:
                srch, _, s_rtt = self.b.ask(
                    "search name %s" % name,
                    lambda b: (b.strip() or None) if b.strip() else None, 15.0)
                self.a.send("select planet %s" % name, 1.0)
                obj, _, o_rtt = self.b.ask(
                    "get status object",
                    lambda b: (b.strip() or None) if len(b.strip()) > 3 else None,
                    12.0)
                quoted = None
                if not obj and " " in name:
                    self.a.send('select planet "%s"' % name, 1.0)
                    quoted, _, q_rtt = self.b.ask(
                        "get status object",
                        lambda b: (b.strip() or None) if len(b.strip()) > 3
                        else None, 12.0)
                    if quoted:
                        obj, o_rtt = quoted, q_rtt
            rec = by_name.get(name)
            res["probes"].append({
                "name": name, "from": src,
                "present_old": name in names and name not in missing_old,
                "present_new": name in names and name not in missing_new,
                "in_dump": name in names,
                "old_visible": (rec or {}).get("old", {}).get("visible")
                               if rec else None,
                "new_visible": (rec or {}).get("new", {}).get("visible")
                               if rec else None,
                "search_reply": (srch or "")[:160],
                "search_rtt_s": round(s_rtt, 2),
                "get_object": (obj or "")[:400], "get_object_rtt_s": round(o_rtt, 2),
                "select_needed_quotes": bool(quoted),
            })
        return res

    # ---- the quit
    def quit_app(self):
        self.phase = "quitting"
        self.write_state()
        applog_mark = self.applog.stat().st_size if self.applog.exists() else 0
        self.vulk.poll()
        stalls_before = dict(self.stall)
        t0 = time.time()
        try:
            with self.block:
                self.a.send("shutdown action now", 0.4)
        except OSError as e:                                      # noqa: BLE001
            self.log("shutdown send raised %r (the socket may be closing)" % e)
        hung, killed = False, False
        try:
            self.proc.wait(timeout=180)
        except subprocess.TimeoutExpired:
            hung = True
            self.proc.kill()
            killed = True
            try:
                self.proc.wait(timeout=30)
            except subprocess.TimeoutExpired:
                pass
        wall = time.time() - t0
        rc = self.proc.returncode
        time.sleep(2)
        tail = ""
        try:
            with open(self.applog, "rb") as fh:
                fh.seek(applog_mark)
                tail = fh.read().decode("latin-1", "replace")
        except OSError:
            pass
        faults = [l.strip() for l in tail.splitlines() if TEARDOWN_FAULT.search(l)]
        self.vulk.poll()
        why = []
        if rc != 0:
            why.append("exit code %s (%s)"
                       % (rc, "signal %d" % -rc if rc and rc < 0 else "non-zero"))
        if wall > HUNG_S:
            why.append("wall-to-exit %.1f s exceeds the %.1f s bound" % (wall, HUNG_S))
        if faults:
            why.append("%d teardown fault line(s): %s"
                       % (len(faults), faults[0][:160]))
        q = {"exit_code": rc, "wall_to_exit_s": round(wall, 2), "hung": hung,
             "killed": killed, "teardown_faults": faults[:20],
             "teardown_tail": tail[-4000:],
             "stall_detected_at_quit": self.stall["detected"],
             "stall_very_long_at_quit": self.stall["very_long"],
             "stall_detected_before_quit": stalls_before["detected"],
             "proc_comm_hits_after": no_instance()}
        if why:
            self.raise_flag("F3", "; ".join(why), **q)
        return q

    # ---- the run
    def run(self):
        self.t0 = time.time()
        self.phase = "launching"
        self.session_id = resolve_session()
        self.vulk = LogTail(self.home / "log", "vulkan",
                            ["Frame stall detected",
                             "This frame stall is very long"])
        self.script_tail = LogTail(self.home / "log", "script",
                                   ["ScriptMgr::script action pause",
                                    "ScriptMgr: script end",
                                    "ScriptMgr: load"])
        self.log("F95 driver: out=%s hours=%.4f sample=%.1f shows=%d J0=%.6f"
                 % (self.out, self.hours, self.sample_s, len(self.shows), J0))
        env = {**os.environ, "HOME": str(self.farm),
               "DISPLAY": os.environ.get("DISPLAY", ":2")}
        self.proc = subprocess.Popen([self.binary], cwd=str(self.home),
                                     stdout=open(self.applog, "w"),
                                     stderr=subprocess.STDOUT, env=env)
        self.log("app pid %d" % self.proc.pid)
        deadline = time.time() + 300
        if not self.a.connect(deadline) or not self.b.connect(deadline):
            self.phase = "aborted"
            self.raise_flag("F1", "the application never accepted a connection "
                                  "on port %d within 300 s (poll %s)"
                            % (PORT, self.proc.poll()), terminal=True)
            if self.proc.poll() is None:
                self.proc.kill()
            self.finish({})
            return
        tcp_wait = round(time.time() - self.t0, 1)
        self.log("TCP up after %.1f s on two sockets" % tcp_wait)
        self.phase = "soaking"
        self.write_state()

        th = threading.Thread(target=self.sampler, daemon=True)
        th.start()

        t_end = self.t0 + self.hours * 3600.0
        cycles = []
        try:
            while time.time() < t_end and not self.stop_evt.is_set():
                self.cycle += 1
                c0 = time.time()
                c_start_iso = now_iso()
                played, timeouts, capped, resumed = 0, 0, 0, 0
                snaps = []
                probe_shows = {p["from"] for p in self.authored}
                for idx, rel in enumerate(self.shows, 1):
                    if time.time() >= t_end or self.stop_evt.is_set():
                        break
                    self.show = rel
                    s_iso = now_iso()
                    r = self.play_show(rel)
                    played += 1
                    resumed += r["resumed"]
                    if r["timeout"]:
                        timeouts += 1
                    if r["capped"]:
                        capped += 1
                    # the finer, secondary series: RSS at every SHOW boundary
                    alive = self.proc.poll() is None
                    stt = read_status(self.proc.pid) if alive else {}
                    self.shows_w.writerow({
                        "cycle": self.cycle, "idx": idx, "t_start_iso": s_iso,
                        "vmrss_kb": stt.get("vmrss_kb", ""),
                        "vmsize_kb": stt.get("vmsize_kb", ""),
                        "threads": stt.get("threads", ""),
                        "fds": read_fds(self.proc.pid) if alive else "",
                        "scriptlog_b": self.script_size(), **r})
                    if rel in probe_shows and not self.stop_evt.is_set() \
                            and self.proc.poll() is None:
                        try:
                            snaps.append(self.authoring_snapshot(self.cycle, rel))
                        except Exception as e:                    # noqa: BLE001
                            self.log("authoring snapshot after %s raised %r\n%s"
                                     % (rel, e, traceback.format_exc()))
                    self.write_state()
                self.show = "(interlude)"
                complete = (played == len(self.shows))
                il = {}
                # The interlude runs at the natural end of a cycle and at T+H
                # (so the last, partial cycle still has a boundary reading);
                # it is SKIPPED when the run was stopped or a terminal flag
                # fired - at that point the fastest correct thing is to
                # preserve the evidence, not to add a dump to it.
                if not self.terminal.is_set() and not self.stop_evt.is_set() \
                        and self.proc.poll() is None:
                    il = self.interlude(self.cycle)
                st = read_status(self.proc.pid) if self.proc.poll() is None else {}
                au = (il.get("authoring") or {})
                row = {"cycle": self.cycle, "complete": int(complete),
                       "t_start_iso": c_start_iso, "t_end_iso": now_iso(),
                       "wall_s": round(time.time() - c0, 1),
                       "shows_played": played, "shows_timeout": timeouts,
                       "shows_capped": capped, "pauses_resumed": resumed,
                       "bodies_old": au.get("bodies_old", ""),
                       "bodies_new": au.get("bodies_new", ""),
                       "bodies_both": au.get("bodies_both", ""),
                       "old_only": au.get("old_only", ""),
                       "new_only": au.get("new_only", ""),
                       "vmrss_kb": st.get("vmrss_kb", ""),
                       "vmsize_kb": st.get("vmsize_kb", ""),
                       "threads": st.get("threads", ""),
                       "fds": read_fds(self.proc.pid)
                              if self.proc.poll() is None else "",
                       "gpu_mib": read_gpu_mib(self.proc.pid)
                                  if self.proc.poll() is None else "",
                       "applog_b": self.applog.stat().st_size
                                   if self.applog.exists() else "",
                       "scriptlog_b": self.script_size(),
                       "stall_detected": self.stall["detected"],
                       "stall_very_long": self.stall["very_long"],
                       "dump_file": str(il.get("dump", "")),
                       "dump_bytes": il.get("dump_bytes", ""),
                       "dump_diff_vs_cycle2": "",
                       "jd_after": (il.get("get_position") or "")}
                cycles.append({**row, "interlude": il,
                               "authoring_snapshots": snaps})
                self.cycles_w.writerow(row)
                self.log("cycle %d done: %s shows, %d timeout, %d capped, %d "
                         "pause(s) resumed, %.1f s wall, VmRSS %s kB, bodies "
                         "old/new %s/%s"
                         % (self.cycle, played, timeouts, capped, resumed,
                            row["wall_s"], row["vmrss_kb"],
                            row["bodies_old"], row["bodies_new"]))
                self.write_state()
        except Exception as e:                                    # noqa: BLE001
            # WITH THE TRACEBACK.  Leg 1 recorded only the repr and the origin
            # then had to be reasoned out rather than read; an exception that
            # takes hours to reproduce must say where it came from the first
            # time it happens.
            tb = traceback.format_exc()
            self.log("driver loop raised %r\n%s" % (e, tb))
            self.flag_detail.append({"flag": "DRIVER-EXCEPTION",
                                     "at_iso": now_iso(), "why": repr(e),
                                     "traceback": tb})

        q = {}
        if self.proc.poll() is None:
            q = self.quit_app()
        else:
            q = {"exit_code": self.proc.returncode, "wall_to_exit_s": None,
                 "note": "the process was already gone before the quit"}
        self.finish({"cycles": cycles, "quit": q, "tcp_wait_s": tcp_wait})

    def finish(self, extra):
        self.stop_evt.set()
        time.sleep(1.5)
        moved = self.check_frozen()
        if moved and "F4" not in self.flags:
            self.raise_flag("F4", "a frozen real-HOME file moved: %s"
                            % [m["file"] for m in moved], moved=moved)
        # OVER THE RUN'S OWN FROZEN SET, never a recomputed default one: the
        # F95 code called `frozen_md5s()` with no argument, which would have
        # written the eleven-file default set out of an F98 run and made the
        # in/out comparison compare two different sets.
        out_md5 = {}
        for f in self.frozen_in:
            try:
                out_md5[f] = md5(f)
            except OSError:
                out_md5[f] = "MISSING"
        atomic_write_json(self.out / "frozen_out.json", out_md5)
        farm_out = farm_sts_md5s(self.farm, self.dirs)
        atomic_write_json(self.out / "farm_sts_out.json", farm_out)
        farm_in = dict(self.cfg.get("farm_sts_in") or {})
        farm_moved = [k for k, v in farm_in.items() if farm_out.get(k) != v]

        # the dump diff, cycle 2 as the reference (cycle 1 is first-touch)
        diffs = {}
        try:
            files = sorted(self.out.glob("cycle_*.json"))
            good = [f for f in files if f.stat().st_size > 0]
            if len(good) >= 3:
                ref = dump_constant_view(good[1])
                for f in good[2:]:
                    diffs[f.name] = dump_diff(ref, dump_constant_view(f))
        except Exception as e:                                    # noqa: BLE001
            diffs["error"] = repr(e)

        series = []
        try:
            with open(self.out / "cycles.csv") as fh:
                for r in csv.DictReader(fh):
                    if r["complete"] == "1" and r["vmrss_kb"]:
                        series.append(int(r["vmrss_kb"]))
        except OSError:
            pass
        leak = leak_verdict(series)

        self.phase = "done"
        verdict = {
            "out": str(self.out), "binary": self.binary,
            "binary_md5": md5(self.binary),
            "hours_requested": self.hours,
            "wall_s": round(time.time() - self.t0, 1) if self.t0 else 0,
            "samples": self.i, "cycles": self.cycle,
            "sample_s": self.sample_s, "J0": J0,
            "flags": self.flags, "flag_detail": self.flag_detail,
            "stall": dict(self.stall),
            "leak": leak, "leak_series_kb": series,
            "dump_diff_vs_cycle2": diffs,
            "frozen_moved": moved,
            "frozen_count": len(self.frozen_in),
            "frozen_digest_in": frozen_digest(self.frozen_in),
            "frozen_digest_out": frozen_digest(out_md5),
            "farm_sts_moved": farm_moved,
            "playlist_dirs": self.dirs, "cap_s": self.cap,
            "shows": self.shows,
            **extra,
        }
        atomic_write_json(self.out / "verdict.json", verdict)
        self.write_state()
        self.log("DONE  flags=%s  cycles=%d  samples=%d  leak=%s"
                 % (self.flags or ["none"], self.cycle, self.i,
                    leak["verdict"]))


# ==================================================================== verbs

# BOTH WORD ORDERS.  The command surface takes its arguments as key/value
# pairs in any order, and the corpus uses both: `06.sts` writes 1013 lines of
# `body name "<X>" ... action load` and `14.sts` writes 528 of
# `body action load ... name <X>`.  A census that greps only `body action load`
# - the one the F98 dispatch and the ledger's "3000" both used - misses the
# larger of the two shows entirely (Sec.11.218).
LOAD_RE = re.compile(
    r"^body\s+(?=.*\baction\s+load\b)(?=.*?\bname\s+(\"[^\"]+\"|\S+))")


def authoring_shows(dirs, home=REAL_HOME):
    """-> [(rel, uncommented `body action load` count, [first name, first
    Star_ name]), ...] over the playlist, sorted by count.

    A `#`-commented `body action load` is NOT an authored body: `14.sts` opens
    with one (`#body action load mode in_galaxy name Solsys ...`), which is
    also why the line COUNT of that token (529) is not the number of bodies
    the show authors (528)."""
    out = []
    for d in dirs:
        for p in sorted((Path(home) / "scripts" / d).glob("*.sts")):
            n, first, first_star = 0, None, None
            for raw in p.read_text(encoding="latin-1").splitlines():
                s = raw.strip()
                if s.startswith("#"):
                    continue
                m = LOAD_RE.match(s)
                if not m:
                    continue
                n += 1
                name = m.group(1).strip('"')
                if first is None:
                    first = name
                if first_star is None and re.search(r"\bfilename\s+Star_", s):
                    first_star = name
            if n:
                out.append(("%s/%s" % (d, p.name), n, first, first_star))
    return sorted(out, key=lambda r: -r[1])


def probe_bodies(dirs, home=REAL_HOME, limit=2, played=None):
    """The authoring probes, DERIVED from the corpus and never recalled.

    The two shows that author the most bodies, each contributing the FIRST
    body it loads, plus that show's first STAR load when the show has one and
    it is a different body (`14.sts` opens on a hidden `Sphere` and its first
    star is the next line, so both are probed - the section asks for
    `06old.sts`'s first satellite and `14.sts`'s first star)."""
    # ONLY from shows this run actually PLAYS: a probe for a body no played
    # show authors would report "absent from both halves" every cycle and read
    # as a finding about the application instead of about the playlist.
    cand = [r for r in authoring_shows(dirs, home)
            if played is None or r[0] in set(played)]
    probes, seen = [], set()
    for rel, n, first, first_star in cand[:limit]:
        for name, kind in ((first, "first authored body"),
                           (first_star, "first Star_ body")):
            if name and name not in seen:
                seen.add(name)
                probes.append({"name": name, "from": rel, "kind": kind,
                               "show_loads": n})
    return probes


def verb_start(a):
    out = Path(a.out).resolve()
    root = Path(a.root).resolve()
    if root not in out.parents and out != root:
        raise SystemExit("REFUSED: the outdir must live under %s (the task's "
                         "farm root, --root); got %s" % (root, out))
    if (out / "state.json").exists():
        raise SystemExit("REFUSED: %s already carries a run (state.json). Use a "
                         "fresh outdir - a soak's evidence is never overwritten."
                         % out)
    out.mkdir(parents=True, exist_ok=True)

    hits = no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running (pids %s) - "
                         "Sec.11.121(m)'s concurrent-instance assert" % hits)

    if not a.skip_canary:
        r = subprocess.run([str(HERE / "f56_canary.sh"), "--no-scene"],
                           capture_output=True, text=True, cwd=str(HERE))
        (out / "canary.txt").write_text(r.stdout + r.stderr)
        if r.returncode != 0:
            raise SystemExit("REFUSED: the environment canary exited %d - STOP "
                             "and report, never mitigate (Sec.11.174(h)). See %s"
                             % (r.returncode, out / "canary.txt"))
        print("canary --no-scene exit 0 -> %s" % (out / "canary.txt"))

    dirs = list(a.playlist_dir) if a.playlist_dir else list(PLAYLIST_DIRS)
    skip = list(a.skip_show or [])
    shows = playlist_shows(dirs=dirs, skip=skip)
    if not shows:
        raise SystemExit("REFUSED: the playlist is empty")
    frozen = frozen_md5s(dirs=dirs)
    farm = out / "farm"
    asserts = build_farm(farm, shows)
    farm_in = farm_sts_md5s(farm, dirs)
    probes = probe_bodies(dirs, played=shows)
    cfg = {"out": str(out), "farm": str(farm), "bin": str(Path(a.bin).resolve()),
           "bin_md5": md5(a.bin), "hours": a.hours, "sample": a.sample,
           "shows": shows, "frozen_in": frozen, "farm_asserts": asserts,
           "playlist_dirs": dirs, "root": str(root), "cap": a.cap,
           "skipped_shows": skip, "skip_reason": a.skip_reason,
           "grace_s": SHOW_GRACE_S, "authored_probes": probes,
           "farm_sts_in": farm_in,
           "frozen_digest_in": frozen_digest(frozen),
           "J0": J0, "display": os.environ.get("DISPLAY"),
           "started_iso": now_iso(), "criteria_sha": hashlib.md5(
               CRITERIA.encode()).hexdigest()[:8]}
    atomic_write_json(out / "config.json", cfg)
    atomic_write_json(out / "frozen_in.json", frozen)
    atomic_write_json(out / "farm_sts_in.json", farm_in)

    driverlog = open(out / "driver.log", "a")
    p = subprocess.Popen(
        ["setsid", "nohup", sys.executable, str(HERE / "f95_soak.py"),
         "--driver", str(out)],
        stdout=driverlog, stderr=subprocess.STDOUT,
        stdin=subprocess.DEVNULL, start_new_session=True,
        cwd=str(HERE), env=dict(os.environ))
    print("farm     : %s (%d asserts, config.ini md5 %s)"
          % (farm, len(asserts), asserts["config.ini.md5"]))
    print("playlist : %d show(s) in %s%s"
          % (len(shows), "/".join(dirs),
             (": " + ", ".join(shows)) if len(shows) <= 12 else
             (": %s ... %s" % (shows[0], shows[-1]))))
    print("cap      : %s   grace %.0f s" % (("%.0f s" % a.cap) if a.cap
                                            else "none (F95's behaviour)",
                                            SHOW_GRACE_S))
    print("probes   : %s" % ", ".join("%s (%s, %s)"
                                      % (p["name"], p["from"], p["kind"])
                                      for p in probes) or "none")
    print("frozen   : %d files recorded in, digest %s"
          % (len(frozen), frozen_digest(frozen)[:8]))
    print("farm .sts: %d copies recorded in" % len(farm_in))
    print("driver   : launched detached (setsid), wrapper pid %d, log %s"
          % (p.pid, out / "driver.log"))
    print("poll with: f95_soak.py wait %s <sec<=540>" % out)
    return 0


def _read_state(out):
    try:
        return json.loads((Path(out) / "state.json").read_text())
    except Exception:                                             # noqa: BLE001
        return {}


def _read_samples(out):
    p = Path(out) / "samples.csv"
    if not p.exists():
        return []
    try:
        with open(p) as fh:
            return list(csv.DictReader(fh))
    except OSError:
        return []


def _fmt_sample(r):
    return ("#%-5s %s +%-8ss cyc %-3s %-28s RSS %8s kB  fds %-4s gpu %-5s "
            "stall %s/%s  rtt %-8s ms  jd %-12s scr %-5s lock %-4s la %-5s %s"
            % (r.get("i"), r.get("wall_iso"), r.get("t_rel_s"), r.get("cycle"),
               (r.get("show") or "")[:28], r.get("vmrss_kb"), r.get("fds"),
               r.get("gpu_mib"), r.get("stall_detected"), r.get("stall_very_long"),
               r.get("probe_rtt_ms"), r.get("sim_jd"), r.get("screensaver"),
               r.get("locked_hint"), r.get("loadavg1"),
               ("FLAGS=" + r["flags"]) if r.get("flags") else ""))


def verb_status(a):
    st = _read_state(a.out)
    if not st:
        print("no state.json under %s - the driver has not written yet" % a.out)
        return 1
    rows = _read_samples(a.out)
    print("phase      : %s%s" % (st.get("phase"),
                                 "  (FINISHED)" if st.get("finished") else ""))
    print("app pid    : %s   driver pid %s" % (st.get("pid"), st.get("driver_pid")))
    print("started    : %s   now %s" % (st.get("t0_iso"), st.get("now_iso")))
    print("elapsed    : %.1f s of %.1f s target"
          % (st.get("elapsed_s") or 0, st.get("target_s") or 0))
    print("cycle      : %s   show %s" % (st.get("cycle"), st.get("show")))
    print("samples    : %s (csv rows %d)" % (st.get("samples"), len(rows)))
    print("stalls     : %s" % st.get("stall"))
    print("FLAGS      : %s" % (st.get("flags") or "none"))
    for d in st.get("flag_detail") or []:
        line = "   %s @%s sample %s: %s" % (d.get("flag"), d.get("at_iso"),
                                            d.get("sample"), d.get("why", ""))
        print(line[:400])
    if rows:
        print("last       : %s" % _fmt_sample(rows[-1]))
    v = Path(a.out) / "verdict.json"
    if v.exists():
        vd = json.loads(v.read_text())
        print("VERDICT    : flags %s | cycles %s | samples %s | leak %s"
              % (vd.get("flags") or "none", vd.get("cycles"), vd.get("samples"),
                 (vd.get("leak") or {}).get("verdict")))
        print("leak series: %s" % (vd.get("leak_series_kb") or []))
        nd = {k: len(v) for k, v in (vd.get("dump_diff_vs_cycle2") or {}).items()
              if isinstance(v, list)}
        print("dump diffs : %s" % (nd or "n/a"))
        print("quit       : %s" % json.dumps(
            {k: v2 for k, v2 in (vd.get("quit") or {}).items()
             if k not in ("teardown_tail",)})[:600])
    return 0


def verb_wait(a):
    sec = min(float(a.sec), 540.0)
    if float(a.sec) > 540.0:
        print("NOTE: capped to 540 s (an executor call stays under nine minutes)")
    end = time.time() + sec
    seen = len(_read_samples(a.out))
    print("waiting up to %.0f s from %s (already %d sample rows)"
          % (sec, now_iso(), seen))
    flagged = set(_read_state(a.out).get("flags") or [])
    while time.time() < end:
        rows = _read_samples(a.out)
        for r in rows[seen:]:
            print(_fmt_sample(r))
        seen = len(rows)
        st = _read_state(a.out)
        fl = set(st.get("flags") or [])
        new = fl - flagged
        if new:
            for d in st.get("flag_detail") or []:
                if d.get("flag") in new:
                    print("*** FLAG %s @%s sample %s: %s"
                          % (d["flag"], d.get("at_iso"), d.get("sample"),
                             d.get("why")))
            print("returning early on new FAIL flag(s): %s" % sorted(new))
            return 0
        if st.get("finished"):
            print("the driver has FINISHED (phase %s)" % st.get("phase"))
            return 0
        time.sleep(3)
    st = _read_state(a.out)
    print("still running: phase %s, elapsed %.1f s, cycle %s, samples %s, "
          "flags %s" % (st.get("phase"), st.get("elapsed_s") or 0,
                        st.get("cycle"), st.get("samples"),
                        st.get("flags") or "none"))
    return 0


def verb_stop(a):
    p = Path(a.out) / "stop.request"
    p.write_text(now_iso() + "\n")
    print("stop requested at %s -> %s" % (now_iso(), p))
    return 0


def verb_selftest(a):
    """Control (iii): the LEAK rule fed a monotone and a non-monotone series."""
    print("=" * 74)
    print("F95 SELFTEST - the verdict functions, each shown BOTH ways")
    print("=" * 74)
    ok = True

    mono = [100, 110, 120, 130, 140, 150]
    r = leak_verdict(mono)
    print("\n(1) LEAK rule, MONOTONE series %s" % mono)
    print("    -> %s  (%s)" % (r["verdict"], r["why"]))
    print("    steps %s  per_cycle %.1f  floor %s" % (r["steps"], r["per_cycle"],
                                                      r["floor"]))
    ok &= (r["verdict"] == "LEAK")

    non = [100, 130, 128, 141, 139, 150]
    r2 = leak_verdict(non)
    print("\n(2) LEAK rule, NON-MONOTONE series %s" % non)
    print("    -> %s  (%s)" % (r2["verdict"], r2["why"]))
    print("    steps %s  span %s  floor %s" % (r2["steps"], r2["span"], r2["floor"]))
    ok &= (r2["verdict"] == "NO LEAK BY THIS RULE")

    # the one that matters most: a series that RISES overall but is not
    # monotone must NOT read as a leak, and its floor must be stated.
    rise = [100, 200, 150, 260]
    r3 = leak_verdict(rise)
    print("\n(3) rises overall, not monotone %s" % rise)
    print("    -> %s ; span %s against floor %s" % (r3["verdict"], r3["span"],
                                                    r3["floor"]))
    ok &= (r3["verdict"] == "NO LEAK BY THIS RULE")

    short = [100, 110, 120]
    r4 = leak_verdict(short)
    print("\n(4) too few boundaries %s -> %s" % (short, r4["verdict"]))
    ok &= (r4["verdict"] == "INCONCLUSIVE")

    print("\n(5) TEARDOWN-FAULT matcher, both ways")
    pos = ["terminate called without an active exception",
           "Segmentation fault (core dumped)",
           "free(): invalid pointer",
           "*** stack smashing detected ***"]
    neg = ["(Info ): Vulkan instance created",
           "big moon.jpg is ready",
           "ScriptMgr: script end"]
    for s in pos:
        hit = bool(TEARDOWN_FAULT.search(s))
        print("    MATCH %-5s %s" % (hit, s[:60]))
        ok &= hit
    for s in neg:
        hit = bool(TEARDOWN_FAULT.search(s))
        print("    MATCH %-5s %s" % (hit, s[:60]))
        ok &= not hit

    print("\n(5b) F2's SECOND arm - GAINS, not a cumulative count.  The HANG "
          "control\n     showed the cumulative form latching onto every later "
          "sample.")
    cases = [((0, 1, "false", "no"), True, "0 -> 1 unlocked: fires"),
             ((1, 1, "false", "no"), False, "1 -> 1 unlocked: quiet again"),
             ((1, 2, "false", "no"), True, "1 -> 2 unlocked: fires again"),
             ((0, 1, "true", "no"), False, "0 -> 1 but the screensaver is ON"),
             ((0, 1, "false", "yes"), False, "0 -> 1 but the session is LOCKED"),
             ((2, 1, "false", "no"), False, "a counter that went DOWN")]
    for args, want, label in cases:
        got = very_long_arm(*args)
        print("    %-5s (want %-5s) %s" % (got, want, label))
        ok &= (got == want)

    print("\n(6) `get status position` reply matcher, both ways")
    good = "  43.60; 1.44;    150.00; 2460000.500000;   0.000000;"
    bad = "Mars(P);"
    print("    good -> %r" % want_position(good))
    print("    bad  -> %r" % want_position(bad))
    ok &= (want_position(good) is not None and want_position(bad) is None)

    print("\n(7) DUMP-DIFF over the expected-constant set, both ways")
    a1 = {"header": {"jd": 2460000.5, "timeSpeed": 0, "timePaused": False},
          "bodies": {"Mars": {"old.ecl": [1.0, 2.0, 3.0], "new.parent": "Sun"},
                     "Moon": {"old.ecl": [0.1, 0.2, 0.3]}}}
    a2 = json.loads(json.dumps(a1))
    print("    identical -> %d diff(s)" % len(dump_diff(a1, a2)))
    ok &= (len(dump_diff(a1, a2)) == 0)
    a3 = json.loads(json.dumps(a1))
    a3["bodies"]["Mars"]["old.ecl"][2] = 3.0000001
    del a3["bodies"]["Moon"]
    a3["header"]["jd"] = 2460000.5001
    d = dump_diff(a1, a3)
    print("    perturbed -> %d diff(s): %s"
          % (len(d), [x["where"] for x in d]))
    ok &= (len(d) == 3)

    print("\n(8) THE LOOP-AWARE DURATION MODEL, both ways, on written files")
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        cases = [
            ("no loop", "wait duration 2\nwait duration 3\n"
                        "script action pause\n", 5.0, 5.0, 1, 1),
            ("one loop x10", "struct loop 10\nwait duration 2\n"
                             "script action pause\nstruct loop end\n",
             2.0, 20.0, 1, 10),
            ("nested 10x3", "struct loop 10\nstruct loop 3\n"
                            "wait duration 1\nstruct loop end\n"
                            "struct loop end\n", 1.0, 30.0, 0, 0),
            ("wait AFTER the loop is not multiplied",
             "struct loop 10\nwait duration 1\nstruct loop end\n"
             "wait duration 7\n", 8.0, 17.0, 0, 0),
            ("comments and blanks ignored",
             "# struct loop 99\n\nwait duration 4\n", 4.0, 4.0, 0, 0),
        ]
        for label, txt, w_own, w_exp, w_p, w_pe in cases:
            f = Path(td) / "c.sts"
            f.write_text(txt)
            m = show_model(f)
            good = (m["own"] == w_own and m["expanded"] == w_exp
                    and m["pauses"] == w_p and m["pauses_expanded"] == w_pe)
            print("    %-42s own %7.2f (want %7.2f)  expanded %8.2f (want "
                  "%8.2f)  %s" % (label, m["own"], w_own, m["expanded"],
                                  w_exp, "ok" if good else "MISMATCH"))
            ok &= good
        # the real corpus file the whole fix is about, read from the real HOME
        real = REAL_HOME / "scripts" / "fscripts" / "panorama0.sts"
        if real.is_file():
            m = show_model(real)
            print("    %-42s own %7.2f  expanded %8.2f  loops %s breaks %d"
                  % ("fscripts/panorama0.sts (the infinite one)", m["own"],
                     m["expanded"], m["loops"], m["breaks"]))
            ok &= (m["expanded"] > 1000 * m["own"])

    print("\n(9) CAPPED vs SHOW-TIMEOUT - the two labels a show can earn, and")
    print("    the decision shown BOTH ways at one cap")

    class _FakeCap:
        cap = 60.0
        show_budget = Driver.show_budget
    with tempfile.TemporaryDirectory() as td:
        d = _FakeCap()
        rows = [("short show, ends by itself", "wait duration 5\n",
                 False, 65.0),
                ("long by LOOP -> capped", "struct loop 1000\n"
                 "wait duration 1\nstruct loop end\n", True, 60.0),
                ("long by AUTHORED waits -> capped",
                 "wait duration 100\n", True, 60.0),
                ("exactly at the cap -> NOT capped",
                 "wait duration 60\n", False, 120.0)]
        for label, txt, want_cap, want_deadline in rows:
            f = Path(td) / "c.sts"
            f.write_text(txt)
            own, exp, planned, deadline, will_cap, _m = d.show_budget(f)
            good = (will_cap == want_cap
                    and abs(deadline - want_deadline) < 1e-9)
            print("    %-38s expanded %8.2f  will_cap %-5s (want %-5s)  "
                  "deadline %6.1f (want %6.1f) %s"
                  % (label, exp, will_cap, want_cap, deadline, want_deadline,
                     "ok" if good else "MISMATCH"))
            ok &= good
        d2 = _FakeCap()
        d2.cap = None
        f = Path(td) / "c.sts"
        f.write_text("struct loop 1000\nwait duration 1\nstruct loop end\n")
        own, exp, planned, deadline, will_cap, _m = d2.show_budget(f)
        good = (will_cap is False and deadline == 1000.0 + SHOW_GRACE_S)
        print("    %-38s cap=None -> will_cap %s, deadline %.1f = F95's "
              "modelled+grace %s" % ("no cap at all (F95's default)", will_cap,
                                     deadline, "ok" if good else "MISMATCH"))
        ok &= good

    print("\n(9b) THE FOUR OUTCOMES OF A SHOW-PLAY, each shown to be reachable")
    ocases = [((True, 5.0, 65.0, False), "ended", "ended by itself"),
              ((True, 61.0, 60.0, True), "ended",
               "ended at the very cap - still `ended`"),
              ((False, 60.0, 60.0, True), "CAPPED", "deadline, model said long"),
              ((False, 65.0, 65.0, False), "SHOW-TIMEOUT",
               "deadline, model did NOT say long -> a FINDING"),
              ((False, 47.2, 60.0, True), "INTERRUPTED",
               "the DEATH control's own case: killed at 47 s of a 60 s "
               "deadline"),
              ((False, 3.0, 65.0, False), "INTERRUPTED", "T+H arrived under it")]
    for args, want, label in ocases:
        got = show_outcome(*args)
        print("    %-12s (want %-12s) %s" % (got, want, label))
        ok &= (got == want)

    print("\n(10) THE AGGREGATE FROZEN DIGEST is per-file and order-free")
    m1 = {"/a": "11111111111111111111111111111111",
          "/b": "22222222222222222222222222222222"}
    m2 = {"/b": "22222222222222222222222222222222",
          "/a": "11111111111111111111111111111111"}
    m3 = {"/a": "11111111111111111111111111111111",
          "/b": "33333333333333333333333333333333"}
    print("    same files, other order -> %s" % (frozen_digest(m1)[:8]
                                                 == frozen_digest(m2)[:8]))
    print("    one file changed        -> %s" % (frozen_digest(m1)[:8]
                                                 != frozen_digest(m3)[:8]))
    ok &= (frozen_digest(m1) == frozen_digest(m2)
           and frozen_digest(m1) != frozen_digest(m3))

    print("\n" + "=" * 74)
    print("SELFTEST: %s" % ("PASS" if ok else "FAIL"))
    print("=" * 74)
    return 0 if ok else 1


def predict_cycle_wall(shows, cap, home=REAL_HOME, per_show_overhead=1.8,
                       interlude_s=35.0):
    """The predicted wall time of ONE complete cycle, from the model alone.

    Committed BEFORE the launch, with its terms visible, because a prediction
    whose terms are not separable cannot be wrong in an informative way:

      play   sum over shows of min(expanded, cap)  - what the model says the
             shows themselves take.  It UNDERSTATES every show with no
             authored `wait` (89 of the 136 `fscripts/` shows model as 0.0 s
             and still take real time to load their media).
      over   a fixed per-show cost: one `script action play` send with its
             1.0 s drain, the 0.5 s poll granularity, and for an unfinished
             show the `script action end` + 1.0 s settle.
      inter  the cycle-boundary interlude: select, two `get`s, the pinned-clock
             dual dump and the authoring probes.
      K      THE UNKNOWN: every show that neither ends nor is capped costs its
             grace on top.  The model cannot see it, so it is reported as a
             per-timeout term instead of being folded into a single number."""
    play = 0.0
    capped = []
    for rel in shows:
        m = show_model(Path(home) / "scripts" / rel)
        e = m["expanded"]
        if cap is not None and e > cap:
            capped.append(rel)
            e = cap
        play += e
    over = per_show_overhead * len(shows)
    base = play + over + interlude_s
    return {"play_s": play, "overhead_s": over, "interlude_s": interlude_s,
            "base_s": base, "capped": capped,
            "per_timeout_s": SHOW_GRACE_S, "shows": len(shows)}


def verb_plan(a):
    dirs = list(a.playlist_dir) if a.playlist_dir else list(PLAYLIST_DIRS)
    cap = a.cap
    skip = list(a.skip_show or [])
    print(CRITERIA)
    print("=" * 78)
    print("THE RUN'S PARAMETERS (F98 generalisation; F95's defaults are the")
    print("no-argument case, and the criteria above are the same string in the")
    print("same place for both - `criteria_sha` proves it).")
    print("=" * 78)
    print("  playlist directories : %s" % ", ".join(dirs))
    print("  never played         : %s (the app plays it at launch itself)"
          % ", ".join(sorted(NEVER_PLAYED)))
    print("  not played this run  : %s" % (", ".join(excluded_dirs(dirs)) or "-"))
    print("  SHOWS SKIPPED        : %s%s"
          % (", ".join(skip) or "-",
             ("  <- %s" % a.skip_reason) if a.skip_reason else ""))
    print("  per-show CAP         : %s" % (("%.0f s" % cap) if cap
                                           else "none (F95's behaviour)"))
    print("  show grace           : %.0f s (SHOW-TIMEOUT = modelled + grace)"
          % SHOW_GRACE_S)
    print("  farm root default    : %s" % FARM_ROOT)
    print("""
  THREE SENTENCES IN THE CRITERIA ABOVE DESCRIBE F95's PLAYLIST AND NOT
  NECESSARILY THIS ONE - re-stated here rather than edited there, because that
  string is the committed criterion of a delivered campaign and its md5 is
  recorded in F95's own `config.json`:
    - "the eight shows are image overlays and do not move the camera": the
      `fscripts/` corpus DOES move the camera, authors bodies and plays media.
      Nothing in the instrument depended on that sentence and nothing asserts
      it; the expected-constant set is observer-INdependent by construction.
    - "it exercises the media path and the script engine but not body
      authoring": on `--playlist-dir fscripts` it exercises body authoring -
      706 uncommented `body action load` lines in 7 shows - which is the whole
      reason this second run exists (Sec.11.215(m)).
    - the handed-in stall baseline ("2 per ~92 s launch") was a two-run sample
      (Sec.11.215(a)); the rate is recorded here too and gates nothing.""")

    shows = playlist_shows(dirs=dirs, skip=skip)
    auth = {rel: (n, first, star)
            for rel, n, first, star in authoring_shows(dirs)}
    print("\nTHE DURATION MODEL, EVERY SHOW (`sts_duration.parse`, ONE home):")
    print("  %-30s %9s %11s %7s %7s %6s %8s %s"
          % ("show", "own s", "expand s", "pauses", "pausesX", "lines",
             "loads", "verdict"))
    tot_own = tot_exp = 0.0
    ncap = 0
    for s in shows:
        m = show_model(REAL_HOME / "scripts" / s)
        tot_own += m["own"]
        tot_exp += m["expanded"]
        will_cap = cap is not None and m["expanded"] > cap
        ncap += 1 if will_cap else 0
        note = "CAPPED at %.0f s" % cap if will_cap else ""
        if m["breaks"]:
            note += (" " if note else "") + "[%d break: upper bound]" % m["breaks"]
        print("  %-30s %9.2f %11.2f %7d %7d %6d %8s %s"
              % (s, m["own"], m["expanded"], m["pauses"],
                 m["pauses_expanded"], m["lines"],
                 auth.get(s, (0,))[0] or "", note))
    print("  %-30s %9.2f %11.2f   (%d shows, %d capped)"
          % ("TOTAL", tot_own, tot_exp, len(shows), ncap))

    p = predict_cycle_wall(shows, cap)
    print("\nPREDICTED WALL TIME OF ONE COMPLETE CYCLE (committed before the "
          "launch):")
    print("  play %.1f s + per-show overhead %.1f s (%d x %.1f) + interlude "
          "%.1f s" % (p["play_s"], p["overhead_s"], p["shows"], 1.8,
                      p["interlude_s"]))
    print("  = BASE %.1f s = %.1f min, PLUS %.0f s for every show that neither"
          % (p["base_s"], p["base_s"] / 60.0, p["per_timeout_s"]))
    print("    ends nor is capped (K, the model's blind term)")
    print("  => K=0: %.1f min | K=5: %.1f min | K=10: %.1f min | K=20: %.1f min"
          % tuple((p["base_s"] + k * p["per_timeout_s"]) / 60.0
                  for k in (0, 5, 10, 20)))

    if auth:
        print("\nBODY AUTHORING IN THIS PLAYLIST (uncommented `body action "
              "load`):")
        for rel, (n, first, star) in sorted(auth.items(), key=lambda kv: -kv[1][0]):
            print("  %-30s %4d load(s)  first %-18s first Star_ %s"
                  % (rel, n, first, star or "-"))
        print("  probes armed at every cycle boundary: %s"
              % ", ".join("%s (%s)" % (x["name"], x["from"])
                          for x in probe_bodies(dirs, played=shows)))

    fs = frozen_set(dirs=dirs)
    md5s = {str(f): md5(f) for f in fs}
    print("\nFROZEN REAL-HOME FILES asserted md5 in == out (%d), aggregate "
          "digest %s" % (len(fs), frozen_digest(md5s)[:8]))
    if len(fs) <= 16:
        for f in fs:
            print("    %s  %s" % (md5s[str(f)][:8], f))
    else:
        sts = {k: v for k, v in md5s.items() if k.endswith(".sts")}
        print("    %s  %s" % (md5s[str(fs[0])][:8], fs[0]))
        print("    %s  %s" % (md5s[str(fs[1])][:8], fs[1]))
        print("    ... and %d .sts files, their own aggregate digest %s"
              % (len(sts), frozen_digest(sts)[:8]))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--driver", default=None, help=argparse.SUPPRESS)
    sub = ap.add_subparsers(dest="verb")

    def playlist_args(p):
        p.add_argument("--playlist-dir", action="append", default=None,
                       metavar="D",
                       help="a script directory to play, repeatable "
                            "(default: %s)" % " ".join(PLAYLIST_DIRS))
        p.add_argument("--cap", type=float, default=None, metavar="S",
                       help="per-show cap in seconds: a show whose MODELLED "
                            "duration exceeds it is ended at the cap and "
                            "logged CAPPED (default: none, F95's behaviour)")
        p.add_argument("--skip-show", action="append", default=None,
                       metavar="REL",
                       help="leave a show out of the playlist, repeatable. "
                            "For a show measured to take the application "
                            "down; always with --skip-reason")
        p.add_argument("--skip-reason", default="", metavar="TEXT",
                       help="why a show is skipped - stored in config.json "
                            "and printed by the report")

    p = sub.add_parser("start")
    p.add_argument("out")
    p.add_argument("--hours", type=float, default=3.0)
    p.add_argument("--sample", type=float, default=30.0)
    p.add_argument("--bin", default=str(DEFAULT_BIN))
    p.add_argument("--root", default=str(FARM_ROOT), metavar="ABS",
                   help="the farm root the outdir must live under "
                        "(default: %s)" % FARM_ROOT)
    playlist_args(p)
    p.add_argument("--skip-canary", action="store_true",
                   help="ONLY for a control run that follows a green canary in "
                        "the same minute; the soak never uses it")
    p.set_defaults(fn=verb_start)

    p = sub.add_parser("status")
    p.add_argument("out")
    p.set_defaults(fn=verb_status)

    p = sub.add_parser("wait")
    p.add_argument("out")
    p.add_argument("sec")
    p.set_defaults(fn=verb_wait)

    p = sub.add_parser("stop")
    p.add_argument("out")
    p.set_defaults(fn=verb_stop)

    p = sub.add_parser("selftest")
    p.set_defaults(fn=verb_selftest)

    p = sub.add_parser("plan")
    playlist_args(p)
    p.set_defaults(fn=verb_plan)

    a = ap.parse_args()
    if a.driver:
        d = Driver(a.driver)
        # the stop file is polled by the driver loop through this thread
        def watch():
            f = Path(a.driver) / "stop.request"
            while not d.stop_evt.is_set():
                if f.exists():
                    d.log("stop.request seen - ending now")
                    d.stop_evt.set()
                    return
                time.sleep(2)
        threading.Thread(target=watch, daemon=True).start()
        d.run()
        return 0
    if not a.verb:
        ap.print_help()
        return 2
    return a.fn(a)


if __name__ == "__main__":
    sys.exit(main())
