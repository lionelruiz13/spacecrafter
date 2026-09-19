#!/bin/bash
# F122 -- the go-to's 156 000 km: the adjacency arm F121's elimination never
# ran. One FRESH launch per invocation, with the per-frame seam recorder ARMED
# (SC_SEAM_RECORD=1, core.cpp Core::init) -- which since INTENT S11.246 also
# carries `seam.travels`, one record per travel INSTALL per registry.
#
#   ./f122_run.sh <label> <driver.py> [binary]
#     label      subdir under artifacts/f122/  (also the F122_LEG default)
#     driver.py  socket driver, run with <outdir> as argv[1]
#     binary     default $SC_BIN or ../../build-claude/src/spacecrafter
#
# f121_run.sh's structure, with ONE difference that matters and one that does
# not. The one that matters: the two per-launch gates the F122 dispatch makes
# standing -- `f116_assert.sh` AND the `--no-scene` canary -- run HERE, before
# every launch, so no leg of this task can be run without them and a reader can
# see in meta.txt that they ran. (f121_run.sh inlined the instance + GPU halves
# of f116_assert.sh; calling the helper is the same three channels from their
# one home, I2.) The one that does not: the artifact root.
#
# Asserts, recorded and not assumed: binary md5 + mtime; f116_assert.sh (comm |
# /proc/<pid>/exe | TCP 7805 instance probe + GPU headroom, S11.238); the
# `--no-scene` environment canary (S11.176 -- non-zero is a STOP, never a
# re-bank); config.ini AND ssystem.ini md5 in == out (pristine 03fbee59 /
# 545a51ef); the driver under an explicit plain `timeout` (never -s KILL, which
# bounds nothing in this session type, S0.5(b)).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
DRIVER=${2:?driver.py}
BIN="${3:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
OUT="$HERE/artifacts/f122/$LABEL"
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F122 run '$LABEL' ==="
log "wall clock start : $(LC_ALL=C date '+%F %T %Z')"
log "driver           : $DRIVER   leg=${F122_LEG:-$LABEL}"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(LC_ALL=C stat -c %y "$BIN")"
log "seam recorder    : SC_SEAM_RECORD=1 (armed)"

ASSERT=$(bash "$HERE/f116_assert.sh" "$LABEL" 2>&1); ARC=$?
log "$ASSERT"
[ "$ARC" -eq 0 ] || { log "ABORT: f116_assert.sh exit $ARC (2=engine live, 3=GPU)"; exit 2; }

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

# PRE-REGISTERED DISCRIMINATOR (prediction.txt M1): the D8 use-site barrier
# REFUSES a use whose parent has published no frame for its parked children and
# says so once per body (ModularBody.cpp:503). If the point anchor's position
# reads as the root origin because of THAT, this line is in the log; if it
# reads as the origin for any other reason, it is not. Counted either way, so
# the zero is a measurement and not an absence (Q-67).
APPLOG=~/.spacecrafter/log/spacecrafter.log
log "D8 'has not published a position frame' lines : $(/usr/bin/grep -c 'has not published a' "$APPLOG" 2>/dev/null || echo 0)"
log "  of which name temp_point                    : $(/usr/bin/grep -c "Position of 'temp_point' was used" "$APPLOG" 2>/dev/null || echo 0)"

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "wall clock end   : $(LC_ALL=C date '+%F %T %Z')"
echo "--- driver tail ---"
tail -8 "$OUT/drive.log"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || exit 4
exit $RC
