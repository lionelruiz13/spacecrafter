#!/bin/bash
# F45 — §5.88's owed datum: what an EMPTY spectral array costs, measured against a
# populated one. RECORD-ONLY: no product code, no data-root write, no real-HOME edit.
#
# ============================ THE COST SHAPE (source read, before any run) =========
# `spectral_array` (hip_star_mgr.cpp:61) is read by exactly ONE function,
# `HipStarMgr::convertToSpectralType` (hip_star_mgr.cpp:74-83), which is called from
# exactly TWO sites, both text builders in hip_star_wrapper.cpp:
#   :151  StarWrapper1::getInfoString       -> Core::getSelectedObjectInfo (core.hpp:387)
#                                          -> app_command_interface.cpp:1220
#                                             = the TCP `get status object` reply. ON DEMAND.
#   :196  StarWrapper1::getShortInfoString  -> Core::getSelectedObjectShortInfo (core.hpp:398)
#                                          -> ui_tuiconf.cpp:126, inside UI::drawGravityUi
#                                          -> ui.cpp:278 (gated FlagShowGravityUi)
#                                          -> app.cpp:883, inside App::draw. PER FRAME.
# The star channel's PIXELS do not touch it: zone_array.cpp:293/300-302/312-314/382/
# 389-391/401-403 colour every star from `HipStarMgr::color_table[s->getBVIndex()]`,
# and color_table is filled from stars.ini's [colors] section (hip_star_mgr.cpp:112-128).
# With size==0 the guard at hip_star_mgr.cpp:76 takes EVERY call with getSpInt()!=0 and
# writes an L_ERROR through cLog, whose write is a line + an UNCONDITIONAL flush
# (log.cpp:152-156), under a mutex.
#
# ============================ PREDICTIONS (committed BEFORE measuring) =============
# P1  Star-channel pixels: IDENTICAL between legs. The spectral array reaches no pixel,
#     so a screenshot comparison CANNOT discriminate empty from populated. Named in
#     advance precisely so its null is a result and not an alibi.
# P2  `get status object` on a selected Star1 with getSpInt()!=0:
#       empty leg    -> the line "Spectral Type: " is PRESENT and its value is EMPTY
#                       (the `oss <<` at :151 is unconditional once getSpInt() is set).
#       populated leg-> the same line carries a token from stars_hip_sp_0v0_0.cat.
# P3  spacecrafter.log:
#       empty leg    -> >=1 line "convertToSpectralType: bad index: N, max: 0" (L_ERROR)
#       populated leg-> 0 such lines for every index N < 4122 (the file's line count);
#                       any residual would read "max: 4122" and is a DIFFERENT finding.
# P4  Positive control ON THE SAME LAUNCH, both legs: cat_hip_cids_file_name names
#     stars_hip_cids_0v0_0.cat, which DOES exist in the data root, so
#     convertToComponentIds behaves identically in both legs. If the component-id
#     branch is alive in both while the spectral one differs, the difference is
#     attributable to the spectral file alone and not to a dead readout.
# P5  Per frame (phase C: `flag show_tui_short_obj_info on` + a selected Star1):
#     the empty leg writes ONE flushed log line per drawn frame; the populated leg
#     writes none. Count over the dwell ~ frame count.
# P6  Cost on the D11 denominator (1 ms/frame): the added work is one formatted line
#     + one ofstream flush per frame, in the UI_DRAW phase. Predicted order 1-20 us,
#     i.e. 0.1-2% of the budget. MEASURED, not assumed, from statistics.dat's
#     EXECUTOR_DRAW -> UI_DRAW interval; a null there is reported as an upper bound.
#
# ============================ THE CONTROL =========================================
# §5.90's split roots, verified at source for THIS file specifically: the NAME comes
# from getConfigDir()+"stars.ini" (hip_star_mgr.cpp:452, config dir = $HOME/.spacecrafter/,
# main.cpp:185) and the FILE from getDataRoot()+"stars/" (hip_star_mgr.cpp:490), where
# getDataRoot() is the compile-time CONFIG_DATA_DIR (main.cpp:186) = /usr/local/share/
# spacecrafter/. So the split does NOT defeat the control: the data root carries
# stars_hip_sp_0v0_0.cat, and a farm stars.ini naming THAT version populates the array
# with no data-root write at all. The two legs differ in exactly one key's value.
#
#   ./f45_run.sh <leg>      leg = empty | full
#     empty : cat_hip_sp_file_name = stars_hip_sp_0v0_4.cat  (the FIELD value; absent)
#     full  : cat_hip_sp_file_name = stars_hip_sp_0v0_0.cat  (present, 4122 lines)
#
# The farm is a symlink mirror of ~/.spacecrafter with stars.ini, config.ini and log/
# as REAL files (the f38_config.sh pattern). config.ini differs from the field's in ONE
# key, [debug] query_statistics = true — that is the INSTRUMENT and it is identical in
# both legs, so it is not the variable. Field md5s are asserted in==out.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LEG=${1:?leg: empty|full}
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
FARM=/tmp/f45home
OUT="$HERE/artifacts/f45/${2:-$LEG}"   # optional 2nd arg = output tag, for A/A repeats
DATAROOT=/usr/local/share/spacecrafter
mkdir -p "$OUT"; rm -f "$OUT"/*.log "$OUT"/*.txt "$OUT"/*.ini "$OUT"/*.dat "$OUT"/*.png "$OUT"/*.json
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

case "$LEG" in
    empty) SPFILE=stars_hip_sp_0v0_4.cat ;;
    full)  SPFILE=stars_hip_sp_0v0_0.cat ;;
    *) echo "unknown leg $LEG"; exit 2 ;;
esac

log "=== F45 leg '$LEG'  (cat_hip_sp_file_name = $SPFILE) ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "binary mtime     : $(stat -c %y "$BIN")"

# --- field pristine, in -----------------------------------------------------
CFG_F=~/.spacecrafter/config.ini
SSY_F=~/.spacecrafter/ssystem.ini
STA_F=~/.spacecrafter/stars.ini
CFG_IN=$(md5sum "$CFG_F" | cut -d' ' -f1)
SSY_IN=$(md5sum "$SSY_F" | cut -d' ' -f1)
STA_IN=$(md5sum "$STA_F" | cut -d' ' -f1)
log "field config.ini  md5 (in) = $CFG_IN"
log "field ssystem.ini md5 (in) = $SSY_IN"
log "field stars.ini   md5 (in) = $STA_IN"

# --- data root, as read (never written) -------------------------------------
log "--- data root spectral/cids files, before the run ---"
ls -la "$DATAROOT/stars/" | tee -a "$META"
DR_IN=$(cd "$DATAROOT/stars" && md5sum ./* | md5sum | cut -d' ' -f1)
log "data root stars/ aggregate md5 (in) = $DR_IN"
log "requested spectral file present in data root: $([ -f "$DATAROOT/stars/$SPFILE" ] && echo YES || echo NO)"
log "spectral file line count (0 if absent) : $( [ -f "$DATAROOT/stars/$SPFILE" ] && wc -l < "$DATAROOT/stars/$SPFILE" || echo 0)"

# --- display + concurrency (positively mapped, both ways) --------------------
export XAUTHORITY=$(ls /tmp/rt-claude/.mutter-Xwaylandauth.* 2>/dev/null | head -1)
export DISPLAY=:2
log "XAUTHORITY       : ${XAUTHORITY:-<NONE>}"
[ -n "${XAUTHORITY:-}" ] || { log "ABORT: no Xwayland auth file"; exit 3; }
DPY=$(xdpyinfo 2>&1 | head -1); log "xdpyinfo         : $DPY"
echo "$DPY" | grep -q 'name of display' || { log "ABORT: display not reachable"; exit 3; }

n=0; for p in /proc/[0-9]*; do [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && n=$((n+1)); done
log "concurrent spacecrafter instances (pre) : $n"
[ "$n" -eq 0 ] || { log "ABORT: another spacecrafter is running"; exit 2; }

# --- build the farm ---------------------------------------------------------
rm -rf "$FARM"; mkdir -p "$FARM/.spacecrafter"
for f in ~/.spacecrafter/*; do
    b=$(basename "$f")
    case "$b" in config.ini|stars.ini|log) continue ;; esac
    ln -s "$f" "$FARM/.spacecrafter/$b"
done
mkdir -p "$FARM/.spacecrafter/log"
cp "$CFG_F" "$FARM/.spacecrafter/config.ini"
cp "$STA_F" "$FARM/.spacecrafter/stars.ini"
CFG="$FARM/.spacecrafter/config.ini"
STA="$FARM/.spacecrafter/stars.ini"

# the instrument (identical in both legs)
sed -i 's/^query_statistics *=.*/query_statistics               = true/' "$CFG"
# the variable (the ONE difference between legs)
sed -i "s/^cat_hip_sp_file_name *=.*/cat_hip_sp_file_name           = $SPFILE/" "$STA"

