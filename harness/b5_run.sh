#!/bin/bash
# B5 fresh-launch runner (draw-half mode independence, INTENT 6.9).
# Same pattern as b10_run.sh: init_fov=340 + fisheye for the harness,
# config.ini restored byte-identically (md5 asserted). The driver reads
# $OUT/app.log for the executor-mode witnesses, so the app logs THERE.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b5_drawhalf.py}
OUT=${2:-$HERE/artifacts/b5}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json

MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.bak"
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"

echo "binary: $BIN"; ls -l --time-style=full-iso "$BIN"
DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
done
sleep 8

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
DRC=$?
echo "driver exit=$DRC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }

cp "$OUT/config.ini.bak" "$CFG"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config md5 in=$MD5_IN out=$MD5_OUT $([ "$MD5_IN" = "$MD5_OUT" ] && echo OK || echo MISMATCH)"
echo "--- driver log ---"
cat "$OUT/drive.log"
exit $DRC
