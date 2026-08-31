#!/usr/bin/env python3
"""F71 scope 4: what the engine accepts that `doc/superscript.sts` never shows.

    python3 f71_coverage.py            # writes artifacts/f71/missing_doc.json

WHAT THIS IS FOR.  `doc/superscript.sts` is the tester's file -- years of
documentation, and for years the project's only functional test.  It is
READ-ONLY here and nothing in this task edits it (S11.149(e)).  What this
produces is a PROPOSAL: the parts of the command surface the engine accepts that
his file never shows an author how to use, so he can decide what deserves a line.

THE CRITERION, AND WHY IT IS DELIBERATELY WEAK.  Deciding "is this documented?"
properly would need the file tokenized exactly as `parseCommand` reads it, and a
second implementation of that parser is precisely the divergence risk C1 exists
to prevent.  So this asks a question no parser is needed for: does the spelling
appear in the file AT ALL, as a whole word?

That over-counts coverage on purpose.  A name mentioned once inside an unrelated
sentence counts as present.  The consequence is the property that matters for a
proposal: the ABSENT list is a FLOOR.  Everything on it is genuinely
undocumented -- the six letters are not in the file -- so nothing here can be a
false accusation against the tester's work.  The real gap is larger than what
this reports, never smaller.

THE ONE THING IT DOES PARSE, exactly and from the engine.  Whether a line is
executed at all: `src/scriptModule/script.cpp:114` drops a line iff its first
byte is '#', 0, CR or LF.  That is a byte test, not a grammar, so reimplementing
it carries no C1 risk.  It buys the second tier:

  ABSENT_ENTIRELY     the spelling is nowhere in the file
  COMMENT_ONLY        it appears, but only in lines the engine never executes --
                      described and never demonstrated, so never part of the
                      functional test the file also serves as
  NEVER_WITH_COMMAND  it appears on executed lines, but never on one of the
                      command being asked about. Family names share spellings
                      (`planet_orbits` is both a flag and a colour), so this is
                      the honest state for "the file drives it, but never THIS
                      way" -- and calling that "absent" would be a false claim
                      about the tester's file
  EXERCISED           it appears on an executed line of the right command

ENCODING.  The file is ISO-8859 with CRLF and the Bash `grep` wrapper skips it
silently (CLAUDE.md).  Read as bytes, decoded latin-1 so every byte maps to one
character; the names being searched for are all ASCII, so matching is exact and
the accented bytes are simply carried along.
"""

import json
import os
import re

CODE = "/home/claude/spacecrafter"
WITNESS = os.path.join(CODE, "doc/superscript.sts")
GRAMMAR = os.path.join(CODE, "util/scedit/grammar/sc-grammar.json")
OUT = "/home/claude/spacecrafter/claude/harness/artifacts/f71"


def load_lines():
    raw = open(WITNESS, "rb").read()
    md5 = __import__("hashlib").md5(raw).hexdigest()
    lines = []
    for i, b in enumerate(raw.split(b"\n"), 1):
        # script.cpp:114 -- the drop test is on the FIRST BYTE of the raw line.
        first = b[:1]
        executed = bool(b.strip()) and first not in (b"#", b"\0", b"\r", b"\n")
        lines.append((i, b.decode("latin-1"), executed))
    return lines, md5, len(raw)


def classify(name, lines, restrict_command=None):
    """Return (state, evidence_line).

    States, chosen so the label cannot overstate the gap:

      ABSENT_ENTIRELY     the six letters are not in the file, anywhere
      COMMENT_ONLY        present, but only in lines the engine never executes
      NEVER_WITH_COMMAND  present on executed lines, but never on one belonging
                          to the command being asked about -- this is the state
                          a family member reaches when its spelling is shared.
                          `planet_orbits` is both a flag name and a colour name;
                          the file drives it as a flag on many lines and never
                          as a colour, and calling that "absent" would be wrong.
      EXERCISED           present on an executed line of the right command

    The command test is the FIRST TOKEN of the line, lowercased, which is what
    parseCommand takes as the command; the key/value split after it is not
    parsed here (see the module docstring for why not).
    """
    pat = re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")
    seen_comment = None
    seen_other_command = None
    for num, text, executed in lines:
        if not pat.search(text):
            continue
        if executed:
            if restrict_command is not None:
                head = text.strip().split()
                if not head or head[0].lower() != restrict_command:
                    if seen_other_command is None:
                        seen_other_command = num
                    continue
            return "EXERCISED", num
        if seen_comment is None:
            seen_comment = num
    if seen_other_command is not None:
        return "NEVER_WITH_COMMAND", seen_other_command
    if seen_comment is not None:
        return "COMMENT_ONLY", seen_comment
    return "ABSENT_ENTIRELY", None


