#!/bin/bash
# F123 -- the body that disappears when too close (INTENT S11.248, the hole
# ruled on at S11.113(c)(ii)).  One FRESH launch per invocation.
#
#   ./f123_run.sh <label> [binary]
#     label   subdir under artifacts/f123/ (also the F123_LEG default)
#     binary  default $SC_BIN or ../../build-claude/src/spacecrafter
#
# f122_run.sh's shape, kept deliberately: the two per-launch gates this round
# makes standing -- `f116_assert.sh` (comm | /proc/<pid>/exe | TCP 7805 instance
# probe + GPU headroom, S11.238) AND the `--no-scene` environment canary
# (S11.176; non-zero is a STOP, never a re-bank) -- run HERE, before every
# launch, so no leg of this task can be run without them and a reader can see in
# meta.txt that they ran.  The binary's md5 is recorded per run because F122's
# contaminated arm (S11.246(h)) was caught by exactly that line.
#
# The seam recorder is NOT armed: this task measures what is DRAWN, not what the
# two camera registries computed, and an unused per-frame ring is one more thing
# the frame does while the measurement runs.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
BIN="${2:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
OUT="$HERE/artifacts/f123/$LABEL"
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F123 run '$LABEL' ==="
log "wall clock start : $(LC_ALL=C date '+%F %T %Z')"
log "leg              : ${F123_LEG:-$LABEL}"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(LC_ALL=C stat -c %y "$BIN")"

ASSERT=$(bash "$HERE/f116_assert.sh" "$LABEL" 2>&1); ARC=$?
log "$ASSERT"
[ "$ARC" -eq 0 ] || { log "ABORT: f116_assert.sh exit $ARC (2=engine live, 3=GPU, 4=probe)"; exit 2; }

CAN=$(bash "$HERE/f56_canary.sh" --no-scene 2>&1 | tail -4); CRC=$?
log "--- canary --no-scene (exit $CRC) ---"
log "$CAN"
[ "$CRC" -eq 0 ] || { log "ABORT: canary non-zero -- STOP and report (S11.174(h)), never mitigate"; exit 5; }

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
UP=0
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; UP=1; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
[ "$UP" = 1 ] || { log "ABORT: no tcp"; kill -9 $GDBPID 2>/dev/null; exit 1; }

# The `default` leg grabs the launch transient and must NOT wait it out; every
# other leg lets the initial async texture loads quiesce first (f122_run.sh's 8 s).
case "${F123_LEG:-$LABEL}" in
    default*) log "no quiesce sleep (this leg measures the launch transient)";;
    *) sleep 8;;
esac

timeout 900 python3 "$HERE/f123_drive.py" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
log "driver exit=$RC"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "wall clock end   : $(LC_ALL=C date '+%F %T %Z')"
echo "--- driver tail ---"
tail -30 "$OUT/drive.log"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || exit 4
exit $RC
