#!/bin/bash
# B10-cmd swallow-guard runner (INTENT 11.79(e) D9key). App UNDER gdb
# (ptrace_scope=1 blocks attach) with b10_cmd_probe.gdb; the driver issues
# 3 valid datum + 3 valid ground + 4 bogus-spelling commands. Asserts the
# PROBE hit counts (3/3/0). Stale-instance guard + config restore.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/b10cmd_gdb}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"
rm -f "$OUT"/gdb.log

pkill -f "build-claude/src/spacecrafter" 2>/dev/null && sleep 3
fuser -k 7805/tcp 2>/dev/null && sleep 2
ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "FATAL: port 7805 still held"; exit 1; }

MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.bak"
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"

for attempt in 1 2 3; do
    echo "=== gdb launch attempt $attempt ==="
    DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b10_cmd_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
    GDBPID=$!
    UP=0
    for i in $(seq 1 45); do
        sleep 2
        if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; UP=1; break; fi
        kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp (attempt $attempt)"; break; }
    done
    [ "$UP" = 1 ] && break
    kill -9 $GDBPID 2>/dev/null; sleep 3
done
[ "$UP" = 1 ] || { echo "FATAL: no gdb launch survived"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
sleep 8

DISPLAY=${DISPLAY:-:2} python3 "$HERE/b10_cmd_gdb.py" "$OUT" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 5; kill -9 $GDBPID 2>/dev/null; }

cp "$OUT/config.ini.bak" "$CFG"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config md5 in=$MD5_IN out=$MD5_OUT $([ "$MD5_IN" = "$MD5_OUT" ] && echo OK || echo MISMATCH)"

DATUM=$(grep -ac "PROBE setBodyDatumRadius ENTERED" "$OUT/gdb.log")
GROUND=$(grep -ac "PROBE setBodyGroundRadius ENTERED" "$OUT/gdb.log")
echo "=== swallow-guard counts ==="
echo "datum PROBE hits = $DATUM (expect 3)"
echo "ground PROBE hits = $GROUND (expect 3)"
if [ "$DATUM" = 3 ] && [ "$GROUND" = 3 ]; then
    echo "SWALLOW-GUARD PASS: 3 valid datum + 3 valid ground landed 1:1; 4 bogus spellings swallowed (0 extra)"
    RC=0
else
    echo "SWALLOW-GUARD FAIL"
    RC=1
fi
echo "--- driver log ---"
cat "$OUT/drive.log"
exit $RC
