#!/bin/bash
# B5-oort-2 EARTH-ANCHORED power-of-two distance ladder runner (INTENT §6.9 /
# §11.96 [vixy 2026-07-24]). ONE fresh launch, ONE path, one exponent range -
# the caller invokes it once per path (fresh launches per path, per the spec).
# Enables the config-gated pilot (flag_experimental_oort=true) + init_fov=340,
# FISHEYE; config.ini restored byte-identically (md5 asserted). Override the
# binary with SC_BIN (used for the 'preold' pre-seed-change capture).
#
#   b5_ladder_run.sh <new|old|preold> <absOutdir> <start> <stop> <step>
#
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
PATHSEL=${1:?path new|old|preold|prenew}
OUT=${2:?abs outdir}
# exponents: either "@file" as $3, or start stop step
if [[ "${3:-}" == @* ]]; then EXPARGS="${3}"; else EXPARGS="${3:-0} ${4:-34} ${5:-1}"; fi
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"

MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.bak.$PATHSEL"
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"
if grep -q '^flag_experimental_oort' "$CFG"; then
    sed -i 's/^flag_experimental_oort *=.*/flag_experimental_oort         = true/' "$CFG"
else
    sed -i '/^\[rendering\]/a flag_experimental_oort         = true' "$CFG"
fi

echo "path=$PATHSEL binary: $BIN"; ls -l --time-style=full-iso "$BIN"
DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app_$PATHSEL.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app_$PATHSEL.log"; cp "$OUT/config.ini.bak.$PATHSEL" "$CFG"; exit 1; }
done
sleep 8

python3 "$HERE/b5_ladder.py" capture "$PATHSEL" "$OUT" $EXPARGS > "$OUT/drive_$PATHSEL.log" 2>&1
DRC=$?
echo "driver exit=$DRC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }

cp "$OUT/config.ini.bak.$PATHSEL" "$CFG"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
echo "config md5 in=$MD5_IN out=$MD5_OUT $([ "$MD5_IN" = "$MD5_OUT" ] && echo OK || echo MISMATCH)"
echo "--- driver tail ($PATHSEL) ---"; tail -20 "$OUT/drive_$PATHSEL.log"
exit $DRC
