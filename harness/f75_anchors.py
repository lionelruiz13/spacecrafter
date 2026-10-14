#!/usr/bin/env python3
"""f75_anchors.py -- the grammar's engine anchors: parse, pin, map to HEAD, sweep.

WHAT THIS IS FOR (task F75, mirror-ledger journal 2026-09-01a).
`util/scedit/grammar/sc-grammar.json` and its four fragments `grammar/args/
unit-{1..4}.json` carry `file:line` anchors into the engine.  They were pinned
ONCE, at file level, and the engine has moved since, so a reader standing at
HEAD lands on unrelated code (parent INTENT 11.190(e)).  This tool

  1. PARSES every anchor, enumerating the shapes it meets (a shape it cannot
     read is a REPORTED residual, never a skipped one);
  2. RECOVERS each anchor's pin -- the engine tree it was written against --
     from the grammar file's own history in the code repo;
  3. MAPS each (pin, file, line) to HEAD BY CONTENT: the text at the pin must
     be the text at HEAD.  A line whose text no longer exists at HEAD is
     FLAGGED with both texts, never re-pointed by arithmetic;
  4. SWEEPS both halves identically -- the merged file and the fragments carry
     the same facts and `checkFragments` (util/scedit/src/main.cpp) compares
     them on every seed-gate run, so the two must move together.

The write path is TEXT-LEVEL string replacement on the raw bytes, because the
fragments are hand-formatted and do NOT round-trip through json.dumps.  The
transformation is a pure function of a string's CONTENT (its pin is recovered
from its content/path, not from where it sits), so replacing every occurrence
of an old string by its new form is well defined across the five files.

Usage:
  f75_anchors.py shapes      -- shape census, with counts
  f75_anchors.py pins        -- pin recovery + its validation
  f75_anchors.py map         -- the HEAD partition (writes f75/anchor_map.json)
  f75_anchors.py sweep       -- apply (writes the five files) ; --dry to not write
  f75_anchors.py verify      -- post-sweep byte proofs
"""
import json, os, re, subprocess, sys, collections, difflib

REPO = "/home/claude/spacecrafter"
HARN = os.path.dirname(os.path.abspath(__file__))
OUT  = os.path.join(HARN, "f75")

# THE ANCHOR GRAMMAR LIVES IN THE CODE TREE, and this file imports it (I2).
# `util/scedit/tests/anchor_gate.py` is the gate everybody who builds scedit
# runs, so it owns the parse and resolution layer: the shapes, the file
# resolution, the pin->line mapping, the disambiguation rules and their
# recorded readings. A second copy here would be two implementations of one
# contract, drifting quietly the first time either was corrected -- and this
# task exists because of exactly that failure mode one level up.
sys.path.insert(0, os.path.join(REPO, "util", "scedit", "tests"))
from anchor_gate import (                                  # noqa: E402
    MERGED, FRAGS, TARGETS, DEFAULT_FILE, SUBMODULE, RESPELL, FILE_OVERRIDES,
    Tok, tokenize, walk, load, git, blob, lines_at, repo_index, resolve_file,
    enclosing_symbol, linemap, classify, classify_any, sub_commit,
    disambiguate, _score, ELEM, FUNC, QUOTED)

# --------------------------------------------------------------------------
# 3. PIN RECOVERY
#
# The grammar lives in the CODE repo, so its own history names, for every
# anchor, the engine tree it was written against.  Two methods, and the second
# is the one that survives contact with the record:
#
#   CONTENT PICKAXE  the earliest revision whose blob holds this exact string.
#     REJECTED as primary: F70's ASCII sweep (1012c643) REWROTE 215 anchor
#     strings without touching a single line number, so the pickaxe dates those
#     anchors to the sweep -- months after they were written, and after
#     app_command_interface.cpp had moved twice.
#
#   PATH TRACKING (primary)  the earliest revision at which this JSON path
#     already carried THIS anchor token list.  A rewrite that leaves the line
#     numbers alone is transparent to it, which is exactly the property the
#     pickaxe lacks.
#
# The pin of a string is one revision; every anchor in it inherits it.
# --------------------------------------------------------------------------
def revisions(path):
    return git("log", "--format=%H", "--reverse", "--", path).decode().split()

def anchor_key(s):
    """The line-number identity of a string: what a rewrite must change and an
    accent removal must not."""
    return tuple((t.kind, t.head, tuple((a, b) for a, b, _, _ in t.elems)) for t in tokenize(s))

def recover_pins(path):
    """-> {json_path: (rev, method)} for every anchor-bearing string at HEAD.

    A string is dated by the earliest revision of the grammar that already held
    THIS anchor token list on a string that is recognisably the same string.
    Neither half of that is optional, and each was learned from a miss:

      * keying on the JSON PATH alone dates `lint_seeds[11]` to 7fd5ea75,
        because a seed was later inserted ahead of it and every index shifted.
        The anchors it carries (`commandStruct :4604-4661 / :4675-4699`) land
        exactly on the if-block and the loop-block at 2fe14699 and nowhere
        else -- so the path is not the identity.
      * keying on the exact TEXT alone dates 198 strings to F70's ASCII sweep,
        which changed `S5` for a section sign and no line number at all.

    So the key is the anchor token list, matched anywhere in the revision, with
    a similarity floor of 0.85 -- high, because the two rewrites this has to see
    through changed 2 bytes in 180 (a section sign) and 0 bytes (an index
    shift); a looser floor let ONE F71 string match an unrelated 73cc7b80
    string that cited the same lines. The floor keeps two unrelated strings
    that happen to cite the same lines from being confused for one another.
    """
    revs = revisions(path)
    today = {p: s for p, s in walk(load(path)) if tokenize(s)}
    want = {p: anchor_key(s) for p, s in today.items()}
    pin, how = {}, {}
    for r in revs:
        g = load(path, r)
        if g is None:
            continue
        bykey = collections.defaultdict(list)
        for p2, s2 in walk(g):
            k = anchor_key(s2)
            if k:
                bykey[k].append((p2, s2))
        for p2, k in want.items():
            if p2 in pin or k not in bykey:
                continue
            best = max((difflib.SequenceMatcher(a=s2, b=today[p2],
                                                autojunk=False).ratio(), q)
                       for q, s2 in bykey[k])
            if best[0] >= 0.85:
                pin[p2] = r
                how[p2] = ("same-path" if best[1] == p2 else "moved-path")
    out = {}
    for p2 in today:
        out[p2] = (pin.get(p2, revs[-1]), how.get(p2, "unrecovered"))
    return out, today


# --------------------------------------------------------------------------
# COMMANDS
# --------------------------------------------------------------------------
def cmd_shapes():
    tot = collections.Counter(); ex = {}
    per = collections.Counter()
    for path in TARGETS:
        for p, s in walk(load(path)):
            for t in tokenize(s):
                sub = []
                if t.head and re.search(r'\s$', s[t.start:t.end].split(":")[0] or " "):
                    pass
                if "-" in t.listtext: sub.append("range")
                if "," in t.listtext: sub.append("comma-list")
                if t.kind != "FILE" and t.file is None: sub.append("no-file")
                k = t.kind + ("|" + "|".join(sub) if sub else "")
                tot[k] += 1; per[(path, t.kind)] += 1
                ex.setdefault(k, s[max(0, t.start - 22):t.end + 4])
    print("ANCHOR SHAPES (all five files), token = one `:<line-list>` occurrence")
    for k, v in tot.most_common():
        print("  %6d  %-26s  ...%s" % (v, k, ex[k].replace("\n", " ")[:56]))
    print("  %6d  TOTAL tokens" % sum(tot.values()))
    refs = 0
    for path in TARGETS:
        for p, s in walk(load(path)):
            for t in tokenize(s):
                refs += len(t.elems)
    print("  %6d  TOTAL references (a comma element and a range are one each)" % refs)
    print("\nPER FILE (tokens by class)")
    for path in TARGETS:
        row = {k[1]: v for k, v in per.items() if k[0] == path}
        print("  %-44s %s" % (path.split("/")[-1], row))

