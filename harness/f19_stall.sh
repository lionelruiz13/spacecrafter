#!/bin/bash
# ============================================================================
# F19 — §5.59's discriminating check: does a teardown request get serviced when
# the drawing worker cannot complete the frame the main loop is waiting for?
#
# §5.59's regime occurs ~1 cycle in 30 under load, which is not an instrument.
# This one is DETERMINISTIC and single-variable: a SCRATCH binary (never
# committed, built from the delivered source + one injection) parks the worker
# inside submit() SC_F19_STALL seconds after start, with the frame's
# hasCompleted never set, and releases it exactly when teardown reaches
# DrawHelper::stop(). That is the regime §11.125(i) attributed at source: the
# main loop parked in an untimed atomic wait, servicing nothing.
#
# The SAME binary carries both counterfactuals, so the arms differ by ONE
# runtime condition rather than by a build:
#   NOFIX=all      - §5.59's abandon disabled AND the shutdown-site worker stop
#                    disabled  = the delivered-before-F19 behaviour
#   NOFIX=appstop  - abandon on, shutdown-site worker stop off  = F19's first
#                    commit only (the arm the 30-cycle campaign caught a fire in)
#   NOFIX=<empty>  - both on = what F19 delivers
#
# Reported per arm: seconds from the signal to process exit, and the exit code.
# BOUND = 45 s, the same detector the B7 hunt calls HUNG.
#
# Usage: DISPLAY=:2 [STALL=25] [BOUND=45] ./f19_stall.sh <arm-label> [NOFIX]
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${F19_STALL_BIN:-/tmp/claude-1003/-home-claude-spacecrafter/4938e05f-ed9f-4612-af84-2c1b3d213193/scratchpad/sc_stall/spacecrafter}"
LABEL="${1:?arm label}"
NOFIX="${2:-}"
STALL="${STALL:-25}"
BOUND="${BOUND:-45}"
# Injection flavour: HOLD=1 parks the worker until teardown closes the queue
# (models a worker only teardown can release); SLEEP=<D> makes it merely LATE
# by D seconds (models a worker that progresses on its own).
HOLD="${HOLD:-1}"
SLEEP_D="${SLEEP_D:-}"
# RELOAD=1 drives `body action reload` continuously, so the main thread is
# parked inside the RELOAD's quiesce rather than inside App::draw - the second
# entry into the same regime (INTENT 5.58 x 5.59).
RELOAD="${RELOAD:-0}"
OUT="$HERE/artifacts/f19_stall"
mkdir -p "$OUT"
LOG="$OUT/$LABEL.log"
CFG=~/.spacecrafter/config.ini
MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)

pgrep -x spacecrafter >/dev/null && { echo "REFUSING: a spacecrafter instance is already running"; exit 3; }

echo "==== f19_stall arm=$LABEL NOFIX='${NOFIX}' stall=${STALL}s bound=${BOUND}s $(date -Iseconds)"
echo "bin md5=$(md5sum "$BIN" | cut -d' ' -f1)  cfg md5=$MD5_REF  loadavg=$(cut -d' ' -f1-3 /proc/loadavg)"

ENVV=(SC_F19_STALL="$STALL")
[ -n "$SLEEP_D" ] && ENVV+=(SC_F19_SLEEP="$SLEEP_D") || ENVV+=(SC_F19_HOLD="$HOLD")
[ -n "$NOFIX" ] && ENVV+=(SC_F19_NOFIX="$NOFIX")
echo "env: ${ENVV[*]}"
DISPLAY=${DISPLAY:-:2} env "${ENVV[@]}" "$BIN" > "$LOG" 2>&1 &
PID=$!

tcp_send() {
  { exec 3<>/dev/tcp/127.0.0.1/7805 || return 1
    local c
    for c in "$@"; do printf '%s\n' "$c" >&3; sleep 0.2; done
    sleep 0.2
    exec 3>&-
  } 2>/dev/null
}

# Wait for the app to be UP (its own TCP port) before the stall arms.
UP=0
for i in $(seq 1 60); do
  sleep 1
  ss -ltn 2>/dev/null | grep -q ':7805 ' && { UP=1; break; }
  kill -0 "$PID" 2>/dev/null || break
done
[ "$UP" = 1 ] || { echo "RESULT $LABEL: STARTUP-DIED (no TCP)"; kill -9 "$PID" 2>/dev/null; exit 4; }
echo "tcp up after ${i}s"

RELPID=""
if [ "$RELOAD" = 1 ]; then
  ( while :; do tcp_send "body action reload"; sleep 0.3; done ) &
  RELPID=$!
  echo "reload driver pid=$RELPID"
fi

# Wait until the app's OWN watchdog says the frame is stalled - the positive
# witness that the regime was entered, rather than an assumption that it was.
# The baseline matters: this host logs frame stalls during startup under load,
# so only an INCREASE after the injection is armed witnesses the injection.
BASE=$(grep -ac "Frame stall detected" "$LOG")
echo "frame-stall baseline before arming: $BASE"
ARM_T0=$(date +%s)
STALLED=0
for i in $(seq 1 120); do
  sleep 1
  [ $(( $(date +%s) - ARM_T0 )) -ge "$STALL" ] || continue
  [ "$(grep -ac 'Frame stall detected' "$LOG")" -gt "$BASE" ] && { STALLED=1; break; }
done
echo "stall witnessed=$STALLED after ${i}s (baseline $BASE)"

[ -n "$RELPID" ] && { kill -9 "$RELPID" 2>/dev/null; pkill -9 -P "$RELPID" 2>/dev/null; }
T0=$(date +%s%N)
kill -INT "$PID" 2>/dev/null
EXITED=0
for i in $(seq 1 "$BOUND"); do
  kill -0 "$PID" 2>/dev/null || { EXITED=1; break; }
  sleep 1
done
if [ "$EXITED" = 1 ]; then
  wait "$PID" 2>/dev/null; rc=$?
  T1=$(date +%s%N)
  echo "RESULT $LABEL: EXITED rc=$rc after $(( (T1-T0)/1000000 )) ms  (bound ${BOUND}s)  stall_witnessed=$STALLED"
else
  echo "RESULT $LABEL: HUNG - no exit within ${BOUND}s  stall_witnessed=$STALLED"
  kill -9 "$PID" 2>/dev/null
fi
[ -n "$RELPID" ] && { kill -9 "$RELPID" 2>/dev/null; pkill -9 -P "$RELPID" 2>/dev/null; }
echo "main-thread park site (watchdog stack, last):"
sed 's/\x1b\[[0-9;]*m//g' "$LOG" | grep -aE "^ +[0-9]+# " | tail -12 | sed 's/^/    /'
pkill -9 -x spacecrafter 2>/dev/null
sleep 1
echo "cfg md5 after=$(md5sum "$CFG" | cut -d' ' -f1) ref=$MD5_REF"
