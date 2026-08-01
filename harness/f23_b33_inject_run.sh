#!/bin/bash
# B33's two LATENT members (INTENT §11.108(f) / §11.131): the view offset and
# the mount. No shipped channel splits their two authorities, so the divergence
# is INJECTED — a fresh launch under gdb whose stdin is a FIFO the driver writes
# to, exactly the b21_keypath instrument, calling `Camera::setViewOffset` /
# `Camera::setMount` on the path that DRAWS. Temp-HOME farm; frozen md5s
# asserted in == out on the REAL tree, which this run must never touch.
#
# Usage: f23_b33_inject_run.sh [outdir]     SC_BIN=<path> to test another binary.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f23/inject}
mkdir -p "$OUT"; rm -f "$OUT"/*.json "$OUT"/gdb.log "$OUT"/drive.log

REAL=~/.spacecrafter
CFG_IN=$(md5sum "$REAL/config.ini" | cut -d' ' -f1)
SSY_IN=$(md5sum "$REAL/ssystem.ini" | cut -d' ' -f1)
echo "config.ini md5 (in) = $CFG_IN"
echo "ssystem.ini md5 (in) = $SSY_IN"
echo "binary = $BIN"; ls -l --time-style=+%H:%M:%S "$BIN"

# §11.121(m): no other instance, ANY account, before a measurement launch.
if pgrep -f 'spacecrafter$' > /dev/null || pgrep -x spacecrafter > /dev/null; then
    echo "CONCURRENT INSTANCE PRESENT - refusing to launch (§11.121(m))"
    pgrep -a -x spacecrafter
    exit 5
fi

FARM="$OUT/farm"
python3 -c "
import sys; sys.path.insert(0, '$HERE')
import b25_galactic as b
print(b.build_farm(farm='$FARM', dotted=False, corpus=None))
" > "$OUT/farm.path" || { echo "farm build failed"; exit 1; }
DST=$(cat "$OUT/farm.path")
echo "farm = $DST"

FIFO="$OUT/gdbin"
rm -f "$FIFO"; mkfifo "$FIFO"

cd "$DST" || exit 1
DISPLAY=${DISPLAY:-:2} HOME="$FARM" gdb -q "$BIN" < "$FIFO" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; echo "gdb pid=$GDBPID"
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

python3 "$HERE/f23_b33_inject.py" "$OUT" "$GDBPID" "$FIFO" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 2; echo "quit" >&3; sleep 3; kill -9 $GDBPID 2>/dev/null; }
exec 3>&-
pkill -9 -x spacecrafter 2>/dev/null
rm -f "$FIFO"

CFG_OUT=$(md5sum "$REAL/config.ini" | cut -d' ' -f1)
SSY_OUT=$(md5sum "$REAL/ssystem.ini" | cut -d' ' -f1)
echo "config.ini md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
echo "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || { echo "FROZEN CORPUS WRITTEN"; exit 3; }
echo "--- driver result ---"
grep -aE '^(PASS|FAIL|--|OK|[0-9]+ FAILS)' "$OUT/drive.log" || tail -20 "$OUT/drive.log"
exit $RC
