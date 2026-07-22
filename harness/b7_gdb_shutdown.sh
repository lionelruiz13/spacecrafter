#!/bin/bash
# B7 §11.15d shutdown-segfault CONTEXT probe (2026-07-22, investigation only).
# Launches the app UNDER gdb (ptrace_scope blocks attach), waits for TCP, sends
# a CLEAN `shutdown action now`, and lets gdb catch a SIGSEGV at teardown with a
# backtrace. This is the context-capture channel the plain exit-code instrument
# lacks (no in-app SIGSEGV handler exists — signals.cpp registers only
# TSTP/CONT/TERM; ulimit -c=0 + apport core_pattern => no local core either).
# N minimal launch+shutdown cycles (the §11.47 canonical probe shape).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT="$HERE/artifacts/b7_gdb"
CFG=~/.spacecrafter/config.ini
N=${1:-4}
mkdir -p "$OUT"
MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.ref"

for c in $(seq 1 "$N"); do
  echo "======== GDB CYCLE $c ========"
  LOG="$OUT/gdb_${c}.log"
  DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b7_probe.gdb" --args "$BIN" > "$LOG" 2>&1 &
  GDBPID=$!
  UP=0
  for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "cycle $c: tcp up ${i}x2s"; UP=1; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "cycle $c: gdb died before tcp"; break; }
  done
  if [ "$UP" != 1 ]; then echo "cycle $c: no tcp"; kill -9 $GDBPID 2>/dev/null; grep -aiE "stall|SIGSEGV|Frame stall|App::draw" "$LOG" | head; continue; fi
  sleep 6
  python3 - <<'PY'
import socket,time
try:
    s=socket.create_connection(("127.0.0.1",7805),timeout=10)
    s.sendall(b"shutdown action now\n"); time.sleep(1); s.close(); print("sent shutdown")
except Exception as e:
    print("send FAILED:",e)
PY
  for i in $(seq 1 30); do kill -0 $GDBPID 2>/dev/null || break; sleep 1; done
  kill -0 $GDBPID 2>/dev/null && { echo "cycle $c: gdb still alive, killing"; kill -9 $GDBPID; }
  # Classify: clean exit vs SIGSEGV fire
  if grep -aq "exited normally" "$LOG"; then
    echo "cycle $c: CLEAN  [Inferior exited normally]"
  elif grep -aqE "SIGSEGV|Segmentation fault" "$LOG"; then
    echo "cycle $c: *** SIGSEGV FIRE ***"
    grep -aE "SIGSEGV|#[0-9]+ " "$LOG" | head -30
  else
    echo "cycle $c: OTHER (see $LOG)"; grep -aiE "stall|App::draw|exited|signal" "$LOG" | head
  fi
  cp "$OUT/config.ini.ref" "$CFG"   # clean shutdown rewrites config
  sleep 1
done
echo "======== DONE (config md5 $(md5sum "$CFG" | cut -d' ' -f1) ref=$MD5_REF) ========"
