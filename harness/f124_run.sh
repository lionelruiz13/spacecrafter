#!/bin/bash
# F124 -- one FRESH launch under a FIFO-fed gdb, for any driver that injects calls.
#
#   ./f124_run.sh <label> <driver.py> [driver args...]
#     label       subdir under artifacts/f124/
#     driver.py   run as: driver.py <outdir> <gdb_pid> <gdb_fifo> [driver args...]
#                 (the signature of b21_keypath.py, which this runner can run as is)
#     SC_BIN      another binary; F124_TIMEOUT the driver's bound in seconds (default 480)
#     F124_OUT_ROOT  another output root (default artifacts/f124): records land in <root>/<label>
#
# What it is made of, and why it exists: the launch asserts are f121_run.sh's (binary md5,
# the three-channel instance probe, the GPU gate, config.ini AND ssystem.ini md5 in == out,
# the driver under a plain `timeout`); the gdb on a FIFO is b21_keypath_run.sh's. No runner
# had both, and each FIFO runner hardcodes its driver. Teardown signals this launch's own
# pids, never a name.
# Exit: the driver's, or 2 engine live, 3 no GPU room, 4 field pair changed, 5 engine left over.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
DRIVER=${2:?driver.py}
shift 2
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT="${F124_OUT_ROOT:-$HERE/artifacts/f124}/$LABEL"
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log "$OUT"/gdbin
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F124 run '$LABEL' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "driver           : $DRIVER $*"
log "driver md5       : $(md5sum "$HERE/$DRIVER" | cut -d' ' -f1)"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"
log "display          : DISPLAY=${DISPLAY:-unset} $(LC_ALL=C xdpyinfo 2>/dev/null | /usr/bin/grep -m1 dimensions | tr -s ' ')"

INST=$(bash "$HERE/sc_instances.sh" --assert "$LABEL" 2>&1); IRC=$?
log "$INST"
[ "$IRC" -eq 0 ] || { log "ABORT: instance probe exit $IRC"; exit 2; }

GPU=$(python3 "$HERE/sc_gpu.py" --need bank --label "$LABEL" 2>&1); GRC=$?
log "$GPU"
[ "$GRC" -eq 0 ] || { log "ABORT: not enough GPU headroom to launch"; exit 3; }

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); SSY_IN=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (in) = $CFG_IN"
log "ssystem.ini md5 (in) = $SSY_IN"

FIFO="$OUT/gdbin"
mkfifo "$FIFO"
DISPLAY=${DISPLAY:-:2} gdb -q "$BIN" < "$FIFO" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
# Keep one writer open: gdb quits on EOF of its stdin.
exec 3> "$FIFO"
{
  echo "set pagination off"
  echo "set confirm off"
  echo "set height 0"
  echo "handle SIGUSR1 nostop noprint pass"
  echo "run"
} >&3
log "launch wall clock: $(date '+%F %T %Z')"

UP=0
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | /usr/bin/grep -q ':7805 '; then log "tcp up after ${i}x2s"; UP=1; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; break; }
done
RC=1
if [ "$UP" -eq 1 ]; then
    sleep 8   # let the initial async texture loads quiesce
    ENGINE_PIDS=$(pgrep -P $GDBPID | tr '\n' ' ')
    log "engine pid(s) under gdb: $ENGINE_PIDS"
    timeout "${F124_TIMEOUT:-480}" python3 "$HERE/$DRIVER" "$OUT" "$GDBPID" "$FIFO" "$@" > "$OUT/drive.log" 2>&1
    RC=$?
    log "driver exit=$RC"
fi
sleep 2
ENGINE_PIDS=${ENGINE_PIDS:-$(pgrep -P $GDBPID | tr '\n' ' ')}
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 2; echo "quit" >&3; sleep 3; }
exec 3>&-
kill -0 $GDBPID 2>/dev/null && kill -9 $GDBPID 2>/dev/null
for p in $ENGINE_PIDS; do kill -0 "$p" 2>/dev/null && kill -9 "$p" 2>/dev/null; done
rm -f "$FIFO"
sleep 2

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
LEFT=$(bash "$HERE/sc_instances.sh" --assert "$LABEL-after" 2>&1); LRC=$?
log "$LEFT"
log "wall clock end   : $(date '+%F %T %Z')"
echo "--- driver result ---"
LC_ALL=C /usr/bin/grep -aE '^(PASS|FAIL|RESID|NOTE|--|===)' "$OUT/drive.log" || tail -20 "$OUT/drive.log"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || exit 4
[ "$LRC" -eq 0 ] || exit 5
exit $RC
