#!/bin/bash
# F99 -- one leg of the `location_orbit` null-parent guard: one command, one exit
# code (INTENT S11.219; rows S5.141, S5.50).
#
#   DISPLAY=:2 ./f99_run.sh <absOutdir> --bin <binary> --tag pre|post
#     F99_SKIP_CANARY=1    skip the environment canary (say why, in writing)
#
# What this wrapper owns (the F90/F91/F97 shape):
#   * the ENVIRONMENT CANARY (S11.176), `--no-scene`: this leg reads exit codes,
#     log lines and dump halves and makes NO photometric claim, so the scene arm
#     is not needed.  A canary red STOPS the run; it is never mitigated and never
#     widened (S11.174(h)).
#   * the FROZEN-FILE ASSERT, md5 in == out, on the real ~/.spacecrafter's
#     config.ini and ssystem.ini.  The legs run on a private farm
#     (b3_farm.sh under /home/claude/sc-f99), so this asserts the farm did its
#     job rather than hoping it did.
#   * the CONCURRENT-INSTANCE assert on /proc/<pid>/comm, every account
#     (S11.134(b): `pgrep -f <path>` self-matches its own wrapper).
#
# NOTE, and it is the reason this wrapper exists at all for a crash leg: the
# `pre` leg is EXPECTED to kill the app.  A dead app is this leg's RED half, not
# an instrument fault -- so the driver's own exit code is the verdict, and the
# post-run cleanup below must not read a corpse as a concurrent instance.
#
# Exit: 0 all gates of the tag green; non-zero on any FAIL, a canary red, a
# concurrent instance, or a frozen file that moved.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT=${1:?usage: f99_run.sh <absOutdir> --bin PATH --tag pre|post}
shift || true

BIN=""
TAG=""
ARGS=("$@")
for ((i=0; i<${#ARGS[@]}; i++)); do
    case "${ARGS[$i]}" in
        --bin) BIN="${ARGS[$((i+1))]}" ;;
        --tag) TAG="${ARGS[$((i+1))]}" ;;
    esac
done
BIN="${BIN:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
mkdir -p "$OUT"

FILES=( "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini" )

echo "=== F99 location_orbit null-parent leg  $(date '+%Y-%m-%d %H:%M:%S %Z')"
echo "    binary  = $BIN"
ls -l --time-style=+%H:%M:%S "$BIN"; md5sum "$BIN"
echo "    display = ${DISPLAY:-<unset>}"
echo "    tag     = $TAG"

HITS=""
for p in /proc/[0-9]*/comm; do
    [ -r "$p" ] || continue
    if [ "$(cat "$p" 2>/dev/null)" = "spacecrafter" ]; then HITS="$HITS ${p%/comm}"; fi
done
if [ -n "$HITS" ]; then
    echo "ABORT: another spacecrafter process exists:$HITS"; exit 2
fi
echo "    /proc comm assert: no spacecrafter running"

if [ "${F99_SKIP_CANARY:-0}" != "1" ]; then
    bash "$HERE/f56_canary.sh" --no-scene --out "$OUT/canary" > "$OUT/canary.log" 2>&1
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

DISPLAY=${DISPLAY:-:2} python3 "$HERE/f99_locguard.py" "$OUT" "$@" 2>&1 | tee "$OUT/drive.log"
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
