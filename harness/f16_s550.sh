#!/bin/bash
# ============================================================================
# F16 (INTENT §11.124) — §5.50's own discriminating check.
#
# The row's reproduction, VERBATIM from §5.50: a script pushes a body whose
# coord_func is an orbit family only the experimental path knows. The old
# path's chain of responsibility does not handle it, used to log and then
# carry on with a null orbit, and the app died mid-session.
#
# Three legs in ONE launch, so nothing is compared across runs:
#   1. THE REPRO   push ProbeA with coord_func = surface_point.
#                  PRE-fix: the app dies here. POST-fix: it must survive, log
#                  the §2(f) diagnostic naming body + value + valid values,
#                  skip the body on the old path — and the EXPERIMENTAL path
#                  must still create it, because SSystemFactory::addBody feeds
#                  both from the same map and the old path returning is
#                  exactly what lets the new one run.
#   2. CONTROL A   push ProbeB with coord_func = still_orbit, a value the old
#                  path DOES know: the bail-out must not have broken the
#                  ordinary push channel — the body must land on BOTH paths.
#   3. CONTROL B   the app must still be alive and drawing afterwards
#                  (screenshot + a dual_dump that lists the shipped corpus).
# Liveness is asserted PID-pinned, and each leg is timestamped in the log so a
# death can be attributed to the leg that caused it.
#
# Usage: DISPLAY=:2 [SC_BIN=...] ./f16_s550.sh <outdir>
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f16_s550}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"
# ABSOLUTE, always: the app's cwd is ~/.spacecrafter, so a relative filename
# handed to `body action screenshot` / `dual_dump` resolves THERE, the write
# fails on a missing directory, and the check reads as "no output" - a probe
# that cannot succeed (measured 2026-07-31: the first run of this script wrote
# neither shot nor dump for exactly this reason).
OUT=$(cd "$OUT" && pwd)
rm -f "$OUT"/*.png "$OUT"/*.log "$OUT"/*.json

MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
SSMD5_REF=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "binary: $BIN  ($(stat -c %y "$BIN"))"
echo "config md5=$MD5_REF  ssystem md5=$SSMD5_REF"
pgrep -x spacecrafter >/dev/null && { echo "FATAL: a spacecrafter instance is already running"; exit 2; }

tcp_send() {
  { exec 3<>/dev/tcp/127.0.0.1/7805 || return 1
    local c
    for c in "$@"; do printf '%s\n' "$c" >&3; sleep 0.25; done
    sleep 0.3
    exec 3>&-
  } 2>/dev/null
}
alive() { kill -0 "$APPPID" 2>/dev/null && echo ALIVE || echo DEAD; }

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 1
    ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "tcp up after ${i}s"; break; }
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; exit 1; }
done
sleep 5
tcp_send "flag experimental_path on" "timerate rate 0" "meteors zhr 0" "date jday 2461234.0"
sleep 2
echo "before any push: $(alive)"

# ---- LEG 1: the §5.50 reproduction, verbatim ----
echo "=== LEG 1: push surface_point (the repro) ==="
tcp_send "body action load name ProbeA parent Earth type Artificial radius 50 coord_func surface_point orbit_lon 5.3667 orbit_lat 43.3 orbit_alt 400000 tex_map bodies/generic.png"
sleep 4
L1=$(alive); echo "after surface_point push: $L1"
[ "$L1" = DEAD ] && { echo "*** THE §5.50 KILL REPRODUCED ***"; tail -6 "$OUT/app.log"; }

# ---- LEG 2: control - a coord_func the old path knows ----
if [ "$L1" = ALIVE ]; then
  echo "=== LEG 2: push still_orbit (control: the ordinary push channel) ==="
  tcp_send "body action load name ProbeB parent Earth type Artificial radius 50 coord_func still_orbit orbit_x 0.001 orbit_y 0 orbit_z 0 tex_map bodies/generic.png"
  sleep 4
  echo "after still_orbit push: $(alive)"

  # ---- LEG 3: control - still alive, still drawing, corpus intact ----
  echo "=== LEG 3: control - the app still draws ==="
  tcp_send "select planet Earth" "flag track_object on" "moveto altitude 2e7 duration 0"
  sleep 3
  tcp_send "body action screenshot filename $OUT/shot_after_push.png"
  sleep 2
  tcp_send "body action dual_dump filename $OUT/dump.json"
  sleep 3
  tcp_send "flag track_object off"
  sleep 1
  echo "before teardown: $(alive)"
  tcp_send "shutdown action now"
fi

EX=0
for i in $(seq 1 45); do kill -0 $APPPID 2>/dev/null || { EX=1; break; }; sleep 1; done
if [ "$EX" != 1 ]; then echo "!! HUNG"; kill -9 $APPPID 2>/dev/null; RC=hung
else wait $APPPID 2>/dev/null; RC=$?; fi
echo "exit rc=$RC"

echo "--- LEG 1 evidence: the old path's diagnostic (§2(f): what fired, valid values, fix) ---"
grep -a "ProbeA" "$OUT/app.log" | head -6
echo "--- LEG 1 evidence: did the EXPERIMENTAL path still create ProbeA? ---"
echo "'Loading body ProbeA' lines: $(grep -ac 'Loading body ProbeA' "$OUT/app.log")"
echo "--- LEG 2 evidence: the ordinary push channel ---"
echo "'Loading new Stellar System object... ProbeB' (old path): $(grep -ac 'Stellar System object... ProbeB' "$OUT/app.log")"
echo "'Loading body ProbeB' (new path): $(grep -ac 'Loading body ProbeB' "$OUT/app.log")"
echo "dump body lines total: $(grep -ac '"type": *"body"' "$OUT/dump.json" 2>/dev/null || echo 0)"
# The dual_dump carries BOTH halves per body and a null half where a path has
# no such body (§11.3 class), which makes it the direct witness for this check:
# ProbeA must have NO old half and a live new one (skipped here, created there),
# ProbeB must have both (the ordinary push channel untouched).
python3 - "$OUT/dump.json" <<'PY'
import json, sys
want = {"ProbeA": (True, False), "ProbeB": (False, False)}  # (old is None, new is None)
seen = {}
for line in open(sys.argv[1]):
    l = line.strip().rstrip(",")
    if "Probe" not in l:
        continue
    try:
        o = json.loads(l)
    except Exception:
        continue
    if o.get("type") == "body" and o.get("name") in want:
        seen[o["name"]] = (o.get("old") is None, o.get("new") is None)
for name, exp in want.items():
    got = seen.get(name)
    ok = "OK  " if got == exp else "FAIL"
    print(f"  {ok} {name}: old_absent/new_absent = {got}, expected {exp}")
PY
echo "--- shot ---"
ls -la "$OUT"/*.png 2>/dev/null
echo "--- frozen pair in==out ---"
M=$(md5sum "$CFG" | cut -d' ' -f1); S=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "config $M $([ "$M" = "$MD5_REF" ] && echo MATCH || echo DRIFT)"
echo "ssystem $S $([ "$S" = "$SSMD5_REF" ] && echo MATCH || echo DRIFT)"
