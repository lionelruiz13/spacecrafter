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
HEAD_PIN = None            # resolved at run time: the code HEAD we anchor against

MERGED = "util/scedit/grammar/sc-grammar.json"
FRAGS  = ["util/scedit/grammar/args/unit-%d.json" % i for i in (1, 2, 3, 4)]
TARGETS = [MERGED] + FRAGS

# --------------------------------------------------------------------------
# git helpers (all reads; nothing here writes to a repository)
# --------------------------------------------------------------------------
_blob_cache = {}
def git(*a, repo=REPO, ok=(0,)):
    r = subprocess.run(["git", "-C", repo] + list(a), capture_output=True)
    if r.returncode not in ok:
        return None
    return r.stdout

def blob(rev, path, repo=REPO):
    key = (repo, rev, path)
    if key not in _blob_cache:
        b = git("show", "%s:%s" % (rev, path), repo=repo, ok=(0, 128))
        _blob_cache[key] = None if b is None else b.decode("utf-8", "replace")
    return _blob_cache[key]

def lines_at(rev, path, repo=REPO):
    b = blob(rev, path, repo=repo)
    return None if b is None else b.split("\n")

# --------------------------------------------------------------------------
# 1. THE ANCHOR GRAMMAR
#
# Every occurrence of ':' followed by a digit is a CANDIDATE.  Three classes,
# decided by what sits immediately before the colon:
#
#   FILE    glued  `app_command_interface.cpp:3958`      -- names its own file
#   SYMBOL  glued  `parseCommand:141`, `executeCommand :191-302`
#                  -- a function name, not a file: the line is in the string's
#                     CURRENT file (the last FILE anchor seen, else the string's
#                     implicit default)
#   LOOSE   not glued to a name  `... + :335`, `(:2479-2530)`, `/:3511`
#                  -- a continuation inheriting the current file
#
# and one NON-anchor class that must never be touched:
#
#   TIME    a DIGIT immediately before the colon  `00:00:00`, `23:59`
#
# A line list is `N`, `N-M`, or either of those repeated after commas
# (`3923, 3927, 3931, 3935`).  Every element is an independent reference.
# --------------------------------------------------------------------------
LINELIST = r'\d+(?:\s*-\s*\d+)?(?:\s*,\s*\d+(?:\s*-\s*\d+)?)*'
CAND     = re.compile(r'(?<![\d])' + r':(' + LINELIST + r')')
HEADRX   = re.compile(r'([A-Za-z0-9_./+-]*[A-Za-z0-9_+-])(\s*)$')
FILERX   = re.compile(r'[A-Za-z0-9_./+-]*[A-Za-z0-9_+-]\.[A-Za-z][A-Za-z0-9]*$')
ELEM     = re.compile(r'(\d+)(\s*-\s*)?(\d+)?')

class Tok:
    __slots__ = ("start", "end", "kind", "head", "listtext", "elems", "file", "flat")
    def __init__(self, start, end, kind, head, listtext, elems):
        self.start, self.end, self.kind = start, end, kind
        self.head, self.listtext, self.elems = head, listtext, elems
        self.file = self.flat = None
    def __repr__(self):
        return "<%s %s %s -> %s>" % (self.kind, self.head, self.listtext, self.file)

