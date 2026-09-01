#!/usr/bin/env python3
"""f76_corpus.py - the disposition table for scedit's findings over the SHIPPED
scripts (scedit INTENT S5 item 16; constraint C3).

C3 says a lint rule ships only when the shipped corpus produces zero FALSE
positives, and that the TRUE findings are recorded upstream instead of being
silenced.  The 408 installed scripts met the thirteen shipped rules for the
first time on 2026-08-30 and produced 1661 findings that nobody had judged:
"by inspection plausibly all TRUE, but plausibly is not a disposition"
(journal 2026-08-30e).  This script is the judgment, made machine-checkable.

SHAPE (f70_dispositions.py's, deliberately): a MECHANICAL census joined to
HAND-MADE dispositions.  The census comes from `scedit --check` itself - never
from a second reading of the scripts - and each disposition carries the GROUND
it was decided on (an engine file:line that was read, or a live observation).
A finding that matches no disposition row is an ERROR, not a default: an
unjudged finding is exactly the one that would ship unnoticed.

    cd claude/harness && ./f76_corpus.py > artifacts/f76/dispositions.tsv
    ./f76_corpus.py --strict        # exit 1 if anything is UNADJUDICATED

THE FOUR DISPOSITIONS (the task's taxonomy, F76):

  TRUE-shipped    the script is wrong; the engine does what the message says.
                  Routes to the script-surface owner (SCRIPT_SURFACE.md, SS-n)
                  - it is his file and his intent, never ours to edit (D9).
  TRUE-generator  the file is generated and the defect is the generator's.
  FALSE-POSITIVE  scedit is wrong: a C1 defect of a rule or of the grammar.
                  Fixed in the task that finds it - a rule that fires falsely
                  on shipped content does not ship.
  ENGINE          the engine's behaviour is the defect and the script is
                  reasonable.  Routes to the parent ledger (S5.79), recorded
                  and not fixed.

THE CORPUS IS NEVER DECODED.  It is untracked field data in ISO-8859 or
anything else; only scedit reads its bytes.  This script reads scedit's OUTPUT,
and writes the table as pure ASCII (any high byte is escaped), so the artifact
is greppable and D14-clean whatever the scripts hold.

THE TWIN RULE.  43 md5-identical groups exist under ~/.spacecrafter/scripts
(mostly `navigation/fscripts/X` mirroring `fscripts/X`), so one authored defect
can appear on several rows and one fix must land in every member.  The TWINS
column names the other members, measured by md5 at run time, so the table
itself carries the fact instead of a sentence somewhere else remembering it.
"""

import argparse
import collections
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))          # /home/claude/spacecrafter
DEFAULT_SCEDIT = os.path.join(REPO, "util/scedit/build-f76/scedit")
DEFAULT_GRAMMAR = os.path.join(REPO, "util/scedit/grammar/sc-grammar.json")
DEFAULT_CORPUS = os.path.expanduser("~/.spacecrafter/scripts")

LINE_RE = re.compile(rb"^(.*?):(\d+): (warning|error): (.*) \[-W([a-z-]+)\]$")


def ascii_only(b):
    """Bytes -> a pure-ASCII string, high bytes shown as \\xNN.  The corpus is
    not decoded: an escape is a faithful record of a byte, a decode is a
    guess about an encoding nobody declared."""
    if isinstance(b, str):
        b = b.encode("utf-8")
    return b.decode("ascii", "backslashreplace")


