#!/bin/bash
# F100 -- one run of the frozen-readout legs: one command, one exit code
# (INTENT 11.220; row 5.139, mechanism reading 11.216(j1)).
#
#   DISPLAY=:2 ./f100_run.sh <absOutdir> --tag pre|post|nomemo [--bin PATH]
#                            [--clock pinned,running] [--stages ...]
#     SC_BIN=<path>         binary under test (default build-claude/src/spacecrafter)
#     F100_SKIP_CANARY=1    skip the environment canary (say why, in writing)
#
# The wrapper owns what the driver must not be trusted with (the F90/F96 shape):
#   * the ENVIRONMENT CANARY (11.176), `--no-scene`: these are FUNCTIONAL legs
#     and make no photometric claim.  A canary red STOPS the run; it is never
#     mitigated and never widened (11.174(h)).
#   * the FROZEN-FILE ASSERT, md5 in == out, on the real ~/.spacecrafter's
#     config.ini and ssystem.ini (the driver asserts it again per leg).
#   * the CONCURRENT-INSTANCE assert on /proc/<pid>/comm, every account
#     (11.134(b): `pgrep -f <path>` self-matches its own wrapper).
#   * the LOCK-FILE record: F99's finding -- a crashing or killed leg leaves
#     /tmp/spacecrafter.lock behind.  Recorded before and after; removed only
#     when its pid is dead.
#
# Exit: 0 all gates green; non-zero on any FAIL, a canary red, a concurrent
# instance, or a frozen file that moved.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:?usage: f100_run.sh <absOutdir> --tag pre|post|nomemo [--bin PATH] [...]}
shift || true
mkdir -p "$OUT"

FILES=( "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini" )

echo "=== F100 frozen-readout leg  $(date '+%Y-%m-%d %H:%M:%S %Z')"
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

if [ -e /tmp/spacecrafter.lock ]; then
    LPID=$(cat /tmp/spacecrafter.lock 2>/dev/null | head -1 | tr -dc '0-9')
    if [ -n "$LPID" ] && [ -d "/proc/$LPID" ]; then
        echo "ABORT: /tmp/spacecrafter.lock holds LIVE pid $LPID"; exit 5
    fi
    echo "    stale /tmp/spacecrafter.lock (pid '${LPID:-?}' dead) -- removed, recorded"
    rm -f /tmp/spacecrafter.lock
else
    echo "    lock file: absent"
fi

if [ "${F100_SKIP_CANARY:-0}" != "1" ]; then
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

DISPLAY=${DISPLAY:-:2} python3 "$HERE/f100_freeze.py" "$OUT" --bin "$BIN" "$@" \
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

if [ -e /tmp/spacecrafter.lock ]; then
    echo "    lock file AFTER the run: present ($(cat /tmp/spacecrafter.lock | head -1))"
else
    echo "    lock file AFTER the run: absent"
fi

echo "    driver exit = $RC ; frozen files moved = $MOVED"
[ "$MOVED" = "1" ] && exit 4
exit $RC
