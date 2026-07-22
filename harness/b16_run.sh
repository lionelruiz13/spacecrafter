#!/bin/bash
# B16 `body action reload` observable run - one fresh launch (INTENT 11.55).
# The app runs UNDER gdb (ptrace_scope=1 blocks attach) with b16_probe.gdb:
# the breakpoint on SSystemFactory::reloadCurrentSystem is the "the command
# reached its handler" evidence that does not come from the handler's own log.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b16_reload.py}
OUT=${2:-$HERE/artifacts/b16}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b16_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $GDBPID 2>/dev/null && { echo "killing app"; kill -INT $GDBPID; sleep 5; kill -9 $GDBPID 2>/dev/null; }
echo "--- probe lines ---"
grep -a "PROBE" "$OUT/gdb.log"
echo "--- handler log lines ---"
grep -a "reloaded (observer state kept)\|Cannot reload the system\|System reload:" ~/.spacecrafter/log/spacecrafter.log
echo "--- driver log ---"
cat "$OUT/drive.log"