log "--- farm stars.ini [stars] section ---"
sed -n '/^\[stars\]/,/^\[/p' "$STA" | grep -v '^\[colors\]' | tee -a "$META"
log "--- farm config.ini [debug] section ---"
sed -n '/^\[debug\]/,/^\[/p' "$CFG" | tee -a "$META"
cp "$STA" "$OUT/stars_before.ini"; cp "$CFG" "$OUT/config_before.ini"
STA_FARM_IN=$(md5sum "$STA" | cut -d' ' -f1); log "farm stars.ini md5 (in) = $STA_FARM_IN"

# --- the fixed HP list, read from the INSTALLED data (cited fetch, no recall) -
# name.fab is HIP|name; the first N distinct HIPs give a deterministic, leg-independent
# sample of bright (level-0/1) stars, which are the only ones this install loads at all
# (§5.90: levels 2 and 3 fail, 26 561 stars).
awk -F'|' '{gsub(/ /,"",$1); if ($1 != "" && !seen[$1]++) print $1}' \
    "$DATAROOT/stars/name.fab" | head -24 > "$OUT/hp_list.txt"
log "HP sample (from $DATAROOT/stars/name.fab, first 24 distinct): $(tr '\n' ' ' < "$OUT/hp_list.txt")"

# --- run --------------------------------------------------------------------
LOGF="$FARM/.spacecrafter/log/spacecrafter.log"
HOME=$FARM "$BIN" > "$OUT/app.stdout.log" 2> "$OUT/app.stderr.log" &
APPPID=$!
log "app pid=$APPPID"
UP=no
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; UP=yes; break; fi
    kill -0 $APPPID 2>/dev/null || { log "app died before tcp"; break; }
