#!/bin/bash
# Launch the engine (pid recorded), run b19_hidden_tick.py against it, signal only that pid.
#   bash b19_launch.sh <outdir> [binary]
set -u
H="$(cd "$(dirname "$0")/../.." && pwd)"
OUT=$1; BIN=${2:-$H/../../build-claude/src/spacecrafter}
mkdir -p "$OUT"
bash "$H/sc_instances.sh" --assert b19-launch || exit 2
IN=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ')
echo "binary md5 $(md5sum "$BIN" | cut -c1-8); field in $IN"
DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
PID=$!; echo "engine pid $PID"
UP=0
for i in $(seq 1 60); do sleep 2; ss -ltn 2>/dev/null | /usr/bin/grep -q ':7805 ' && { UP=1; break; }; kill -0 $PID 2>/dev/null || break; done
echo "tcp up=$UP"
RC=9
if [ $UP -eq 1 ]; then sleep 8; PYTHONDONTWRITEBYTECODE=1 timeout 400 python3 "$H/b19_hidden_tick.py" "$OUT" > "$OUT/drive.log" 2>&1; RC=$?; fi
echo "b19 exit $RC"
kill -TERM $PID 2>/dev/null; for i in 1 2 3 4 5 6 7 8 9 10; do kill -0 $PID 2>/dev/null || break; sleep 1; done
kill -0 $PID 2>/dev/null && kill -KILL $PID
sleep 2
OUTF=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ')
echo "field out $OUTF match=$([ "$IN" = "$OUTF" ] && echo yes || echo NO)"
bash "$H/sc_instances.sh" --assert b19-after; echo "after assert $?"
exit $RC
