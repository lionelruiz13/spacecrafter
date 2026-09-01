#!/usr/bin/env python3
"""f77_badscript.py - what a shipped show with one typo does today, on EVERY channel.

    cd claude/harness && DISPLAY=:0 ./f77_badscript.py <outdir> [--bin <binary>]

INTENT 5.116, 5.117 and 5.118 each close with the same owed datum -- *"one
launch with a deliberately bad script"* -- and until it is paid every
consequence claim in them is READ at `d6aec251`, not driven.  This is that
launch.  ONE fresh temp-HOME session, one bad script played as a FILE (which is
the half F76 could not reach: it drove TCP, and 11.187(d)'s asymmetry means a
file-origin refusal is not the same record), and five channels watched at once:

    LOG      the script log, i.e. 5.115's uncapped keepHistory file
    WIRE     the $DIAGON dedicated feedback link (F69)
    TAIL     the `#!` the annotator writes INTO the played file (F63/11.184)
    STATE    what the line left behind
    CONSOLE  stdout vs stderr, split, because print_log = true is shipped

and one thing that is not a channel but the claim itself, SUCCESS, read through
the engine's OWN recorder: `executeCommandStatus` calls `recordCommand` only on
the success branch (app_command_interface.cpp:1310-1312), so a recording taken
while the bad script plays IS the list of lines the engine reported success for.

THE PREDICTIONS ARE IN `artifacts/f77/predictions.md`, COMMITTED BEFORE THIS
FILE EXISTED (harness 3f0dc5d) and asserted unchanged at delivery.  This script
states each one again as a relation between measurements so that it can fail;
a disagreement is the finding, not a defect of the run.

Exit 0 = every prediction held; 1 = at least one did not (printed); 2 = the run
could not be made.

THE SHIPPED CORPUS IS NEVER TOUCHED, and it is asserted rather than intended --
F76's three layers, reused unchanged: every play/record/save names an ABSOLUTE
path (FilePath short-circuits on absolute, file_path.cpp:107-111); the farm's
`scripts` SYMLINK - which build_farm points at the real field data - is replaced
by a real directory holding only a copy of `fscripts/startup.sts`; and an md5
manifest of all 408 shipped scripts is taken before and re-taken after.
"""

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f27_reply as F27                                          # noqa: E402

REPO = HERE.parents[1]
CORPUS = Path.home() / ".spacecrafter" / "scripts"

FAULTS = []
RESULTS = {}


def bad(leg, msg):
    FAULTS.append("%s: %s" % (leg, msg))
    print("FAIL %s: %s" % (leg, msg), flush=True)


