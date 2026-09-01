#!/bin/bash
# F74 -- SS.119 reachability reproduction: builds the REAL
# src/appModule/fontFactory.cpp against a stub harness and runs it in four
# configurations, so the past-the-end dereference in updateAllFont can be
# observed (or refuted) without a full engine build.
#
#   usage: f74_build_run.sh [git-rev]      (default: the working tree)
#
# With a git-rev, fontFactory.{hpp,cpp} are extracted from that revision of the
# CODE repo into an overlay placed FIRST on the include path -- that is how the
# PRE-fix run is produced once the fix has landed.
#
# Configurations (all -O0 -g, -std=gnu++2b as the engine uses CXX_STANDARD 23):
#   plain    no instrumentation      -- what the code does unaided
#   gdebug   -D_GLIBCXX_DEBUG        -- libstdc++ checked iterators: the
#                                       purpose-built detector for exactly this
#                                       UB ("past-the-end iterator")
#   asan     -fsanitize=address      -- reports what ASan can see here
#   ubsan    -fsanitize=undefined    -- reports what UBSan can see here
#
# Exit code is 0 whatever the runs do: the runs' exit codes ARE the measurement
# and are printed per configuration.
set -u

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODE="${CODE_ROOT:-/home/claude/spacecrafter}"
REV="${1:-}"
OUT="${OUT_DIR:-$HERE/out}"
rm -rf "$OUT"; mkdir -p "$OUT"

SRC_CPP="$CODE/src/appModule/fontFactory.cpp"
OVERLAY=""
if [ -n "$REV" ]; then
    mkdir -p "$OUT/overlay/appModule"
    git -C "$CODE" show "$REV:src/appModule/fontFactory.cpp" > "$OUT/overlay/appModule/fontFactory.cpp" || exit 2
    git -C "$CODE" show "$REV:src/appModule/fontFactory.hpp" > "$OUT/overlay/appModule/fontFactory.hpp" || exit 2
    SRC_CPP="$OUT/overlay/appModule/fontFactory.cpp"
    OVERLAY="-I$OUT/overlay"
    echo "### source: fontFactory.{hpp,cpp} @ $REV"
else
    echo "### source: working tree $SRC_CPP"
fi
echo "### md5(fontFactory.cpp) = $(md5sum "$SRC_CPP" | cut -d' ' -f1)"

# The stub tree comes FIRST for tools/ and mediaModule/; appModule/,
# mainModule/ and interfaceModule/ resolve to the REAL headers under src/.
COMMON="-std=gnu++2b -O0 -g -Wall -Wextra $OVERLAY -I$HERE/stub -I$CODE/src"

run_cfg () {
    local name="$1"; shift
    local flags="$1"; shift
    local bin="$OUT/f74_$name"
    echo
    echo "=== config: $name   [$flags]"
    if ! g++ $COMMON $flags -o "$bin" "$SRC_CPP" "$HERE/f74_driver.cpp" 2> "$OUT/$name.build.log"; then
        echo "--- BUILD FAILED, see $OUT/$name.build.log"
        tail -20 "$OUT/$name.build.log"
        return
    fi
    [ -s "$OUT/$name.build.log" ] && { echo "--- build warnings:"; cat "$OUT/$name.build.log"; }
    ASAN_OPTIONS=detect_leaks=0 timeout 60 "$bin" > "$OUT/$name.run.log" 2>&1
    local rc=$?
    echo "--- run exit code: $rc"
    cat "$OUT/$name.run.log"
}

run_cfg plain  ""
run_cfg gdebug "-D_GLIBCXX_DEBUG"
run_cfg asan   "-fsanitize=address -fno-omit-frame-pointer"
run_cfg ubsan  "-fsanitize=undefined -fno-omit-frame-pointer"
echo
echo "### logs in $OUT"
