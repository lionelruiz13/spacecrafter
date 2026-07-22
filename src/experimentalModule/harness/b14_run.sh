#!/bin/bash
# B14 moon-absolute-pole run (INTENT 11.68). Direct launch - the measurement
# is at the loader/orientation layer via dumps (projection-free tilt pieces),
# no gdb / init_fov needed. The CALLER sets ~/.spacecrafter/ssystem.ini state
# (baseline / mutated) before invoking; this script only launches + drives +
# kills, it never mutates data.
#   DISPLAY=:2 ./b14_run.sh <tag> [outdir]
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../../build-claude/src/spacecrafter}"
TAG=${1:-baseline}
OUT=${2:-$HERE/artifacts/b14}
mkdir -p "$OUT"
rm -f "$OUT"/b14_${TAG}_d*.json*

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app_${TAG}.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID tag=$TAG"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; exit 1; }
done
sleep 8   # let async loads quiesce

python3 "$HERE/b14_moon_pole.py" "$OUT" "$TAG" > "$OUT/drive_${TAG}.log" 2>&1
DRC=$?
echo "driver exit=$DRC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }
echo "--- driver log ---"
cat "$OUT/drive_${TAG}.log"
