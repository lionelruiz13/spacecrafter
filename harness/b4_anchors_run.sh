#!/bin/bash
# B4 anchor-kind runner (INTENT §11.111, task F7).
#   usage: b4_anchors_run.sh [outdir] [extra b4_anchors.py args...]
#
# Builds the temp-HOME farm (11.103(a)) and REPLACES its anchor.ini symlink with
# a REAL authored file = the shipped anchor.ini verbatim (grammar-corpus
# coverage: every kind the field file uses must still load) + the B4 test
# anchors appended.  Channel 1 of §2(c) is therefore exercised on a file the
# app reads through the production path, and the real ~/.spacecrafter is never
# written (md5 asserted in == out, exit 3 on a farm leak).
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${1:-$HERE/artifacts/b4}
shift 1 2>/dev/null || true
FARM=${B4_FARM:-/tmp/b4_farm}
mkdir -p "$OUT"

IN_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini ~/.spacecrafter/anchor.ini)
"$HERE/b3_farm.sh" "$FARM" || exit 2
# Authored anchor file: shipped bytes first (kept byte-verbatim - it is the
# grammar corpus), then the B4 declarations before the closing [end].
rm -f "$FARM/.spacecrafter/anchor.ini"
python3 - "$FARM/.spacecrafter/anchor.ini" "$HERE/b4_anchors.ini" <<'EOF'
import sys
dst, extra = sys.argv[1], sys.argv[2]
shipped = open(__import__('os').path.expanduser('~/.spacecrafter/anchor.ini'), 'rb').read()
add = open(extra, 'rb').read()
# The shipped file ends with the [end] marker; the appended blocks go BEFORE it
# so the file keeps its documented shape ("ne pas oublier la balise [end] !").
i = shipped.rfind(b'[end]')
open(dst, 'wb').write(shipped[:i] + add + shipped[i:])
EOF
[ -f "$FARM/.spacecrafter/anchor.ini" ] || { echo "FAIL: authored anchor.ini not written"; exit 2; }

B4_FARM=$FARM python3 "$HERE/b4_anchors.py" "$OUT" "$@"
DRV=$?
OUT_MD5=$(md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini ~/.spacecrafter/anchor.ini)
if [ "$IN_MD5" != "$OUT_MD5" ]; then
    echo "FAIL: real ~/.spacecrafter md5 changed across the run (farm leak)"
    echo "$IN_MD5"; echo "$OUT_MD5"
    [ $DRV -ne 0 ] && exit $DRV
    exit 3
fi
echo "config/ssystem/anchor md5 in=out OK"
exit $DRV
