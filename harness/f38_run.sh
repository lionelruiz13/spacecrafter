#!/bin/bash
# F38 — D15(c) sky-lock write-site mirroring: both-ways discrimination runner.
#
# One FRESH launch per invocation (dirty instrument state invalidates a run).
# The evidence channel is the app's own dual dump (`body action dual_dump`),
# which already carries BOTH paths' flags and view state on this binary:
#   control.skyLock {reported, old, new}   — the B33 dual readout
#   oldView.nav.{flagLockEquPos, flagTraking, equVision, localVision}
#   camera.{skyLocked, tracked, mat, alt, az}   + helioToEye
# so NO instrument code change is needed and the PRE-change binary can be
# measured with exactly the same instrument as the POST-change one.
#
#   ./f38_run.sh <label> [driver] [binary]
#     label   subdir under artifacts/f38/   (e.g. pre, post)
#
# Asserts, recorded not assumed: binary md5+mtime, concurrent-instance count by
# /proc/<pid>/comm (§11.134(b) — pgrep -f self-matches), config.ini AND
# ssystem.ini md5 in==out (pristine pair 03fbee59 / 545a51ef). The field config
# is NOT touched: every setting this run needs is sent as a command.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label (e.g. pre, post)}
DRIVER=${2:-f38_mirror.py}
BIN="${3:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
OUT="$HERE/artifacts/f38/$LABEL"
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log
META="$OUT/f38_meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F38 run '$LABEL' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "driver           : $DRIVER"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"

# Concurrent-instance probe: ANY account, by /proc/<pid>/comm, never by
# command-line text (a `pgrep -f <path>` self-matches the wrapper).
n=0
for p in /proc/[0-9]*; do
    [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && { n=$((n+1)); log "  running: $p"; }
done
log "concurrent instances (pre) : $n"
[ "$n" -eq 0 ] || { log "ABORT: another spacecrafter is running"; exit 2; }

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
DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
log "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "wall clock end   : $(date '+%F %T %Z')"
echo "--- driver tail ---"
tail -8 "$OUT/drive.log"
exit $RC
