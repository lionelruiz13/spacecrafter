#!/bin/bash
# F90 — THE SMOKE SUITE: one command, one exit code (INTENT §11.211).
#
#   DISPLAY=:2 ./f90_rehearsal_run.sh <absOutdir> [extra args to f90_rehearsal.py]
#     SC_BIN=<path>        binary under test  (default: build-claude/src/spacecrafter)
#     F90_SKIP_CANARY=1    skip the environment canary (say why, in writing)
#
# What this wrapper owns, and why it is not inside the python:
#   * the ENVIRONMENT CANARY (§11.176) — a measuring launch does not start on an
#     unverified stack, and a non-zero canary STOPS the run rather than being
#     mitigated (§11.174(h)).  `--no-scene` only: F90 is FUNCTIONAL and makes no
#     photometric claim, so the photometric arm has nothing here to gate.
#   * the FROZEN-FILE ASSERT, md5 in == out, on every file of the real
#     ~/.spacecrafter this run could conceivably reach:
#         config.ini   ssystem.ini            — two shipped writers (§5.42, §5.112)
#         the played show                      — the script annotator rewrites the
#         scripts/fscripts/startup.sts           file it played
#                                                (script_annotator.cpp:163-179)
#     The farm makes all four unreachable; this asserts it instead of trusting it.
#   * the CONCURRENT-INSTANCE assert on /proc/<pid>/comm, every account
#     (§11.134(b): `pgrep -f <path>` self-matches its own wrapper).
#
# Exit: 0 all steps PASS or DIVERGENCE-with-a-row; non-zero on any FAIL, on a
# canary red, on a concurrent instance, or on a frozen file that moved.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:?usage: f90_rehearsal_run.sh <absOutdir> [args...]}
shift || true
mkdir -p "$OUT"

# the show is the one frozen file whose name the caller can change
SHOW=basis/zodiacal_light.sts
PREV=""
for a in "$@"; do [ "$PREV" = "--show" ] && SHOW="$a"; PREV="$a"; done

FILES=(
  "$HOME/.spacecrafter/config.ini"
  "$HOME/.spacecrafter/ssystem.ini"
  "$HOME/.spacecrafter/scripts/$SHOW"
  "$HOME/.spacecrafter/scripts/fscripts/startup.sts"
)

echo "=== F90 smoke suite  $(date '+%Y-%m-%d %H:%M:%S %Z')"
echo "    binary  = $BIN"
ls -l --time-style=+%H:%M:%S "$BIN"; md5sum "$BIN"
echo "    display = ${DISPLAY:-<unset>}"
echo "    outdir  = $OUT"

# ROUTED 2026-09-12 through the ONE home of the instance criterion (F112,
# Sec.11.238): comm | /proc/<pid>/exe | TCP 7805, union.  The inline
# `comm == "spacecrafter"` form this replaced was measured BLIND to a renamed or
# copied engine (Sec.11.231(j2): it read 0 with two staging instances live and
# holding port 7805), and it was copy-pasted into 44 files, so no single edit
# could fix it.  sc_instances.sh --assert prints pid . uid . comm . exe . port
# per hit and exits 0 clear / 2 engine live / 4 the probe could not run.
bash "$HERE/sc_instances.sh" --assert f90-smoke || exit 2

# --- environment canary (§11.176)
# [F112 2026-09-12, §11.238, on F110's finding (§11.234): F90_SKIP_CANARY=1 used to
# leave NO TRACE -- no canary.log, no line in this output -- so an UNVERIFIED run read
# exactly like a verified one, which is the one thing a suite whose whole value is
# "one command, one exit code" must not do.  The skip is now stated, in the output AND
# in a file beside the canary.log that is not there, with what it costs.  This is the
# same class as the §11.174(h) rule it serves: an environment fault (or a deliberate
# bypass of the check for one) is RECORDED, never silent.]
if [ "${F90_SKIP_CANARY:-0}" != "1" ]; then
    bash "$HERE/f56_canary.sh" --no-scene > "$OUT/canary.log" 2>&1
    CRC=$?
    echo "    canary --no-scene exit = $CRC   ($OUT/canary.log)"
    if [ "$CRC" != "0" ]; then
        echo "ABORT: the environment canary is RED. Report it; do not widen it."; exit 3
    fi
else
    echo "    canary --no-scene SKIPPED  (F90_SKIP_CANARY=1)"
    echo "    *** THIS RUN IS UNVERIFIED AGAINST THE BANKED STACK.  The canary is the"
    echo "    *** member that would have caught the 2026-08-29 dim era (§11.176), and"
    echo "    *** it is the member this run did not take.  Any number it produces is a"
    echo "    *** number from an unchecked stack: say so wherever it is used."
    echo "    *** Legitimate reason to skip: the bank is THIS host's process epochs,"
    echo "    *** display and GPU band, so on any other machine the canary is red"
    echo "    *** before it has measured anything (§11.234).  That is a missing bank,"
    echo "    *** not a stopped measurement -- and it is still not a verified stack."
    {
        echo "CANARY SKIPPED -- F90_SKIP_CANARY=1"
        echo "run          : $(date '+%F %T %Z')"
        echo "binary       : $BIN  ($(md5sum "$BIN" | cut -d' ' -f1))"
        echo "display      : ${DISPLAY:-<unset>}"
        echo "host         : $(uname -n)"
        echo "why this file exists: the skip used to leave no trace at all, so an"
        echo "unverified run was indistinguishable from a verified one (F110 finding,"
        echo "§11.234; fixed by F112, §11.238).  This file IS the trace."
    } > "$OUT/canary.SKIPPED"
fi

# --- frozen files, in
declare -a IN
for i in "${!FILES[@]}"; do
    if [ -f "${FILES[$i]}" ]; then IN[$i]=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    else IN[$i]="(absent)"; fi
    echo "    md5 in  ${IN[$i]}  ${FILES[$i]}"
done

DISPLAY=${DISPLAY:-:2} python3 "$HERE/f90_rehearsal.py" "$OUT" --bin "$BIN" "$@" \
    2>&1 | tee "$OUT/drive.log"
RC=${PIPESTATUS[0]}

sleep 2
pkill -9 -x spacecrafter 2>/dev/null

# --- frozen files, out
MOVED=0
for i in "${!FILES[@]}"; do
    if [ -f "${FILES[$i]}" ]; then O=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    else O="(absent)"; fi
    if [ "$O" = "${IN[$i]}" ]; then M=yes; else M=NO; MOVED=1; fi
    echo "    md5 out ${O}  MATCH=$M  ${FILES[$i]}"
done

echo "    driver exit = $RC ; frozen files moved = $MOVED"
[ "$MOVED" = "1" ] && exit 4
exit $RC
