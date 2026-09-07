#!/usr/bin/env bash
#
# f106_pair.sh -- reset the F103 throwaway clone pair to STATE U and prove it.
#
#     ./f106_pair.sh reset     # reset both repos, drop refs/original, assert U
#     ./f106_pair.sh assert    # assert U without touching anything
#
# STATE U is the pristine pre-run state of the pair F103 built
# (harness/artifacts/f103/setup.txt): code 474c595d, harness fe096019, both
# clean, 94 / 798 commits ahead of the scratch origins, cebebf44 present WITH
# its gpgsig header and reachable from master-beta.
#
# Why a script and not three commands typed twice per run: the pair is reset
# BEFORE and AFTER every one of F106's runs, and a run that starts from a state
# nobody asserted measures nothing.  The assertions are the point; the reset is
# the cheap part.
#
# `git clean -x` is used in the HARNESS repo only.  In the CODE repo it would
# delete `claude/` -- the nested harness clone is IGNORED by the code repo, so
# -x sweeps it away, and the fixture would have to be rebuilt.
set -euo pipefail

PAIR=${PAIR:-/home/claude/sc-f103/pair}
CODE="${PAIR}/spacecrafter"
HARN="${CODE}/claude"
CODE_U=474c595d43c93e3e9e1d0d3e9d0a5e4e2ab0e9ff   # resolved below, prefix-checked
CODE_U8=474c595d
HARN_U8=fe096019

drop_original() {                     # filter-branch's backup refs
    local r=$1 ref
    while read -r ref; do
        [ -n "${ref}" ] || continue
        git -C "${r}" update-ref -d "${ref}"
    done < <(git -C "${r}" for-each-ref --format='%(refname)' refs/original 2>/dev/null || true)
}

do_reset() {
    drop_original "${CODE}"; drop_original "${HARN}"
    git -C "${CODE}" reset --hard "${CODE_U8}" >/dev/null
    git -C "${HARN}" reset --hard "${HARN_U8}" >/dev/null
    # -x in the harness only (see the header).  In the code repo, remove
    # untracked non-ignored files only, and never recurse into claude/.
    git -C "${HARN}" clean -xdff >/dev/null
    git -C "${CODE}" clean -dff -e claude >/dev/null
}

assert_u() {
    local fail=0 v
    v=$(git -C "${CODE}" rev-parse --short=8 HEAD); [ "${v}" = "${CODE_U8}" ] || { echo "FAIL code HEAD ${v}"; fail=1; }
    v=$(git -C "${HARN}" rev-parse --short=8 HEAD); [ "${v}" = "${HARN_U8}" ] || { echo "FAIL harness HEAD ${v}"; fail=1; }
    v=$(git -C "${CODE}" status --porcelain | wc -l);  [ "${v}" = 0 ] || { echo "FAIL code dirty ${v}"; fail=1; }
    v=$(git -C "${HARN}" status --porcelain | wc -l);  [ "${v}" = 0 ] || { echo "FAIL harness dirty ${v}"; fail=1; }
    v=$(git -C "${CODE}" rev-list --count '@{upstream}..HEAD'); [ "${v}" = 94 ] || { echo "FAIL code range ${v}"; fail=1; }
    v=$(git -C "${HARN}" rev-list --count '@{upstream}..HEAD'); [ "${v}" = 798 ] || { echo "FAIL harness range ${v}"; fail=1; }
    v=$(git -C "${CODE}" rev-list --count master-beta);         [ "${v}" = 3829 ] || { echo "FAIL branch length ${v}"; fail=1; }
    v=$(git -C "${CODE}" cat-file -p cebebf44 | grep -c '^gpgsig'); [ "${v}" = 1 ] || { echo "FAIL cebebf44 gpgsig ${v}"; fail=1; }
    git -C "${CODE}" merge-base --is-ancestor cebebf44 master-beta || { echo "FAIL cebebf44 unreachable"; fail=1; }
    v=$(git -C "${CODE}" for-each-ref refs/original | wc -l);   [ "${v}" = 0 ] || { echo "FAIL code refs/original ${v}"; fail=1; }
    v=$(git -C "${HARN}" for-each-ref refs/original | wc -l);   [ "${v}" = 0 ] || { echo "FAIL harness refs/original ${v}"; fail=1; }
    if [ "${fail}" = 0 ]; then
        echo "STATE U ok: code ${CODE_U8} / harness ${HARN_U8}, clean, 94/798, master-beta 3829, cebebf44 signed."
    else
        echo "STATE U NOT reached." >&2; return 1
    fi
}

case "${1:-}" in
    reset)  do_reset; assert_u ;;
    assert) assert_u ;;
    *) echo "usage: $0 reset|assert" >&2; exit 1 ;;
esac
