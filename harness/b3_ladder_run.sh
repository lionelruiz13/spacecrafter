#!/bin/bash
# B3 depth-ladder runner (INTENT 5.29 / 5.30 / 5.33; tasks F1-P1, F1-P2).
#   usage: b3_ladder_run.sh <state> [outdir] [extra b3_ladder.py args...]
#     state = none | shell | terrain | atm   (aliases: pre=shell, post=terrain)
#     extra: --site moon|earth|earth_noatm   --families sph,cur
# Builds the temp-HOME farm (11.103(a)), runs the ladder, and ASSERTS that the
# REAL ~/.spacecrafter was not written (md5 in == out; the farm makes that true
# by construction, so a mismatch means the farm leaked - exit 3).  A driver
# failure dominates the md5 verdict (11.103(e) convention).
set -u
STATE=${1:?usage: b3_ladder_run.sh <state> [outdir] [--site ...]}
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${2:-$HERE/artifacts/b3_ladder/$STATE}
shift 2 2>/dev/null || shift 1
FARM=${B3_FARM:-/tmp/b3_farm}
mkdir -p "$OUT"

IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini)
"$HERE/b3_farm.sh" "$FARM" || exit 2
B3_FARM=$FARM python3 "$HERE/b3_ladder.py" "$OUT" "$STATE" "$@"
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