def cmd_pins():
    allpins = {}
    for path in TARGETS:
        pins, today = recover_pins(path)
        allpins[path] = {"pins": {k: list(v) for k, v in pins.items()}, "n": len(today)}
        c = collections.Counter((r, m) for r, m in pins.values())
        print("== %s : %d anchor strings" % (path.split("/")[-1], len(today)))
        for (r, m), n in c.most_common():
            print("     %-9s %-9s %5d" % (r[:8], m, n))
    os.makedirs(OUT, exist_ok=True)
    json.dump(allpins, open(os.path.join(OUT, "pins.json"), "w"), indent=1)
    print("\n-> %s/pins.json" % OUT)

# --------------------------------------------------------------------------
# 7. THE MAP: every reference partitioned
#
#   clean      the text at the pin is at the SAME line at HEAD
#   moved      the text at the pin is at a DIFFERENT line at HEAD (verified)
#   gone       the line's text does not survive to HEAD  -> re-read / NOT AT HEAD
#   no-file    the string never names a file for this token
#   unresolved the named file is not in this tree at this pin
# --------------------------------------------------------------------------
def build_map():
    pins = json.load(open(os.path.join(OUT, "pins.json")))
    result = {}
    for path in TARGETS:
        P = pins[path]["pins"]
        rows = []
        for jp, s in walk(load(path)):
            toks = tokenize(s)
            if not toks:
                continue
            pin = P[jp][0]
            cm = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == MERGED
                          else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
            handler = None
            if cm:
                cmds = (load(path)["families"]["commands"] if path == MERGED
                        else load(path)["commands"])
                handler = cmds.get(cm.group(1), {}).get("handler")
            for ti, t in enumerate(toks):
                evidence = None
                if t.kind == "FILE":
                    rp, how = resolve_file(t.file)
                    basis = "respelled" if how == "respelled" else "named"
                else:
                    rp, how, basis, evidence = disambiguate(t, pin, s, handler)
                    if basis == "FLAG":
                        rt = READ_TABLE.get((t.file, t.listtext)) or \
                             READ_TABLE.get((t.head, t.listtext))
                        if rt and rt[0] == "file":
                            rp, how, basis = rt[1], "repo-relative", "read"
                            evidence = "READ_TABLE"
                if rp is None:
                    for ei, (a, b, _, _) in enumerate(t.elems):
                        rows.append(dict(jp=jp, ti=ti, ei=ei, pin=pin, file=t.file,
                                         basis=basis, ev=evidence, old=[a, b],
                                         verdict="unresolved:" + (how or basis), new=None))
                    continue
                for ei, (a, b, _, _) in enumerate(t.elems):
                    va, na, ta, ha = classify_any(pin, rp, a, how)
                    if b is None:
                        rows.append(dict(jp=jp, ti=ti, ei=ei, pin=pin, file=t.file, repo=rp,
                                         how=how, basis=basis, ev=evidence,
                                         old=[a, None], verdict=va, new=[na, None], text=ta))
                    else:
                        vb, nb, tb, hb = classify_any(pin, rp, b, how)
                        OK = ("clean", "moved", "d14", "d14-moved")
                        v = "clean" if va == vb == "clean" else \
                            (("d14" if "d14" in (va, vb) or "d14-moved" in (va, vb) else "moved")
                             if va in OK and vb in OK else
                             (va if va not in OK else vb))
                        rows.append(dict(jp=jp, ti=ti, ei=ei, pin=pin, file=t.file, repo=rp,
                                         how=how, basis=basis, ev=evidence,
                                         old=[a, b], verdict=v, new=[na, nb],
                                         text=ta, text2=tb))
        result[path] = rows
    return result

def cmd_map():
    res = build_map()
    os.makedirs(OUT, exist_ok=True)
    json.dump(res, open(os.path.join(OUT, "anchor_map.json"), "w"))
    grand = collections.Counter(); basis = collections.Counter()
    for path, rows in res.items():
        basis.update(r.get("basis", "?") for r in rows)
        c = collections.Counter(r["verdict"].split(":")[0] for r in rows)
        print("== %-16s %5d references" % (path.split("/")[-1], len(rows)))
        for k, v in c.most_common():
            print("     %-14s %5d" % (k, v))
        grand.update(c)
    print("== TOTAL             %5d references" % sum(grand.values()))
    for k, v in grand.most_common():
        print("     %-14s %5d" % (k, v))
    print("\nBY BASIS (how the reference's FILE was determined)")
    for k, v in basis.most_common():
        print("     %-26s %5d" % (k, v))
    ev = collections.Counter()
    for rows in res.values():
        for r in rows:
            if r.get("ev"):
                ev[re.sub(r'\(.*?\)', '', r["ev"])] += 1
    print("BY EVIDENCE (which rules decided an inherited file)")
    for k, v in ev.most_common():
        print("     %-26s %5d" % (k, v))
    # what is unresolved, in detail
    print("\nUNRESOLVED file names:")
    d = collections.Counter()
    for rows in res.values():
        for r in rows:
            if r["verdict"].startswith("unresolved"):
                d[(r["file"], r["verdict"])] += 1
    for (f, v), n in d.most_common():
        print("   %-52s %-28s %d" % (f, v, n))

# --------------------------------------------------------------------------
# 8. IS THE IMPLICIT DEFAULT REALLY app_command_interface.cpp?
#
# 951 references sit in strings that name no file at all (`... tested at :225
# and cleared at :229`).  Nothing STATES their referent -- the fragments'
# `_meta.source_anchor_convention` rules on bare BASENAMES, not on bare `:N`.
# So it is measured, by a check that can fail: the grammar already says which
# HANDLER each command has, and a string attached to command X that anchors
# into app_command_interface.cpp must land inside X's handler.  A string about
# `media` landing inside commandDate would refute the default.
# --------------------------------------------------------------------------
def cmd_default_check():
    res = json.load(open(os.path.join(OUT, "anchor_map.json")))
    ok = bad = untestable = 0
    misses = []
    for path, rows in res.items():
        g = load(path)
        cmds = g["families"]["commands"] if path == MERGED else g["commands"]
        for r in rows:
            if r.get("basis") not in ("implicit-default", "default-after-refutation"):
                continue
            m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == MERGED
                         else r'\.commands\.([A-Za-z0-9_]+)\.', r["jp"])
            if not m:
                untestable += 1
                continue
            entry = cmds.get(m.group(1), {})
            handler = entry.get("handler")
            if not handler:
                untestable += 1
                continue
            sym = enclosing_symbol(r["pin"], DEFAULT_FILE, r["old"][0])
            if sym == handler:
                ok += 1
            else:
                bad += 1
                if len(misses) < 12:
                    misses.append((path.split("/")[-1], m.group(1), handler, sym, r["old"][0]))
    print("IMPLICIT-DEFAULT CHECK -- the enclosing function at <default>:<line> at the pin")
    print("  must be the handler the grammar names for the command the string belongs to.")
    print("  agree     %5d" % ok)
    print("  DISAGREE  %5d" % bad)
    print("  untestable%5d  (parse_model / _meta / unit_findings: no command scope)" % untestable)
    for m in misses:
        print("     %-16s %-14s grammar=%-22s enclosing=%-22s :%d" % m)
    return bad == 0