done
if [ "$UP" = yes ]; then
    sleep 6
    python3 "$HERE/${F45_PROBE:-f45_probe.py}" "$OUT" "$LOGF" > "$OUT/drive.log" 2>&1
    log "driver exit=$?"
    for i in $(seq 1 20); do kill -0 $APPPID 2>/dev/null || break; sleep 1; done
fi
if kill -0 $APPPID 2>/dev/null; then
    log "WARN: app still alive after shutdown request; SIGINT then SIGKILL"
    kill -INT $APPPID; sleep 5; kill -9 $APPPID 2>/dev/null
fi
wait $APPPID 2>/dev/null
log "app exit status  : $?"

# --- collect ----------------------------------------------------------------
cp "$LOGF" "$OUT/spacecrafter.log" 2>/dev/null || log "WARN: no app log at $LOGF"
cp "$FARM/.spacecrafter/log/statistics.dat" "$OUT/statistics.dat" 2>/dev/null || log "WARN: no statistics.dat"
cp "$STA" "$OUT/stars_after.ini"; cp "$CFG" "$OUT/config_after.ini"
ls -la "$FARM/.spacecrafter/log/" | tee -a "$META"

# --- read back: nothing in the field or the data root moved -----------------
CFG_OUT=$(md5sum "$CFG_F" | cut -d' ' -f1)
SSY_OUT=$(md5sum "$SSY_F" | cut -d' ' -f1)
STA_OUT=$(md5sum "$STA_F" | cut -d' ' -f1)
DR_OUT=$(cd "$DATAROOT/stars" && md5sum ./* | md5sum | cut -d' ' -f1)
log "field config.ini  md5 (out) = $CFG_OUT  MATCH=$([ "$CFG_IN" = "$CFG_OUT" ] && echo yes || echo NO)"
log "field ssystem.ini md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "field stars.ini   md5 (out) = $STA_OUT  MATCH=$([ "$STA_IN" = "$STA_OUT" ] && echo yes || echo NO)"
log "data root stars/  md5 (out) = $DR_OUT  MATCH=$([ "$DR_IN" = "$DR_OUT" ] && echo yes || echo NO)"
STA_FARM_OUT=$(md5sum "$STA" | cut -d' ' -f1)
log "farm stars.ini md5 (out) = $STA_FARM_OUT  REWRITTEN=$([ "$STA_FARM_IN" = "$STA_FARM_OUT" ] && echo no || echo YES)"

# --- the log evidence, with explicit absence --------------------------------
# `grep | tee` always exits 0, so a silent empty section is indistinguishable from a
# broken probe: capture, then report the hits OR the word NONE (§11.47 rule).
probe() { local d="$1"; local r; r=$(grep -aE "$2" "$OUT/spacecrafter.log" 2>/dev/null); \
          log "$d"; if [ -n "$r" ]; then echo "$r" | head -8 | tee -a "$META"; \
          log "  (total matching lines: $(echo "$r" | grep -c ''))"; else log "  NONE"; fi; }
probe "--- log: convertToSpectralType bad-index reports ---" "convertToSpectralType: bad index"
probe "--- log: convertToComponentIds bad-index reports ---" "convertToComponentIds: bad index"
probe "--- log: the catch that §5.88 says never fires ---" "ERROR while loading data"
probe "--- positive control: the log channel carries OTHER errors (§5.90's fopen) ---" "fopen failed|ZoneArray::create"
probe "--- log: how many stars this install actually loaded ---" "max_geodesic_level|Loading catalog"
log "--- stderr: the catch's console half (§5.88: expected 0 in both legs) ---"
grep -ac 'ERROR while loading data' "$OUT/app.stderr.log" | sed 's/^/  count = /' | tee -a "$META"

log "wall clock end   : $(date '+%F %T %Z')"
