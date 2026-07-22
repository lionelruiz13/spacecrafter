#!/bin/bash
# B29 runtime-color seam run (INTENT §11.65). App under gdb (ptrace_scope=1
# blocks attach); the probe counts setBodyColor / setDefaultBodyColor hits.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b29_color.py}
OUT=${2:-$HERE/artifacts/b29}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b29_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; cat "$OUT/gdb.log"; exit 1; }
done
sleep 8

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $GDBPID 2>/dev/null && { echo "shutting app"; kill -INT $GDBPID; sleep 5; kill -9 $GDBPID 2>/dev/null; }
echo "--- probe counts ---"
grep -ac "PROBE setBodyColor" "$OUT/gdb.log" | sed 's/^/setBodyColor hits: /'
grep -ac "PROBE setDefaultBodyColor" "$OUT/gdb.log" | sed 's/^/setDefaultBodyColor hits: /'
echo "--- probe lines ---"
grep -a "PROBE" "$OUT/gdb.log"
echo "--- shutdown ---"
grep -a "exited normally\|SIGSEGV\|Segmentation" "$OUT/gdb.log" | tail -3
echo "--- driver tail ---"
tail -40 "$OUT/drive.log"
