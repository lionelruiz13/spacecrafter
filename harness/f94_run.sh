#!/bin/bash
# F94 - one command, one exit code (INTENT 11.214).
#
#   DISPLAY=:2 ./f94_run.sh <absOutdir> [extra args to f94_bodyselect.py]
#     --bin <path>   binary under test  (default: build-claude/src/spacecrafter)
#     --tag pre|post which half of the pre/post table this run is
#
# What this wrapper owns, and why it is not inside the python (F90's shape):
#   * the ENVIRONMENT CANARY (§11.176) - a measuring launch does not start on an
#     unverified stack, and a non-zero canary STOPS the run rather than being
#     mitigated (§11.174(h)).  `--no-scene` only: F94 is FUNCTIONAL and makes no
#     photometric claim, so the photometric arm has nothing here to gate.
#   * the FROZEN-FILE ASSERT, md5 in == out, on every file of the real
#     ~/.spacecrafter this run could conceivably reach:
#         config.ini  ssystem.ini              - two shipped writers (§5.42, §5.112)
#         internal/fly_to_selected.sts         - the played consumer; the script
#         scripts/fscripts/startup.sts           annotator rewrites the file it
#                                                played (script_annotator.cpp:163-179)
#     The farm makes all four unreachable; this asserts it instead of trusting it.
#   * the CONCURRENT-INSTANCE assert on /proc/<pid>/comm, every account
#     (§11.134(b): `pgrep -f <path>` self-matches its own wrapper).
#
# Exit: 0 every test readable and every control green; non-zero on any FAIL, on a
# canary red, on a concurrent instance, or on a frozen file that moved.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT=${1:?usage: f94_run.sh <absOutdir> [args...]}
shift || true
mkdir -p "$OUT"

BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
PREV=""
for a in "$@"; do [ "$PREV" = "--bin" ] && BIN="$a"; PREV="$a"; done

FILES=(
  "$HOME/.spacecrafter/config.ini"
  "$HOME/.spacecrafter/ssystem.ini"
  "$HOME/.spacecrafter/scripts/internal/fly_to_selected.sts"
  "$HOME/.spacecrafter/scripts/fscripts/startup.sts"
)

echo "=== F94 body_selected pre/post  $(date '+%Y-%m-%d %H:%M:%S %Z')"
echo "    binary  = $BIN"
ls -l --time-style=+%H:%M:%S "$BIN"; md5sum "$BIN"
echo "    display = ${DISPLAY:-<unset>}"
echo "    outdir  = $OUT"

# --- concurrent instance, any account, /proc/<pid>/comm (§11.134(b))
HITS=""
for p in /proc/[0-9]*/comm; do
    [ -r "$p" ] || continue
    if [ "$(cat "$p" 2>/dev/null)" = "spacecrafter" ]; then HITS="$HITS ${p%/comm}"; fi
done
if [ -n "$HITS" ]; then
    echo "ABORT: another spacecrafter process exists:$HITS"; exit 2
fi
echo "    /proc comm assert: no spacecrafter running"

# --- environment canary (§11.176)
if [ "${F94_SKIP_CANARY:-0}" != "1" ]; then
    bash "$HERE/f56_canary.sh" --no-scene > "$OUT/canary.log" 2>&1
    CRC=$?
    echo "    canary --no-scene exit = $CRC   ($OUT/canary.log)"
    if [ "$CRC" != "0" ]; then
        echo "ABORT: the environment canary is RED. Report it; do not widen it."; exit 3
    fi
fi

# --- frozen files, in
declare -a IN
for i in "${!FILES[@]}"; do
    if [ -f "${FILES[$i]}" ]; then IN[$i]=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    else IN[$i]="(absent)"; fi
    echo "    md5 in  ${IN[$i]}  ${FILES[$i]}"
done

DISPLAY=${DISPLAY:-:2} python3 "$HERE/f94_bodyselect.py" "$OUT" "$@" 2>&1 \
    | tee "$OUT/drive.log"
RC=${PIPESTATUS[0]}

sleep 2
pkill -9 -x spacecrafter 2>/dev/null

# --- frozen files, out
MOVED=0
for i in "${!FILES[@]}"; do
    if [ -f "${FILES[$i]}" ]; then O=$(md5sum "${FILES[$i]}" | cut -d' ' -f1)
    else O="(absent)"; fi
    if [ "$O" = "${IN[$i]}" ]; then M=yes; else M=NO; MOVED=1; fi
    echo "    md5 out ${O}  MATCH=$M  ${FILES[$i]}"
done

echo "    driver exit = $RC ; frozen files moved = $MOVED"
[ "$MOVED" = "1" ] && exit 4
exit $RC
