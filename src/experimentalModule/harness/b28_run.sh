#!/bin/bash
# B28 loader frame-declaration run (INTENT 11.67). Direct launch (the
# measurement is at the loader/orientation layer via dumps - no gdb needed).
# Captures the dump into $OUT for b28_analyze.py. Does NOT touch config.ini
# (tilt pieces are projection/fov-independent).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../../build-claude/src/spacecrafter}"
DRIVER=${1:-b28_frame.py}
OUT=${2:-$HERE/artifacts/b28}
mkdir -p "$OUT"
rm -f "$OUT"/b28_load*.json

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; exit 1; }
done
sleep 8   # let initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
DRC=$?
echo "driver exit=$DRC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }
echo "--- driver log ---"
cat "$OUT/drive.log"
