#!/bin/bash
# B21 key-path unification runner (INTENT §11.71/§11.72(c)). Fresh launch under
# gdb whose STDIN is a FIFO the driver writes to, so the driver can stop the
# inferior (SIGINT to gdb) and call `Core::raiseHeight/lowerHeight/updateMove` -
# the exact functions the joypad/UI path calls - then resume. Config md5 asserted.
#
# Usage: b21_keypath_run.sh [outdir]      SC_BIN=<path> to test another binary.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f4/keypath}
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/gdb.log "$OUT"/drive.log

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1); echo "config.ini md5 (in) = $MD5_IN"
echo "binary = $BIN"; ls -l --time-style=+%H:%M:%S "$BIN"

FIFO="$OUT/gdbin"
rm -f "$FIFO"; mkfifo "$FIFO"

pkill -9 -x spacecrafter 2>/dev/null; sleep 2
DISPLAY=${DISPLAY:-:2} gdb -q "$BIN" < "$FIFO" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; echo "gdb pid=$GDBPID"
# Persistent writer: if the last writer closes, gdb sees EOF on stdin and quits.
exec 3> "$FIFO"
{
  echo "set pagination off"
  echo "set confirm off"
  echo "set height 0"
  echo "handle SIGUSR1 nostop noprint pass"
  echo "run"
} >&3

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
sleep 8

python3 "$HERE/b21_keypath.py" "$OUT" "$GDBPID" "$FIFO" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 2; echo "quit" >&3; sleep 3; kill -9 $GDBPID 2>/dev/null; }
exec 3>&-
pkill -9 -x spacecrafter 2>/dev/null
rm -f "$FIFO"

MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"
echo "--- driver result ---"
grep -aE '^(PASS|FAIL|--|===)' "$OUT/drive.log" || tail -20 "$OUT/drive.log"
exit $RC
