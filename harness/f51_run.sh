#!/bin/bash
# F51 (A) runner — temp-HOME farm (11.103(a)) + the real ~/.spacecrafter md5
# in == out assert, exactly as b3_ladder_run.sh does it (that file is the
# authority for the pattern; this one differs only in which driver it runs).
#   usage: f51_run.sh <absOutdir> [extra f51_dwell.py args...]
set -u
OUT=${1:?usage: f51_run.sh <absOutdir> [--samples N] [--cadence S]}
shift
HERE=$(cd "$(dirname "$0")" && pwd)
FARM=${B3_FARM:-/tmp/f51_farm}
mkdir -p "$OUT"

IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini)
echo "IN : $IN_MD5"
"$HERE/b3_farm.sh" "$FARM" || exit 2
B3_FARM=$FARM python3 "$HERE/f51_dwell.py" "$OUT" "$@"
DRV=$?
OUT_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini)
echo "OUT: $OUT_MD5"
if [ "$IN_MD5" != "$OUT_MD5" ]; then
    echo "FAIL: real ~/.spacecrafter md5 changed across the run (farm leak)"
    [ $DRV -ne 0 ] && exit $DRV
    exit 3
fi
echo "config/ssystem md5 in=out OK"
exit $DRV
