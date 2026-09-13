#!/usr/bin/env bash
# F119 check (c): the fixed supervised-by.sh repoints a code-tree citation that
# the pre-F119 tool left dangling.  RED is banked in pair-run1-red.txt, measured
# the same way on the same pair; this is the GREEN half.
set -uo pipefail
P=/home/claude/sc-f103/pair/spacecrafter
bash /home/claude/spacecrafter/claude/harness/f106_pair.sh reset
cat > "$P/util/scedit/tests/f119_planted.txt" <<'PLANT'
F119 proof fixture -- a code-tree citation in a file that is neither .md nor .json.
The anchor gate's grammar was read at commit e3afca8f, and the README line it
quotes was written there. A rewrite that re-shas e3afca8f leaves this sentence
pointing at an object no branch reaches, and nothing in git will say so.
PLANT
git -C "$P" add util/scedit/tests/f119_planted.txt
git -C "$P" -c user.name='Lionel RUIZ' -c user.email='l@example.com' \
    commit -q -m "F119 proof fixture: a code-tree citation outside .md and .json"
echo "== CHECK (c) GREEN half: the FIXED tool on the same F106 pair =="
echo "   pair before: code $(git -C "$P" rev-parse --short=8 HEAD) / harness $(git -C "$P/claude" rev-parse --short=8 HEAD)"
echo "   the three code-tree citations, BEFORE:"
for s in 54a2b844 ba7a32a8 e3afca8f; do
    printf '     %-10s %s   in %s file(s)\n' "$s" \
      "$(git -C "$P" merge-base --is-ancestor $s master-beta 2>/dev/null && echo REACHABLE || echo DANGLING)" \
      "$(git -C "$P" grep -lwIF "$s" -- . 2>/dev/null | wc -l)"
done
echo
echo "   --harness=HEAD~100..HEAD, because the pair's full harness range contains a"
echo "   malformed 'Code:' trailer and the new step A'' STOPs on it -- see check (e);"
echo "   this is the tool's own documented fix (b) for that STOP."
cd "$P" || exit 1
printf 'y\ny\n' | script -qec "bash /home/claude/spacecrafter/claude/supervised-by.sh --supervisor='Vixy <vixy@example.com>' --harness=HEAD~100..HEAD" /dev/null > /home/claude/sc-f119/run2_new.log 2>&1
echo "   run exit: $?"
echo
echo "   AFTER -- the old tokens are gone from the code tree and the new ones resolve:"
for f in util/scedit/grammar/args/unit-1.json util/scedit/grammar/ss-grammar.json util/scedit/tests/f119_planted.txt; do
    tok=$(grep -oE '\b[0-9a-f]{8}\b' "$P/$f" | head -1)
    printf '     %-46s cites %s  %s\n' "${f##util/scedit/}" "$tok" \
      "$(git -C "$P" merge-base --is-ancestor "$tok" master-beta 2>/dev/null && echo REACHABLE || echo DANGLING)"
done
echo "   old tokens still anywhere in the code tree:"
for s in 54a2b844 ba7a32a8 e3afca8f; do
    printf '     %-10s %s hit(s)\n' "$s" "$(git -C "$P" grep -lwIF "$s" -- . 2>/dev/null | wc -l)"
done
echo
echo "   the CODE closing commit (made FIRST, its expected set proved):"
git -C "$P" log -1 --format='     %h  %an  %s'
git -C "$P" show --stat --format='' HEAD | tail -3 | sed 's/^/     /'
echo "   files it touched, by extension: $(git -C "$P" show --name-only --format='' HEAD | sed 's/.*\.//' | sort | uniq -c | tr '\n' ' ')"
echo "   the HARNESS closing commit's trailer names it:"
git -C "$P/claude" log -1 --format='%B' | grep -i '^Code:' | sed 's/^/     /'
echo "   both trees clean: code $(git -C "$P" status --porcelain | wc -l) / harness $(git -C "$P/claude" status --porcelain | wc -l)"
echo
echo "   the anchor gate in the pair (the ctest the whole task serves):"
(cd "$P" && python3 util/scedit/tests/anchor_gate.py 2>&1 | sed -n '/--- got ---/,$p' \
   | grep -E '^(AT-PIN|GONE|clean|moved|not-at|skipped)' | sed 's/^/     /')
