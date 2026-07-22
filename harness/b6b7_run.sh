#!/bin/bash
# B6 (scene-D view-roll reproduction attempt) + B7 (§11.15d shutdown-segfault
# data point) combined runner — INVESTIGATION ONLY (2026-07-22).
# B6: run the full drive_scenes.py sequence (reproduces the exact tracking-ease
#     history that led to the flake gen_mars_2 = scene D date 2), preserve the
#     date-2 dump per run, so predict.py can measure the P5 view term Dc.
# B7: end each run with a CLEAN `shutdown action now` and capture the APP's own
#     exit code (positive instrument: SIGSEGV -> 139; clean -> 0). ulimit -c=0 +
#     apport core_pattern => no local core file is ever produced, so the exit
#     code is the ONLY positive channel (recorded).
# Config is NOT modified (init_fov stays at the flake's 180; Dc is projection-
# free). Config is restored byte-identical after each clean shutdown (the app
# rewrites config.ini on clean exit).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT="$HERE/artifacts/b6b7"
CFG=~/.spacecrafter/config.ini
NRUNS=${1:-3}
mkdir -p "$OUT"
MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.ref"
echo "config md5 ref=$MD5_REF   binary mtime=$(stat -c %y "$BIN")   HEAD=$(git -C "$HERE" rev-parse --short HEAD)"

for run in $(seq 1 "$NRUNS"); do
  echo "================ RUN $run ================"
  rm -f /tmp/gen_mars_2.json /tmp/gen_a.json
  DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app_run${run}.log" 2>&1 &
  APPPID=$!
  echo "app pid=$APPPID"
  UP=0
  for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; UP=1; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app DIED before tcp (run $run)"; break; }
  done
  if [ "$UP" != 1 ]; then echo "run $run: no tcp, skipping"; kill -9 $APPPID 2>/dev/null; continue; fi
  sleep 8   # async texture quiesce

  python3 "$HERE/drive_scenes.py" > "$OUT/drive_run${run}.log" 2>&1
  echo "driver exit=$?"
  # preserve the scene-D date-2 dump (the flake condition) + scene A for context
  [ -f /tmp/gen_mars_2.json ] && cp /tmp/gen_mars_2.json "$OUT/run${run}_mars2.json"
  [ -f /tmp/gen_a.json ] && cp /tmp/gen_a.json "$OUT/run${run}_a.json"

  # B7: clean shutdown, capture the APP exit code
  python3 - <<'PY'
import socket,time
try:
    s=socket.create_connection(("127.0.0.1",7805),timeout=10)
    s.sendall(b"shutdown action now\n"); time.sleep(1); s.close()
    print("sent shutdown action now")
except Exception as e:
    print("shutdown send FAILED:",e)
PY
  # wait for the app to actually exit, capture its code
  wait $APPPID
  RC=$?
  echo "run $run: APP EXIT CODE = $RC  $([ $RC -eq 0 ] && echo '(clean)' || echo "(NONZERO — signal $((RC-128)) if >128)")"
  # restore config byte-identical (clean shutdown rewrote it)
  cp "$OUT/config.ini.ref" "$CFG"
  echo "config restored md5=$(md5sum "$CFG" | cut -d' ' -f1) (ref=$MD5_REF)"
  sleep 1
done
echo "================ DONE ================"
echo "final config md5=$(md5sum "$CFG" | cut -d' ' -f1) ref=$MD5_REF"
