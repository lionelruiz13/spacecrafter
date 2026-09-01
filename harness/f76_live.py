#!/usr/bin/env python3
"""f76_live.py - the six corpus finding classes, put to the running engine.

F76 scope item 2: at least ONE finding per lint id verified LIVE, never by
reading alone.  A disposition table built from source is a set of predictions
about a program; this is the program answering.  Every leg is BOTH WAYS - the
faulty line AND a control that differs in exactly the thing the finding names -
because a leg that only ever sees the faulty side cannot fail.

    cd claude/harness && DISPLAY=:0 ./f76_live.py <outdir> [--bin <binary>]

Exit 0 = every prediction held; 1 = at least one did not (printed, and a
disagreement is itself a finding - scedit or the engine is wrong and the
table has to say which); 2 = the run could not be made.

THE PREDICTIONS ARE IN THIS FILE AND THIS FILE IS COMMITTED BEFORE THE RUN.
They are stated as relations between measurements, never as absolute values
copied from anywhere: "the duplicate line's jday equals the second value's and
not the first" needs no almanac and cannot be tuned after the fact.

  A duplicate-key      `date utc A utc B` lands on B, not on A (the last write
                       into a std::map wins).  Control: `date utc B` alone.
  B dangling-key       `date utc A f76dangling` still sets A and says nothing;
                       `date f76dangling` alone leaves the map EMPTY and IS
                       refused (app_command_interface.cpp:4106).  So the key is
                       dropped and the rest of the line runs - both halves.
  C unknown-command    `nebula action load ...`, a real corpus line, is refused
                       by name.  Control: a known command in the same shape.
  D unknown-parameter  `flag lanscape off` is refused (the flag NAME).
    (a name)           Control: `flag landscape off` is not.
  E unknown-parameter  `deselect constellation Dor pointer off`, a real corpus
    (a key)            line: the engine runs it and says NOTHING about
                       `pointer`.  The control is the log itself - the
                       `Execute_command` echo proves the line ran, so silence
                       is silence and not absence.
  F unknown-parameter  the ORDER claim, which is what makes scedit's message
    (a set name)       sharper than "the key is ignored": `commandSet` folds
                       with `&&` over a std::map, so a bad name that sorts
                       BEFORE `heading` costs the heading and one that sorts
                       AFTER does not.  Two lines, one letter of difference in
                       a name the engine has never heard of, opposite states.
  G silent-off-value   `flag stars ofn` - is the flag ON or OFF afterwards?
                       Read through a shipped channel: `script action record`
                       writes a toggle back with the value the flag ENDED at
                       (commandFlag rewrites its own commandline), so a toggle
                       recorded as `flag stars 1` proves the flag was OFF when
                       `ofn` left it.  Control: the same pair after `on`.
  H end-without-if     the panorama5 shape, played as a file: the engine writes
                       an L_ERROR line AND a `#!` tail onto the offending line
                       (reportScriptError), and scedit's reading of that
                       annotated file must agree with it - f63_scedit_agree's
                       method, on a file this run produced.

THE SHIPPED CORPUS IS NEVER TOUCHED, and that is asserted rather than intended.
Three layers: (1) every script this run plays or records is named by an
ABSOLUTE path, which FilePath resolves without ever consulting the scripts
directory (file_path.cpp:107-111); (2) the farm's `scripts` symlink - which
build_farm points AT THE REAL FIELD DATA - is replaced by a real directory
holding only a copy of `fscripts/startup.sts`, so the engine's startup is
unchanged and nothing it can reach is the corpus; (3) an md5 manifest of all
408 shipped scripts is taken before the launch and re-taken after, and a single
difference fails the run.  Layer 3 is the one that is evidence: the engine
ANNOTATES faulty script lines in place, so a leg that exercises the block
structure is exactly a leg that would write into the tester's shows.
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
SCEDIT = REPO / "util/scedit/build-f75/scedit"
GRAMMAR = REPO / "util/scedit/grammar/sc-grammar.json"

UTC_A = "2011:06:16T00:00:00"     # a corpus date (fscripts/08.sts:163)
UTC_B = "2020:01:01T12:00:00"     # any other instant; only the RELATION is used

FAULTS = []
RESULTS = {}


def bad(leg, msg):
    FAULTS.append("%s: %s" % (leg, msg))
    print("FAIL %s: %s" % (leg, msg), flush=True)


def good(leg, msg):
    print("ok   %s: %s" % (leg, msg), flush=True)


def manifest():
    out = {}
    for p in sorted(CORPUS.rglob("*.sts")):
        out[str(p)] = hashlib.md5(p.read_bytes()).hexdigest()
    return out


def jday(client):
    """Field 4 of `get status position`, through the subscriber wire."""
    client.send("get status position", pause=1.2)
    pos = F27.positions(bytes(client.buf))
    return pos[-1]["jday"] if pos else None


def heading(client):
    client.send("get status position", pause=1.2)
    pos = F27.positions(bytes(client.buf))
    return pos[-1]["heading"] if pos else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=F27.DEFAULT_BIN)
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    for p in (Path(a.bin), SCEDIT, GRAMMAR, CORPUS):
        if not p.exists():
            print("missing: %s" % p)
            return 2

    before = manifest()
    print("corpus manifest before: %d scripts" % len(before), flush=True)
    (out / "corpus-manifest-before.json").write_text(json.dumps(before, indent=0))

    # --------------------------------------------------- the isolated farm
    play_dir = out / "played"
    play_dir.mkdir(exist_ok=True)
    end_file = play_dir / "f76_end_without_if.sts"
    # The panorama5 shape, reduced to its structure: two blocks opened, three
    # closed.  Line 7 is the one closer too many.
    end_file.write_bytes(b"\n".join([
        b"# f76 leg H: the fscripts/panorama5.sts shape, in this farm only",
        b"struct if 1 inf 2",
        b"struct if 3 diff 4",
        b"flag stars on",
        b"struct if end",
        b"struct if end",
        b"struct if end",
        b"",
    ]))
    end_original = end_file.read_bytes()

    def prepare(dst):
        """Replace the farm's `scripts` SYMLINK - build_farm points it at the
        real field data - with a real directory carrying only the startup
        script the engine plays by name.  Startup behaviour is unchanged
        (script_mgr.cpp:397-401 plays scripts/fscripts/startup.sts and that
        file plays nothing else), and no path under the farm reaches a shipped
        show."""
        link = dst / "scripts"
        assert link.is_symlink(), "the farm no longer symlinks scripts: re-read build_farm"
        link.unlink()
        (link / "fscripts").mkdir(parents=True)
        src = CORPUS / "fscripts" / "startup.sts"
        shutil.copy(src, link / "fscripts" / "startup.sts")
        assert hashlib.md5((link / "fscripts" / "startup.sts").read_bytes()).hexdigest() \
            == hashlib.md5(src.read_bytes()).hexdigest()

    sess = F27.Session(out, "f76", a.bin, prepare=prepare)
    c = sess.client("drive")
    c.send("$LOGON", pause=1.0)        # so `get` answers reach this socket
    time.sleep(1.0)

    def since(mark):
        return sess.lognew(mark)

    def refusals(mark):
        return [l for l in since(mark).splitlines()
                if "Could not execute" in l or "Error executing " in l]

    # ------------------------------------------------------------- leg A
    m = sess.logmark()
    c.send("date utc %s" % UTC_A, pause=1.0)
    j_a = jday(c)
    c.send("date utc %s" % UTC_B, pause=1.0)
    j_b = jday(c)
    c.send("date utc %s utc %s" % (UTC_A, UTC_B), pause=1.0)
    j_dup = jday(c)
    RESULTS["A"] = {"j_a": j_a, "j_b": j_b, "j_dup": j_dup,
                    "refusals": refusals(m)}
    if None in (j_a, j_b, j_dup):
        bad("A", "no position reply came back (%s)" % RESULTS["A"])
    elif abs(j_a - j_b) < 1e-6:
        bad("A", "the two dates are the same instant - the leg cannot discriminate")
    elif abs(j_dup - j_b) < 1e-6 and abs(j_dup - j_a) > 1e-6:
        good("A", "duplicate-key: `utc A utc B` landed on B (%.6f), not on A (%.6f)"
             % (j_b, j_a))
    else:
        bad("A", "duplicate line gave jday %.6f; A=%.6f B=%.6f" % (j_dup, j_a, j_b))

    # ------------------------------------------------------------- leg B
    m = sess.logmark()
    c.send("date utc %s" % UTC_B, pause=1.0)
    c.send("date utc %s f76dangling" % UTC_A, pause=1.0)
    j_dang = jday(c)
    r_dang = refusals(m)
    m2 = sess.logmark()
    c.send("date f76dangling", pause=1.0)
    r_alone = refusals(m2)
    RESULTS["B"] = {"j_dangling": j_dang, "refusals_with_utc": r_dang,
                    "refusals_alone": r_alone}
    if j_dang is not None and abs(j_dang - j_a) < 1e-6 and not r_dang and r_alone:
        good("B", "dangling-key: dropped, the rest of the line ran (jday=A, no "
                  "refusal); alone it leaves an empty command and IS refused")
    else:
        bad("B", "jday %s (A=%s), refusals with utc %s, alone %s"
            % (j_dang, j_a, r_dang, r_alone))

    # ------------------------------------------------------------- leg C
    m = sess.logmark()
    c.send("nebula action load ra 29 de 3.2 magnitude -30 angular_size 250 "
           "name soleil filename circlepos/soleil.png credit ACA "
           "texture_luminance_adjust 1", pause=1.2)
    r_neb = refusals(m)
    log_neb = since(m)
    m2 = sess.logmark()
    c.send("flag stars on", pause=1.0)
    r_ctrl = refusals(m2)
    RESULTS["C"] = {"refusals": r_neb, "control_refusals": r_ctrl,
                    "suggestion": [l for l in log_neb.splitlines() if "mean" in l]}
    if r_neb and not r_ctrl:
        good("C", "unknown-command: refused (%s); a known command in the same "
                  "shape is not" % r_neb[0].strip()[:110])
    else:
        bad("C", "nebula refusals %s / control refusals %s" % (r_neb, r_ctrl))

    # ------------------------------------------------------------- leg D
    m = sess.logmark()
    c.send("flag lanscape off", pause=1.2)
    r_typo = refusals(m)
    log_typo = since(m)
    m2 = sess.logmark()
    c.send("flag landscape off", pause=1.2)
    r_ok = refusals(m2)
    RESULTS["D"] = {"refusals": r_typo, "control_refusals": r_ok,
                    "suggestion": [l for l in log_typo.splitlines() if "mean" in l]}
    if r_typo and not r_ok:
        good("D", "unknown flag NAME: refused (%s); the correct spelling is not"
             % r_typo[0].strip()[:110])
    else:
        bad("D", "lanscape refusals %s / landscape refusals %s" % (r_typo, r_ok))

    # ------------------------------------------------------------- leg E
    m = sess.logmark()
    c.send("deselect constellation Dor pointer off", pause=1.2)
    txt = since(m)
    r_sil = refusals(m)
    ran = [l for l in txt.splitlines() if "Execute_command" in l and "deselect" in l]
    RESULTS["E"] = {"refusals": r_sil, "echo": ran}
    if ran and not r_sil:
        good("E", "unknown argument KEY: the line ran (%s) and the engine said "
                  "nothing about `pointer`" % ran[0].strip()[:90])
    else:
        bad("E", "echo %s / refusals %s" % (ran, r_sil))

    # ------------------------------------------------------------- leg F
    m = sess.logmark()
    c.send("set heading 42", pause=1.5)
    h0 = heading(c)
    c.send("set aaa_f76bogus 1 heading 77", pause=1.5)
    h_before = heading(c)
    r_before = refusals(m)
    m2 = sess.logmark()
    c.send("set zzz_f76bogus 1 heading 99", pause=1.5)
    h_after = heading(c)
    r_after = refusals(m2)
    m3 = sess.logmark()
    c.send("set home_planet Earth duration 0", pause=1.5)
    r_corpus = refusals(m3)
    RESULTS["F"] = {"h0": h0, "h_after_early_bad_name": h_before,
                    "h_after_late_bad_name": h_after,
                    "refusals_early": r_before, "refusals_late": r_after,
                    "refusals_corpus_line": r_corpus}
    okf = (h0 is not None and abs(h0 - 42) < 0.5
           and h_before is not None and abs(h_before - 42) < 0.5
           and h_after is not None and abs(h_after - 99) < 0.5
           and r_before and r_after and r_corpus)
    if okf:
        good("F", "set folds with && over a std::map: a bad name sorting BEFORE "
                  "`heading` cost the heading (still %.2f), one sorting AFTER did "
                  "not (%.2f); the corpus line is refused too" % (h_before, h_after))
    else:
        bad("F", "h0=%s early=%s late=%s refusals early/late/corpus %s/%s/%s"
            % (h0, h_before, h_after, len(r_before), len(r_after), len(r_corpus)))

    # ------------------------------------------------------------- leg G
    rec_ofn = play_dir / "f76_rec_ofn.sts"
    rec_on = play_dir / "f76_rec_on.sts"
    for path, value in ((rec_ofn, "ofn"), (rec_on, "on")):
        c.send("script action record filename %s" % path, pause=1.5)
        c.send("flag stars %s" % value, pause=1.0)
        c.send("flag stars toggle", pause=1.0)
        c.send("script action cancel", pause=1.5)
    def recorded(p):
        return p.read_text(encoding="latin-1").splitlines() if p.exists() else []
    l_ofn, l_on = recorded(rec_ofn), recorded(rec_on)
    t_ofn = [l for l in l_ofn if l.startswith("flag stars ") and l.split()[-1] in ("0", "1")]
    t_on = [l for l in l_on if l.startswith("flag stars ") and l.split()[-1] in ("0", "1")]
    RESULTS["G"] = {"recorded_after_ofn": l_ofn, "recorded_after_on": l_on}
    if t_ofn and t_on and t_ofn[-1].endswith("1") and t_on[-1].endswith("0"):
        good("G", "silent-off-value: after `ofn` the toggle recorded `%s` (it was "
                  "OFF); after `on` it recorded `%s`" % (t_ofn[-1], t_on[-1]))
    else:
        bad("G", "toggle records after ofn %s / after on %s" % (t_ofn, t_on))

    # ------------------------------------------------------------- leg H
    m = sess.logmark()
    c.send("script action play filename %s" % end_file, pause=1.0)
    t = time.time()
    while time.time() - t < 60:
        if "closes nothing" in since(m):
            break
        time.sleep(0.5)
    time.sleep(2.0)
    log_h = since(m)
    engine_lines = [l for l in log_h.splitlines() if "closes nothing" in l]
    annotated = end_file.read_bytes()
    tail_lines = [i + 1 for i, l in enumerate(annotated.decode("latin-1").splitlines())
                  if "#!" in l]
    hist = subprocess.run([str(SCEDIT), "--grammar", str(GRAMMAR), "--history",
                           str(end_file)], capture_output=True, text=True,
                          encoding="latin-1")
    rows = [r.split("\t") for r in hist.stdout.splitlines() if r.count("\t") == 6]
    eng_rows = {int(r[1]): r[3] for r in rows if r[2] == "spacecrafter"}
    sce_rows = {}
    for r in rows:
        if r[2] == "scedit":
            sce_rows.setdefault(int(r[1]), set()).add(r[3])
    agree = [n for n, i in eng_rows.items() if i in sce_rows.get(n, ())]
    RESULTS["H"] = {"log": engine_lines, "annotated_lines": tail_lines,
                    "history_engine": eng_rows,
                    "history_scedit": {k: sorted(v) for k, v in sce_rows.items()},
                    "agree": agree, "file_changed": annotated != end_original}
    (out / "played-after.sts").write_bytes(annotated)
    if (engine_lines and tail_lines == [7] and eng_rows.get(7) == "end-without-if"
            and 7 in agree):
        good("H", "end-without-if: the engine logged it, wrote its `#!` on line 7 "
                  "of the played file, and scedit reads that tail as the same id")
    else:
        bad("H", "log %d line(s), `#!` on %s, history engine %s scedit %s"
            % (len(engine_lines), tail_lines, eng_rows,
               {k: sorted(v) for k, v in sce_rows.items()}))

    # ------------------------------------------------------------- the end
    rc_app = sess.stop(c)
    after = manifest()
    (out / "corpus-manifest-after.json").write_text(json.dumps(after, indent=0))
    moved = sorted(k for k in set(before) | set(after) if before.get(k) != after.get(k))
    RESULTS["corpus_untouched"] = not moved
    RESULTS["corpus_moved"] = moved
    if moved:
        bad("corpus", "%d shipped script(s) CHANGED during the run: %s"
            % (len(moved), moved[:5]))
    else:
        good("corpus", "all %d shipped scripts byte-identical after the run" % len(after))

    RESULTS["app_exit"] = rc_app
    RESULTS["harness_fails"] = F27.FAILS
    (out / "f76_live.json").write_text(json.dumps(RESULTS, indent=1, default=str))
    print("\n%d fault(s); app exit %s; f27 asserts %d"
          % (len(FAULTS) + len(F27.FAILS), rc_app, len(F27.FAILS)), flush=True)
    for f in FAULTS:
        print("  " + f)
    return 1 if (FAULTS or F27.FAILS) else 0


if __name__ == "__main__":
    sys.exit(main())