def tokenize(s):
    """Return the anchor tokens of one string, in order, with `file` filled in
    from the string's own left context.  `file` is None for a token whose file
    could not be determined -- reported, never guessed.

    INHERITANCE IS PARENTHESIS-SCOPED, and it has to be.  A parenthesis is how
    this file writes a SUB-anchor: `app_command_interface.cpp:1887 (W_INDEX=
    "index", base_command_interface.hpp:131) -> applyColor CC_STAR_TABLE :1708`
    -- the `:1708` belongs to app_command_interface.cpp, not to the header the
    parenthesis happened to mention last.  Reading it flat put 89 references
    past the end of a file that was never their file; the out-of-range count is
    what made the bug visible, which is why a shape the parser cannot read is a
    reported residual rather than a skipped one."""
    toks, cur, flat, stack = [], None, None, []
    for m in CAND.finditer(s):
        i = m.start()
        for ch in s[(toks[-1].end if toks else 0):i]:
            if ch == "(":
                stack.append(cur)
            elif ch == ")" and stack:
                cur = stack.pop()
        hm = HEADRX.search(s[:i])
        if hm and not hm.group(2):
            head = hm.group(1)
            if head[-1:].isdigit():
                continue                      # TIME literal: 00:00, 23:59
            kind = "FILE" if FILERX.search(head) else "SYMBOL"
            hstart = i - len(head)
        elif hm and hm.group(2) and FILERX.search(hm.group(1)):
            head, kind, hstart = hm.group(1), "FILE", i - len(hm.group(2)) - len(hm.group(1))
        else:
            head, kind, hstart = None, "LOOSE", i
        elems = []
        for em in ELEM.finditer(m.group(1)):
            a = int(em.group(1)); b = int(em.group(3)) if em.group(3) else None
            elems.append((a, b, em.start(), em.end()))
        t = Tok(hstart, m.end(), kind, head, m.group(1), elems)
        if kind == "FILE":
            cur = flat = head
        t.file = head if kind == "FILE" else cur
        t.flat = head if kind == "FILE" else flat
        toks.append(t)
    return toks

def walk(o, p=""):
    if isinstance(o, dict):
        for k, v in o.items():
            yield from walk(v, p + "." + k)
    elif isinstance(o, list):
        for i, v in enumerate(o):
            yield from walk(v, p + "[%d]" % i)
    elif isinstance(o, str):
        yield p, o

def load(path, rev=None):
    if rev is None:
        return json.load(open(os.path.join(REPO, path), encoding="utf-8"))
    b = blob(rev, path)
    return None if b is None else json.loads(b)

# --------------------------------------------------------------------------
# 2. FILE RESOLUTION  (a bare basename -> its repo path)
# --------------------------------------------------------------------------
SUBMODULE = "src/EntityCore"
# The anchor's file component is part of the anchor: a mis-spelled path whose
# referent is unambiguous at the pin is RESOLVED here, and every such row is
# stated in the delivery.  (F75 boundary: this is the sweep's job; changing an
# anchor's CONTENT is not.)
RESPELL = {"tools/utility.hpp": "src/tools/utility.hpp"}

_index = None
def repo_index():
    global _index
    if _index is None:
        files = git("ls-files").decode().split()
        by_base = collections.defaultdict(list)
        for f in files:
            by_base[os.path.basename(f)].append(f)
        _index = (set(files), by_base)
    return _index

def resolve_file(name):
    """-> (repo_path, how) or (None, reason)."""
    files, by_base = repo_index()
    if name in RESPELL:
        return RESPELL[name], "respelled"
    if name in files:
        return name, "repo-relative"
    if "/" not in name:
        c = by_base.get(name, [])
        if len(c) == 1:
            return c[0], "bare-basename"
        if len(c) > 1:
            return None, "ambiguous-basename(%d)" % len(c)
        return None, "unknown-basename"
    # a path that is not tracked: maybe it is inside the submodule
    if name.startswith(SUBMODULE + "/"):
        return name, "submodule"
    return None, "untracked-path"

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
    """-> {json_path: (rev, method)} for every anchor-bearing string at HEAD."""
    revs = revisions(path)
    today = {p: s for p, s in walk(load(path)) if tokenize(s)}
    want_key  = {p: anchor_key(s) for p, s in today.items()}
    want_text = today
    pin_path, pin_text = {}, {}
    for r in revs:
        g = load(path, r)
        if g is None:
            continue
        seen = dict(walk(g))
        for p, key in want_key.items():
            if p in pin_path:
                continue
            if p in seen and anchor_key(seen[p]) == key:
                pin_path[p] = r
        texts = set(seen.values())
        for p, s in want_text.items():
            if p not in pin_text and s in texts:
                pin_text[p] = r
    out = {}
    for p in today:
        if p in pin_path:
            out[p] = (pin_path[p], "path")
        elif p in pin_text:
            out[p] = (pin_text[p], "content")
        else:
            out[p] = (revs[-1], "unrecovered")
    return out, today

