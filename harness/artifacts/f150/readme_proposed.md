## F150 - the shape of a delivery from one command (`shape/delivery.py`; `shape/decomment.py` the one C++ comment scanner of names.py / writers.py / codeident.py; `shape/writers.py` takes `WORKTREE`; `shape/ratio.py`'s classifier importable and form.py's line rule) - INTENT 11.321 / row B46 / 11.256, 2026-10-11

**What for.** An acceptance judges a delivery on behaviour AND shape (SHAPE.md section 6, fable-dispatch
section 0b (c)). Until this script the shape's numbers had two hand-derived homes - the executor's shape
section and the supervisor's acceptance line - and a typed number is the Q-67 channel (11.316.md:143
re-typed +264 / -79 after its author's own edit had made the commits +265 / -80). One command prints
them; both paste its block.

**Run** - from anywhere (it works at the code repo root); read-only on both trees:

    python3 claude/harness/shape/delivery.py <base> <head>         # a committed delivery
    python3 claude/harness/shape/delivery.py <base> [WORKTREE]     # the tree on disk, untracked src/ files included
    python3 claude/harness/shape/delivery.py <base> <head> --class Projector:src/coreModule/projector.hpp:src/coreModule/projector.cpp
    python3 claude/harness/shape/delivery.py --self-test           # the two recorded deliveries as oracles (~9 s)

**Reading the block.** Eight sections in a fixed order - the script's header is the authority: (1) per
file +code -code +comment -comment +blank -blank, ratio.py's `classify` read on git's -U0 hunks (a
trailing comment is code), the totals beside numstat, net code; (2) names.py BEFORE -> AFTER per touched
header with the names added / removed; (3) writers.py's (field, writer) pairs per class at both ends and
each field whose writer set changed (default classes: a touched header AND its same-stem .cpp; `--class`
replaces them); (4) the added comments' form by form.py's `measure` (which lines are comments: ratio.py's
rule) and a verb-first share that is an UPPER bound - read the first words printed beside it; (5) the
ledger-grammar hits in the added lines; (6) the D14 gate - it reads the code repo's TRACKED files ON DISK,
so the line says whether that tree is <head> - and the delivery's own files read at <head> by the gate's
partition; (7) ratio.py's module numbers at both ends; (8) review.sh's header scope (new is against <base>,
not against the owner's mark) and the touched files outside it. Exit 0 whatever the numbers say; 2 = an
instrument could not produce its section (`ERROR (<section>) ...`) or the tree moved during a WORKTREE run
(run it again).

**Able to fail, first.** `--self-test` finds each expected value's QUOTE verbatim at its line of INTENT
11.317 / 11.316 and parses the value from the quote - none is typed apart from it. On a FAIL it computes
the candidate record-side rules; one that reproduces the record value prints `PASS* ... rule=<the rule>`,
none prints `FAIL ... cause=UNNAMED` (exit 1). At delivery: 163 PASS / 13 PASS* / 0 FAIL, exit 0 (the
records measured before a whitespace re-indent; "new" against the review mark; names added to pre-existing
types; the records' names.py before `decomment.py`, read from harness history at `736d30e`). A FAIL is a
change of an instrument or of a record. In-memory mutations: a trailing comment counted as comment -> 7
UNNAMED probe fails (+4 ratio cross-checks); names.py dropping one name -> 34 UNNAMED.

**Pitfalls, each one measured.** (1) Counts taken before 2026-10-11 (11.321's follow-up) with names.py /
writers.py read `ssystem_factory.hpp` short (78 names of 162; `SSystemFactory` 0 fields) and lost member
functions in four `.cpp` holding a `'"'` char literal (AppCommandInterface 5 of 91, HipStarMgr 52 of 82,
ConstellationMgr, ScriptAnnotator): the old two-step comment stripping; `decomment.py` reads C++ in source
order now - compare an old record with the new block knowing this. (2) The D14 gate
cannot see the harness repo (`claude/` is ignored by the code repo): a harness file's ASCII check is a
byte census (`LC_ALL=C /usr/bin/grep -c -P '[\x80-\xff]' <file>`). (3) form.py classes lines by ratio.py's
`classify` since the same day: SHAPE.md section 4's library row was measured counting 34 `*ptr = ...` code
lines as comments (4.8 / 39 % then, 5.2 / 37 % now). (4) A class whose .cpp alone changed needs `--class`. (5) A WORKTREE run beside an executor that edits
can catch a half-written file: the guard says so (exit 2); run it again.
