#!/bin/bash
# Run F131's legs as gates: launch each leg per pin through f124_run.sh, judge it with f131_analyse.py, print its verdict lines.
#
#   bash f131_run.sh <1-6|all> <on|off|both> [--refuse] [--key]
#   on = the new path pinned, off = the old path, both = new then old; --refuse (obs 2-4): the tester's lines sent as commands,
#   line 38 inside the travel - the lost race, which the natural launch has never reached on this host; --key (obs 4): the
#   lat / lon keys held through UI::handleKeys (SHIFT+KP_6 / KP_8), not the commands they sent on 2026-09-26.
#
# SC_BIN another binary (read by f124_run.sh). Records: artifacts/f131/runs/<clock>/: canary/ and canary.log, one directory
# per launch O<obs>[_ref|_key|_refkey]_<new|old>/ (meta.txt, drive.log, dumps, shots, logs), <label>.runner.txt, summary.txt.
# Exit: 0 every gate row PASS; 1 a gate FAIL (a lost record is one); 2 usage; 6 the environment canary refused, nothing
# launched; 7 f124_run.sh refused a launch (engine live, no GPU room, field pair changed, engine left over) - the run stops.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
usage() { sed -n '4,7p' "$0" >&2; exit 2; }
[ $# -ge 2 ] && [ $# -le 4 ] || usage
OBS=$1; PIN=$2; REF=""; KEY=""
for a in "${@:3}"; do
    case "$a" in --refuse) REF=ref ;; --key) KEY=key ;; *) usage ;; esac
done
VAR=${REF}${KEY}; VAR=${VAR:-nat}
case "$OBS" in
    all) LIST="1 2 3 4 5 6"; [ "$VAR" = ref ] && LIST="2 3 4" ;;
    1|5|6) LIST=$OBS; [ -n "$REF" ] && { echo "--refuse applies to obs 2, 3, 4" >&2; exit 2; } ;;
    2|3|4) LIST=$OBS ;;
    *) usage ;;
esac
[ -n "$KEY" ] && [ "$OBS" != 4 ] && { echo "--key applies to obs 4" >&2; exit 2; }
case "$PIN" in on) PINS=on ;; off) PINS=off ;; both) PINS="on off" ;; *) usage ;; esac

umask 0002
export SC_SEAM_RECORD=1 PYTHONDONTWRITEBYTECODE=1
STAMP=$(date +%Y%m%d-%H%M%S)
ROOT="$HERE/artifacts/f131/runs/$STAMP"
mkdir -p "$ROOT"
SUM="$ROOT/summary.txt"
say() { echo "$@" | tee -a "$SUM"; }
say "=== f131_run.sh $* - $(date '+%F %T %Z') - binary ${SC_BIN:-$HERE/../../build-claude/src/spacecrafter} ==="

bash "$HERE/f56_canary.sh" --no-scene --out "$ROOT/canary" > "$ROOT/canary.log" 2>&1
CRC=$?
say "canary --no-scene exit $CRC"
[ "$CRC" -eq 0 ] || { say "STOP: the environment canary refused, nothing launched - $ROOT/canary.log"; exit 6; }

LAUNCHES=0
for o in $LIST; do
    SUF=""; [ "$VAR" != nat ] && SUF="_$VAR"
    EXTRA="${REF:+--refuse} ${KEY:+--key}"
    NEWDIR=""; OLDDIR=""
    for p in $PINS; do
        side=new; [ "$p" = off ] && side=old
        LABEL="O${o}${SUF}_${side}"
        DIR="$ROOT/$LABEL"
        F124_OUT_ROOT="$ROOT" bash "$HERE/f124_run.sh" "$LABEL" f131_obs.py --obs "$o" --pin "$p" $EXTRA > "$ROOT/$LABEL.runner.txt" 2>&1
        RC=$?
        LAUNCHES=$((LAUNCHES + 1))
        say "launch $LABEL: f124_run.sh exit $RC, field pair in == out $(/usr/bin/grep -c 'MATCH=yes' "$DIR/meta.txt" 2>/dev/null)/2"
        case $RC in
            2|4|5) say "STOP: f124_run.sh refused - $ROOT/$LABEL.runner.txt"; exit 7 ;;
            3) /usr/bin/grep -q 'ABORT: not enough GPU headroom' "$DIR/meta.txt" 2>/dev/null && { say "STOP: no GPU room - $DIR/meta.txt"; exit 7; } ;;
        esac
        python3 "$HERE/f131_analyse.py" judge "$o" "$VAR" "$side" "$DIR" 2>&1 | tee -a "$SUM"
        [ "$o" = 5 ] && python3 "$HERE/f131_analyse.py" table "$DIR"
        if [ "$side" = new ]; then NEWDIR=$DIR; else OLDDIR=$DIR; fi
    done
    if [ -n "$NEWDIR" ] && [ -n "$OLDDIR" ]; then
        python3 "$HERE/f131_analyse.py" cross "$o" "$VAR" "$NEWDIR" "$OLDDIR" 2>&1 | tee -a "$SUM"
    fi
done
NP=$(/usr/bin/grep -c '^PASS ' "$SUM"); NF=$(/usr/bin/grep -c '^FAIL ' "$SUM"); NN=$(/usr/bin/grep -c '^NOTE ' "$SUM")
LOST=$(/usr/bin/grep -c '^FAIL .*observed=no-record' "$SUM")
say "=== f131_run.sh: $LAUNCHES launch(es), $NP PASS, $NF FAIL, $NN NOTE, lost records $LOST - $SUM ==="
[ "$NF" -eq 0 ]
