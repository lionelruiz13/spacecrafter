#!/usr/bin/env python3
"""F71 item 3: per-name documentation for flags, colours and font targets.

Converts three families from the D7 v1 shape (a plain array of names) to the v2
shape (an array of objects) and fills each with a documentation line traced to
the engine, a source anchor, and where it matters a note about the sharp edge.
184 of the 227 family names had no documentation of their own; this closes 153
of them (97 flags + 46 colours + 10 font targets), leaving the 31 the code
cannot answer at the zero-knowledge bar: `obsolete_tokens` (7 names whose
behaviour was REMOVED -- what they used to do is not in the tree) and
`reserved_variables` (24, blocked upstream on app_command_eval.cpp, which is
scedit INTENT S5 item 5 and was deliberately not read here).

Content comes from `harness/f71/*.json`, produced by three source-reading
passes over the registration -> enumerator -> switch-case -> module chain.
Nothing was written that the chain could not answer; the sharp claims were
re-verified independently at source before merge (see the F71 journal entry for
which, and for what the verification found).

The merged contract is the only file touched: `checkFragments` compares the four
extraction fragments against `families.commands` and `families.set_names` only,
and none of these three families appears in them.
"""

import json
import os

ROOT = "/home/claude/spacecrafter/util/scedit/grammar"
SRC = "/home/claude/spacecrafter/claude/harness/f71"

NOTE = {
    "flags": (
        "D7 v2 SHAPE since 2026-08-31 (F71 item 3): `names` is an array of "
        "OBJECTS, one per registered flag, each carrying a zero-knowledge `doc` "
        "line and the `source` chain it was read from (app_command_init.cpp "
        "registration -> FLAG_NAMES enumerator -> the setFlag switch case -> "
        "coreLink -> the owning module). The VALUE grammar is NOT repeated here: "
        "it is one grammar for all 97 names and lives on the `flag` command "
        "entry, which is where a consumer must read it (I2). `notes` carries the "
        "sharp edge where one exists -- a cross-flag side effect, a name that "
        "contradicts its call, or a flag nothing reads."
    ),
    "color_names": (
        "D7 v2 SHAPE since 2026-08-31 (F71 item 3): one object per colour name, "
        "`doc` + `source` traced app_command_init.cpp registration -> "
        "COLORCOMMAND_NAMES -> the applyColor switch case -> the module that owns "
        "what is being coloured. The `color` COMMAND's own argument keys "
        "(property / value / r / g / b / index) stay on the command entry; this "
        "family documents the NAMES only. All 46 have a live applyColor case."
    ),
    "font_targets": (
        "D7 v2 SHAPE since 2026-08-31 (F71 item 3), and the family is now ARMED. "
        "The engine's acceptance test was the blocker and has been read: "
        "`FontFactory::updateFont` (src/appModule/fontFactory.cpp:141-169) looks "
        "the target up in `m_strToTarget`, whose ONLY writer is "
        "`setStrToTarget` (:41-53, called once from the constructor at :38) and "
        "which inserts EXACTLY these ten TF_* spellings, one each "
        "(base_command_interface.hpp:684-693). So the set is closed, and the "
        "lookup is `std::map::find` with the default comparator -- a byte "
        "compare, CASE-SENSITIVE: `Text` and `STARS` are refused. See "
        "`_acceptance_test` for what refusal looks like, which is the reason a "
        "check is worth having at all."
    ),
}


def load(name):
    with open(os.path.join(SRC, name), encoding="utf-8") as f:
        return json.load(f)


def entries(doc, order):
    by = {e["name"]: e for e in doc["names"]}
    if set(by) != set(order):
        raise SystemExit("name set mismatch: %r" % (set(by) ^ set(order)))
    out = []
    for n in order:
        e = by[n]
        o = {"name": n, "doc": e["doc"], "source": e["source"]}
        if e.get("notes"):
            o["notes"] = e["notes"]
        out.append(o)
    return out


def main():
    p = os.path.join(ROOT, "sc-grammar.json")
    with open(p, encoding="utf-8") as f:
        g = json.load(f)
    fam = g["families"]

    for key, src in (("flags", "flags_docs.json"),
                     ("color_names", "colors_docs.json"),
                     ("font_targets", "font_docs.json")):
        doc = load(src)
        order = [n for n in fam[key]["names"]]
        if not all(isinstance(n, str) for n in order):
            raise SystemExit("%s is not v1 any more -- refusing to convert twice" % key)
        fam[key]["names"] = entries(doc, order)
        fam[key]["_schema_note"] = NOTE[key]
        fam[key]["_doc_source"] = doc["_method"]
        if key == "font_targets":
            fam[key]["_acceptance_test"] = doc["_acceptance_test"]
        print("%-14s %3d names -> v2 objects (%d with notes)"
              % (key, len(fam[key]["names"]),
                 sum(1 for n in fam[key]["names"] if "notes" in n)))

    out = json.dumps(g, indent=2, ensure_ascii=False) + "\n"
    if max(out.encode("utf-8")) > 127:
        raise SystemExit("non-ASCII would be written -- D14")
    with open(p, "w", encoding="utf-8") as f:
        f.write(out)


if __name__ == "__main__":
    main()
