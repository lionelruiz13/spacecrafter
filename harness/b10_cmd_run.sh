#!/bin/bash
# B10-cmd fresh-launch runner (INTENT 11.71/11.79(e)). Numeric-layer command
# follow-through via dumps. Copies b10_cmd_script.sts into ~/.spacecrafter/
# scripts/ (channel 2), sets init_fov=340, restores config.ini byte-identical
# (md5 asserted), removes the script afterwards (user dir left as found).
# Stale-instance discipline (b22_live sediment): kill by cmdline + free 7805.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b10_cmd.py}
OUT=${2:-$HERE/artifacts/b10cmd}
CFG=~/.spacecrafter/config.ini
SCRIPTDIR=~/.spacecrafter/scripts
mkdir -p "$OUT"
rm -f "$OUT"/*.json

# --- stale-instance guard ---
pkill -f "build-claude/src/spacecrafter" 2>/dev/null && sleep 3
fuser -k 7805/tcp 2>/dev/null && sleep 2
ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "FATAL: port 7805 still held"; exit 1; }

MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.bak"
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"
cp "$HERE/b10_cmd_script.sts" "$SCRIPTDIR/b10_cmd_script.sts"

for attempt in 1 2 3; do
    echo "=== launch attempt $attempt ==="
    DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.$attempt.log" 2>&1 &
    APPPID=$!
    UP=0
    for i in $(seq 1 45); do
        sleep 2
        if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; UP=1; break; fi
        kill -0 $APPPID 2>/dev/null || { echo "app died before tcp (attempt $attempt)"; break; }
    done
    [ "$UP" = 1 ] && break
    kill -9 $APPPID 2>/dev/null; sleep 3
done
[ "$UP" = 1 ] || { echo "FATAL: no launch survived"; cp "$OUT/config.ini.bak" "$CFG"; rm -f "$SCRIPTDIR/b10_cmd_script.sts"; exit 1; }
sleep 8

DISPLAY=${DISPLAY:-:2} python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
DRC=$?
echo "driver exit=$DRC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }

cp "$OUT/config.ini.bak" "$CFG"
rm -f "$SCRIPTDIR/b10_cmd_script.sts"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config md5 in=$MD5_IN out=$MD5_OUT $([ "$MD5_IN" = "$MD5_OUT" ] && echo OK || echo MISMATCH)"
echo "--- driver log ---"
cat "$OUT/drive.log"
exit $DRC
