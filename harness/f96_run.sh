#!/bin/bash
# F96 -- one run of the view-offset readout legs: one command, one exit code
# (INTENT 11.216; row 5.138).
#
#   DISPLAY=:2 ./f96_run.sh <absOutdir> --tag pre|post [--bin PATH]
#                                       [--stages cmd,cfg,descend] [--jd-sun JD]
#     SC_BIN=<path>        binary under test (default build-claude/src/spacecrafter)
#     F96_SKIP_CANARY=1    skip the environment canary (say why, in writing)
#
# What this wrapper owns (the F90/F91 shape):
#   * the ENVIRONMENT CANARY (11.176), `--no-scene`: these are READOUT legs and
#     they make no photometric claim -- the Sun-low frame pair is RECORDED for
#     the owner's eye and gated by nothing.  The FULL canary is run once,
#     separately, as this task's photometric regression gate.  A canary red
#     STOPS the run; it is never mitigated and never widened (11.174(h)).
#   * the FROZEN-FILE ASSERT, md5 in == out, on the real ~/.spacecrafter's
#     config.ini and ssystem.ini (the driver asserts it again around the legs).
#   * the CONCURRENT-INSTANCE assert on /proc/<pid>/comm, every account
#     (11.134(b): `pgrep -f <path>` self-matches its own wrapper).
#
# Exit: 0 all gates green; non-zero on any FAIL, a canary red, a concurrent
# instance, or a frozen file that moved.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:?usage: f96_run.sh <absOutdir> --tag pre|post [--bin PATH] [--stages ...]}
shift || true
mkdir -p "$OUT"

FILES=( "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini" )

echo "=== F96 offset-frame leg  $(date '+%Y-%m-%d %H:%M:%S %Z')"
echo "    binary  = $BIN"
ls -l --time-style=+%H:%M:%S "$BIN"; md5sum "$BIN"
echo "    display = ${DISPLAY:-<unset>}"
echo "    args    = $*"

HITS=""
for p in /proc/[0-9]*/comm; do
    [ -r "$p" ] || continue
    if [ "$(cat "$p" 2>/dev/null)" = "spacecrafter" ]; then HITS="$HITS ${p%/comm}"; fi
done
if [ -n "$HITS" ]; then
    echo "ABORT: another spacecrafter process exists:$HITS"; exit 2
fi
echo "    /proc comm assert: no spacecrafter running"

if [ "${F96_SKIP_CANARY:-0}" != "1" ]; then
    bash "$HERE/f56_canary.sh" --no-scene > "$OUT/canary.log" 2>&1
    CRC=$?
    echo "    canary --no-scene exit = $CRC   ($OUT/canary.log)"
    if [ "$CRC" != "0" ]; then
        echo "ABORT: the environment canary is RED. Report it; do not widen it."; exit 3
    fi
fi

declare -a IN
for i in "${!FILES[@]}"; do
    IN[$i]=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    echo "    md5 in  ${IN[$i]}  ${FILES[$i]}"
done

DISPLAY=${DISPLAY:-:2} python3 "$HERE/f96_offset.py" "$OUT" --bin "$BIN" "$@" \
    2>&1 | tee "$OUT/drive.log"
RC=${PIPESTATUS[0]}

sleep 2
pkill -9 -x spacecrafter 2>/dev/null

MOVED=0
for i in "${!FILES[@]}"; do
    O=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    if [ "$O" = "${IN[$i]}" ]; then M=yes; else M=NO; MOVED=1; fi
    echo "    md5 out ${O}  MATCH=$M  ${FILES[$i]}"
done

echo "    driver exit = $RC ; frozen files moved = $MOVED"
[ "$MOVED" = "1" ] && exit 4
exit $RC