# --------------------------------------------------------------------------
# 10. THE READ TABLE -- the references no mapping can settle, settled by hand
#
# 14 rows. Each says what was read, at HEAD, and why the row says what it
# says. Nothing here is arithmetic: every one of them is a referent that was
# REWRITTEN between its pin and HEAD, and the question "is this still the same
# thing" is not one a line map can answer.
#
# Key: (repo path or the name as written, the token's line list, the pin).
# Value: ("lines", <new list text>, reason) -- re-anchored by reading
#        ("notathead", None, marker reason, reason) -- C2's honest state; the
#                                              MARKER reason is short and holds
#                                              no `file:line`, or the marker
#                                              would itself become an anchor
#        ("file", <repo path>, reason)      -- the file, not the lines, was
#                                              what could not be decided
# --------------------------------------------------------------------------
READ_TABLE = {
 ("app_command_init.cpp", "112-118"):
   ("file", "src/interfaceModule/app_command_init.cpp",
    "TIE (paren-scoped vs flat inheritance) broken by reading: the sentence is "
    "'registered in the aliases block AFTER the reverse map is built', and the "
    "aliases block is in app_command_init.cpp, not in the header the "
    "parenthesis named last."),
 ("src/interfaceModule/app_command_init.cpp", "44-45"):
   ("lines", "44, 117",
    "The two registrations are no longer adjacent. `camera` is still at 44; the "
    "`flyto` alias moved into the aliases block added 2026-08-31 (7fd5ea75) and "
    "is at 117 at HEAD, its `//alias of camera` comment replaced by the block's "
    "own two-line comment. A range became a pair because the referent did."),
 ("src/interfaceModule/app_command_init.cpp", "45"):
   ("lines", "117", "Same move: the `flyto` registration is at 117 at HEAD."),
 ("src/interfaceModule/app_command_interface.cpp", "1171-1174"):
   ("lines", "1315-1330",
    "executeCommandStatus's else branch. `} else {` is at 1315 at HEAD; the "
    "second log write, `cLog::get()->write(debug_message, ...)`, is at 1330 "
    "carrying the origin tag F68 added. Same two ends, both rewritten by F73's "
    "unified rendering (11.193) -- and the string's PROSE about them is now "
    "stale, which is a content finding and is routed, not applied."),
 ("src/interfaceModule/if_swap.cpp", "40-54"):
   ("lines", "40-54",
    "IfSwap::pop occupies exactly 40-54 at HEAD as at the pin. It reads as "
    "`gone` only because its FIRST line changed, `void` -> `bool` (F72/F73 gave "
    "it a return value so the caller can report). The block did not move."),
 ("src/interfaceModule/if_swap.cpp", "57-61"):
   ("lines", "57-62",
    "IfSwap::push, 57-62 at HEAD: one line longer because it now takes a "
    "`const ScriptOrigin &opener` and pushes it onto m_openers."),
 ("src/interfaceModule/if_swap.cpp", "71-81"):
   ("lines", "73-81",
    "IfSwap::revert, 73-81 at HEAD: `bool` instead of `void` and two comment "
    "lines ahead of it moved its opening line down by two."),
 ("src/interfaceModule/if_swap.cpp", "45"):
   ("notathead", None,
    "the referent left this file: IfSwap::pop now returns false and the caller reports",
    "The `end without if` cLog write is NOT in "
    "if_swap.cpp at HEAD. IfSwap::pop returns false instead and the CALLER "
    "reports (app_command_interface.cpp:4777-4778, reportScriptError with "
    "MSG_END_WITHOUT_IF). The referent left the file, so re-pointing it inside "
    "the file would be false; moving the anchor to another file would change "
    "what the sentence CITES, which is a content decision this task does not "
    "take. Routed: the surrounding prose says the engine logs it there."),
 ("src/interfaceModule/if_swap.cpp", "76"):
   ("notathead", None,
    "the referent left this file: IfSwap::revert now returns false and the caller reports",
    "Same as :45 for `else without if`: IfSwap::revert returns false and the "
    "caller reports (app_command_interface.cpp:4772-4773, MSG_ELSE_WITHOUT_IF)."),
 ("src/scriptModule/script_mgr.cpp", "306-330"):
   ("lines", "310-338",
    "The dispatch loop's body: `uint64_t wait=0;` is at 310 at HEAD (a "
    "ScriptOrigin declaration was added above it) and the executeCommand call "
    "that ends the span is at 338, now `executeCommand(comd, wait, origin)`."),
 ("src/scriptModule/script_mgr.cpp", "327-328"):
   ("lines", "331-332",
    "`if (isInLoop)` and its push: 331-332 at HEAD, the push now recording "
    "`{comd, origin}` instead of the bare line."),
 ("fscripts/panorama5.sts", "102"):
   ("notathead", None,
    "field data, not a file of this repository",
    "FIELD DATA, same file as the row below and the same reason."),
 ("fscripts/panorama5.sts", "100-102"):
   ("notathead", None,
    "field data, not a file of this repository",
    "FIELD DATA. `~/.spacecrafter/scripts/fscripts/panorama5.sts` is a shipped "
    "script, not a file of this repository, so no pin of this tree resolves it "
    "and no gate here can watch it. The anchor is honest and stays as written."),
}
# `_meta.amended[4]`'s `F.sts:1` names harness/artifacts/f63/F.sts -- a
# DERIVED harness artifact (the harness repo git-ignores `artifacts`). It is
# not an engine anchor and not gateable from the code tree; it is excluded by
# name, with this reason, rather than reported as a broken reference. It sits
# in `_meta` and is excluded on that ground as well.

# --------------------------------------------------------------------------
# 11. THE SWEEP
#
# `_meta` IS EXCLUDED, and this is the one exclusion that had to be argued
# rather than declared. The fragments' `_meta` is a MEASUREMENT record taken at
# b12c8cdd: `handler_range` "app_command_interface.cpp:1180-2109" is the range
# over which `args_bracket_expected: 68` was counted, and the 384 rows of
# `accounting` / `other_args_uses` carry their line as a BARE INTEGER
# (`{"line": 1203}`) that no `file:line` parser can see. Sweeping the 69
# `file:line` references of `_meta` while those 384 stayed at b12c8cdd would
# manufacture precisely the silent desync I2 exists to prevent, and would
# break the fragments' own count gate. `_meta.amended` is worse still: it is
# an append-only DATED record, and one of its entries reads "comments.
# script_layer's anchor corrected 114 -> 117 in the same pass" -- rewriting
# 117 would falsify a record of what was done, not update a citation.
#
# So `_meta` keeps `_meta.code` as its own pin, the anchors outside `_meta`
# get `_meta.anchor_pin`, and the two pins carry two DIFFERENT facts.
# --------------------------------------------------------------------------
ANCHOR_PIN_SHA = "54a2b844"
ANCHOR_PIN = "master-beta @ " + ANCHOR_PIN_SHA
SELFPIN = re.compile(r' @ d64fd437\b')
# Two strings state their pin in PROSE rather than in the `@ <sha>` form
# ("Read-only trace at HEAD d64fd437, four hops per name: ..."). Their numbers
# move with everything else, so leaving the sha would make the sentence
# contradict the very anchors it introduces -- a pin is a pin whichever way it
# is spelled.
PROSEPIN = re.compile(r'\bat HEAD d64fd437\b')

