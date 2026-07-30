#!/bin/bash
# ============================================================================
# B7 hunt-3 STRESS LOAD (F8, INTENT §11.122) — reintroduces the host contention
# that §11.95(d) named as the live hypothesis for the §11.15d teardown race.
#
# §11.95(d): the ONE true fire (B23) and the §11.82(d) startup-crash streak both
# occurred in the Minecraft-JVM contention era; the quiet-host hunt saw 0/158.
# The hypothesis is that contention WIDENS the teardown window (loader/executor
# threads vs freed memory, main.cpp:352-384). Contention is therefore the
# VARIABLE UNDER TEST and must be MEASURED, not assumed — `report` prints the
# host state that every batch records.
#
# Four contention channels, chosen to cover what a JVM-class neighbour produces:
#   GPU/driver — N vkcube instances on the same DISPLAY and the same NVIDIA
#                driver the app tears down (the fire is a race AFTER the driver
#                exited: driver-thread contention is the most on-point channel).
#   CPU        — N spinners (scheduler pressure on the app's loader/executor
#                threads, which is the mechanism the hypothesis names).
#   MEMORY     — one churner touching a BOUNDED resident block (a freed-memory
#                race needs an allocator under pressure). Bounded on purpose:
#                a host OOM killed an executor 2026-07-30.
#   IO         — one dd loop (page-cache + writeback pressure).
#
# Usage:  ./b7_stress.sh start [gpu] [cpu] [mem_mb]     (defaults 3 8 2048)
#         ./b7_stress.sh report <label> <logfile>
#         ./b7_stress.sh stop
# PIDs are tracked in an explicit file and killed BY PID — never `pgrep -f` on a
# script name (§11.95(f): that self-matches the invoking shell).
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
PIDF="${B7_STRESS_PIDF:-/tmp/b7_stress.pids}"
SCRATCH="${B7_STRESS_SCRATCH:-/tmp/b7_stress_io}"

start() {
  local NGPU="${1:-3}" NCPU="${2:-8}" MEMMB="${3:-2048}"
  stop >/dev/null 2>&1
  : > "$PIDF"
  # --- memory guard: refuse to start if the host cannot afford the block ---
  local avail; avail=$(awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo)
  if [ "$avail" -lt $((MEMMB + 8192)) ]; then
    echo "REFUSING to start stress: MemAvailable ${avail}MB < ${MEMMB}+8192MB headroom"
    return 3
  fi
  local i
  for i in $(seq 1 "$NGPU"); do
    DISPLAY=${DISPLAY:-:2} vkcube >/dev/null 2>&1 &
    echo $! >> "$PIDF"
  done
  for i in $(seq 1 "$NCPU"); do
    bash -c 'while :; do :; done' >/dev/null 2>&1 &
    echo $! >> "$PIDF"
  done
  python3 -c "
import sys,time
mb=$MEMMB
b=bytearray(mb*1024*1024)
while True:
    for off in range(0, len(b), 1<<22):
        b[off:off+(1<<20)] = b'\xa5'*(1<<20)
    time.sleep(0.05)
" >/dev/null 2>&1 &
  echo $! >> "$PIDF"
  mkdir -p "$SCRATCH"
  bash -c "while :; do dd if=/dev/zero of=$SCRATCH/blob bs=1M count=512 conv=fsync 2>/dev/null; rm -f $SCRATCH/blob; done" >/dev/null 2>&1 &
  echo $! >> "$PIDF"
  sleep 8   # let the load reach steady state before anything is measured
  echo "stress started: gpu=$NGPU cpu=$NCPU mem=${MEMMB}MB io=1  pids=$(wc -l < "$PIDF")"
}

report() {
  local label="${1:-}" out="${2:-/dev/stdout}"
  {
    echo "---- HOST STATE [$label] $(date -Iseconds) ----"
    echo "loadavg: $(cut -d' ' -f1-3 /proc/loadavg)"
    free -m | sed -n '1,2p'
    echo "stress pids alive: $(alive_count)/$( [ -f "$PIDF" ] && wc -l < "$PIDF" || echo 0)"
    echo "top cpu consumers:"
    ps -eo pid,user,pcpu,pmem,comm --sort=-pcpu --no-headers | head -8
    echo "foreign users with procs: $(ps -eo user --no-headers | sort -u | tr '\n' ' ')"
    echo "spacecrafter instances: $(pgrep -x spacecrafter | tr '\n' ' ')"
    echo "userdir md5: cfg=$(md5sum ~/.spacecrafter/config.ini | cut -c1-8) sys=$(md5sum ~/.spacecrafter/ssystem.ini | cut -c1-8)"
    echo "userdir foreign mtimes: $(stat -c '%n=%y' ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tr '\n' ' ')"
  } >> "$out"
}

alive_count() {
  local n=0 p
  [ -f "$PIDF" ] || { echo 0; return; }
  while read -r p; do kill -0 "$p" 2>/dev/null && n=$((n+1)); done < "$PIDF"
  echo "$n"
}

stop() {
  local p
  if [ -f "$PIDF" ]; then
    while read -r p; do
      [ -n "$p" ] || continue
      pkill -9 -P "$p" 2>/dev/null   # dd children of the io loop
      kill -9 "$p" 2>/dev/null
    done < "$PIDF"
    rm -f "$PIDF"
  fi
  rm -rf "$SCRATCH"
  echo "stress stopped"
}

case "${1:-}" in
  start)  shift; start "$@" ;;
  report) shift; report "$@" ;;
  stop)   stop ;;
  *) echo "usage: $0 start|report <label> <log>|stop"; exit 2 ;;
esac
