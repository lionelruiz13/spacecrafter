#!/usr/bin/env bash
# F119 -- the anchor gate as a NEW DEVELOPER reads it, at four states of the tree.
# Run against /home/claude/sc-f119/clone-before: a TRANSPORT clone (objects by
# protocol, so unreachable objects are absent, unlike a hardlinked local clone)
# of /home/claude/spacecrafter, with src/EntityCore cloned from its own GitHub
# remote.  Every number below is this script's own run, never a recalled one.
set -u
C=/home/claude/sc-f119/clone-before
pins() {   # repoint the six pins in the scratch clone, the ACT 1 edit
  for f in util/scedit/grammar/args/unit-1.json util/scedit/grammar/args/unit-2.json \
           util/scedit/grammar/args/unit-3.json util/scedit/grammar/args/unit-4.json \
           util/scedit/grammar/sc-grammar.json; do
      sed -i '/"anchor_pin"/ s/master-beta @ 54a2b844/master-beta @ b45d3b58/' "$C/$f"
  done
  sed -i '/"anchor_pin"/ s/master-beta @ ba7a32a8/master-beta @ 8d41fbe3/' "$C/util/scedit/grammar/ss-grammar.json"
}
tally() { ( cd "$C" && python3 util/scedit/tests/anchor_gate.py 2>&1 \
            | sed -n '/--- got ---/,$p' | grep -E '^(AT-PIN-BROKEN|GONE|UNRESOLVED|clean|moved|not-at-head|skipped)' \
            | tr -s ' ' | tr '\n' ' ' ; echo ) }
echo "== F119: anchor_gate from a transport clone, by state =="
echo "   clone: $C   objects: $(cd $C && for s in 54a2b844 ba7a32a8; do printf '%s=%s ' $s "$(git cat-file -t $s 2>/dev/null || echo absent)"; done)"
echo
echo "A) code 87d429bd, pins as shipped (the state F119 found)"
( cd "$C" && git checkout -q -f 87d429bd ) ; echo "   $(tally)"
echo
echo "B) code fcc277c9 (F110's rehearsal HEAD), pins repointed -- the control for"
echo "   'why 321 there and 359 here': same pins, older engine."
( cd "$C" && git checkout -q -f fcc277c9 ) ; pins ; echo "   $(tally)"
echo
echo "C) code d2fe86c7 (this task's ACT 1 commit, pins + the declared row)"
( cd "$C" && git checkout -q -f d2fe86c7 ) ; echo "   $(tally)"
echo
echo "   The residual 4 at C are the two EntityCore anchor elements of"
echo "   sc-grammar.json:7071 and args/unit-1.json:295.  They resolve at the"
echo "   submodule commit the pin's gitlink names, 7ce58350, reachable from no"
echo "   ref of the EntityCore repository -- so a fresh clone cannot read it and"
echo "   no edit in THIS tree can reach it.  Its twin is main's tip 84f5d94b,"
echo "   same tree 6ee9f6a7; this repository bumped its gitlink there at"
echo "   f4ceb208 (2026-09-05), after these pins were written."
( cd "$C" && git checkout -q -f master-beta 2>/dev/null || git checkout -q -f d2fe86c7 )
