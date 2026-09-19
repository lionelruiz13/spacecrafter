#!/bin/bash
# Regression fixture for ../supervised-by.sh (INTENT S11.259): a throwaway repo PAIR in a temp
# directory, never the real one. It builds every kind of home that can cite a commit the rewrite
# renames, runs the tool with both prompts answered y (through `script`, the tool wants a tty),
# and asserts what must move, what must NOT, and what must be reported.
#   usage: supervised_by_fixture.sh [path to supervised-by.sh]      exit 1 on any FAIL
set -u
TOOL=$(readlink -f "${1:-$(dirname "$0")/../supervised-by.sh}")
LIB="$(dirname "${TOOL}")/sha_map_lib.sh"
T=$(mktemp -d /tmp/sc_supby.XXXXXX); trap 'rm -rf "${T}"' EXIT
export GIT_CONFIG_GLOBAL=/dev/null GIT_COMMITTER_NAME=Runner GIT_COMMITTER_EMAIL=r@x   # no host identity leaks in, the closing commits still have a committer
H='-c user.name=Human -c user.email=h@x'; C='-c user.name=Claude -c user.email=c@x'
fails=0; ok() { echo "PASS $*"; }; ko() { echo "FAIL $*"; fails=$((fails+1)); }

# ---- the pair: code repo with an upstream, harness repo nested as claude/ with its own ----
git init -q --bare "${T}/code.git"; git init -q --bare "${T}/harness.git"
git init -q -b master-beta "${T}/code"; cd "${T}/code"
echo 'claude/' > .gitignore; echo base > a.txt
git ${H} add -A; git ${H} commit -q -m "base"; git remote add origin "${T}/code.git"; git push -q -u origin master-beta
echo one >> a.txt;  git ${C} commit -q -a --author='Claude Fable 5.1 <c@x>' -m "first agent commit

Co-Authored-By: Claude Fable 5.1 <c@x>"
OLD1=$(git rev-parse HEAD)
echo two >> a.txt;  git ${C} commit -q -a --author='Claude Fable 5.1 <c@x>' -m "second agent commit

Co-Authored-By: Claude Fable 5.1 <c@x>"
OLD2=$(git rev-parse HEAD)
git update-ref refs/review/headers "${OLD1}"            # custom namespace   -> must move
git tag light "${OLD1}"                                 # lightweight tag    -> must move
git branch side "${OLD1}"                               # another branch     -> must move
git ${H} tag -a annotated -m "annotated" "${OLD1}"      # annotated tag      -> must be REPORTED
git worktree add -q --detach "${T}/wt" "${OLD1}"        # detached worktree  -> must be REPORTED

git init -q -b CC-harness "${T}/code/claude"; cd "${T}/code/claude"
cp "${TOOL}" "${LIB}" .; mkdir -p harness/artifacts drawer/archive sha-maps "${T}/outside"
echo "doc cites ${OLD1:0:8} and ${OLD2:0:10}" > README.md
echo "# controlled against ${OLD1:0:8}"       > tool.py
echo "recorded at ${OLD1:0:8}"                > harness/artifacts/evidence.txt
echo "{\"at\": \"${OLD1:0:8}\"}"              > drawer/archive/manifest.json
printf '%s\tevidence\n' "${OLD1}"             > record.tsv
echo "queue entry read at ${OLD2:0:8}"        > "${T}/outside/Q.md"
printf '# outside homes\n%s\n%s/missing\n' "${T}/outside" "${T}" > sha-maps/also.txt
git ${H} add -A; git ${H} commit -q -m "harness base"; git remote add origin "${T}/harness.git"; git push -q -u origin CC-harness
chmod 444 tool.py "${T}/outside/Q.md"            # read-only to their own owner -> the tool must restore the bit itself (the sudo arm needs a second user: NOT reached here)
echo more >> README.md; git ${C} commit -q -a --author='Claude Fable 5.1 <c@x>' -m "harness agent commit

Code: master-beta @ ${OLD2:0:8}

Co-Authored-By: Claude Fable 5.1 <c@x>"

# ---- run the tool: supervisor given, "y" to the rewrite, "y" to the repoint ----
cd "${T}/code"
printf 'y\ny\n' | script -qfec "./claude/supervised-by.sh --supervisor='Vixy <v@x>'" /dev/null > "${T}/run.log" 2>&1
RC=$?
NEW1=$(git rev-parse HEAD~1 2>/dev/null); NEW2=$(git rev-parse HEAD 2>/dev/null)
# a code-side repoint commit may sit on top: find the rewritten pair by subject instead
NEW1=$(git log --format='%H %s' | awk '/first agent commit/{print $1}'); NEW2=$(git log --format='%H %s' | awk '/second agent commit/{print $1}')