def rewrite_string(path, jp, s, rows_by_key):
    """-> (new_s, [notes]) ; new_s is s when nothing moves."""
    if jp.startswith("._meta"):
        return s, []
    toks = tokenize(s)
    if not toks:
        return s, []
    out, notes, last = [], [], 0
    for ti, t in enumerate(toks):
        rows = rows_by_key.get((jp, ti), [])
        lo, hi = t.lspan
        pieces, cur, marker = [], 0, None
        for ei, em in enumerate(ELEM.finditer(t.listtext)):
            r = rows[ei] if ei < len(rows) else None
            etext = em.group(0)
            rt = READ_TABLE.get((t.file, etext))
            if rt is None and r:
                rt = READ_TABLE.get((r.get("repo"), etext))
            pieces.append(t.listtext[cur:em.start()])
            if rt and rt[0] == "lines":
                pieces.append(rt[1])
                notes.append(("read", t.file, etext, rt[1], rt[2]))
            elif rt and rt[0] == "notathead":
                pieces.append(etext)
                marker = " @ %s [NOT AT HEAD: %s]" % (
                    (r["pin"][:8] if r else "?"), rt[2])
                notes.append(("notathead", t.file, etext, None, rt[-1]))
            elif r and r["verdict"] in ("clean", "moved", "d14", "d14-moved"):
                a, b = r["new"]
                pieces.append(str(a) + (em.group(2) + str(b) if em.group(3) else ""))
            else:
                pieces.append(etext)
                marker = " @ %s [NOT AT HEAD: file not resolvable in this tree]" % (
                    r["pin"][:8] if r else "?")
                notes.append(("unresolved", t.file, etext, None,
                              r["verdict"] if r else "no row"))
            cur = em.end()
        pieces.append(t.listtext[cur:])
        out.append(s[last:lo]); out.append("".join(pieces)); last = hi
        if marker:
            out.append(s[last:t.end]); out.append(marker); last = t.end
    out.append(s[last:])
    new = "".join(out)
    # The self-pin folds into the file pin whenever this string keeps no
    # deviation of its own -- INCLUDING when none of its numbers moved. Folding
    # only the strings that moved would leave 51 of the 60 F71 pins behind,
    # saying a per-string thing the file now says once (I2, and it is the whole
    # point of the exercise).
    if "[NOT AT HEAD:" not in new:
        new = SELFPIN.sub("", new)
        new = PROSEPIN.sub("at HEAD " + ANCHOR_PIN_SHA, new)
    return new, notes

def build_rewrites():
    res = json.load(open(os.path.join(OUT, "anchor_map.json")))
    rewrites, allnotes = {}, []
    for path in TARGETS:
        by = collections.defaultdict(list)
        for r in res[path]:
            by[(r["jp"], r["ti"])].append(r)
        for k in by:
            by[k].sort(key=lambda r: r["ei"])
        for jp, s in walk(load(path)):
            new, notes = rewrite_string(path, jp, s, by)
            if new != s:
                if s in rewrites and rewrites[s] != new:
                    raise SystemExit("CONTENT COLLISION: one string, two rewrites\n%r" % s)
                rewrites[s] = new
            for n in notes:
                allnotes.append((path, jp) + n)
    return rewrites, allnotes

def cmd_read_table_check():
    """Every read-table row is keyed WITHOUT its pin. That is only legitimate
    if the row's text at each of its pins is the same text -- otherwise two
    pins would be two different referents wearing one key. Checked, not
    assumed."""
    res = json.load(open(os.path.join(OUT, "anchor_map.json")))
    byrow = collections.defaultdict(set)
    for rows in res.values():
        for r in rows:
            lt = ("%d-%d" % tuple(r["old"])) if r["old"][1] else str(r["old"][0])
            for k in ((r.get("file"), lt), (r.get("repo"), lt)):
                if k in READ_TABLE and r.get("repo"):
                    byrow[k].add((r["pin"], r["repo"], tuple(r["old"])))
    bad = 0
    for k, v in sorted(byrow.items()):
        texts = set()
        for pin, rp, old in v:
            ls = lines_at(pin, rp)
            texts.add(tuple(ls[n - 1] for n in old if n) if ls else None)
        state = "OK" if len(texts) == 1 else "DIFFERS"
        if len(texts) != 1:
            bad += 1
        print("  %-7s %-44s %-9s %d pin(s): %s" %
              (state, k[0][:44], k[1], len(v), " ".join(sorted(p[:8] for p, _, _ in v))))
    print("rows whose pins disagree on the text: %d" % bad)
    return bad == 0


AMENDED_NEW = (
    "2026-09-01 (F75): every engine anchor OUTSIDE `_meta` re-resolved against "
    "code " + ANCHOR_PIN_SHA + " and rewritten to the line that holds the text it was "
    "written against; the pin becomes ONE file-level fact, `_meta.anchor_pin`, "
    "and the 60 per-string `@ d64fd437` pins fold into it. 6663 references swept "
    "(the 69 inside `_meta` are NOT: that block is a measurement record taken at "
    "`code`, whose 384 accounting rows carry their line as a bare integer no "
    "`file:line` parser can see, so moving half of it is I2's silent desync). "
    "Six referents were re-anchored BY READING and four carry `[NOT AT HEAD]` "
    "because their referent left the file or the file is not in this tree - the "
    "table and its reasons are in `claude/harness/f75_anchors.py` (READ_TABLE). "
    "A ctest, `anchor_gate`, now reds when the engine moves under them.")
SUPERSEDE = (" [SUPERSEDED 2026-09-01, F75: the anchors resolve at HEAD again. "
             "The pin is now `_meta.anchor_pin` and it is " + ANCHOR_PIN + "; "
             "the `@ <sha>` form survives only on a reference whose pin must "
             "deviate from the file's, and after this sweep that is only the "
             "`[NOT AT HEAD]` rows. This sentence is kept, not deleted: it is "
             "the record of the state F71 left and F75 closed.]")

def add_file_pin(path, out):
    """One file-level pin per file, added where a reader of `_meta` will look.
    The fragments are hand-formatted, so this is a text insertion beside their
    existing `code` line -- which STAYS, because it pins that block's own
    measurements and this one pins the anchors: two facts, two places."""
    if path == MERGED:
        g = json.loads(out)
        m = g["_meta"]
        new = {}
        for k, v in m.items():
            new[k] = v
            if k == "merged":
                new["anchor_pin"] = (
                    ANCHOR_PIN + " -- every `file:line` anchor in this file OUTSIDE "
                    "`_meta` resolves at this commit, except a reference carrying its "
                    "own `@ <sha>`. `_meta`'s own anchors keep the `merged`/`seeded` "
                    "pins above: that block is a dated measurement record, not a "
                    "citation. Gate: `anchor_gate` (ctest).")
        m2 = dict(new)
        m2["amended"] = list(m["amended"])
        m2["amended"][5] = m2["amended"][5] + SUPERSEDE
        m2["amended"].append(AMENDED_NEW)
        g["_meta"] = m2
        return json.dumps(g, indent=2, ensure_ascii=False) + "\n"
    # The fragments are hand-formatted and their indentation is not uniform
    # (unit-2 indents `_meta` two spaces where the others indent four), so the
    # insertion reuses the line's OWN indent rather than assuming one.
    m = re.search(r'^([ \t]*)"code": "master-beta @ b12c8cdd",[ \t]*\n', out, re.M)
    assert m, path
    add = (m.group(1) + '"anchor_pin": "' + ANCHOR_PIN + ' -- every `file:line` anchor '
           'in this fragment OUTSIDE `_meta` resolves at this commit; `code` above '
           "stays the pin of this `_meta` block's own measurements (handler_range, "
           'args_bracket, accounting), which are NOT swept.",\n')
    return out[:m.end()] + add + out[m.end():]