# --------------------------------------------------------------- the census
def census(scedit, grammar, corpus):
    """Every finding scedit reports over the corpus, in its own order."""
    files = []
    for root, _dirs, names in os.walk(corpus):
        for n in names:
            if n.endswith(".sts"):
                files.append(os.path.join(root, n))
    files.sort()
    if not files:
        print("no .sts file under %s - the shipped corpus is absent here; this "
              "table cannot be produced (and a silent empty one would be the "
              "vacuous pass C3 exists to prevent)" % corpus, file=sys.stderr)
        sys.exit(2)
    out = subprocess.run([scedit, "--grammar", grammar, "--check"] + files,
                         capture_output=True)
    if out.returncode > 1:
        print("scedit --check failed (rc %d): %s"
              % (out.returncode, ascii_only(out.stderr)[:400]), file=sys.stderr)
        sys.exit(2)
    if out.stderr.strip():
        print("scedit wrote to stderr: %s" % ascii_only(out.stderr)[:400], file=sys.stderr)
        sys.exit(2)
    rows = []
    for raw in out.stdout.splitlines():
        m = LINE_RE.match(raw)
        if not m:
            print("unparsed --check line: %s" % ascii_only(raw)[:200], file=sys.stderr)
            sys.exit(2)
        path = m.group(1).decode("ascii", "backslashreplace")
        rel = os.path.relpath(path, corpus)
        rows.append((rel, int(m.group(2)), m.group(5).decode("ascii"),
                     m.group(3).decode("ascii"), ascii_only(m.group(4))))
    return files, rows


def twins(files, corpus):
    """md5 -> the group of identical files, as corpus-relative paths."""
    by = collections.defaultdict(list)
    for f in files:
        with open(f, "rb") as fh:
            by[hashlib.md5(fh.read()).hexdigest()].append(os.path.relpath(f, corpus))
    return {p: sorted(set(g) - {p}) for g in by.values() for p in g if len(g) > 1}


# ---------------------------------------------------------- the dispositions
# Each row: (defect, matcher, disposition, signal, route, ground).
#
# The matcher is (rel-path, line, lint-id) -> bool.  FIRST match wins, so the
# order below is part of the data: a narrow row must precede the broad one it
# carves out of.  `ground` names the engine text that was READ (anchors at code
# e3afca8f, the tree F75 re-anchored the grammar against) or the live
# observation that settled it - never a plausibility.
#
# SIGNAL is the second question, and it is the one F77 needs: when this line
# runs, does the engine SAY anything?  Three answers exist on this corpus -
# LOG-DEBUG (a refusal through the funnel or the unknown-command emitter, tagged
# `(Debug): ` per S5.117), LOG-ERROR+ANNOTATE (reportScriptError, which also
# writes a `#!` tail into the script file), and SILENT.  The column is a
# PREDICTION until a live leg confirms it; every class was confirmed, see
# artifacts/f76/live/.
_F = "fscripts/"
_N = "navigation/fscripts/"


def _at(*pairs):
    s = set(pairs)
    return lambda rel, line, ident: (rel, line) in s


def _msg(ident_want, *files):
    fs = set(files)
    return lambda rel, line, ident: ident == ident_want and rel in fs


