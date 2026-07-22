#!/bin/bash
# B23 planet-grid tropic/polar-circle run - one FRESH launch (INTENT 11.57).
# No gdb probe: the instruments are the process DUMP (the grid module's
# dumpState -> baked tropic/polar latitudes + the flag echo) and the composed
# SCREEN (a self-referential px diff against a measured noise floor). The known
# intermittent shutdown segfault (INTENT 11.15d) fires AFTER the driver exits
# and does not affect the artifacts.
#
# PRECONDITION: ssystem.ini must carry `planet_grid = true` on Earth, Jupiter,
# Uranus, Moon, Sun (test-only; the dispatch restores it byte-identically).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b23_grid.py}
OUT=${2:-$HERE/artifacts/b23}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

echo "binary: $BIN"; ls -l --time-style=full-iso "$BIN"
DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 40); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }
echo "--- driver tail ---"
tail -40 "$OUT/drive.log"
exit $RC