def cmd_sweep():
    dry = "--dry" in sys.argv
    rewrites, notes = build_rewrites()
    if not dry:
        json.dump(rewrites, open(os.path.join(OUT, "rewrites.json"), "w"))
    print("distinct strings rewritten: %d" % len(rewrites))
    total = 0
    for path in TARGETS:
        full = os.path.join(REPO, path)
        raw = open(full, encoding="utf-8").read()
        out, n = raw, 0
        for old, new in rewrites.items():
            a, b = json.dumps(old, ensure_ascii=False), json.dumps(new, ensure_ascii=False)
            c = out.count(a)
            if c:
                out = out.replace(a, b); n += c
        out = add_file_pin(path, out)
        json.loads(out)                      # it must still be JSON
        print("  %-18s %4d string occurrences replaced" % (path.split("/")[-1], n))
        total += n
        if not dry:
            open(full, "w", encoding="utf-8").write(out)
    print("total occurrences replaced: %d" % total)
    print("\nREAD-TABLE / NOT-AT-HEAD actions applied:")
    seen = set()
    for p, jp, kind, f, old, new, why in notes:
        k = (kind, f, old, new)
        if k in seen:
            continue
        seen.add(k)
        print("  %-11s %-42s %-9s -> %-9s %s" % (kind, (f or "?")[:42], old, new or "(kept)", why[:60]))




# --------------------------------------------------------------------------
# 12. THE BYTE PROOF
#
# The fragments are hand-formatted and do NOT round-trip through json.dumps,
# so their write path is text replacement -- which is only safe if it can be
# shown to have touched nothing else. It is shown by INVERSION: undo every
# rewrite and remove the one inserted line, and the file must be byte-identical
# to the committed one. An empty diff there is the whole claim.
# --------------------------------------------------------------------------
def cmd_verify():
    # The rewrite map is READ BACK from what the sweep recorded, never
    # recomputed from the swept files: recomputing would ask the tool whether
    # it agrees with itself.
    import gzip
    f = os.path.join(OUT, "rewrites.json")
    rewrites = json.load(gzip.open(f + ".gz", "rt") if os.path.exists(f + ".gz")
                         else open(f))
    # The base is the code HEAD the sweep ran against, named rather than
    # inferred: once the sweep is committed, `HEAD` is the swept state and a
    # proof against it would be vacuous.
    base = sys.argv[sys.argv.index("--base") + 1] if "--base" in sys.argv else ANCHOR_PIN_SHA
    inverse = {}
    for old, new in rewrites.items():
        if new in inverse and inverse[new] != old:
            raise SystemExit("inverse is not a function: %r" % new)
        inverse[new] = old
    ok = True
    for path in TARGETS:
        cur = open(os.path.join(REPO, path), encoding="utf-8").read()
        was = blob(base, path)
        undone = cur
        for new, old in inverse.items():
            undone = undone.replace(json.dumps(new, ensure_ascii=False),
                                    json.dumps(old, ensure_ascii=False))
        # remove the inserted pin line(s) / the _meta edits
        undone = re.sub(r'^[ \t]*"anchor_pin": .*\n', "", undone, flags=re.M)
        if path == MERGED:
            g = json.loads(undone)
            g["_meta"]["amended"] = [a.replace(SUPERSEDE, "")
                                     for a in g["_meta"]["amended"]
                                     if a != AMENDED_NEW]
            undone = json.dumps(g, indent=2, ensure_ascii=False) + "\n"
        same = undone == was
        ok &= same
        n = 0 if same else sum(1 for a, b in zip(undone.split("\n"), was.split("\n")) if a != b)
        print("  %-18s non-anchor bytes diff: %s%s" %
              (path.split("/")[-1], "EMPTY" if same else "NOT EMPTY", "" if same else " (%d lines)" % n))
        if not same:
            for a, b in zip(undone.split("\n"), was.split("\n")):
                if a != b:
                    print("     was : %s" % b[:150]); print("     undo: %s" % a[:150]); break
    print("BYTE PROOF: %s" % ("PASS" if ok else "FAIL"))
    return ok


# --------------------------------------------------------------------------
# 13. MOVING THE PIN (F158, 2026-10-14; parent INTENT 11.331)
#
# After F75 a string's pin is no longer its HISTORY: every reference outside
# `_meta` is a citation AT its file's `_meta.anchor_pin` (that is where the gate
# reads it), and `cmd_pins` above dates sc-grammar.json's strings to eleven
# grammar commits (measured 2026-10-14, artifacts/f158/pins-before.txt) -- when
# a STRING last changed, not which tree its numbers are numbers of. So in a
# ROUND the source pin is READ the way the gate reads it (`file_pin`, and a
# reference's own `@ <sha>`, `Tok.pin`), and the target is a NAMED commit,
# never HEAD or the working tree: another task may be editing the engine while
# the sweep runs (F154 did, during F158).
#
# A round moves THREE pointer classes to one pin: the `file:line` references
# (content-mapped, as above, the hand-read rest in the round's read table) and
# the two colon-less classes, `lines` / `handler_body_lines` and the
# vocabulary's `header_line`, which are DERIVED at the target by the gate's own
# resolvers (`handler_range`, `define_lines`) -- the sweep writes what the gate
# will check, from the gate's function, so the two cannot disagree (I2).
#
# One row per round; the next pin move adds a row and changes nothing else.
# --------------------------------------------------------------------------
import anchor_gate as AG                                   # noqa: E402

ROUNDS = {
    "f158": dict(
        to="9d461bd5", label="main @ 9d461bd5",
        out=os.path.join(OUT, "f158"),
        read_table=os.path.join(OUT, "read_table_f158.json"),
        amended=(
            "2026-10-14 (F158): the anchor pin MOVES to main @ 9d461bd5 (from "
            "master-beta @ b45d3b58; ss-grammar.json from master-beta @ 8d41fbe3), "
            "the same commit for every pointer class in the six contract files: every "
            "`file:line` reference outside `_meta` re-resolved by content, the ones "
            "whose text left the engine read by hand (re-anchored where the construct "
            "moved, `[NOT AT HEAD]` with its own `@ <sha>` where it left; table "
            "claude/harness/f75/read_table_f158.json); and the two classes no gate "
            "saw, the commands' `lines` (with `handler_body_lines`) and the "
            "vocabulary's `header_line`, derived at the pin from `handler` and "
            "`macro` and gated by `anchor_gate`, which also reads a reference's own "
            "`@ <sha>` now. `_meta` keeps its own pins (`code`, `merged`, `seeded`). "
            "Parent INTENT 11.331, scedit journal 2026-10-14b.")),
}

def opt(name, default=None):
    return sys.argv[sys.argv.index(name) + 1] if name in sys.argv else default

def jload(path):
    """Read a round record as written, or as committed (gz)."""
    import gzip
    return json.load(open(path) if os.path.exists(path) else gzip.open(path + ".gz", "rt"))

def the_round():
    return ROUNDS[opt("--round")]

def load_read_table(R):
    if not os.path.exists(R["read_table"]):
        return {}
    rows = json.load(open(R["read_table"]))
    rt = {}
    for row in rows:
        k = (row["repo"], row["elem"], row["pin"])
        if k in rt:
            raise SystemExit("read table: two rows for one referent %r" % (k,))
        rt[k] = row
    return rt

def cmds_of(path, g):
    return g["families"]["commands"] if path == MERGED else g.get("commands", {})

def _combine(va, vb):
    # the gate's rule for a range (anchor_gate.run), verbatim in effect
    OK = ("clean", "moved", "d14", "d14-moved")
    if vb is None:
        return va
    return ("clean" if va == vb == "clean" else
            ("moved" if va in OK and vb in OK else (va if va not in OK else vb)))