def good(leg, msg):
    print("ok   %s: %s" % (leg, msg), flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def manifest():
    return {str(p): md5(p) for p in sorted(CORPUS.rglob("*.sts"))}


def readout(client):
    client.sock.sendall(b"get status position\n")
    reply, _lat, _raw = client.poll_for_reply(6.0)
    return reply


def jday(client):
    r = readout(client)
    return r["jday"] if r else None


def conf_value(text, section, key):
    """The value of `key` inside `[section]` of an ini text, or None."""
    cur = None
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
        elif cur == section and "=" in s:
            k, v = s.split("=", 1)
            if k.strip() == key:
                return v.strip()
    return None


def refusal_lines(text):
    """BOTH shapes (11.194(l)1): a named origin logs one `Error executing`
    line and never says `Could not execute`; an origin with no name still
    writes the legacy pair."""
    return [l for l in text.splitlines()
            if "Could not execute" in l or "Error executing " in l]


def wait_for(sess, mark, needle, seconds=90):
    t = time.time()
    while time.time() - t < seconds:
        if needle in sess.lognew(mark):
            return True
        time.sleep(0.5)
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=F27.DEFAULT_BIN)
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    if not Path(a.bin).exists():
        print("missing binary: %s" % a.bin)
        return 2
    if not CORPUS.exists():
        print("missing corpus: %s" % CORPUS)
        return 2

    before = manifest()
    (out / "corpus-manifest-before.json").write_text(json.dumps(before, indent=0))
    print("corpus manifest before: %d scripts" % len(before), flush=True)

    play = out / "played"
    play.mkdir(exist_ok=True)
    rec_file = play / "f77_record.sts"
    sl_on = play / "f77_sl_on.txt"
    sl_true = play / "f77_sl_true.bin"
    sl_off = play / "f77_sl_off.txt"
    dump0 = out / "dump-moonscale-0.json"
    dump5 = out / "dump-moonscale-5.json"

    # ------------------------------------------------------- the bad script
    # One deliberately bad line per mechanism, each isolated by a neutral
    # neighbour, and every line's identity carried by its own LINE NUMBER
    # (since F72 the log tags a file refusal with `<file>:<line>`), so
    # attribution needs no positional argument at all.
    bad_lines = [
        "# F77 - one deliberately bad line per mechanism, neutral neighbours between.",  # 1
        "wait duration 0.2",                                                    # 2
        "flag atmosphere yes",                                                  # 3   5.116(a)
        "flag atmosphere toggle",                                               # 4   state read
        "wait duration 0.2",                                                    # 5
        "flag atmosphere on",                                                   # 6   the accepted control
        "flag atmosphere toggle",                                               # 7   state read
        "wait duration 0.2",                                                    # 8
        "flag atmosphere Off",                                                  # 9   5.116(a), the fold
        "flag atmosphere toggle",                                               # 10  state read
        "wait duration 0.2",                                                    # 11
        "set moon_scale big",                                                   # 12  5.116(b)
        "wait duration 0.2",                                                    # 13
        "set stall_radius_unit = 5.0",                                          # 14  5.118 o 5.116
        "wait duration 0.2",                                                    # 15
        "configuration module star_lines action load_star name f77seed "
        "star_name 1000 star_pos 1,2,3",                                        # 16  SS-26 seed
        "configuration module star_lines action save name %s binary_mode on" % sl_on,     # 17
        "configuration module star_lines action save name %s binary_mode true" % sl_true,  # 18
        "configuration module star_lines action save name %s binary_mode off" % sl_off,   # 19
        "wait duration 0.2",                                                    # 20
        "flag nosuchflag on",                                                   # 21  POSITIVE CONTROL
        "wait duration 0.2",                                                    # 22
    ]
    bad_file = play / "f77_bad.sts"
    bad_file.write_text("\n".join(bad_lines) + "\n")
    bad_md5_before = md5(bad_file)
    LINE = {t: i + 1 for i, t in enumerate(bad_lines)}

    # the `#!` writer's own control: the ruled class, in a file of its own, so
    # that "no tail on the bad script" is a fact about the funnel and not about
    # a dead annotator.
    end_file = play / "f77_end_without_if.sts"
    end_text = ("# F77 leg I: the ruled class, so the `#!` writer is\n"
                "struct if end\n")
    end_file.write_text(end_text)
    end_md5_before = md5(end_file)

    def prepare(dst):
        """F76's layer 2, unchanged: replace the farm's `scripts` SYMLINK -
        build_farm points it AT THE REAL FIELD DATA - with a real directory
        carrying only the startup script the engine plays by name."""
        link = dst / "scripts"
        assert link.is_symlink(), "the farm no longer symlinks scripts: re-read build_farm"
        link.unlink()
        (link / "fscripts").mkdir(parents=True)
        src = CORPUS / "fscripts" / "startup.sts"
        shutil.copy(src, link / "fscripts" / "startup.sts")
        assert md5(link / "fscripts" / "startup.sts") == md5(src)

    sess = F27.Session(out, "f77", a.bin, prepare=prepare,
                       stderr_path=out / "f77.stderr")
    farm_conf = sess.dst / "config.ini"
    conf_in = farm_conf.read_text(encoding="latin-1")
    RESULTS["farm_config_in"] = {
        "print_log": conf_value(conf_in, "debug", "print_log"),
        "moon_scale": conf_value(conf_in, "viewing", "moon_scale"),
        "flag_moon_scaled": conf_value(conf_in, "viewing", "flag_moon_scaled"),
        "flag_atmosphere": conf_value(conf_in, "landscape", "flag_atmosphere"),
        "stall_radius_unit": conf_value(conf_in, "navigation", "stall_radius_unit"),
    }
    print("farm config in: %s" % RESULTS["farm_config_in"], flush=True)
    if RESULTS["farm_config_in"]["print_log"] != "true":
        bad("setup", "print_log is not true in the farm config -- 5.117's "
                     "console half cannot be measured on the shipped default")

    drive = sess.client("drive")
    drive.send("$LOGON", pause=1.0)      # so `get` answers reach this socket
    diag = sess.client("diag")
    diag.send("$DIAGON", pause=1.0)      # and ONLY this: no $LOGON here
    time.sleep(1.0)
    diag_conf = diag.all
    RESULTS["diag_confirmation_bytes"] = len(diag_conf)

    # --------------------------------------------------------- the clock
    # F76's measured hazard: startup.sts sets `timerate rate 1`, so anything
    # compared for equality across seconds drifts.  Frozen, and the drift
    # measured both ways rather than swallowed by a tolerance.
    d0 = jday(drive)
    time.sleep(3.0)
    d1 = jday(drive)
    drive.send("timerate rate 0", pause=1.0)
    f0 = jday(drive)
    time.sleep(3.0)
    f1 = jday(drive)
    RESULTS["clock"] = {"running_3s": None if None in (d0, d1) else d1 - d0,
                        "frozen_3s": None if None in (f0, f1) else f1 - f0}
    print("clock: %s running / %s frozen" % (RESULTS["clock"]["running_3s"],
                                             RESULTS["clock"]["frozen_3s"]), flush=True)

    # ============================================================= P8: TCP
    # The WIRE's positive control.  If this is empty, nothing in this run is
    # evidence about the wire.
    mark = sess.logmark()
    wire_mark = len(diag.all)
    drive.send("flag nosuchflag on", pause=1.5)
    diag.read(2.0)
    p8_wire = diag.all[wire_mark:]
    p8_log = sess.lognew(mark)
    p8_records = [m for m in F27.messages(p8_wire) if m.startswith("$DIAG|")]
    p8_ref = refusal_lines(p8_log)
    RESULTS["P8"] = {"wire_bytes": len(p8_wire),
                     "wire_records": p8_records,
                     "log": p8_log.splitlines()}
    tcp_shaped = [l for l in p8_ref
                  if re.search(r"Error executing tcp#\d+: flag nosuchflag on #! ", l)]
    if (len(p8_wire) > 0 and p8_records
            and p8_records[0].startswith("$DIAG|tcp#")
            and "Unrecognized or malformed flag argument" in p8_records[0]
            and tcp_shaped):
        good("P8", "TCP control lit both channels: wire %d B, %r; log %r"
             % (len(p8_wire), p8_records[0].strip()[:100],
                tcp_shaped[0].strip()[-95:]))
    else:
        bad("P8", "wire %d B records=%s / log refusals=%s"
            % (len(p8_wire), p8_records, p8_ref))

    # ======================================================= P1..P7: FILE
    drive.send("script action record filename %s" % rec_file, pause=1.5)
    mark = sess.logmark()
    wire_mark = len(diag.all)
    drive.send("script action play filename %s" % bad_file, pause=1.0)
    reached = wait_for(sess, mark, "Execute_command script action end", 120)
    time.sleep(3.0)
    file_log = sess.lognew(mark)
    diag.read(2.0)
    file_wire = diag.all[wire_mark:]
    drive.send("script action cancelrecord", pause=2.0)
    bad_md5_after = md5(bad_file)
    recorded = rec_file.read_text(encoding="latin-1").splitlines() if rec_file.exists() else []
    (out / "file-leg-log.txt").write_text(file_log)
    (out / "file-leg-wire.bin").write_bytes(file_wire)
    RESULTS["file_leg"] = {
        "script_end_reached": reached,
        "log": file_log.splitlines(),
        "wire_bytes": len(file_wire),
        "wire_records": [m for m in F27.messages(file_wire) if m.startswith("$DIAG|")],
        "recorded": recorded,
        "bad_md5_before": bad_md5_before, "bad_md5_after": bad_md5_after,
    }
    if not reached:
        bad("file", "the played script never reached its end -- every per-line "
                    "claim below is about a partial run")

    echoes = [l for l in file_log.splitlines() if "Execute_command " in l]
    file_refusals = refusal_lines(file_log)
    blank_debug = [l for l in file_log.splitlines()
                   if l.rstrip().endswith("(Debug):")]
    suggestions = [l for l in file_log.splitlines() if "Did you mean" in l]

    def echoed(cmd):
        return [l for l in echoes if l.rstrip().endswith("Execute_command " + cmd)]

    def refused_at(n):
        return [l for l in file_refusals if "%s:%d:" % (bad_file, n) in l]

    def in_record(cmd):
        return [l for l in recorded if l.strip() == cmd]

    def toggle_after(n):
        """The recorded toggle that FOLLOWS the value at line n: `commandFlag`
        rewrites its own commandline on a toggle with the value the flag ENDS
        at, so `flag atmosphere 1` means it was 0 when the toggle ran."""
        idx = [i for i, l in enumerate(recorded)
               if l.strip() == bad_lines[n - 1]]
        for i in idx:
            for l in recorded[i + 1:i + 6]:
                m = re.match(r"^flag atmosphere ([01])$", l.strip())
                if m:
                    return m.group(1)
        return None

    # --- the three coerced flag values and their accepted control
    for tag, n, want_state in (("P1", 3, "1"), ("P2", 6, "0"), ("P3", 9, "1")):
        cmd = bad_lines[n - 1]
        silent = (not refused_at(n)) and echoed(cmd)
        state = toggle_after(n)
        RESULTS[tag] = {"line": n, "cmd": cmd, "echo": bool(echoed(cmd)),
                        "refusals": refused_at(n), "recorded": in_record(cmd),
                        "toggle_records": state}
        if silent and in_record(cmd) and state == want_state:
            good(tag, "`%s` (line %d): echoed, no refusal, RECORDED (success), "
                      "next toggle recorded `flag atmosphere %s` -> the flag was %s"
                 % (cmd, n, state, "OFF" if state == "1" else "ON"))
        else:
            bad(tag, "`%s` (line %d): echo=%s refusals=%s recorded=%s toggle=%s "
                     "(wanted %s)" % (cmd, n, bool(echoed(cmd)), refused_at(n),
                                      in_record(cmd), state, want_state))

    # P2 vs P1/P3 must actually differ, or the state channel proves nothing
    if RESULTS["P1"]["toggle_records"] == RESULTS["P2"]["toggle_records"]:
        bad("P1/P2", "the coerced value and the accepted one left the SAME "
                     "recorded toggle -- this channel cannot discriminate")

    # --- P4: set moon_scale big
    cmd4 = bad_lines[11]
    RESULTS["P4"] = {"line": 12, "cmd": cmd4, "echo": bool(echoed(cmd4)),
                     "refusals": refused_at(12), "recorded": in_record(cmd4)}
    if echoed(cmd4) and not refused_at(12) and in_record(cmd4):
        good("P4", "`%s` (line 12): echoed, no refusal, RECORDED (success)" % cmd4)
    else:
        bad("P4", "`%s`: echo=%s refusals=%s recorded=%s"
            % (cmd4, bool(echoed(cmd4)), refused_at(12), in_record(cmd4)))

    # --- P5: set stall_radius_unit = 5.0
    cmd5 = bad_lines[13]
    RESULTS["P5"] = {"line": 14, "cmd": cmd5, "echo": bool(echoed(cmd5)),
                     "refusals": refused_at(14), "recorded": in_record(cmd5)}
    if echoed(cmd5) and not refused_at(14) and in_record(cmd5):
        good("P5", "`%s` (line 14): echoed, no refusal, RECORDED (success) -- "
                   "the `=` became 0 and 0 was eaten by the >1.0 guard" % cmd5)
    else:
        bad("P5", "`%s`: echo=%s refusals=%s recorded=%s"
            % (cmd5, bool(echoed(cmd5)), refused_at(14), in_record(cmd5)))

    # --- P6: SS-26, one word and two meanings
    def head(p):
        return p.read_bytes()[:64] if p.exists() else b""
    RESULTS["P6"] = {
        "on": {"exists": sl_on.exists(), "size": sl_on.stat().st_size if sl_on.exists() else None,
               "head": head(sl_on).decode("latin-1")},
        "true": {"exists": sl_true.exists(), "size": sl_true.stat().st_size if sl_true.exists() else None,
                 "head": head(sl_true).hex()},
        "off": {"exists": sl_off.exists(), "size": sl_off.stat().st_size if sl_off.exists() else None,
                "head": head(sl_off).decode("latin-1")},
        "refusals_16_19": [refused_at(n) for n in (16, 17, 18, 19)],
    }
    text_marker = b"# Created by SC StarLines::saveHipCatalogue"
    on_is_text = sl_on.exists() and sl_on.read_bytes().startswith(text_marker)
    true_is_bin = sl_true.exists() and not sl_true.read_bytes().startswith(text_marker)
    off_is_text = sl_off.exists() and sl_off.read_bytes().startswith(text_marker)
    same_on_off = (sl_on.exists() and sl_off.exists()
                   and sl_on.read_bytes() == sl_off.read_bytes())
    if on_is_text and true_is_bin and off_is_text and same_on_off:
        good("P6", "SS-26 live: `binary_mode on` wrote the SAME bytes as "
                   "`binary_mode off` (%d B, text) and `binary_mode true` wrote "
                   "something else (%d B) -- `on` means NO to strToBool, while "
                   "`flag atmosphere on` means YES to isTrue on the same launch"
             % (sl_on.stat().st_size, sl_true.stat().st_size))
    else:
        bad("P6", "on_text=%s true_bin=%s off_text=%s on==off=%s"
            % (on_is_text, true_is_bin, off_is_text, same_on_off))

    # --- P7: the positive control at FILE origin
    p7_ref = refused_at(21)
    p7_shaped = [l for l in p7_ref if l.rstrip().endswith(
        "Error executing %s:21: flag nosuchflag on #! Unrecognized or malformed "
        "flag argument" % bad_file)]
    RESULTS["P7"] = {"refusals": p7_ref, "shaped": p7_shaped,
                     "blank_debug_lines": blank_debug,
                     "suggestions": suggestions,
                     "recorded": in_record("flag nosuchflag on"),
                     "wire_records_during_file_leg":
                         RESULTS["file_leg"]["wire_records"]}
    if (p7_shaped and blank_debug and suggestions
            and not in_record("flag nosuchflag on")):
        good("P7", "FILE control: the F73 shape verbatim, a BLANK (Debug): line "
                   "before it, a suggestion, and the line is ABSENT from the "
                   "recording (failure)")
    else:
        bad("P7", "shaped=%s blank=%d suggestions=%d recorded=%s"
            % (bool(p7_shaped), len(blank_debug), len(suggestions),
               in_record("flag nosuchflag on")))

    # --- the TAIL claim, and the WIRE's silence at file origin
    if bad_md5_after == bad_md5_before:
        good("TAIL", "the played file is BYTE-IDENTICAL after the run (%s) -- "
                     "the `#!` in the log line is the as-if line, never a write"
             % bad_md5_after[:8])
    else:
        bad("TAIL", "the played file CHANGED: %s -> %s"
            % (bad_md5_before[:8], bad_md5_after[:8]))
    if len(file_wire) == 0:
        good("WIRE", "0 bytes on the $DIAGON link for the whole file leg, "
                     "refusal included -- 11.187(d)'s asymmetry, measured")
    else:
        bad("WIRE", "%d bytes arrived on the $DIAGON link during a FILE-origin "
                    "leg: %s" % (len(file_wire),
                                 RESULTS["file_leg"]["wire_records"]))

    # ================================================== P4 channel 2: the dome
    drive.send("select planet Moon pointer off", pause=1.0)
    drive.send("flag track_object on", pause=2.0)
    drive.send("zoom fov 10 duration 0", pause=2.0)
    drive.send("body action dual_dump filename %s" % dump0, pause=3.0)
    drive.send("configuration action save", pause=2.0)
    conf_after_bad = farm_conf.read_text(encoding="latin-1")
    (out / "config-after-badscript.ini").write_text(conf_after_bad, encoding="latin-1")
    drive.send("set moon_scale 5", pause=1.5)
    drive.send("body action dual_dump filename %s" % dump5, pause=3.0)
    drive.send("configuration action save", pause=2.0)
    conf_restored = farm_conf.read_text(encoding="latin-1")
    (out / "config-after-restore.ini").write_text(conf_restored, encoding="latin-1")

    def moon(dumpfile):
        if not Path(dumpfile).exists():
            return None
        import dumpread
        for line in Path(dumpfile).read_text(encoding="latin-1").splitlines():
            if '"name":"Moon"' not in line:
                continue
            try:
                rec = json.loads(dumpread.sanitize_nonfinite(line))
            except ValueError:
                return {"unparseable": line[:200]}
            o, n = rec.get("old"), rec.get("new")
            return {"old_screenSz": o.get("screenSz") if o else None,
                    "old_visible": o.get("visible") if o else None,
                    "new_scaling": n.get("scaling") if n else None,
                    "new_scalingTarget": n.get("scalingTarget") if n else None,
                    "new_screenSize": n.get("screenSize") if n else None}
        return None

    m0, m5 = moon(dump0), moon(dump5)
    RESULTS["P4_state"] = {
        "config_moon_scale_after_bad": conf_value(conf_after_bad, "viewing", "moon_scale"),
        "config_moon_scale_restored": conf_value(conf_restored, "viewing", "moon_scale"),
        "config_flag_atmosphere_after_bad": conf_value(conf_after_bad, "landscape", "flag_atmosphere"),
        "config_stall_after_bad": conf_value(conf_after_bad, "navigation", "stall_radius_unit"),
        "moon_at_scale_0": m0, "moon_at_scale_5": m5,
    }
    v0 = RESULTS["P4_state"]["config_moon_scale_after_bad"]
    v5 = RESULTS["P4_state"]["config_moon_scale_restored"]
    if v0 is not None and float(v0) == 0.0 and v5 is not None and float(v5) == 5.0:
        good("P4-value", "`set moon_scale big` left [viewing] moon_scale = %s "
                         "(shipped 5); `set moon_scale 5` restored %s" % (v0, v5))
    else:
        bad("P4-value", "config moon_scale after bad=%s restored=%s" % (v0, v5))
    if m0 and m5 and m0.get("old_screenSz") is not None and m5.get("old_screenSz") is not None:
        if m0["old_screenSz"] == 0.0 and m5["old_screenSz"] > 0.0:
            good("P4-dome", "the Moon's drawn screen size is %s at scale 0 and "
                            "%s at scale 5 (old-path readout; visible %s -> %s)"
                 % (m0["old_screenSz"], m5["old_screenSz"],
                    m0["old_visible"], m5["old_visible"]))
        else:
            bad("P4-dome", "screenSz at scale 0 = %s, at scale 5 = %s"
                % (m0["old_screenSz"], m5["old_screenSz"]))
    else:
        bad("P4-dome", "the dump did not yield a Moon record: %s / %s" % (m0, m5))

    # P5's state half: the negative, verified the only way it can be
    s_bad = RESULTS["P4_state"]["config_stall_after_bad"]
    if s_bad == RESULTS["farm_config_in"]["stall_radius_unit"]:
        good("P5-state", "`stall_radius_unit` in the saved config is unchanged "
                         "(%s): saveCurrentConfig does not write it back, no "
                         "getter exists, no dump carries it -- the dropped "
                         "value is on NO shipped channel" % s_bad)
    else:
        bad("P5-state", "stall_radius_unit moved in the saved config: %s -> %s"
            % (RESULTS["farm_config_in"]["stall_radius_unit"], s_bad))

    # ==================================================== P9: the `#!` writer
    mark = sess.logmark()
    drive.send("script action play filename %s" % end_file, pause=1.0)
    wait_for(sess, mark, "closes nothing", 60)
    time.sleep(3.0)
    end_log = sess.lognew(mark)
    end_after = end_file.read_bytes()
    RESULTS["P9"] = {
        "log": [l for l in end_log.splitlines() if "closes nothing" in l],
        "file_after": end_after.decode("latin-1"),
        "md5_before": end_md5_before, "md5_after": md5(end_file),
    }
    # WHICH LINES THE ENGINE WROTE ON is a diff against the bytes THIS SCRIPT
    # wrote, never a search for `#!`. The first version searched, and line 1 of
    # this very fixture quotes `#!` inside an author comment - so the needle
    # matched text the harness had authored itself and called a held prediction
    # a failure. The fixture is deliberately kept as it is: a line whose own
    # text contains `#!` and which the engine leaves alone is a free control on
    # the writer's idempotency rule.
    b_lines = end_text.splitlines()
    a_lines = end_after.decode("latin-1").splitlines()
    written = [i + 1 for i, (x, y) in enumerate(zip(b_lines, a_lines)) if x != y]
    appended = {i: a_lines[i - 1][len(b_lines[i - 1]):] for i in written}
    RESULTS["P9"]["lines_written_by_engine"] = written
    RESULTS["P9"]["appended"] = appended
    tail_lines = written
    err_line = [l for l in RESULTS["P9"]["log"] if "(Error)" in l]
    if (err_line and tail_lines == [2]
            and appended[2].startswith(" #! this 'struct if end' closes nothing")):
        good("P9", "the `#!` writer WAS armed this launch: an L_ERROR log line "
                   "and a tail APPENDED to line 2 of its own file (line 1, whose "
                   "author text contains `#!`, untouched) -- so the bad script's "
                   "untouched bytes are the funnel's doing, not a dead writer")
    else:
        bad("P9", "error lines %s, engine wrote lines %s (%s)"
            % (err_line, tail_lines, appended))

    # ==================================================== the end, then CONSOLE
    rc_app = sess.stop(drive)
    after = manifest()
    (out / "corpus-manifest-after.json").write_text(json.dumps(after, indent=0))
    moved = sorted(k for k in set(before) | set(after) if before.get(k) != after.get(k))
    RESULTS["corpus_moved"] = moved
    if moved:
        bad("corpus", "%d shipped script(s) CHANGED: %s" % (len(moved), moved[:5]))
    else:
        good("corpus", "all %d shipped scripts byte-identical after the run" % len(after))

    # std::cout is block-buffered when it is not a tty, so the console is read
    # AFTER the child exits and attributed by CONTENT, never by time.
    stdout_txt = (out / "f77.applog").read_text(encoding="latin-1", errors="replace")
    stderr_txt = (out / "f77.stderr").read_text(encoding="latin-1", errors="replace")
    needle = "Unrecognized or malformed flag argument"
    ruled = "closes nothing"
    RESULTS["P10"] = {
        "stdout_bytes": len(stdout_txt), "stderr_bytes": len(stderr_txt),
        "refusal_on_stdout": [l for l in stdout_txt.splitlines() if needle in l][:4],
        "refusal_on_stderr": [l for l in stderr_txt.splitlines() if needle in l][:4],
        "ruled_on_stdout": [l for l in stdout_txt.splitlines() if ruled in l][:4],
        "ruled_on_stderr": [l for l in stderr_txt.splitlines() if ruled in l][:4],
        "execute_echo_on_stdout":
            len([l for l in stdout_txt.splitlines() if "Execute_command " in l]),
    }
    p = RESULTS["P10"]
    if (p["refusal_on_stdout"] and not p["refusal_on_stderr"]
            and p["ruled_on_stderr"] and not p["ruled_on_stdout"]):
        good("P10", "console, both ways: the L_DEBUG refusal is on STDOUT and "
                    "not stderr; the L_ERROR ruled-class line is on STDERR and "
                    "not stdout. print_log puts EVERY line on a console; the "
                    "severity chooses which one.")
    else:
        bad("P10", "refusal stdout=%d stderr=%d / ruled stdout=%d stderr=%d"
            % (len(p["refusal_on_stdout"]), len(p["refusal_on_stderr"]),
               len(p["ruled_on_stdout"]), len(p["ruled_on_stderr"])))

    RESULTS["app_exit"] = rc_app
    RESULTS["f27_asserts"] = F27.FAILS
    RESULTS["predictions_md5"] = md5(HERE / "artifacts/f77/predictions.md")
    (out / "f77_result.json").write_text(json.dumps(RESULTS, indent=1, default=str))
    print("\n%d fault(s); app exit %s; f27 asserts %d"
          % (len(FAULTS) + len(F27.FAILS), rc_app, len(F27.FAILS)), flush=True)
    for f in FAULTS:
        print("  " + f)
    return 1 if (FAULTS or F27.FAILS) else 0


if __name__ == "__main__":
    sys.exit(main())
