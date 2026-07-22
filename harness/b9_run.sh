#!/bin/bash
# B9 az-convention probe/lock - one FRESH launch (INTENT §11.4/§11.60).
# Instrument = the process DUMP (dual_dump altaz_old/altaz_new + <file>.navstr
# caller-visible strings). No gdb. Config.ini untouched (settings sent as
# commands, but home_planet/date are runtime); md5 asserted in/out.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b9_azconv.py}
OUT=${2:-$HERE/artifacts/b9}
mkdir -p "$OUT"
rm -f "$OUT"/*.json "$OUT"/*.navstr "$OUT"/*.log

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (in) = $MD5_IN"
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

MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"
echo "--- driver tail ---"
tail -60 "$OUT/drive.log"
exit $RC