def _gate_verdict(raw, marked):
    if raw == "clean":
        return "clean"
    if raw in ("moved", "d14", "d14-moved"):
        return "moved"
    return "declared" if marked else {"gone": "GONE", "unresolved": "UNRESOLVED",
                                      "at-pin-broken": "AT-PIN-BROKEN"}[raw]

def round_rows(R, paths=None):
    """Every `file:line` reference of the contract files, resolved exactly as
    anchor_gate.run() resolves it, its HEAD side read at the round's target."""
    to = R["to"]
    res = {}
    for path in (paths or TARGETS):
        pin0 = AG.file_pin(path)
        g = load(path)
        cmds = cmds_of(path, g)
        rows = []
        for jp, s in walk(g):
            if jp.startswith("._meta"):
                continue
            toks = tokenize(s)
            if not toks:
                continue
            marked = bool(AG.MARKER.search(s))
            m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == MERGED
                         else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
            handler = cmds.get(m.group(1), {}).get("handler") if m else None
            for ti, t in enumerate(toks):
                pin = t.pin or pin0
                if (jp, ti) in FILE_OVERRIDES:
                    rp, how = FILE_OVERRIDES[(jp, ti)], "repo-relative"
                elif t.kind == "FILE":
                    rp, how = resolve_file(t.file)
                else:
                    rp, how, _b, _e = disambiguate(t, pin, s, handler)
                ems = list(ELEM.finditer(t.listtext))
                for ei, (a, b, _, _) in enumerate(t.elems):
                    row = dict(jp=jp, ti=ti, ei=ei, pin=pin, own=bool(t.pin), kind=t.kind,
                               file=t.file, repo=rp, how=how, marked=marked,
                               elem=ems[ei].group(0), old=[a, b],
                               override=(jp, ti) in FILE_OVERRIDES,
                               written=(resolve_file(t.file)[0]
                                        if t.kind != "FILE" and t.file else None))
                    if rp is None:
                        row["raw"] = "unresolved"
                    else:
                        vs = [classify_any(pin, rp, n, how, head=to) if n is not None else None
                              for n in (a, b)]
                        if any(v and v[0] in ("no-file", "out-of-range") for v in vs):
                            row["raw"] = "at-pin-broken"
                        else:
                            row["raw"] = _combine(vs[0][0], vs[1][0] if vs[1] else None)
                            row["new"] = [vs[0][1], vs[1][1] if vs[1] else None]
                            row["text"] = [vs[0][2], vs[1][2] if vs[1] else None]
                    row["verdict"] = _gate_verdict(row["raw"], marked)
                    rows.append(row)
        res[path] = rows
    return res

def round_fields(R, rev):
    """The two colon-less classes: stored value vs the gate's derivation at `rev`."""
    out = []
    for path in [MERGED] + FRAGS:
        g = load(path)
        for name, e in cmds_of(path, g).items():
            if not isinstance(e, dict):
                continue
            for f in ("lines", "handler_body_lines"):
                if f in e:
                    out.append(dict(path=path, field=f, key=name, old=e[f],
                                    new=AG.handler_range(rev, e["handler"])))
    for tok, spec in load(MERGED)["argument_token_vocabulary"]["tokens"].items():
        if isinstance(spec, dict) and "header_line" in spec:
            out.append(dict(path=MERGED, field="header_line", key=tok, macro=spec["macro"],
                            old=spec["header_line"],
                            new=AG.define_lines(rev, spec["macro"], tok)))
    return out

def round_pins():
    R = the_round()
    print("ROUND %s: the source pin of every reference is its file's `_meta.anchor_pin`,"
          " or the `@ <sha>` the reference carries itself" % opt("--round"))
    allp = {}
    for path in TARGETS:
        pin0 = AG.file_pin(path)
        own = []
        n = 0
        for jp, s in walk(load(path)):
            if jp.startswith("._meta"):
                continue
            toks = tokenize(s)
            n += bool(toks)
            own += [(jp, ti, t.pin) for ti, t in enumerate(toks) if t.pin]
        allp[path] = dict(file_pin=pin0, strings=n, own=own)
        print("  %-16s %5d strings at %s; own-pinned tokens: %d %s" % (
            path.split("/")[-1], n, pin0, len(own),
            dict(collections.Counter(p for _, _, p in own))))
    os.makedirs(R["out"], exist_ok=True)
    json.dump(allp, open(os.path.join(R["out"], "pins.json"), "w"), indent=1)

def round_map():
    R = the_round()
    res = round_rows(R)
    os.makedirs(R["out"], exist_ok=True)
    json.dump(res, open(os.path.join(R["out"], "anchor_map.json"), "w"))
    grand = collections.Counter()
    print("ROUND %s: every reference from its pin to %s (git show, never the tree)" % (opt("--round"), R["to"]))
    for path, rows in res.items():
        c = collections.Counter(r["verdict"] for r in rows)
        grand.update(c)
        print("  %-16s %5d  %s" % (path.split("/")[-1], len(rows), dict(sorted(c.items()))))
    print("  %-16s %5d  %s" % ("TOTAL", sum(grand.values()), dict(sorted(grand.items()))))
    hidden = [(p, r) for p, rows in res.items() for r in rows
              if r["marked"] and not r["own"] and r["raw"] not in ("clean", "moved", "d14", "d14-moved")]
    print("  in a MARKED string without its own pin, not resolving (the gate counts them declared): %d" % len(hidden))
    for p, r in hidden:
        print("     %s %s %s:%s raw=%s" % (p.split("/")[-1], r["jp"], r["repo"], r["elem"], r["raw"]))
    # A content rule that overrides the file the sentence's own context names is
    # a reading to check by eye (F158 found two wrong since F75 and F80).
    ovr = [(p, r) for p, rows in res.items() for r in rows
           if r["written"] and r["written"] != r["repo"] and not r["override"]]
    print("  inherited-file tokens whose written context names another file than the one chosen: %d" % len(ovr))
    for p, r in ovr:
        print("     %s %s ti=%d :%s written=%s chosen=%s" % (p.split("/")[-1], r["jp"], r["ti"],
              r["elem"], r["written"], r["repo"]))
    # the GONE table, both texts, one row per reference
    gone = []
    for p, rows in res.items():
        for r in rows:
            if r["own"] or r["raw"] in ("clean", "moved", "d14", "d14-moved"):
                continue
            if r["raw"] == "unresolved" and r["marked"]:
                continue
            ls_pin = lines_at(r["pin"], r["repo"]) if r["repo"] else None
            ls_to = lines_at(R["to"], r["repo"]) if r["repo"] else None
            a, b = r["old"]
            gone.append(dict(contract=p, jp=r["jp"], ti=r["ti"], ei=r["ei"], repo=r["repo"],
                             elem=r["elem"], pin=r["pin"], raw=r["raw"], marked=r["marked"],
                             at_pin=ls_pin[a - 1:(b or a)] if ls_pin else None,
                             at_to_same_numbers=ls_to[a - 1:(b or a)] if ls_to and a <= len(ls_to) else None,
                             enclosing_at_pin=enclosing_symbol(r["pin"], r["repo"], a) if r["repo"] else None))
    out = opt("--gone", os.path.join(R["out"], "gone.json"))
    json.dump(gone, open(out, "w"), indent=1)
    refs = collections.Counter((g["repo"], g["elem"], g["pin"]) for g in gone)
    print("  GONE references: %d over %d distinct referents -> %s" % (len(gone), len(refs), out))
    byfile = collections.Counter(g["repo"] for g in gone)
    for f, n in byfile.most_common():
        print("     %4d  %s" % (n, f))
    return res

