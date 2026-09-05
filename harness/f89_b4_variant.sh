#!/bin/bash
# F89 - b4's scene run with ONE scene variable changed: the star twinkle
# (INTENT §11.208; the red at §11.205(g), the two failed experiments at (h)).
#
#   usage: SC_BIN=<binary> B4_FARM=<farm> ./f89_b4_variant.sh <outdir> <on|off>
#
# WHY THIS EXISTS BESIDE b4_anchors_run.sh (I2 - the duplication is deliberate
# and bounded): F89's boundary forbids touching `b4_anchors.py` unless the
# twinkle hypothesis is CONFIRMED, and `b4_anchors_run.sh` builds the farm and
# launches in one breath with no seam to change a config key in.  So this script
# re-uses the two real components unchanged - `b3_farm.sh` for the farm and
# `b4_anchors.py` for the whole scene and every check - and duplicates only the
# eight-line anchor-authoring block of `b4_anchors_run.sh:20-32`, byte for byte,
# plus the same md5 in==out assert.  The GATE is still `b4_anchors_run.sh`; this
# is the experiment's runner and nothing else may cite it as the gate.
#
# The twinkle is switched at the FARM's config.ini (`flag_star_twinkle`, read at
# core.cpp:346), not by a command, because b4's scene owns its own sends and the
# flag must be off from the first frame.  The effective value is reported by the
# dump itself (`oldView.stars.twinkleAmountEff`), so the switch is verified from
# the run's own output rather than assumed - `f89_p7.py scalars` prints it.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${1:?outdir}
TWINKLE=${2:?on|off}
FARM=${B4_FARM:-/home/claude/sc-f89/b4_farm}
mkdir -p "$OUT"

case "$TWINKLE" in on) VAL=true ;; off) VAL=false ;; *) echo "FAIL: twinkle must be on|off"; exit 2 ;; esac

# Concurrent-instance assert (§11.121(m), the f26_epoch.sh /proc/<pid>/comm
# shape - never `pgrep -f`, which self-matches the wrapper).
for p in /proc/[0-9]*/comm; do
    c=$(cat "$p" 2>/dev/null) || continue
    case "$c" in *spacecrafter*) echo "FAIL: a spacecrafter process exists ($p = $c)"; exit 4 ;; esac
done

IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini ~/.spacecrafter/anchor.ini)
"$HERE/b3_farm.sh" "$FARM" || exit 2
rm -f "$FARM/.spacecrafter/anchor.ini"
python3 - "$FARM/.spacecrafter/anchor.ini" "$HERE/b4_anchors.ini" <<'EOF'
import sys
dst, extra = sys.argv[1], sys.argv[2]
shipped = open(__import__('os').path.expanduser('~/.spacecrafter/anchor.ini'), 'rb').read()
add = open(extra, 'rb').read()
i = shipped.rfind(b'[end]')
open(dst, 'wb').write(shipped[:i] + add + shipped[i:])
EOF
[ -f "$FARM/.spacecrafter/anchor.ini" ] || { echo "FAIL: authored anchor.ini not written"; exit 2; }

# The one variable of this experiment.
sed -i "s/^flag_star_twinkle .*/flag_star_twinkle              = $VAL/" "$FARM/.spacecrafter/config.ini"
grep -q "^flag_star_twinkle *= *$VAL\$" "$FARM/.spacecrafter/config.ini" \
    || { echo "FAIL: farm config twinkle not set to $VAL"; exit 2; }
echo "farm twinkle: $(grep '^flag_star_twinkle' "$FARM/.spacecrafter/config.ini")"

B4_FARM=$FARM python3 "$HERE/b4_anchors.py" "$OUT"
DRV=$?
OUT_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini ~/.spacecrafter/anchor.ini)
if [ "$IN_MD5" != "$OUT_MD5" ]; then
    echo "FAIL: real ~/.spacecrafter md5 changed across the run (farm leak)"
    echo "$IN_MD5"; echo "$OUT_MD5"
    [ $DRV -ne 0 ] && exit $DRV
    exit 3
fi
echo "config/ssystem/anchor md5 in=out OK"
echo "farm twinkle after the run: $(grep '^flag_star_twinkle' "$FARM/.spacecrafter/config.ini")"
exit $DRV