def main():
    lines, md5, nbytes = load_lines()
    g = json.load(open(GRAMMAR, encoding="utf-8"))
    fam = g["families"]
    cmds = fam["commands"]

    report = {
        "_produced": "2026-08-31, F71 scope 4",
        "_witness": {"path": "doc/superscript.sts", "md5": md5, "bytes": nbytes,
                     "lines": len(lines),
                     "executed_lines": sum(1 for _, _, e in lines if e)},
        "_grammar": "util/scedit/grammar/sc-grammar.json at code master-beta @ 94f2af65",
        "_criterion": ("whole-word presence in the file; ABSENT is a FLOOR (nothing here "
                       "is in the file at all). COMMENT_ONLY = described but never "
                       "demonstrated, so never covered by the functional test the file "
                       "doubles as. Executed-line test = script.cpp:114, a first-byte rule."),
        "sections": {},
    }

    def bucket(label, names, restrict=None):
        rows = {"ABSENT_ENTIRELY": [], "NEVER_WITH_COMMAND": [], "COMMENT_ONLY": [], "EXERCISED": 0}
        for n, meta in names:
            state, line = classify(n, lines, restrict(n) if restrict else None)
            if state == "EXERCISED":
                rows["EXERCISED"] += 1
            else:
                row = {"name": n, "line": line}
                row.update(meta)
                rows[state].append(row)
        report["sections"][label] = rows
        print("%-16s absent %3d  never-with-cmd %3d  comment-only %3d  exercised %3d"
              % (label, len(rows["ABSENT_ENTIRELY"]), len(rows["NEVER_WITH_COMMAND"]),
                 len(rows["COMMENT_ONLY"]), rows["EXERCISED"]))

    # commands
    bucket("commands", [(k, {"doc": (v.get("doc") or "")[:90]})
                        for k, v in cmds.items()
                        if not k.startswith("_") and isinstance(v, dict)])

    # argument keys, scoped to their own command's lines
    keys = []
    for cn, ce in cmds.items():
        if cn.startswith("_") or not isinstance(ce, dict):
            continue
        args = ce.get("args")
        if not isinstance(args, dict):
            continue
        for k, spec in args.items():
            if k.startswith("_") or not isinstance(spec, dict):
                continue
            keys.append((k, {"command": cn, "doc": (spec.get("doc") or "")[:90]}))
    # scope each key to lines whose first token is its command
    rows = {"ABSENT_ENTIRELY": [], "NEVER_WITH_COMMAND": [], "COMMENT_ONLY": [], "EXERCISED": 0}
    for k, meta in keys:
        state, line = classify(k, lines, meta["command"])
        if state == "EXERCISED":
            rows["EXERCISED"] += 1
        else:
            rows[state].append({"name": k, "line": line, **meta})
    report["sections"]["argument_keys"] = rows
    print("%-16s absent %3d  never-with-cmd %3d  comment-only %3d  exercised %3d"
          % ("argument_keys", len(rows["ABSENT_ENTIRELY"]), len(rows["NEVER_WITH_COMMAND"]),
             len(rows["COMMENT_ONLY"]), rows["EXERCISED"]))

    def members(family):
        out = []
        for n in fam[family]["names"]:
            if isinstance(n, str):
                out.append((n, {}))
            else:
                out.append((n["name"], {"doc": (n.get("doc") or "")[:90]}))
        return out

    for f, restrict in (("flags", "flag"), ("color_names", "color"),
                        ("set_names", "set"), ("font_targets", "font")):
        rows = {"ABSENT_ENTIRELY": [], "NEVER_WITH_COMMAND": [], "COMMENT_ONLY": [], "EXERCISED": 0}
        for n, meta in members(f):
            state, line = classify(n, lines, restrict)
            if state == "EXERCISED":
                rows["EXERCISED"] += 1
            else:
                rows[state].append({"name": n, "line": line, **meta})
        report["sections"][f] = rows
        print("%-16s absent %3d  never-with-cmd %3d  comment-only %3d  exercised %3d"
              % (f, len(rows["ABSENT_ENTIRELY"]), len(rows["NEVER_WITH_COMMAND"]),
                 len(rows["COMMENT_ONLY"]), rows["EXERCISED"]))

    os.makedirs(OUT, exist_ok=True)
    p = os.path.join(OUT, "missing_doc.json")
    text = json.dumps(report, indent=1, ensure_ascii=False) + "\n"
    if max(text.encode()) > 127:
        raise SystemExit("non-ASCII in the report -- D14")
    open(p, "w", encoding="utf-8").write(text)
    print("\nwrote " + p)


if __name__ == "__main__":
    main()
