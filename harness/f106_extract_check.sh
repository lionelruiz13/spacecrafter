#!/usr/bin/env bash
#
# f106_extract_check.sh <repo> <range>
#
# The --commit-filter of supervised-by.sh compares the ORIGINAL commit against
# what filter-branch is asking it to build.  Two of the five comparisons use an
# extraction of my own instead of asking git for the field:
#
#   * the MESSAGE, split off at the first truly empty line of the raw object
#     (`${OBJ%%$'\n\n'*}`), where filter-branch uses a read-until-empty-line loop
#     over the same bytes (git-filter-branch:465-474);
#   * the PARENTS, read from the object's own `parent` header lines, where the
#     mandate named `git rev-parse <sha>^@`.
#
# An extraction that is merely "obviously the same" is an assumption.  This
# script measures both over every commit of a range and prints the number of
# disagreements, which must be 0.  It is also able to fail: run it with
# F106_MUTATE=1 and the message split is done at the first line that is empty
# OR BLANK, which is exactly the mistake a gpgsig block invites -- on a signed
# commit the split then lands inside the armour and the comparison disagrees.
set -euo pipefail

REPO=${1:?repo}; RANGE=${2:?range}
MUT=${F106_MUTATE:-0}
n=0; bad_msg=0; bad_par=0; signed=0

while read -r sha; do
    [ -n "${sha}" ] || continue
    n=$((n + 1))

    OBJ=$(git -C "${REPO}" cat-file commit "${sha}"; printf X); OBJ=${OBJ%X}
    if [ "${MUT}" = 1 ]; then
        # the mutant: treat a line of blanks as the end of the header
        MINE=$(printf '%s' "${OBJ}" | sed -e '1,/^[[:space:]]*$/d'; printf X)
    else
        HEAD_PART=${OBJ%%$'\n\n'*}
        MINE=${OBJ#"${HEAD_PART}"$'\n\n'}; MINE="${MINE}X"
    fi
    MINE=${MINE%X}

    # git's own: skip header lines up to the first EMPTY one, then cat.
    GITS=$( { while IFS='' read -r hl && test -n "${hl}"; do :; done; cat; } \
            < <(git -C "${REPO}" cat-file commit "${sha}"); printf X)
    GITS=${GITS%X}

    if [ "${MINE}" != "${GITS}" ]; then
        bad_msg=$((bad_msg + 1))
        echo "  MSG DIFFERS ${sha:0:8}  mine=$(printf '%s' "${MINE}" | wc -c)B git=$(printf '%s' "${GITS}" | wc -c)B"
    fi

    MP=""
    while IFS= read -r l; do
        case "${l}" in
            "parent "*) MP="${MP}${l#parent }"$'\n' ;;
            gpgsig*)    signed=$((signed + 1)) ;;
        esac
    done <<< "${OBJ%%$'\n\n'*}"
    RP=""
    while read -r p; do
        case "${p}" in "") ;; *) RP="${RP}${p}"$'\n' ;; esac
    done < <(git -C "${REPO}" rev-parse "${sha}^@" 2>/dev/null || true)
    if [ "${MP}" != "${RP}" ]; then
        bad_par=$((bad_par + 1))
        echo "  PARENTS DIFFER ${sha:0:8}"
    fi
done < <(git -C "${REPO}" rev-list "${RANGE}")

echo "checked ${n} commit(s) in ${REPO} ${RANGE} (mutate=${MUT}): ${signed} signed;" \
     "message-split disagreements ${bad_msg}, parent-list disagreements ${bad_par}"
[ "${bad_msg}" -eq 0 ] && [ "${bad_par}" -eq 0 ]
