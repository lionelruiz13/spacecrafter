#!/bin/bash
# F38 — D15(d): the config channel of the camera-state init, on a TEMP-HOME farm.
#
# The FIELD config (`~/.spacecrafter/config.ini`, pristine md5 03fbee59) is never
# touched: the farm is a symlink mirror of ~/.spacecrafter with config.ini and
# log/ as REAL files, so the app reads a modified config and writes its log
# beside it while every data directory stays the shipped one (11 GiB — copying
# it is not an option, and a copy would answer a different question anyway).
#
#   ./f38_config.sh <leg>
#     default    pristine config, NO new key      -> today's behaviour, no D12 log
#     acting     attached=false + lock=true       -> both act, both LOGGED (D12)
#     probe_same unknown key, version UNCHANGED   -> D13: key survives untouched
#     probe_ver  unknown key, version BUMPED      -> D13: CheckConfig's full pass
#
# `probe_same`/`probe_ver` answer D13 (an older parser must tolerate the new
# key) WITHOUT an old binary, and the substitution is exact rather than
# convenient: "unknown key" is a property of the KEY SET the running build
# registers, not of the build's age, and the code path an old build would take
# is the same CheckConfig/InitParser code this one runs. The probe key
# `f38_probe_unknown` is unknown to THIS build in exactly the way `attached`
# would be unknown to a pre-F38 one. (Veto point: it does not cover a
# hypothetical old build whose parser differs in more than its key table.)
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LEG=${1:?leg: default|acting|probe_same|probe_ver}
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
FARM=/tmp/f38home
OUT="$HERE/artifacts/f38/cfg_$LEG"
mkdir -p "$OUT"; rm -f "$OUT"/*.json "$OUT"/*.log "$OUT"/*.txt
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

FIELD=~/.spacecrafter/config.ini
FIELD_IN=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "=== F38 config leg '$LEG' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "field config md5 (in) = $FIELD_IN"

n=0; for p in /proc/[0-9]*; do [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && n=$((n+1)); done
log "concurrent instances (pre) : $n"
[ "$n" -eq 0 ] || { log "ABORT: another spacecrafter is running"; exit 2; }

# --- build the farm -------------------------------------------------------
rm -rf "$FARM"; mkdir -p "$FARM/.spacecrafter"
for f in ~/.spacecrafter/*; do
    b=$(basename "$f")
    [ "$b" = "config.ini" ] && continue
    [ "$b" = "log" ] && continue
    ln -s "$f" "$FARM/.spacecrafter/$b"
done
mkdir -p "$FARM/.spacecrafter/log"
cp "$FIELD" "$FARM/.spacecrafter/config.ini"
CFG="$FARM/.spacecrafter/config.ini"

ins() {  # ins <key line>  — insert into [navigation]
    python3 - "$CFG" "$1" <<'PY'
import sys
p, line = sys.argv[1], sys.argv[2]
d = open(p, encoding='iso-8859-1').read().split('\n')
i = d.index('[navigation]')
d.insert(i + 1, line)
open(p, 'w', encoding='iso-8859-1').write('\n'.join(d))
PY
}

case "$LEG" in
    default)    ;;
    acting)     ins "attached                       = false"
                ins "flag_lock_sky_position         = true" ;;
    probe_same) ins "f38_probe_unknown              = 1" ;;
    probe_ver)  ins "f38_probe_unknown              = 1"
                sed -i 's/^version .*= .*/version                        = 0000.00.00/' "$CFG" ;;
esac
log "--- [navigation] of the farm config, before the run ---"
sed -n '/^\[navigation\]/,/^\[/p' "$CFG" | tee -a "$META"
cp "$CFG" "$OUT/config_before.ini"
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); log "farm config md5 (in) = $CFG_IN"

# --- run ------------------------------------------------------------------
cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
run
GDB
HOME=$FARM DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
UP=no
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; UP=yes; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; break; }
done
if [ "$UP" = yes ]; then
    sleep 8
    python3 "$HERE/f38_config_probe.py" "$OUT" > "$OUT/drive.log" 2>&1
    log "driver exit=$?"
    sleep 2
fi
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }

# --- read back ------------------------------------------------------------
CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
log "farm config md5 (out) = $CFG_OUT  REWRITTEN=$([ "$CFG_IN" = "$CFG_OUT" ] && echo no || echo YES)"
cp "$CFG" "$OUT/config_after.ini"
FIELD_OUT=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_OUT=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "field config md5 (out) = $FIELD_OUT  MATCH=$([ "$FIELD_IN" = "$FIELD_OUT" ] && echo yes || echo NO)"
log "field ssystem md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"

LOGF=$(ls -t "$FARM"/.spacecrafter/log/spacecrafter*.log 2>/dev/null | head -1)
log "app log file     : $LOGF"
# grep-with-explicit-absence: `grep | tee` always exits 0, so an empty section
# would be indistinguishable from a broken probe. Capture, then report either
# the hits or the word NONE — the negative reading has to be a positive record.
probe() { local d="$1"; shift; local r; r=$(grep -aE "$1" "$LOGF");     if [ -n "$r" ]; then log "$d"; echo "$r" | tee -a "$META"; else log "$d"; log "  NONE"; fi; }
probe "--- app log: the D12 channel (acting configured values) ---" "config\.ini \[navigation\]"
probe "--- app log: parser missing-key warnings for the new keys ---" "configuration key .navigation:(attached|flag_lock_sky_position).|def_bool navigation"
# Positive control for the probe itself: the parser's missing-key warning DOES
# fire in this log for other keys, so a NONE above is evidence and not a dead
# grep (the §11.47 silent-no-op-probe rule).
probe "--- positive control: the same warning for ANY key ---" "can't find the configuration key"
probe "--- app log: CheckConfig key/section removals ---" "has been removed from config\.ini"
probe "--- app log: config.ini up-to-date decision ---" "config\.ini is up to date"
log "--- [navigation] of the farm config, after the run ---"
sed -n '/^\[navigation\]/,/^\[/p' "$CFG" | tee -a "$META"
log "--- camera state as the app dumped it ---"
python3 - "$OUT" <<'PY' | tee -a "$META"
import json, sys, os
p = os.path.join(sys.argv[1], "startup.json")
if not os.path.exists(p):
    print("  (no dump)"); raise SystemExit
h = json.loads(open(p).readline())
c = h["camera"]; k = h["control"]["skyLock"]
print(f"  camera.freeMode={c['freeMode']}  camera.boundToSurface={c['boundToSurface']}  "
      f"camera.skyLocked={c['skyLocked']}")
print(f"  control.skyLock reported={k['reported']} old={k['old']} new={k['new']}")
print(f"  camera.reference={c['reference']}  longitude={c['longitude']:.6f} latitude={c['latitude']:.6f} "
      f"distance={c['distance']:.9f}  position={c['position']}")
print(f"  oldView.nav.flagLockEquPos={h['oldView']['nav']['flagLockEquPos']}")
PY
log "wall clock end   : $(date '+%F %T %Z')"
