#!/bin/bash
# B3 depth-ladder runner (INTENT 5.29 / 11.101(c) / task F1-P1).
#   usage: b3_ladder_run.sh <pre|post> [outdir]
# Builds the temp-HOME farm (11.103(a)), runs the ladder, and ASSERTS that the
# REAL ~/.spacecrafter was not written (md5 in == out; the farm makes that true
# by construction, so a mismatch means the farm leaked - exit 3).  A driver
# failure dominates the md5 verdict (11.103(e) convention).
set -u
STATE=${1:?usage: b3_ladder_run.sh <pre|post> [outdir]}
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${2:-$HERE/artifacts/b3_ladder/$STATE}
FARM=${B3_FARM:-/tmp/b3_farm}
mkdir -p "$OUT"

IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini)
"$HERE/b3_farm.sh" "$FARM" || exit 2
B3_FARM=$FARM python3 "$HERE/b3_ladder.py" "$OUT" "$STATE"
DRV=$?
OUT_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini)
if [ "$IN_MD5" != "$OUT_MD5" ]; then
    echo "FAIL: real ~/.spacecrafter md5 changed across the run (farm leak)"
    echo "$IN_MD5"; echo "$OUT_MD5"
    [ $DRV -ne 0 ] && exit $DRV
    exit 3
fi
echo "config/ssystem md5 in=out OK"
exit $DRV
