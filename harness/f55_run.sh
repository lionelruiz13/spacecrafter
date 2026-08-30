#!/bin/bash
# F55 runner — f51_run.sh's pattern (that file is the authority) plus ONE extra
# assert this task is the first to need: F55 writes a `startup.sts`, and the
# owner's own `~/.spacecrafter/scripts/fscripts/startup.sts` is a REAL file that
# a b3-style farm would have exposed through a symlink.  f55_farm.sh removes
# that exposure; this asserts the removal WORKED, by md5, in == out.
#   usage: f55_run.sh <absOutdir> [extra f55_sampler.py args...]
set -u
OUT=${1:?usage: f55_run.sh <absOutdir> [--no-burst] [--fps N]}
shift
HERE=$(cd "$(dirname "$0")" && pwd)
FARM=${F55_FARM:-/tmp/f55_farm}
mkdir -p "$OUT"

STS=~/.spacecrafter/scripts/fscripts/startup.sts
IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini "$STS")
echo "IN : $IN_MD5"
F55_FARM=$FARM python3 "$HERE/f55_sampler.py" "$OUT" "$@"
DRV=$?
OUT_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini "$STS")
echo "OUT: $OUT_MD5"
if [ "$IN_MD5" != "$OUT_MD5" ]; then
    echo "FAIL: real ~/.spacecrafter md5 changed across the run (farm leak)"
    [ $DRV -ne 0 ] && exit $DRV
    exit 3
fi
echo "config/ssystem/startup.sts md5 in=out OK"
exit $DRV
