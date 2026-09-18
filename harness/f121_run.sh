#!/bin/bash
# F121 -- the dual-camera seam. One FRESH launch per invocation, with the
# per-frame seam recorder ARMED (SC_SEAM_RECORD=1, core.cpp Core::init).
#
#   ./f121_run.sh <label> <driver.py> [binary]
#     label      subdir under artifacts/f121/
#     driver.py  socket driver, run with <outdir> as argv[1]
#     binary     default $SC_BIN or ../../build-claude/src/spacecrafter
#
# Structure and asserts are f114_run.sh's, unchanged in substance -- the one
# difference that matters is SC_SEAM_RECORD in the launch environment. Asserts,
# recorded and not assumed: binary md5 + mtime; the three-channel instance
# probe (sc_instances.sh: comm | /proc/<pid>/exe | TCP 7805, S11.238); GPU
# headroom against the canary's own BANK_GPU_NEED_MIB; config.ini AND
# ssystem.ini md5 in == out (pristine 03fbee59 / 545a51ef); the driver under an
# explicit plain `timeout` (never -s KILL, which bounds nothing in this session
# type, S0.5 (b)).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
DRIVER=${2:?driver.py}
BIN="${3:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
OUT="$HERE/artifacts/f121/$LABEL"
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F121 run '$LABEL' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "driver           : $DRIVER"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"
log "seam recorder    : SC_SEAM_RECORD=1 (armed)"

INST=$(bash "$HERE/sc_instances.sh" --assert "$LABEL" 2>&1); IRC=$?
log "$INST"
[ "$IRC" -eq 0 ] || { log "ABORT: another engine is live"; exit 2; }

GPU=$(python3 "$HERE/sc_gpu.py" --need bank --label "$LABEL" 2>&1); GRC=$?
log "$GPU"
[ "$GRC" -eq 0 ] || { log "ABORT: not enough GPU headroom to launch"; exit 3; }

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); SSY_IN=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (in) = $CFG_IN"
log "ssystem.ini md5 (in) = $SSY_IN"

cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
run
GDB
SC_SEAM_RECORD=1 DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

timeout 900 python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
log "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "armed line in app log: $(grep -c 'SC_SEAM_RECORD is set' ~/.spacecrafter/log/spacecrafter.log 2>/dev/null || echo '?')"
log "wall clock end   : $(date '+%F %T %Z')"
echo "--- driver tail ---"
tail -8 "$OUT/drive.log"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || exit 4
exit $RC
