#!/bin/bash
# F26 / INTENT §5.62 - the owed isolation measurement: the same frozen mid-band
# scene on the pre-§5.52 binary AND on the §5.52-carrying child, INSIDE ONE
# EPOCH (the row's own consequence: a mid-band disc ratio is not a quantity that
# survives comparison across epochs, so both legs are taken here, minutes apart).
#
# The SCENE is F18's, unchanged: this script does not reimplement it, it drives
# f18_run.sh + f18_disc.py (harness a0671a4, one commit, no drift) and adds the
# row's own preconditions as RECORDED asserts rather than as assumptions.
#
#   ./f26_epoch.sh <label> <binary>
#     label   subdir under artifacts/f26/
#     binary  absolute path to the spacecrafter to measure
#
# Per run this records: wall clock, binary md5, concurrent-instance count,
# frozen config/ssystem md5 in and out, the texture-cache (t-*.dat) listing
# before and after, and the f18_disc.py table (which carries the old-path disc
# sums - the row's own bit-stability control - beside the new-path ones).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
BIN=${2:?binary}
OUT="$HERE/artifacts/f26/$LABEL"
mkdir -p "$OUT"
# .txt, not .log: f18_run.sh clears *.log/*.json/*.png in its outdir at start.
META="$OUT/f26_meta.txt"
: > "$META"

log() { echo "$@" | tee -a "$META"; }

log "=== F26 run $LABEL ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"

# Concurrent-instance assert (§11.121(m)) - ANY account, ANY build dir. NOT
# f18_run.sh's own pattern: that one is 'spacecrafter/build.*/src/spacecrafter'
# and the F26 binaries live OUTSIDE the code tree (/home/claude/sc-f26/build-*),
# so it would miss exactly the processes this campaign can leave behind. The
# bracket keeps the pattern from matching this script's own command line.
CONC=$(pgrep -c -f '[s]rc/spacecrafter' || true)
log "concurrent insts : ${CONC:-0}"
if [ "${CONC:-0}" != "0" ]; then log "ABORT: concurrent instance"; exit 2; fi

CFG=~/.spacecrafter/config.ini
SSY=~/.spacecrafter/ssystem.ini
log "config.ini  md5 in  : $(md5sum $CFG | cut -d' ' -f1)   (pristine 03fbee59bc3ec506c58f0a3f1e1d73df)"
log "ssystem.ini md5 in  : $(md5sum $SSY | cut -d' ' -f1)   (pristine 545a51ef76294891579a1fc2fe13792b)"
log "modularSystem enabled (must be empty): [$(ls ~/.spacecrafter/modularSystem/ 2>/dev/null | grep -v '\.disabled$' | tr '\n' ' ')]"
log "beta_features.ini   : $(test -e ~/.spacecrafter/beta_features.ini && echo PRESENT || echo absent)"
find ~/.spacecrafter -name 't-*.dat' -printf '%T@ %p\n' | sort > "$OUT/tdat_before.txt"
log "t-*.dat count/newest before: $(wc -l < "$OUT/tdat_before.txt") / $(tail -1 "$OUT/tdat_before.txt" | cut -d' ' -f2-) $(date -d @$(tail -1 "$OUT/tdat_before.txt" | cut -d' ' -f1 | cut -d. -f1) '+%F %T')"

SC_BIN="$BIN" "$HERE/f18_run.sh" "$OUT" > "$OUT/run.log" 2>&1
log "f18_run.sh exit  : $?"

log "config.ini  md5 out : $(md5sum $CFG | cut -d' ' -f1)"
log "ssystem.ini md5 out : $(md5sum $SSY | cut -d' ' -f1)"
find ~/.spacecrafter -name 't-*.dat' -printf '%T@ %p\n' | sort > "$OUT/tdat_after.txt"
if diff -q "$OUT/tdat_before.txt" "$OUT/tdat_after.txt" > /dev/null; then
    log "t-*.dat texture cache: UNCHANGED during this run"
else
    log "t-*.dat texture cache: CHANGED during this run  <-- the row's precondition"
    diff "$OUT/tdat_before.txt" "$OUT/tdat_after.txt" | tee -a "$META"
fi

python3 "$HERE/f18_disc.py" "$OUT" > "$OUT/f26_disc_result.json" 2>&1
log "f18_disc.py exit : $?"
log "--- disc table ---"
tail -5 "$OUT/f26_disc_result.json" | tee -a "$META"
log "--- md5 in==out asserted by the driver ---"
grep -a 'md5' "$OUT/drive.log" | tee -a "$META"
log "wall clock end   : $(date '+%F %T %Z')"
