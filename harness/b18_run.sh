#!/bin/bash
# B18 equatorial-mount sky-lock (old flag_lock_equ_pos) observable run - one
# fresh launch (INTENT 11.58). The app runs UNDER gdb (ptrace_scope=1 blocks
# attach) with b18_probe.gdb: the breakpoint on Camera::setSkyLock is the "the
# command reached the new-path Camera" evidence that does not come from a log.
# Config.ini is not touched (settings are sent as commands); md5 asserted.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b18_skylock.py}
OUT=${2:-$HERE/artifacts/b18}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json

CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (in) = $MD5_IN"

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b18_probe.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; sed -n '1,40p' "$OUT/gdb.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $GDBPID 2>/dev/null && { echo "killing app"; kill -INT $GDBPID; sleep 5; kill -9 $GDBPID 2>/dev/null; }

MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"

echo "--- setSkyLock probe fires (should be one per real flag command) ---"
grep -a "PROBE setSkyLock" "$OUT/gdb.log" || echo "(none)"
echo "--- app exit ---"
grep -a "Inferior 1" "$OUT/gdb.log" || echo "(no inferior-exit line)"
echo "--- driver tail ---"
tail -5 "$OUT/drive.log"
