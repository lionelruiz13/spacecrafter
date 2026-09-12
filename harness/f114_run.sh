#!/bin/bash
# F114 -- S5.100 + S5.101: `zoom auto in` starts the new path's tracking and
# `zoom auto initial` re-aims the drawn view. One FRESH launch per invocation.
#
#   ./f114_run.sh <label> <driver.py> [binary] [config-sed-expr]
#     label             subdir under artifacts/f114/
#     driver.py         socket driver, run with <outdir> as argv[1]
#     binary            default $SC_BIN or ../../build-claude/src/spacecrafter
#     config-sed-expr   if given, the launch runs on a temp-HOME FARM under
#                       /home/claude/sc-f114/farm with the field config COPIED
#                       and this sed expression applied to the copy (the f38
#                       scenes pattern). The FIELD config is never written.
#
# Asserts, recorded not assumed:
#   * binary md5 + mtime;
#   * concurrent instances THREE ways -- /proc/<pid>/comm (the standing probe),
#     /proc/<pid>/exe under /home/claude/sc-*/ or */build*/src/ (the staging
#     blind spot, S11.231(j2)), and TCP 7805 held by anyone;
#   * config.ini AND ssystem.ini md5 in == out (pristine 03fbee59 / 545a51ef);
#   * the driver under an explicit timeout (Q-68).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
DRIVER=${2:?driver.py}
BIN="${3:-${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}}"
CFGSED="${4:-}"
OUT="$HERE/artifacts/f114/$LABEL"
FARM=/home/claude/sc-f114/farm
mkdir -p "$OUT"; rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log "$OUT"/*.navstr
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F114 run '$LABEL' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "driver           : $DRIVER"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"

n=0
for p in /proc/[0-9]*; do
    [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && { n=$((n+1)); log "  comm hit : $p"; }
    e=$(readlink "$p/exe" 2>/dev/null || true)
    case "$e" in
        /home/claude/sc-*/*|*/build*/src/*) n=$((n+1)); log "  exe hit  : $p -> $e";;
    esac
done
port=$(ss -ltn 2>/dev/null | grep -c ':7805 ')
log "concurrent instances (comm+exe) : $n ; port 7805 holders : $port"
[ "$n" -eq 0 ] && [ "$port" -eq 0 ] || { log "ABORT: another engine is live"; exit 2; }

vram=$(nvidia-smi --query-gpu=memory.used --format=csv,noheader,nounits)
log "gpu used (MiB)   : $vram"
[ "$vram" -le 4000 ] || { log "ABORT: VRAM used $vram MiB > 4000"; nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv | tee -a "$META"; exit 3; }

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); SSY_IN=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (in) = $CFG_IN"
log "ssystem.ini md5 (in) = $SSY_IN"

RUNHOME=$HOME
if [ -n "$CFGSED" ]; then
    rm -rf "$FARM"; mkdir -p "$FARM/.spacecrafter"
    for f in ~/.spacecrafter/*; do
        b=$(basename "$f")
        [ "$b" = "config.ini" ] && continue
        [ "$b" = "log" ] && continue
        ln -s "$f" "$FARM/.spacecrafter/$b"
    done
    mkdir -p "$FARM/.spacecrafter/log"
    cp "$CFG" "$FARM/.spacecrafter/config.ini"
    sed -i "$CFGSED" "$FARM/.spacecrafter/config.ini"
    RUNHOME=$FARM
    log "farm             : $FARM"
    log "farm sed         : $CFGSED"
    log "farm init_view_pos: $(grep -a '^init_view_pos' "$FARM/.spacecrafter/config.ini")"
    log "farm config md5  : $(md5sum "$FARM/.spacecrafter/config.ini" | cut -d' ' -f1)"
fi

cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
run
GDB
HOME=$RUNHOME DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
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

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1); SSY_OUT=$(md5sum "$SSY" | cut -d' ' -f1)
log "config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "wall clock end   : $(date '+%F %T %Z')"
echo "--- driver tail ---"
tail -8 "$OUT/drive.log"
[ "$CFG_IN" = "$CFG_OUT" ] && [ "$SSY_IN" = "$SSY_OUT" ] || exit 4
exit $RC
