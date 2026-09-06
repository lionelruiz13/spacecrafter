#!/bin/bash
# F99 -- the SITE attribution for S5.141's arm B, under gdb.
#
# The flushed log already names the PATH (old entered, new never entered:
# f99_locguard.py's P2.2 pair).  It cannot name the LINE, and the old path's
# `location_orbit` branch dereferences `parent` four times (protosystem.cpp:599,
# :600, :605, :607) -- so which of them faults is a claim the log cannot make.
# This leg makes it, from the faulting frame.
#
# S0's sediment, both halves:
#   * LAUNCH under gdb -- ptrace_scope is 1 on this host, so attach is blocked.
#   * PASS SIGUSR1 -- it is the app's own stall watchdog's signal and gdb stops
#     on it by default, which would look exactly like the fault we are hunting.
#
# The commands are fed over TCP by a background subshell of THIS script (one
# foreground Bash-tool call, no run_in_background -- S11.98(h)).
#
#   DISPLAY=:2 ./f99_gdb.sh <absOutdir> <binary>
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT=${1:?usage: f99_gdb.sh <absOutdir> <binary>}
BIN=${2:?usage: f99_gdb.sh <absOutdir> <binary>}
FARM=${F99_GDB_FARM:-/home/claude/sc-f99/farm_gdb}
mkdir -p "$OUT"

for p in /proc/[0-9]*/comm; do
    [ -r "$p" ] || continue
    [ "$(cat "$p" 2>/dev/null)" = "spacecrafter" ] && { echo "ABORT: instance running"; exit 2; }
done
[ -f /tmp/spacecrafter.lock ] && {
    LP=$(cat /tmp/spacecrafter.lock); [ -d "/proc/$LP" ] || rm -f /tmp/spacecrafter.lock; }

bash "$HERE/b3_farm.sh" "$FARM" >/dev/null || { echo "farm failed"; exit 1; }

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
SS_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)

cat > "$OUT/f99.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
handle SIGPIPE nostop noprint pass
run
echo \n===== SIGNAL CAUGHT =====\n
info program
echo \n===== BACKTRACE (10 innermost) =====\n
bt 10
echo \n===== FRAME 0 =====\n
frame 0
info frame
echo \n===== THE PARENT POINTER AT THE FAULT =====\n
info locals
echo \n===== SOURCE AT THE FAULT =====\n
list
kill
quit
GDB

# The driver: wait for the port, push the control, then arm B.
(
  for i in $(seq 1 90); do
      ss -ltn 2>/dev/null | grep -q ':7805 ' && break
      sleep 1
  done
  sleep 4
  exec 3<>/dev/tcp/127.0.0.1/7805 || exit 1
  printf '%s\n' "timerate rate 0" >&3; sleep 1
  printf '%s\n' "body action load name ProbeCtlC parent none type Artificial radius 50 coord_func still_orbit orbit_x 0.001 orbit_y 0 orbit_z 0 tex_map bodies/generic.png" >&3
  sleep 3
  printf '%s\n' "body action load name ProbeLocB parent none type Artificial radius 50 coord_func location_orbit orbit_lon 5.3667 orbit_lat 43.3 orbit_alt 400000 tex_map bodies/generic.png" >&3
  sleep 8
  exec 3>&- 2>/dev/null
) &
FEEDER=$!

HOME="$FARM" DISPLAY=${DISPLAY:-:2} gdb -batch -x "$OUT/f99.gdb" \
    --cd "$FARM/.spacecrafter" "$BIN" > "$OUT/gdb.log" 2>&1
GRC=$?
wait $FEEDER 2>/dev/null

cp "$FARM/.spacecrafter/log/spacecrafter.log" "$OUT/spacecrafter.log" 2>/dev/null
echo "gdb exit = $GRC"
echo "--- the faulting frame ---"
sed -n '/SIGNAL CAUGHT/,/THE PARENT POINTER/p' "$OUT/gdb.log"
echo "--- frozen pair ---"
M=$(md5sum "$CFG" | cut -d' ' -f1); S=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "config  $M  $([ "$M" = "$MD5_IN" ] && echo MATCH || echo DRIFT)"
echo "ssystem $S  $([ "$S" = "$SS_IN" ] && echo MATCH || echo DRIFT)"
