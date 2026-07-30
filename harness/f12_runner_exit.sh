#!/bin/bash
# Exit-code matrix for b10_cmd_battery_run.sh's restore/assert tail (§11.101(g)
# echo-not-assert class; F0's precedent for this technique is §11.103(e)).
#
# WHAT IT IS FOR. The tail of the battery runner decides whether the run passed:
# config.ini restored byte-identically, the frozen ssystem corpus still the
# pristine one, scene E clean. Those branches cannot all be reached live (the
# shipped corpus must never be mutated - §2.0 D9), so the block is EXTRACTED
# VERBATIM from the runner and driven with injected variables. The extraction is
# asserted line-by-line against the runner first, so this file cannot drift into
# testing a block the runner no longer has.
#
#     cd claude/harness && ./f12_runner_exit.sh
#
# Every case states the expected code; the script fails loudly on any mismatch.

set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
RUNNER="$HERE/b10_cmd_battery_run.sh"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
RC=0

# ---- extract the tail verbatim, then prove it IS the runner's tail ----------
awk '/^# ============ restore ============$/{f=1} f' "$RUNNER" > "$WORK/tail.sh"
NLINES=$(wc -l < "$WORK/tail.sh")
[ "$NLINES" -ge 15 ] || { echo "FAIL: extracted tail is only $NLINES lines"; exit 1; }
MATCHED=0
while IFS= read -r line; do
    if grep -qxF -- "$line" "$RUNNER"; then MATCHED=$((MATCHED+1)); fi
done < "$WORK/tail.sh"
if [ "$MATCHED" = "$NLINES" ]; then
    echo "ok:   extracted tail is verbatim ($MATCHED/$NLINES lines found literally in the runner)"
else
    echo "FAIL: only $MATCHED/$NLINES extracted lines are literally in the runner"; RC=1
fi

# ---- drive it ---------------------------------------------------------------
# HOME is redirected so the block's own `~/.spacecrafter/ssystem.ini` reference
# is exercised (it is the line under test), and the real tree is never read.
run_case() {  # $1 label  $2 expected  $3 config-restored(y/n)  $4 corpus(pristine/mutated)  $5 SE  $6 FAILN
    local label=$1 expect=$2 restored=$3 corpus=$4 se=$5 failn=$6
    local home="$WORK/$label"
    mkdir -p "$home/.spacecrafter" "$home/out"
    printf 'the pristine corpus\n' > "$home/.spacecrafter/ssystem.ini"
    printf 'config as found\n' > "$home/.spacecrafter/config.ini"
    local pristine
    pristine=$(md5sum "$home/.spacecrafter/ssystem.ini" | cut -d' ' -f1)
    if [ "$corpus" = mutated ]; then
        printf 'somebody edited the frozen corpus\n' > "$home/.spacecrafter/ssystem.ini"
    fi
    if [ "$restored" = y ]; then
        cp "$home/.spacecrafter/config.ini" "$home/out/config.ini.bak"
    else
        printf 'a config this runner failed to restore\n' > "$home/out/config.ini.bak"
    fi
    {
        echo 'set -u'
        echo "CFG=$home/.spacecrafter/config.ini"
        echo "OUT=$home/out"
        echo "MD5_IN=$(md5sum "$home/.spacecrafter/config.ini" | cut -d' ' -f1)"
        echo "SSYS_IN=$(md5sum "$home/.spacecrafter/ssystem.ini" | cut -d' ' -f1)"
        echo "SSYS_PRISTINE=$pristine"
        echo "SE=$se"
        echo "FAILN=$failn"
        cat "$WORK/tail.sh"
    } > "$home/case.sh"
    HOME="$home" bash "$home/case.sh" > "$home/case.log" 2>&1
    local got=$?
    if [ "$got" = "$expect" ]; then
        echo "ok:   $label -> exit $got (expected $expect)"
    else
        echo "FAIL: $label -> exit $got, expected $expect"; RC=1
        sed 's/^/        /' "$home/case.log"
    fi
}

run_case green            0 y pristine 0 0
run_case config_not_restored 3 n pristine 0 0
run_case corpus_mutated   4 y mutated  0 0
run_case sceneE_failed    7 y pristine 1 0
run_case sceneE_faillines 7 y pristine 0 2
# precedence: a driver failure dominates a corpus mismatch, and a corpus
# mismatch dominates a config restore failure - stated in the runner, asserted
# here.
run_case driver_over_corpus 7 n mutated 1 3
run_case corpus_over_config 4 n mutated 0 0

[ $RC = 0 ] && echo "ALL OK" || echo "FAILURES"
exit $RC