DEFECTS = [
 # ---- duplicate-key -------------------------------------------------------
 ("halo-twice-in-the-comet-template",
  lambda rel, line, ident: ident == "duplicate-key" and rel in (
      "internal/comet-particles.sts", "internal/comet.sts", _N + "W17.sts"),
  "TRUE-shipped", "SILENT", "SS-39",
  "`args` is `std::map<string,string>` [observed: src/tools/utility.hpp:85] and the "
  "extraction loop assigns `arguments[key] = value` [observed: "
  "app_command_interface.cpp:182], so a repeated key keeps the LAST value written. "
  "Measured on all 1595 of these lines: the two `halo` values are the same token "
  "(`true`), so last-wins costs nothing here - the finding is true and the effect is "
  "nil. One authored template, copied: the same `... halo true color <c> tex_map "
  "bodies/asteroid.png halo true tex_halo ...` run appears in all three files"),
 ("one-missing-space-shifts-a-whole-body-line",
  _at(("fscripts/06old.sts", 286)),
  "TRUE-shipped", "SILENT", "SS-33",
  "The line writes `color0.5,0.5,0.5` with no space, so from that token on every "
  "key/value pair is off by one: `color0.5,0.5,0.5`->`tex_map`, and further along "
  "`0.997306872521443` becomes a KEY twice (duplicate-key) while the final "
  "`0.7,0.7,0.8` becomes a key with nothing after it (dangling-key). Mechanism: "
  "`while (commandstr >> key >> value)` [observed: app_command_interface.cpp:164] "
  "pairs tokens positionally and knows nothing about which names are keys"),
 # ---- dangling-key --------------------------------------------------------
 ("credit-written-with-a-space-drops-the-rest",
  _msg("dangling-key", "internal/deepsky_drawings.sts"),
  "TRUE-shipped", "SILENT", "SS-32",
  "Five of the file's 60 `dso action load` lines break its own convention: 55 write "
  "`credit Laurent_Ferrero` as ONE token, these five write the name with a space "
  "(`credit Jere Kahampaa`) or omit `credit` entirely. The pairing loop then reads "
  "`Kahampaa` as a key whose value is `texture_luminance_adjust`, and `1` is left "
  "with nothing after it. Consequence at the engine: `args[W_TEXTURE]` "
  "(`texture_luminance_adjust` [observed: base_command_interface.hpp:117]) is empty, "
  "`evalDouble(\"\")` returns 0.0 [observed: app_command_interface.cpp:4631-4635], and "
  "0.0 is what `loadNebula` receives [observed: :1613-1616] instead of the 1 the "
  "line asks for"),
 # ---- unknown-parameter, KEYS THE HANDLER NEVER READS (silent) ------------
 ("credit-written-with-a-space-drops-the-rest",
  _msg("unknown-parameter", "internal/deepsky_drawings.sts"),
  "TRUE-shipped", "SILENT", "SS-32",
  "Same five lines, the other half of the same shift: the author's surname is read "
  "as an argument key. `commandDso` reads only action/path/name/ra/de/magnitude/"
  "angular_size/rotation/filename/credit/texture_luminance_adjust/distance/"
  "constellation/type [observed: app_command_interface.cpp:1599-1620]"),
 ("deselect-does-not-read-pointer",
  _msg("unknown-parameter", _N + "13.sts"),
  "TRUE-shipped", "SILENT", "SS-34",
  "`commandDeselect` reads exactly one key, `constellation` [observed: "
  "app_command_interface.cpp:3314-3322]; `pointer` is never looked at and the "
  "command reports success. The spelling is real on the OTHER command - `select "
  "... pointer off` - which is where the 27 lines got it"),
 ("audio-does-not-read-output_rate",
  _msg("unknown-parameter", _F + "K9.sts", _N + "K9.sts", "internal/white_room.sts",
       "internal/white_room_old.sts", "internal/white_room_open_only.sts"),
  "TRUE-shipped", "SILENT", "SS-34",
  "`commandAudio` reads volume/nopause/action/filename/loop and nothing else "
  "[observed: app_command_interface.cpp:3037-3095]; the sample rate is not a script "
  "parameter at all. The audio still plays - only the rate request is dropped"),
 ("media-spells-it-keycolor",
  _msg("unknown-parameter", _F + "W06.sts", _N + "W06.sts"),
  "TRUE-shipped", "SILENT", "SS-34",
  "The engine's key is `keycolor`, one word [observed: "
  "base_command_interface.hpp:189 `#define W_KEYCOLOR \"keycolor\"`, read at "
  "app_command_interface.cpp:3235 and :3882]. `key_color` is never read"),
 ("date-does-not-read-duration",
  _at((_F + "08.sts", 163)),
  "TRUE-shipped", "SILENT", "SS-34",
  "`commandDate` reads jday/local/utc/relative and their siblings [observed: "
  "app_command_interface.cpp:3964-4030]; `duration` is not among them. The date IS "
  "set (the `utc` branch runs and returns), so only the animation the word promises "
  "is missing - the same vocabulary SS-2 asks about on `set`"),
 # ---- unknown-parameter, NAMES THE ENGINE REFUSES (logged) ----------------
 ("set-stops-at-the-first-unknown-name",
  _at((_F + "S13.sts", 2), (_N + "S13.sts", 2)),
  "TRUE-shipped", "LOG-DEBUG", "SS-35",
  "Sharper than 'the key is ignored', and the message says so. `commandSet` loops "
  "`returnValue = returnValue && evalCommandSet(...)` over `args` [observed: "
  "app_command_interface.cpp:2273-2287]; `args` is a std::map, so the loop runs in "
  "KEY ORDER, and `duration` sorts before `home_planet`. `duration` hits "
  "`APP_FLAG_NONE`, which sets debug_message and returns "
  "`executeCommandStatus()` = false [observed: :2384-2391, :1307-1341] - after which "
  "`&&` short-circuits and `evalCommandSet` is never called for `home_planet`. The "
  "home planet is not set and the line reports a refusal"),
 ("a-flag-name-the-engine-does-not-have",
  _at(("internal/clear_mess.sts", 39), ("internal/clearVR360.sts", 14),
      (_N + "09.sts", 22), ("shows/3d_sky.sts", 12),
      ("shows/image_spherical.sts", 13)),
  "TRUE-shipped", "LOG-DEBUG", "SS-36",
  "`setFlag` looks the name up in `m_flags` and refuses when it is absent "
  "[observed: app_command_interface.cpp:450-460], `commandFlag` then sets "
  "debug_message [observed: :1343-1352]. Verified independently of the grammar: the "
  "97 `m_flags[...]` registrations of app_command_init.cpp resolved through the "
  "ACP_FN_* macros of base_command_interface.hpp contain none of `suntrace`, "
  "`ground`, `show_selected_object_info`, `lanscape` - and the same extraction "
  "reproduces the grammar's 97/97, 43/43 set names and 46/46 colour names exactly. "
  "`suntrace` IS a command (ACP_CN_SUNTRACE, base_command_interface.hpp:393), "
  "`lanscape` is one letter off `landscape` (:455)"),
 # ---- unknown-command -----------------------------------------------------
 ("nebula-is-spelled-dso",
  _at(("internal/skypole.sts", 12), (_N + "16.sts", 64), (_N + "16.sts", 65)),
  "TRUE-shipped", "LOG-DEBUG", "SS-37",
  "`nebula` is in neither `m_commands` nor the grammar; the command that takes "
  "exactly these keys (ra/de/magnitude/angular_size/name/filename/credit/"
  "texture_luminance_adjust) is `dso` [observed: app_command_interface.cpp:"
  "1599-1620]. The dispatch refuses the line at :367-381 and calls "
  "`searchSimilarCommand`; nothing is drawn"),
 ("two-lines-that-are-not-script",
  _at((_F + "06old.sts", 104), (_F + "06old.sts", 299)),
  "TRUE-shipped", "LOG-DEBUG", "SS-33",
  "`LS` and `==> e_sats-tle-new.sts <==` are the residue of a file concatenation "
  "(the second is a `head`/`tail` multi-file banner), sitting between body "
  "definitions. `command` is lowercased [observed: app_command_interface.cpp:161] "
  "and neither `ls` nor `==>` is in `m_commands`, so both are refused at :367-381"),
 # ---- end-without-if ------------------------------------------------------
 ("one-struct-if-end-too-many",
  _at((_F + "panorama5.sts", 102), (_N + "panorama5.sts", 102)),
  "TRUE-shipped", "LOG-ERROR+ANNOTATE", "SS-25",
  "Lines 96-97 open two `struct if`, lines 100-102 close three. `commandStruct`'s "
  "`W_END` branch calls `ifSwap->pop()` and reports when it returns false "
  "[observed: app_command_interface.cpp:4776-4780], and `reportScriptError` writes "
  "at L_ERROR AND annotates the faulty line in the file with a `#!` tail "
  "[observed: :215-239]. Already SS-25 in the script-surface channel (opened "
  "2026-08-31 from a python pre-scan); this is the first time the shipped checker "
  "itself reports it. The message CARRIED a false clause when this row was first "
  "written - it said the engine logs \"end without if\", a string code 2b8ec034 "
  "stopped writing on 2026-08-31 - and that was corrected at code 8eeffb9d before "
  "this table was delivered; the finding itself never moved"),
 # ---- silent-off-value ----------------------------------------------------
 ("a-typo-that-happens-to-mean-what-was-meant",
  _at((_F + "M17.sts", 16)),
  "TRUE-shipped", "SILENT", "SS-38",
  "`flag stars ofn`. `convertStrToFlagValues` returns FV_TOGGLE for `toggle`, FV_ON "
  "when `Utility::isTrue` says so, and FV_OFF for EVERYTHING else [observed: "
  "app_command_interface.cpp:1298-1305]; `isTrue` accepts only 4-byte TRUE, 2-byte "
  "ON (case-folded) and the single character 1 [observed: src/tools/utility.hpp:"
  "160-171], so a 3-character token cannot reach it. `ofn` therefore turns the "
  "stars OFF, which is what the line's neighbours (`flag nebulae off`, `flag "
  "bright_nebulae off`) say was meant - the typo is invisible because it is "
  "correct by accident. S5.116's first FIELD instance, measured"),
]


