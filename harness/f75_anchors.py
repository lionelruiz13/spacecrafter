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
    __slots__ = ("start", "end", "kind", "head", "listtext", "elems", "file", "flat", "lspan")
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
        t.lspan = (m.start(1), m.end(1))
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
    rewrites = json.load(open(os.path.join(OUT, "rewrites.json")))
    inverse = {}
    for old, new in rewrites.items():
        if new in inverse and inverse[new] != old:
            raise SystemExit("inverse is not a function: %r" % new)
        inverse[new] = old
    ok = True
    for path in TARGETS:
        cur = open(os.path.join(REPO, path), encoding="utf-8").read()
        was = blob("HEAD", path)
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


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "shapes"
    {"shapes": cmd_shapes, "pins": cmd_pins, "map": cmd_map,
     "default-check": cmd_default_check, "sweep": cmd_sweep,
     "read-table-check": cmd_read_table_check, "verify": cmd_verify}[cmd]()