[ "${RC}" -eq 0 ] && ok "tool exit 0" || { ko "tool exit ${RC}"; tail -25 "${T}/run.log"; }
[ -n "${NEW1}" ] && [ "${NEW1}" != "${OLD1}" ] && ok "history rewritten (${OLD1:0:8} -> ${NEW1:0:8})" || ko "history not rewritten"
for r in refs/review/headers refs/tags/light refs/heads/side; do
    [ "$(git rev-parse "${r}")" = "${NEW1}" ] && ok "ref moved: ${r}" || ko "ref NOT moved: ${r} -> $(git rev-parse --short "${r}")"
done
[ "$(git rev-parse 'annotated^{commit}')" = "${OLD1}" ] && grep -q "annotated tag annotated" "${T}/run.log" \
    && ok "annotated tag left alone and REPORTED" || ko "annotated tag: not reported, or moved"
grep -q "detached worktree ${T}/wt" "${T}/run.log" && ok "detached worktree REPORTED" || ko "detached worktree not reported"
git for-each-ref refs/original | grep -q . && ok "refs/original backup untouched" || ko "refs/original missing"
cd claude
grep -q "${NEW1:0:8}" README.md && grep -q "${NEW2:0:10}" README.md && ok "*.md repointed (both prefix lengths)" || ko "*.md not repointed: $(cat README.md)"
grep -q "${NEW1:0:8}" tool.py        && ok "live non-.md file repointed although read-only (tool.py)" || ko "tool.py not repointed: $(cat tool.py)"
grep -q "${OLD1:0:8}" harness/artifacts/evidence.txt && ok "evidence drawer untouched (harness/artifacts)" || ko "EVIDENCE REWRITTEN: harness/artifacts"
grep -q "${OLD1:0:8}" drawer/archive/manifest.json   && ok "evidence drawer untouched (archive/)"          || ko "EVIDENCE REWRITTEN: archive/"
grep -q "${OLD1}" record.tsv                         && ok "evidence untouched (*.tsv)"                    || ko "EVIDENCE REWRITTEN: *.tsv"
grep -q "${NEW2:0:8}" "${T}/outside/Q.md"            && ok "outside home repointed although read-only (sha-maps/also.txt)"    || ko "outside home not repointed: $(cat "${T}/outside/Q.md")"
grep -q "missing', which does not exist" "${T}/run.log" && ok "missing outside path reported, not fatal" || ko "missing outside path not reported"
[ -z "$(git status --porcelain)" ] && ok "harness tree clean after the closing commit" || ko "harness tree dirty: $(git status --porcelain | head -3 | tr '\n' ' ')"
git log -1 --format='%an|%B' | grep -q "^Vixy|" && git log -1 --format=%B | grep -q "^Code: master-beta @ " && ok "closing commit by the supervisor, with its Code: trailer" || ko "no closing commit by the supervisor with a Code: trailer"
# The residual scan has exactly two things left to say here, and both are the report-only kind.
N=$(grep -c '^STILL STALE' "${T}/run.log")
[ "${N}" -eq 2 ] && ok "residual scan: 2 lines, the two homes it cannot move" || { ko "residual scan: ${N} line(s), want 2"; grep '^STILL STALE' "${T}/run.log"; }
# Second run: nothing is selected any more, so it must be a no-op that exits 0 and moves nothing.
cd "${T}/code"; BEFORE=$(git for-each-ref | md5sum)
printf 'y\ny\n' | script -qfec "./claude/supervised-by.sh --supervisor='Vixy <v@x>'" /dev/null > "${T}/run2.log" 2>&1
RC2=$?; AFTER=$(git for-each-ref | md5sum)
[ "${RC2}" -eq 0 ] && [ "${BEFORE}" = "${AFTER}" ] && ok "second run is a no-op (exit 0, no ref moved)" || { ko "second run: exit ${RC2}, refs $([ "${BEFORE}" = "${AFTER}" ] && echo same || echo CHANGED)"; tail -8 "${T}/run2.log"; }
echo; [ "${fails}" -eq 0 ] && echo "=== ALL PASS ===" || { echo "=== ${fails} FAILURE(S) === (log: kept below)"; sed -n '1,400p' "${T}/run.log" | tail -60; }
exit $((fails > 0))