def disposition(rel, line, ident):
    for d in DEFECTS:
        if d[1](rel, line, ident):
            return d
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scedit", default=DEFAULT_SCEDIT)
    ap.add_argument("--grammar", default=DEFAULT_GRAMMAR)
    ap.add_argument("--corpus", default=DEFAULT_CORPUS)
    ap.add_argument("--strict", action="store_true",
                    help="exit 1 if any finding is UNADJUDICATED")
    a = ap.parse_args()
    for p in (a.scedit, a.grammar):
        if not os.path.exists(p):
            print("missing: " + p, file=sys.stderr)
            return 2
    if not os.path.isdir(a.corpus):
        print("the shipped corpus is not installed at %s - SKIPPING LOUDLY "
              "rather than reporting an empty table" % a.corpus, file=sys.stderr)
        return 2

    files, rows = census(a.scedit, a.grammar, a.corpus)
    tw = twins(files, a.corpus)

    print("\t".join(["file", "line", "id", "severity", "defect", "disposition",
                     "signal", "route", "twins", "ground", "message"]))
    by_disp = collections.Counter()
    by_id = collections.Counter()
    by_sig = collections.Counter()
    unjudged = 0
    for rel, line, ident, sev, msg in rows:
        d = disposition(rel, line, ident)
        if d is None:
            unjudged += 1
            defect, disp, sig, route, ground = "-", "UNADJUDICATED", "-", "-", "-"
        else:
            defect, disp, sig, route, ground = d[0], d[2], d[3], d[4], d[5]
        by_disp[disp] += 1
        by_id[ident] += 1
        by_sig[sig] += 1
        print("\t".join([rel, str(line), ident, sev, defect, disp, sig, route,
                         ",".join(tw.get(rel, [])) or "-", ground, msg]))

    print("# %d findings over %d files in %d scripts; by id: %s; by disposition: %s; "
          "by engine signal: %s"
          % (len(rows), len({r[0] for r in rows}), len(files),
             " ".join("%s=%d" % kv for kv in sorted(by_id.items())),
             " ".join("%s=%d" % kv for kv in sorted(by_disp.items())),
             " ".join("%s=%d" % kv for kv in sorted(by_sig.items()))),
          file=sys.stderr)
    print("# %d distinct authored defects; %d finding(s) the engine says nothing about"
          % (len({d[0] for d in DEFECTS}), by_sig.get("SILENT", 0)), file=sys.stderr)
    if unjudged:
        print("# %d finding(s) carry no disposition row" % unjudged, file=sys.stderr)
        if a.strict:
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
