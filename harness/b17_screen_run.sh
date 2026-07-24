#!/bin/bash
# B17 view_offset verification runner (INTENT 11.63/11.79(c)). Fresh launch under
# gdb (SIGUSR1 watchdog); runs a self-asserting driver (default b17_screen.py;
# pass b17_arming.py for the arming lifecycle). Config md5 asserted (no config
# key touched - the offset is sent as commands). Do NOT pass SC_BIN=<default
# path> (the stale-guard would self-kill, INTENT 11.85(f)); uses pkill -x.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b17_screen.py}
OUT=${2:-$HERE/artifacts/b17screen}
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1); echo "config.ini md5 (in) = $MD5_IN"

cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
run
GDB
pkill -9 -x spacecrafter 2>/dev/null; sleep 2
DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; echo "gdb pid=$GDBPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
sleep 8
python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }
pkill -9 -x spacecrafter 2>/dev/null

MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"
echo "--- driver result ---"
grep -aE '^(PASS|FAIL|===)' "$OUT/drive.log" || tail -20 "$OUT/drive.log"
exit $RC
