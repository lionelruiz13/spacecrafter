#!/bin/bash
# B17 command-spelling probe runner (INTENT 13.B B17). App UNDER b17_probe.gdb;
# the breakpoint on Core::setViewOffset is the "reached its handler" evidence.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../../build-claude/src/spacecrafter}"
OUT="$HERE/artifacts/b17"
mkdir -p "$OUT"
rm -f "$OUT/gdb.log" "$OUT/drive.log"

pkill -9 -f "build-claude/src/spacecrafter" 2>/dev/null
sleep 2

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b17_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

UP=0
for i in $(seq 1 45); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after $((i*2))s"; UP=1; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb exited at $((i*2))s (app died)"; break; }
done

if [ "$UP" = "1" ]; then
    sleep 5
    python3 "$HERE/b17_seam.py" > "$OUT/drive.log" 2>&1
    echo "driver exit=$?"
    sleep 2
fi

kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }
echo "=== PROBE lines ==="
grep -a "PROBE" "$OUT/gdb.log"
echo "=== driver log ==="
cat "$OUT/drive.log" 2>/dev/null || echo "(no drive.log - app never served TCP)"
