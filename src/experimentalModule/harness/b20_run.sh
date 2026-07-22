#!/bin/bash
# B20 anchored-galactic regression run - one fresh launch (INTENT 11.59).
# The camera dump IS the instrument (mat-layer reference/distance), so no gdb
# and no init_fov requirement.  config.ini is not touched (settings are sent
# as commands); md5 asserted in==out.  Fresh launch per run (dirty instrument
# state invalidates the run - harness posture).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../../build-claude/src/spacecrafter}"
DRIVER=${1:-b20_anchored_galactic.py}
OUT=${2:-$HERE/artifacts/b20g}
mkdir -p "$OUT"
rm -f "$OUT"/*.json

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (in) = $MD5_IN"

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"

for i in $(seq 1 40); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -30 "$OUT/app.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
if kill -0 $APPPID 2>/dev/null; then echo "app ALIVE post-run"; kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; else echo "app NOT alive post-run"; fi

MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"
echo "--- driver output ---"
cat "$OUT/drive.log"
exit $RC