# --------------------------------------------------------------------------
# 4. CONTENT MAPPING pin -> HEAD
# --------------------------------------------------------------------------
_map_cache = {}
def linemap(rev, path, head="HEAD", repo=REPO):
    """1-based line map rev->head for lines that survive verbatim; None where
    the line's text does not exist at head in that position."""
    key = (repo, rev, path, head)
    if key in _map_cache:
        return _map_cache[key]
    a = lines_at(rev, path, repo=repo)
    b = lines_at(head, path, repo=repo)
    if a is None or b is None:
        _map_cache[key] = (None, a, b)
        return _map_cache[key]
    m = {}
    sm = difflib.SequenceMatcher(a=a, b=b, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                m[i1 + k + 1] = j1 + k + 1
    _map_cache[key] = (m, a, b)
    return _map_cache[key]

def classify(rev, repo_path, n, repo=REPO, head="HEAD"):
    """-> (verdict, new_line, text_at_pin, text_at_head)."""
    m, a, b = linemap(rev, repo_path, head=head, repo=repo)
    if m is None:
        return ("no-file", None, None, None)
    if n < 1 or n > len(a):
        return ("out-of-range", None, None, None)
    src = a[n - 1]
    if n in m:
        j = m[n]
        return (("clean" if j == n else "moved"), j, src, b[j - 1])
    # SECOND PASS -- the D14 rescue.  F70 transliterated the whole tree
    # (`S5.2` for the section sign, `+-90deg` for the degree sign), so a line
    # that carried an accent has different BYTES at HEAD while being the same
    # line.  That is a dated, whole-tree, characterised transformation, not a
    # rewrite, and refusing to see it would report a referent as lost when it
    # is sitting in place.  The rescue is bounded on both sides by the nearest
    # lines that DID map, so it can only find the referent where the referent
    # can be, and it fires only when the pin line has non-ASCII and the
    # candidate is pure ASCII.  Every rescue is recorded with both texts.
    if any(ord(c) > 127 for c in src):
        lo = max((k for k in m if k < n), default=None)
        hi = min((k for k in m if k > n), default=None)
        j0 = (m[lo] + 1) if lo is not None else 1
        j1 = (m[hi] - 1) if hi is not None else len(b)
        best, bestr = None, 0.0
        for j in range(max(1, j0), min(len(b), j1) + 1):
            cand = b[j - 1]
            if any(ord(c) > 127 for c in cand):
                continue
            r = difflib.SequenceMatcher(a=src, b=cand, autojunk=False).ratio()
            if r > bestr:
                best, bestr = j, r
        if best is not None and bestr >= 0.80:
            return ("d14" if best == n else "d14-moved", best, src, b[best - 1])
    return ("gone", None, src, (b[n - 1] if 1 <= n <= len(b) else None))

# --------------------------------------------------------------------------
# 5. THE STRING'S IMPLICIT FILE
#
# 306 distinct strings carry LOOSE/SYMBOL anchors with NO file anchor anywhere
# in them (`... tested at :225 and cleared at :229`).  The fragments' own
# `_meta.source_anchor_convention` rules on bare BASENAMES, not on bare `:N`,
# so the referent of these is not STATED anywhere.  It is however MEASURABLE:
# the enclosing function at `<default>:<line>` at the pin is named in the
# string itself far more often than chance, and that is a check that can fail.
# --------------------------------------------------------------------------
DEFAULT_FILE = "src/interfaceModule/app_command_interface.cpp"
FUNC = re.compile(r'^[A-Za-z_][\w:<>,&*\s]*?([A-Za-z_]\w*)\s*\([^;]*\)\s*(?:const\s*)?\{?\s*$')

def enclosing_symbol(rev, repo_path, n):
    ls = lines_at(rev, repo_path)
    if ls is None or n < 1 or n > len(ls):
        return None
    for i in range(n - 1, -1, -1):
        line = ls[i]
        if not line or line[0] in " \t/#*}":
            continue
        m = FUNC.match(line.rstrip())
        if m:
            return m.group(1)
    return None

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
# 9. DISAMBIGUATION BY CONTENT
#
# A bare `:N` inherits a file, and WHICH file is not decidable from the text.
# Two readings, and each has a counter-example against the other:
#
#   PAREN-SCOPED  `app_command_interface.cpp:1887 (W_INDEX="index",
#                  base_command_interface.hpp:131) -> applyColor
#                  CC_STAR_TABLE :1708`
#                 -- the header is a SUB-anchor; :1708 is the .cpp.
#   FLAT          `AppCommandColor::setClassicColor takes `debug_message` BY
#                  VALUE (app_command_color.cpp:58-59) while the constructor
#                  holds it by reference (:92-94)`
#                 -- the parenthesis IS the subject; :92-94 is app_command_color.
#
# So the reading is decided by CONTENT, against the pin, by rules that can all
# fail, and every decision is recorded per reference:
#
#   R1  the enclosing function at <candidate>:<line> is NAMED in the string
#   R2  the string belongs to command X and the enclosing function is the
#       handler the grammar itself gives X
#   R3  a quoted fragment of the string (`...` or "...") occurs in the
#       anchored lines
#   R4  the candidate file is NAMED in the string (an unstated default never
#       outranks a file the author actually wrote)
#   R0  in range at the pin (necessary, never sufficient)
#
# A tie between two candidates that both score, or a token no rule reaches, is
# FLAGGED and read by hand -- never decided by the tool.
# --------------------------------------------------------------------------
QUOTED = re.compile(r'`([^`]{5,60})`|"([^"]{5,60})"')

def _score(cand, pin, lo, hi, s, handler, named=False):
    rp, how = resolve_file(cand)
    if rp is None:
        return None
    ls = lines_at(pin, rp)
    if ls is None or lo < 1 or (hi or lo) > len(ls):
        return None
    sc, why = 1, ["R0"]
    if named:
        sc += 2; why.append("R4")
    sym = enclosing_symbol(pin, rp, lo)
    if sym and sym in s:
        sc += 5; why.append("R1")
    if sym and handler and sym == handler:
        sc += 4; why.append("R2")
    body = "\n".join(ls[lo - 1:(hi or lo)])
    for m in QUOTED.finditer(s):
        frag = m.group(1) or m.group(2)
        if frag and frag in body:
            sc += 3; why.append("R3(%s)" % frag[:18]); break
    return (sc, "+".join(why), rp, how)

def disambiguate(t, pin, s, handler):
    """-> (repo_path, how, basis, evidence) or (None, None, 'FLAG', reason)."""
    # Candidates are deduplicated by their RESOLVED PATH, not by spelling: the
    # bare `app_command_interface.cpp` and the repo-relative default are the
    # same file, and treating them as rivals manufactured 36 ties and 261
    # coin-flips that were never ambiguities.
    cands, seen = [], set()
    for name, tag in ((t.file, "inherited-paren"), (t.flat, "inherited-flat"),
                      (DEFAULT_FILE, "implicit-default")):
        if not name:
            continue
        rp0, _how0 = resolve_file(name)
        if rp0 is None or rp0 in seen:
            continue
        seen.add(rp0)
        r = _score(name, pin, t.elems[0][0], t.elems[0][1], s, handler,
                   named=(tag != "implicit-default"))
        if r:
            cands.append((r[0], tag, r[1], r[2], r[3]))
    if not cands:
        return (None, None, "FLAG", "no candidate in range at the pin")
    cands.sort(key=lambda c: -c[0])
    if len(cands) > 1 and cands[0][0] == cands[1][0] and cands[0][0] > 1:
        return (None, None, "FLAG", "tie %s vs %s" % (cands[0][1], cands[1][1]))
    return (cands[0][3], cands[0][4], cands[0][1], cands[0][2])

# --------------------------------------------------------------------------
# 6. THE SUBMODULE (src/EntityCore) -- read-only, resolved at ITS pinned commit
# --------------------------------------------------------------------------
def sub_commit(rev):
    out = git("ls-tree", rev, SUBMODULE)
    if not out:
        return None
    m = re.search(rb'commit ([0-9a-f]{40})', out)
    return m.group(1).decode() if m else None

def classify_any(pin, repo_path, n, how):
    """classify() with the submodule routed to its own repository."""
    if how == "submodule":
        a, b = sub_commit(pin), sub_commit("HEAD")
        if not a or not b:
            return ("no-file", None, None, None)
        rel = repo_path[len(SUBMODULE) + 1:]
        return classify(a, rel, n, repo=os.path.join(REPO, SUBMODULE), head=b)
    return classify(pin, repo_path, n)

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

if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "shapes"
    {"shapes": cmd_shapes, "pins": cmd_pins, "map": cmd_map,
     "default-check": cmd_default_check}[cmd]()