def round_check():
    """Every unresolving reference has its read-table row, and every row is used."""
    R = the_round()
    rt = load_read_table(R)
    res = jload(os.path.join(R["out"], "anchor_map.json"))
    need, used, corr = collections.Counter(), set(), collections.Counter()
    for p, rows in res.items():
        for r in rows:
            k = (r["repo"], r["elem"], r["pin"])
            if r["own"] or r["raw"] in ("clean", "moved", "d14", "d14-moved"):
                # a resolving reference is read only to CORRECT it (class X)
                if not r["own"] and k in rt and rt[k].get("cls") == "X":
                    corr[k] += 1; used.add(k)
                continue
            if r["raw"] == "unresolved" and r["marked"]:
                continue
            need[k] += 1
            if k in rt:
                used.add(k)
    print("corrections of RESOLVING references (class X): %d rows, %d references"
          % (len(corr), sum(corr.values())))
    missing = [k for k in need if k not in rt]
    unused = [k for k in rt if k not in used]
    acts = collections.Counter(rt[k]["action"] for k in used)
    refs = collections.Counter()
    for k in used:
        refs[rt[k]["action"]] += need[k]
    print("read table: %d rows; referents needing a row %d (references %d); missing %d; unused %d"
          % (len(rt), len(need), sum(need.values()), len(missing), len(unused)))
    print("  by action (referents): %s" % dict(acts))
    print("  by action (references): %s" % dict(refs))
    for k in missing[:40]:
        print("  MISSING %s:%s @ %s (%d refs)" % (k[0], k[1], k[2], need[k]))
    for k in unused:
        print("  UNUSED  %s:%s @ %s" % k)
    # a re-anchored row must land on a line that exists at the target
    bad = 0
    for k in used:
        row = rt[k]
        if row["action"] in ("lines", "unmark"):
            ls = lines_at(R["to"], k[0])
            for em in ELEM.finditer(row["new"]):
                for n in (em.group(1), em.group(3)):
                    if n and not (1 <= int(n) <= len(ls)):
                        bad += 1
                        print("  OUT OF RANGE at %s: %s:%s" % (R["to"], k[0], n))
            if not row.get("reason"):
                bad += 1
                print("  NO REASON %s:%s" % k[:2])
        elif row["action"] == "notathead":
            if re.search(r':\d|@\s*[0-9a-f]{7}', row["marker"]):
                bad += 1
                print("  MARKER CARRIES AN ANCHOR %s:%s" % k[:2])
    print("read-table check: %s" % ("PASS" if not missing and not unused and not bad else "FAIL"))
    return not missing and not unused and not bad

UNMARK = re.compile(r'\s*\[NOT AT HEAD: [^\]]+\]')

def round_rewrite(s, rows_of, rt):
    """-> new string. A token carrying its own `@ <sha>` is not touched: its
    numbers are numbers OF that commit. A token with a declared element keeps
    ALL its numbers and gains the pin they are numbers of."""
    toks = tokenize(s)
    out, last = [], 0
    for ti, t in enumerate(toks):
        if t.pin:
            continue
        rows = rows_of(ti)
        acts = [rt.get((r["repo"], r["elem"], r["pin"])) for r in rows]
        lo, hi = t.lspan
        pieces, cur, marker, unmark = [], 0, None, False
        declared = [a for a in acts if a and a["action"] == "notathead"]
        for ei, em in enumerate(ELEM.finditer(t.listtext)):
            r, row, etext = rows[ei], acts[ei], em.group(0)
            pieces.append(t.listtext[cur:em.start()])
            if declared:
                pieces.append(etext)
            elif row and row["action"] == "lines":
                pieces.append(row["new"])
            elif row and row["action"] == "unmark":
                pieces.append(row["new"]); unmark = True
            elif row and row["action"] == "keep":
                pieces.append(etext)
            elif r["raw"] in ("clean", "moved", "d14", "d14-moved"):
                a, b = r["new"]
                pieces.append(str(a) + (em.group(2) + str(b) if em.group(3) else ""))
            elif r["raw"] == "unresolved" and r["marked"]:
                pieces.append(etext)
            else:
                raise SystemExit("UNREAD reference %s:%s @ %s (%s) -- read it first"
                                 % (r["repo"], etext, r["pin"], r["raw"]))
            cur = em.end()
        if declared:
            if len(set(d["marker"] for d in declared)) != 1:
                raise SystemExit("two markers on one token: %r" % t.listtext)
            marker = " @ %s [NOT AT HEAD: %s]" % (rows[0]["pin"][:8], declared[0]["marker"])
        pieces.append(t.listtext[cur:])
        out.append(s[last:lo]); out.append("".join(pieces)); last = hi
        if marker:
            out.append(s[last:t.end]); out.append(marker); last = t.end
        if unmark:
            mm = UNMARK.match(s, t.end)
            if not mm:
                raise SystemExit("unmark: no marker after %r" % t.listtext)
            out.append(s[last:t.end]); last = mm.end()
    out.append(s[last:])
    return "".join(out)

JSTR = re.compile(r'"(?:[^"\\]|\\.)*"')

def _replace_strings(text, rewrites):
    """Single pass over the JSON string literals: no rewrite can feed another."""
    n = [0]
    def sub(m):
        lit = m.group(0)
        if text[m.end():m.end() + 1] == ":" or text[m.end():m.end() + 2] == " :":
            return lit                                   # a key, never an anchor string
        v = json.loads(lit)
        if v not in rewrites:
            return lit
        if json.dumps(v, ensure_ascii=False) != lit:
            raise SystemExit("a literal not in json.dumps form: %s" % lit[:80])
        n[0] += 1
        return json.dumps(rewrites[v], ensure_ascii=False)
    return JSTR.sub(sub, text), n[0]

FIELDRX = {
    "lines": re.compile(r'("lines": ")(\d+-\d+)(")'),
    "handler_body_lines": re.compile(r'("handler_body_lines": ")(\d+-\d+)(")'),
    "header_line": re.compile(r'("((?:[^"\\]|\\.)*)": \{\s*"macro": "(W_\w+)",\s*"header_line": )(\d+|\[[\d,\s]*\])'),
}

def _fmt_lines(old_text, new):
    """Write `new` (an int or a list of ints) in the layout `old_text` has."""
    nums = re.findall(r'\d+', old_text)
    new = new if isinstance(new, list) else [new]
    if len(nums) != len(new):
        raise SystemExit("header_line arity changed: %s -> %r" % (old_text, new))
    it = iter(new)
    return re.sub(r'\d+', lambda m: str(next(it)), old_text)

def _replace_fields(path, text, fields, inverse=False):
    """Single pass per field class. `lines` / `handler_body_lines` are keyed by
    their old value (a function of the entry: checked); `header_line` by the
    token whose object it sits in."""
    n = collections.Counter()
    mine = [f for f in fields if f["path"] == path]
    for fld in ("lines", "handler_body_lines"):
        mp = {}
        for f in mine:
            if f["field"] != fld:
                continue
            a, b = (f["new"], f["old"]) if inverse else (f["old"], f["new"])
            if mp.setdefault(a, b) != b:
                raise SystemExit("%s: %s %s maps to two values" % (path, fld, a))
        def sub(m, mp=mp, fld=fld):
            if m.group(2) not in mp:
                raise SystemExit("%s: %s %s has no derivation row" % (path, fld, m.group(2)))
            n[fld] += 1
            return m.group(1) + mp[m.group(2)] + m.group(3)
        text = FIELDRX[fld].sub(sub, text)
    hl = {f["key"]: f for f in mine if f["field"] == "header_line"}
    if hl:
        def subh(m):
            tok = json.loads('"%s"' % m.group(2))
            if tok not in hl:
                return m.group(0)
            f = hl[tok]
            n["header_line"] += 1
            return m.group(1) + _fmt_lines(m.group(4), f["old"] if inverse else f["new"])
        text = FIELDRX["header_line"].sub(subh, text)
    return text, n

