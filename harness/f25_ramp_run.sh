#!/bin/bash
# F25 — B34's interactive view/zoom ramps (INTENT §11.133). Fresh launch per run,
# concurrent-instance assert, frozen md5 in == out around the whole run.
#
# Usage: f25_ramp_run.sh [outdir] [extra args to f25_ramp.py]
#        SC_BIN=<path>   binary under test (default: the build tree)
#        F25_PRE=1       expect the PRE-FIX behaviour (adds --pre)
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f25/run}
shift || true
mkdir -p "$OUT"

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); SSY_IN=$(md5sum "$SSY" | cut -d' ' -f1)
echo "config.ini md5 (in)  = $CFG_IN"
echo "ssystem.ini md5 (in) = $SSY_IN"
echo "binary = $BIN"; ls -l --time-style=+%H:%M:%S "$BIN"; md5sum "$BIN"

# §11.121(m): the confound is INTRA-account, so assert on the process, any owner.
if pgrep -x spacecrafter > /dev/null; then
    echo "ABORT: another spacecrafter process exists"; pgrep -af spacecrafter; exit 2
fi

EXTRA=""
[ "${F25_PRE:-0}" = "1" ] && EXTRA="--pre"
DISPLAY=${DISPLAY:-:2} python3 "$HERE/f25_ramp.py" "$OUT" --bin "$BIN" $EXTRA "$@" \
    2>&1 | tee "$OUT/drive.log"
RC=${PIPESTATUS[0]}
sleep 2
pkill -9 -x spacecrafter 2>/dev/null

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
echo "config.ini md5 (out)  = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
echo "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
echo "driver exit = $RC"
exit $RC
