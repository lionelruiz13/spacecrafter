#!/bin/bash
# F30 / INTENT §5.60 - WHICH allocation is the single 2.68 GB device allocation,
# and is it one buffer or one pool?
#
#   ./f30_alloc.sh <label> [icd]
#     label   subdir under artifacts/f30/
#     icd     "lvp" (software ICD, the regime §11.125(e) measured, where
#             maxMemoryAllocationSize = 0x80000000 makes the limit BITE) or
#             "native" (the proprietary driver, where the limit hides the
#             request). Default lvp.
#
# The datum is the ALLOCATION IDENTITY, not the crash: the app is driven under
# gdb (ptrace_scope=1 forbids attach, so it is launched UNDER gdb) with a
# breakpoint on every vkAllocateMemory; each size is recorded and every
# allocation above 1 GiB dumps its C++ backtrace, which names the requesting
# subsystem and discriminates a POOL CHUNK (MemoryManager::allocateChunk with
# chunkSize) from ONE RESOURCE (allocateChunk with a specificChunkSize, or
# dmalloc with a resource's own memRequirements.size).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
ICD=${2:-lvp}
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT="$HERE/artifacts/f30/$LABEL"
mkdir -p "$OUT"
META="$OUT/meta.txt"
: > "$META"
log() { echo "$@" | tee -a "$META"; }

log "=== F30 §5.60 allocation-identity run: $LABEL (icd=$ICD) ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"

# Concurrent-instance assert (§0.5, instrument per §11.134(b)): /proc/<pid>/comm,
# which covers every account and carries no command-line text to self-match.
CONC=$(/usr/bin/grep -l -x 'spacecrafter' /proc/[0-9]*/comm 2>/dev/null | wc -l)
for c in $(/usr/bin/grep -l -x 'spacecrafter' /proc/[0-9]*/comm 2>/dev/null); do
    log "  running: $c  cmdline=[$(tr '\0' ' ' < "$(dirname "$c")/cmdline")]"
done
log "concurrent insts : ${CONC:-0}"
if [ "${CONC:-0}" != "0" ]; then log "ABORT: concurrent instance"; exit 2; fi

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
log "config.ini  md5 in  : $(md5sum $CFG | cut -d' ' -f1)   (pristine 03fbee59bc3ec506c58f0a3f1e1d73df)"
log "ssystem.ini md5 in  : $(md5sum $SSY | cut -d' ' -f1)   (pristine 545a51ef76294891579a1fc2fe13792b)"

# Display: this session type inherits a WRONG XAUTHORITY (harness/README.md).
export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.* 2>/dev/null | head -1)
# Force :2 - this session type inherits DISPLAY=:0, which does not exist here;
# ${DISPLAY:-:2} would silently keep the wrong one (measured: "ABORT: no display").
export DISPLAY=${F30_DISPLAY:-:2}
export XDG_RUNTIME_DIR=/run/user/$(id -u)
log "XAUTHORITY       : $XAUTHORITY"
log "DISPLAY          : $DISPLAY"
if ! xdpyinfo >/dev/null 2>&1; then log "ABORT: no display"; exit 3; fi
log "display          : OK ($(xdpyinfo | /usr/bin/grep -m1 'dimensions:' | tr -s ' '))"

if [ "$ICD" = "lvp" ]; then
    export VK_DRIVER_FILES=/usr/share/vulkan/icd.d/lvp_icd.json
    export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json
    log "ICD              : $VK_DRIVER_FILES (llvmpipe; maxMemoryAllocationSize 0x80000000)"
else
    log "ICD              : loader default (proprietary driver)"
fi

cd "$HERE"
log "cwd              : $(pwd)"
log "--- gdb run (timeout ${F30_TIMEOUT:-240}s) ---"
# NOT `timeout -s KILL`: measured in this session type, `timeout -s KILL 3
# sleep 30` returns rc=124 only after the child's FULL 30 s - it reports the
# timeout and does not terminate the child, so it bounds NOTHING (this cost the
# native1 run 10 m 39 s and needed a manual kill). Plain `timeout` (SIGTERM) is
# measured WORKING on the same shape: `timeout 3 sleep 20` -> rc=124 at 3 s.
# A leftover sweep follows, since a killed gdb orphans its inferior.
timeout "${F30_TIMEOUT:-240}" \
    gdb -batch -x "$HERE/f30_alloc.gdb" --args "$BIN" \
    > "$OUT/gdb.log" 2> "$OUT/gdb.err"
RC=$?
log "gdb rc           : $RC  (124 = bounded by timeout, i.e. the app was still running)"
for c in $(/usr/bin/grep -l -x 'spacecrafter' /proc/[0-9]*/comm 2>/dev/null); do
    p=$(basename "$(dirname "$c")")
    log "  sweeping orphaned inferior pid $p"
    kill -9 "$p" 2>/dev/null
done
log "wall clock end   : $(date '+%F %T %Z')"
log "config.ini  md5 out : $(md5sum $CFG | cut -d' ' -f1)"
log "ssystem.ini md5 out : $(md5sum $SSY | cut -d' ' -f1)"
log "alloc lines      : $(/usr/bin/grep -c '^ALLOC ' "$OUT/gdb.log" 2>/dev/null || echo 0)"
log "big allocations  : $(/usr/bin/grep -c '^=== BIG ALLOCATION' "$OUT/gdb.log" 2>/dev/null || echo 0)"
log "2684360960 seen  : $(/usr/bin/grep -c 'size=2684360960' "$OUT/gdb.log" 2>/dev/null || echo 0)"
echo "artifacts in $OUT"
