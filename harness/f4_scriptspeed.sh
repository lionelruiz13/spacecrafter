#!/bin/bash
# INTENT §11.108 rider - `script speed faster|slower|default` reports FAILURE
# for a command that worked (app_command_interface.cpp:2421-2434 applies the
# speed change, then falls through to an unconditional
# `debug_message = "missing action argument"`). Two consequences, both on the
# operator's channel: the script log says "Could not execute", and
# `executeCommandStatus()` skips `recordCommand`, so the command is DROPPED
# from a recorded show (app_command_interface.cpp, executeCommandStatus).
#
# The observable is the SCRIPT log. Bounded to this run by byte offset, since
# the file is appended across launches.
#
# usage: f4_scriptspeed.sh <outdir>      SC_BIN=<path> to test another binary.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f4/scriptspeed}
mkdir -p "$OUT"
LOG=$(ls -t ~/.spacecrafter/log/script-*.log 2>/dev/null | head -1)
[ -n "$LOG" ] || { echo "no script log yet - it is created at first launch"; LOG=/dev/null; }
CFG=~/.spacecrafter/config.ini
MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
echo "binary = $BIN"; ls -l --time-style=+%H:%M:%S "$BIN"
echo "config.ini md5 (in) = $MD5_IN"

pkill -9 -x spacecrafter 2>/dev/null; sleep 2
OFF=$( [ -f "$LOG" ] && stat -c%s "$LOG" || echo 0 )
DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 40); do
    sleep 2
    ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "tcp up after ${i}x2s"; break; }
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; exit 1; }
done
sleep 6
LOG=$(ls -t ~/.spacecrafter/log/script-*.log 2>/dev/null | head -1)   # may have rotated in
python3 - "$OUT" <<'PY'
import socket, time, sys
s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
for c in ("script speed faster", "script speed slower", "script speed default",
          "script speed bogus_value"):
    s.sendall((c + "\n").encode()); time.sleep(1.2)
    print(">>", c, flush=True)
s.close()
PY
sleep 3
kill -INT $APPPID 2>/dev/null; sleep 4; kill -9 $APPPID 2>/dev/null
sleep 1

echo "--- script-log lines produced by this run ---"
tail -c +$((OFF+1)) "$LOG" > "$OUT/script_tail.log" 2>/dev/null
grep -a -E "script speed|missing action|unknown parameter" "$OUT/script_tail.log" || echo "(none)"
# The refusal shape changed at F73 (INTENT 11.194): a TCP-origin refusal is ONE
# line, `Error executing tcp#<id>: script speed ... #! <message>`, and the old
# `Could not execute: <line>` companion is gone for it. Both shapes are counted
# so the 4-vs-1 split below keeps meaning the same thing on either binary.
N=$(grep -acE "Could not execute: script speed|Error executing .*: script speed" \
        "$OUT/script_tail.log" 2>/dev/null || echo 0)
echo "COULD_NOT_EXECUTE_COUNT=$N   (pre-fix expectation: 4 = every leg incl. the bogus one;"
echo "                              post-fix expectation: 1 = the bogus leg ONLY)"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config.ini md5 (out) = $MD5_OUT  MATCH=$([ "$MD5_IN" = "$MD5_OUT" ] && echo yes || echo NO)"
