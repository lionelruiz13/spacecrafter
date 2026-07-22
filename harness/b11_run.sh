#!/bin/bash
# B11 trail-recording-gate run - one FRESH launch under gdb (INTENT 11.56).
# ptrace_scope=1 blocks attach, so the app is launched UNDER gdb with
# b11_probe.gdb: the breakpoints answer two questions no application log can -
# "did the command reach its handler" (spelling) and "did the accumulation
# code run at all after the flag went off" (the DoD-4 work-stopped question).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../../build-claude/src/spacecrafter}"
DRIVER=${1:-b11_trail_gate.py}
OUT=${2:-$HERE/artifacts/b11}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

echo "binary: $BIN"
ls -l --time-style=full-iso "$BIN"

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b11_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 3
kill -0 $GDBPID 2>/dev/null && { echo "killing app"; kill -INT $GDBPID; sleep 5; kill -9 $GDBPID 2>/dev/null; }
echo "--- probe lines (chronological) ---"
grep -a "PROBE" "$OUT/gdb.log"
echo "--- probe counts ---"
echo "planetsSetFlagTrails: $(grep -ac 'PROBE planetsSetFlagTrails' "$OUT/gdb.log")"
echo "setPlanetHidden:      $(grep -ac 'PROBE setPlanetHidden' "$OUT/gdb.log")"
echo "accumulate RAN:       $(grep -ac 'PROBE accumulate RAN' "$OUT/gdb.log")"
exit $RC