def _label(text):
    m = re.search(r'"anchor_pin": "([^"]*?) -- ', text)
    if not m or len(re.findall(r'"anchor_pin": "', text)) != 1:
        raise SystemExit("anchor_pin: not exactly one line")
    return m

def round_sweep():
    R = the_round()
    dry = "--dry" in sys.argv
    only = opt("--only")
    paths = [p for p in TARGETS if not only or p.split("/")[-1] in only.split(",")]
    res = jload(os.path.join(R["out"], "anchor_map.json"))
    rt = load_read_table(R)
    recf = os.path.join(R["out"], "rewrites.json")
    rec = (jload(recf) if os.path.exists(recf) or os.path.exists(recf + ".gz")
           else {"strings": {}, "fields": [], "labels": {}, "amended": None})
    fields = [f for f in round_fields(R, R["to"]) if f["path"] in paths]
    for f in fields:
        if f["new"] is None:
            raise SystemExit("no derivation at %s for %s %s" % (R["to"], f["field"], f["key"]))
    for path in paths:
        by = collections.defaultdict(list)
        for r in res[path]:
            by[(r["jp"], r["ti"])].append(r)
        for k in by:
            by[k].sort(key=lambda r: r["ei"])
        strmap = {}
        for jp, s in walk(load(path)):
            if jp.startswith("._meta") or not tokenize(s):
                continue
            new = round_rewrite(s, lambda ti, jp=jp: by[(jp, ti)], rt)
            if new != s:
                if strmap.setdefault(s, new) != new or rec["strings"].setdefault(s, new) != new:
                    raise SystemExit("CONTENT COLLISION: one string, two rewrites\n%r" % s)
        full = os.path.join(REPO, path)
        raw = open(full, encoding="utf-8").read()
        out, ns = _replace_strings(raw, strmap)
        out, nf = _replace_fields(path, out, fields)
        m = _label(out)
        rec["labels"][path] = [m.group(1), R["label"]]
        out = out[:m.start(1)] + R["label"] + out[m.end(1):]
        if path == MERGED:
            if json.dumps(json.loads(out), indent=2, ensure_ascii=False) + "\n" != out:
                raise SystemExit("the merged file no longer round-trips; refusing the JSON-level append")
            g = json.loads(out)
            g["_meta"]["amended"].append(R["amended"])
            rec["amended"] = R["amended"]
            out = json.dumps(g, indent=2, ensure_ascii=False) + "\n"
        json.loads(out)
        print("  %-16s strings %4d (distinct %4d)  fields %s  pin %s -> %s" % (
            path.split("/")[-1], ns, len(strmap), dict(nf), rec["labels"][path][0], R["label"]))
        if not dry:
            open(full, "w", encoding="utf-8").write(out)
    rec["fields"] = [f for f in rec["fields"] if f["path"] not in paths] + fields
    if not dry:
        json.dump(rec, open(recf, "w"), indent=1)
    print("%s; record %s" % ("DRY" if dry else "WRITTEN", recf))

def round_verify():
    """The byte proof by INVERSION, from the record the sweep wrote: undo the
    pin label, the amended line, the fields and the strings, and every file must
    be byte-identical to its committed self at the round's target."""
    R = the_round()
    rec = jload(os.path.join(R["out"], "rewrites.json"))
    inverse = {}
    for old, new in rec["strings"].items():
        if inverse.setdefault(new, old) != old:
            raise SystemExit("inverse is not a function: %r" % new)
    ok = True
    for path in TARGETS:
        cur = open(os.path.join(REPO, path), encoding="utf-8").read()
        was = blob(R["to"], path)
        undone = cur
        if path == MERGED and rec.get("amended"):
            g = json.loads(undone)
            am = g["_meta"]["amended"]
            if am[-1] != rec["amended"]:
                raise SystemExit("the amended line is not the last one")
            g["_meta"]["amended"] = am[:-1]
            undone = json.dumps(g, indent=2, ensure_ascii=False) + "\n"
        if path in rec["labels"]:
            m = _label(undone)
            if m.group(1) != rec["labels"][path][1]:
                raise SystemExit("%s: label %r is not the swept one" % (path, m.group(1)))
            undone = undone[:m.start(1)] + rec["labels"][path][0] + undone[m.end(1):]
        undone, nf = _replace_fields(path, undone, rec["fields"], inverse=True)
        undone, ns = _replace_strings(undone, inverse)
        same = undone == was
        ok &= same
        print("  %-16s undone: strings %4d fields %-40s -> bytes vs %s: %s" % (
            path.split("/")[-1], ns, dict(nf), R["to"], "IDENTICAL" if same else "DIFFER"))
        if not same:
            for a, b in zip(undone.split("\n"), was.split("\n")):
                if a != b:
                    print("     was : %s" % b[:150]); print("     undo: %s" % a[:150]); break
    print("BYTE PROOF: %s" % ("PASS" if ok else "FAIL"))
    return ok

def round_flips():
    """After a sweep: every inherited-file token re-read at the NEW pin with its
    NEW number must name the same file as the map read at the old pin. Moving
    the pin re-runs the content rules (anchor_gate.disambiguate) on other text,
    so a reading can change with no number of its own moving (F158: four did)."""
    R = the_round()
    res = jload(os.path.join(R["out"], "anchor_map.json"))
    n, flips = 0, []
    for path in TARGETS:
        g = load(path)
        cmds = cmds_of(path, g)
        by = collections.defaultdict(list)
        for r in res[path]:
            by[(r["jp"], r["ti"])].append(r)
        pin0 = AG.file_pin(path)
        for jp, s in walk(g):
            if jp.startswith("._meta"):
                continue
            m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == MERGED
                         else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
            handler = cmds.get(m.group(1), {}).get("handler") if m else None
            for ti, t in enumerate(tokenize(s)):
                if t.kind == "FILE" or (jp, ti) in FILE_OVERRIDES:
                    continue
                n += 1
                rp = disambiguate(t, t.pin or pin0, s, handler)[0]
                old = by.get((jp, ti))
                if not old or rp != old[0]["repo"]:
                    flips.append((path.split("/")[-1], jp, ti, t.listtext,
                                  old[0]["repo"] if old else None, rp))
    print("inherited-file tokens re-read at the new pin: %d; reading changed: %d" % (n, len(flips)))
    for f in flips:
        print("  %s %s ti=%d :%s  was %s  now %s" % f)
    print("flips check: %s" % ("PASS" if not flips else "FAIL"))
    return not flips

def round_fields_report():
    """The two colon-less classes BEFORE a sweep: stored vs derived at the old
    pin, the merge sha and the target."""
    R = the_round()
    for rev in [opt("--at")] if opt("--at") else [AG.file_pin(MERGED), "b12c8cdd", R["to"]]:
        rows = round_fields(R, rev)
        c = collections.Counter((f["field"], "equal" if f["old"] == f["new"] else
                                 ("underivable" if f["new"] is None else "differ")) for f in rows)
        print("at %s: %s" % (rev, dict(sorted(c.items()))))


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "shapes"
    if "--round" in sys.argv:
        {"pins": round_pins, "map": round_map, "read-table-check": round_check,
         "sweep": round_sweep, "verify": round_verify, "fields": round_fields_report,
         "flips": round_flips}[cmd]()
    else:
        {"shapes": cmd_shapes, "pins": cmd_pins, "map": cmd_map,
         "default-check": cmd_default_check, "sweep": cmd_sweep,
         "read-table-check": cmd_read_table_check, "verify": cmd_verify}[cmd]()
