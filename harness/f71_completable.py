#!/usr/bin/env python3
"""F71 item 12: the `completable` marker on values[], as DATA.

THE PROBLEM, as the ledger recorded it.  An argument spec's `values` array mixes
literal values (`current`, `toggle`) with DESCRIPTIONS of the rest of the domain
(`<file name>`, `anything else = off`), and nothing in the schema separates them.
scedit therefore guessed: `isCompletableLiteral` in
`util/scedit/src/sc_docindex.cpp:25-36` offers any entry that is a bare
`[A-Za-z0-9_]+` word -- 166 of the 234 distinct entries -- and the ledger noted
ONE known survivor of that rule, `xRRGGBB`, which is a shape and not a value.

WHAT THE PASS FOUND.  There are FIVE, not one.  Besides `xRRGGBB` (7 specs), the
`value_docs` half of the heuristic -- which reads the map's KEYS as values --
picks up four more, all of them documentation labels rather than things an
author may type:

  * `bat`, `swf`, `png`   in `external_viewer.filename`.  That map is keyed by
    FILE EXTENSION (`"avi / mov / mpg / mp4"`, `".sh"`, `"bat"`, ...) while the
    values are `<name>.bat` and friends; and the handler refuses anything under
    five characters anyway (app_command_interface.cpp:2401-2448).
  * `in`                  in `zoom.auto`.  The domain is an OPEN set: `out` and
    `initial` are recognised and every other word means zoom-in.  The value_docs
    entry for `in` says so in as many words ("any word that is neither 'out' nor
    'initial' does this, including the word 'in' itself").  It is typeable and it
    works -- but offering it would teach that `in` is a recognised token, which
    it is not, and an author who learned that would reasonably expect `out` and
    `in` to be a pair.  Same reasoning as `camera value` in f71_defaults.py.

THE SHAPE CHOSEN, and why it is not the `completable: true` boolean the ledger
sketched.  Two fields:

  `completable`          the offerable subset, listed.  A consumer no longer
                         guesses at all -- which is the point of the item.
  `completable_excluded` {token: why}, only where something was excluded.

The duplication this introduces (a token now appears in `values` and again in
`completable`) is real, and I2 calls that a pending silent desync.  It is made
NOT silent by a validator check added in the same commit: for every spec, the
bare-token set of `values` union the `value_docs` keys must equal `completable`
union the keys of `completable_excluded`, exactly.  So a token added later to
`values` fails the seed gate until somebody classifies it -- the gate cannot be
satisfied by silence, only by a decision.  That is the same resolution the merge
used for its own two-copies exposure (`checkFragments`).
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from f71_patch import patch_file   # noqa: E402

import json   # noqa: E402

ROOT = "/home/claude/spacecrafter/util/scedit/grammar"
BARE = re.compile(r"[A-Za-z0-9_]+")

# token -> (spec predicate, reason).  Keyed by the spec it applies to so the
# exclusion is scoped, not a global blacklist that might silently swallow a
# legitimate same-spelled value somewhere else.
EXCLUSIONS = {
    ("body", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("color", "value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("constellation", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("illuminate", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("image", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("media", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("text", "color_value"): {"xRRGGBB": "a SHAPE, not a value: RR/GG/BB stand for two hexadecimal digits each"},
    ("external_viewer", "filename"): {
        "bat": "a value_docs KEY naming a file extension, not a value: the value is `<name>.bat`, and the handler refuses anything shorter than five characters (app_command_interface.cpp:2401-2448)",
        "swf": "a value_docs KEY naming a file extension, not a value: the value is `<name>.swf`",
        "png": "a value_docs KEY naming a file extension, not a value: the value is `<name>.png`",
    },
    ("zoom", "auto"): {
        "in": "typeable and it does work, but only because the domain is OPEN: `out` and `initial` are recognised and EVERY other word zooms in. Offering `in` would teach a recognised token that does not exist, and would suggest `in`/`out` are a pair (value_docs says so itself)",
    },
}


def main():
    merged_path = os.path.join(ROOT, "sc-grammar.json")
    with open(merged_path, encoding="utf-8") as f:
        g = json.load(f)
    cmds = g["families"]["commands"]

    by_file = {merged_path: []}
    n_specs = n_tokens = n_excluded = n_empty = 0
    for cn in cmds:
        if cn.startswith("_"):
            continue
        args = cmds[cn].get("args") or {}
        if not isinstance(args, dict):
            continue
        for key, spec in args.items():
            if key.startswith("_") or not isinstance(spec, dict):
                continue
            if "values" not in spec and "value_docs" not in spec:
                continue
            cand = set()
            for v in spec.get("values") or []:
                if BARE.fullmatch(v):
                    cand.add(v)
            for v in spec.get("value_docs") or {}:
                if BARE.fullmatch(v):
                    cand.add(v)
            excl = EXCLUSIONS.get((cn, key), {})
            for t in excl:
                if t not in cand:
                    raise SystemExit("exclusion %r not a candidate in %s.%s" % (t, cn, key))
            offer = sorted(cand - set(excl))
            n_specs += 1
            n_tokens += len(offer)
            n_excluded += len(excl)
            if not offer:
                n_empty += 1

            unit = cmds[cn]["unit"]
            frag = os.path.join(ROOT, "args", "unit-%d.json" % unit)
            by_file.setdefault(frag, [])
            pm = ["families", "commands", cn, "args", key]
            pf = ["commands", cn, "args", key]
            # Anchor after `values` when there is one, else after `value_docs`:
            # both come before `default` in the extraction's field order.
            after = "value_docs" if "values" not in spec else "values"
            by_file[merged_path].append((pm, after, "completable", offer))
            by_file[frag].append((pf, after, "completable", offer))
            if excl:
                by_file[merged_path].append((pm, "completable", "completable_excluded", excl))
                by_file[frag].append((pf, "completable", "completable_excluded", excl))

    total = 0
    for path, edits in sorted(by_file.items()):
        total += patch_file(path, edits)
        print("%-52s %3d field insertions" % (os.path.basename(path), len(edits)))
    print("specs marked: %d (of which %d offer nothing), tokens offered: %d, excluded with a reason: %d"
          % (n_specs, n_empty, n_tokens, n_excluded))


if __name__ == "__main__":
    main()
