#!/usr/bin/env python3
"""anchor_gate.py -- the grammar's engine anchors still point at the engine.

WHY THIS EXISTS. Every `file:line` in `grammar/sc-grammar.json` and the four
`grammar/args/unit-*.json` fragments is a citation into spacecrafter's source,
and C2 is the constraint they implement: no entry without a source anchor. An
anchor that no longer resolves is, in effect, an entry without one -- and the
failure is SILENT, which is what makes it worth a gate. Between 2026-08-04 and
2026-09-01 every anchor in this file went stale and nothing said so; the first
three anyone opened landed on unrelated code (claude/INTENT 11.190(e)).

WHAT IT CHECKS, and why in this order.

 1. EVERY REFERENCE RESOLVES AT ITS PIN. The pin is a commit, so this cannot
    rot -- that is the whole point of pinning, and checking it is what makes
    the pin sufficient rather than decorative. A reference past the end of its
    file at its own pin is a broken citation whatever HEAD does.

 2. THE HEAD MAPPING IS REPORTED, not assumed: for each reference, is the text
    it was written against still at that line (clean), somewhere else (moved),
    or gone. `gone` is RED unless the reference says so itself with a
    `[NOT AT HEAD: ...]` marker -- C2's honest state, which a reader can see.

 3. THE COUNTS MATCH tests/anchor-expected.txt. This is what makes the gate
    fire when the ENGINE moves rather than only when the grammar does: the day
    a handler shifts, `clean` falls and `moved` rises, and somebody has to
    look. It is a record, not a silencer (README, Verification): the file is
    edited deliberately, never regenerated blind.

`_meta` IS OUT OF SCOPE and stays out. Those blocks are dated measurement
records pinned by their own `code` / `merged` fields -- the fragments' 384
`accounting` rows carry their line as a bare integer no `file:line` parser can
see, so half of that record is invisible here and moving the other half would
be I2's silent desync. `_meta.anchor_pin` is the pin of everything else.

Run: python3 tests/anchor_gate.py [--record]   (ctest target `anchor_gate`)
"""
import json, os, re, subprocess, sys, collections, difflib

# The repository root, found from this file rather than hard-coded, so the gate
# runs from any build directory and from a clone at any path.
HARN = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HARN, "..", "..", ".."))

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

WORKTREE = "\0WORKTREE"     # the files as they are ON DISK, not as committed

def blob(rev, path, repo=REPO):
    key = (repo, rev, path)
    if rev == WORKTREE:
        # The HEAD side of every comparison is the WORKING TREE, deliberately.
        # A gate that only saw committed state would go green on the very edit
        # that breaks the anchors and red only after it was pushed; this way an
        # engine change and the citations into it are reconciled in the same
        # working session, which is the only moment somebody still knows why
        # the line moved.
        if key not in _blob_cache:
            f = os.path.join(repo, path)
            # BINARY read, decoded by hand: text mode translates newlines, and
            # a CRLF file would then differ from its own committed bytes on
            # every line -- four references read as `gone` before this was
            # fixed, none of them touched by anybody.
            _blob_cache[key] = (open(f, "rb").read().decode("utf-8", "replace")
                                if os.path.exists(f) else None)
        return _blob_cache[key]
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
    # SECOND PASS -- the D14 rescue.  (F70 is behind us, so on a clean tree
    # this fires nowhere; it stays because a pin older than F70 still needs it.)  F70 transliterated the whole tree
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

def classify_any(pin, repo_path, n, how, head="HEAD"):
    """classify() with the submodule routed to its own repository."""
    if how == "submodule":
        a, b = sub_commit(pin), sub_commit("HEAD")
        if not a or not b:
            return ("no-file", None, None, None)
        rel = repo_path[len(SUBMODULE) + 1:]
        # The submodule is read-only here and stays at ITS pinned commit.
        return classify(a, rel, n, repo=os.path.join(REPO, SUBMODULE), head=b)
    return classify(pin, repo_path, n, head=head)


# --------------------------------------------------------------------------
# THE ONE READING A RULE CANNOT MAKE
#
# `flyto.registration` ends `... registered in the aliases block AFTER the
# reverse map is built (:112-118, since 2026-08-31)`. Parenthesis-scoped
# inheritance says app_command_init.cpp; flat inheritance says
# base_command_interface.hpp; the line is in range in BOTH, and no content rule
# separates them. The SENTENCE separates them -- the aliases block is in
# app_command_init.cpp -- so the reading is recorded here, keyed by the JSON
# path and the token's position, both of which survive a sweep of the numbers.
# One row. If a second is ever needed, it belongs here beside this one and not
# in a heuristic.
# --------------------------------------------------------------------------
FILE_OVERRIDES = {
    (".families.commands.flyto.registration", 2): "src/interfaceModule/app_command_init.cpp",
    (".commands.flyto.registration", 2):          "src/interfaceModule/app_command_init.cpp",
}

# --------------------------------------------------------------------------
# THE GATE
# --------------------------------------------------------------------------
EXPECTED = os.path.join(HARN, "anchor-expected.txt")
MARKER = re.compile(r'\[NOT AT HEAD: [^\]]+\]')

def file_pin(path):
    """The ONE file-level pin. `_meta.anchor_pin` says which commit every
    anchor outside `_meta` resolves at; `_meta.code` / `_meta.merged` pin the
    `_meta` block's own measurements and are NOT this."""
    g = load(path)
    p = g["_meta"].get("anchor_pin")
    if not p:
        raise SystemExit("%s: _meta.anchor_pin missing -- the pin is the gate" % path)
    return p.split("@")[1].strip().split()[0]

def run():
    tally = collections.Counter()
    hard = []
    for path in TARGETS:
        pin = file_pin(path)
        g = load(path)
        cmds = g["families"]["commands"] if path == MERGED else g["commands"]
        for jp, s in walk(g):
            if jp.startswith("._meta"):
                tally["skipped(_meta)"] += sum(len(t.elems) for t in tokenize(s))
                continue
            toks = tokenize(s)
            if not toks:
                continue
            marked = bool(MARKER.search(s))
            m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == MERGED
                         else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
            handler = cmds.get(m.group(1), {}).get("handler") if m else None
            for ti, t in enumerate(toks):
                if (jp, ti) in FILE_OVERRIDES:
                    rp, how = FILE_OVERRIDES[(jp, ti)], "repo-relative"
                elif t.kind == "FILE":
                    rp, how = resolve_file(t.file)
                    if rp is None:
                        if marked:
                            tally["not-at-head(declared)"] += len(t.elems); continue
                        hard.append("%s %s: file %r is not in this tree and the "
                                    "reference carries no [NOT AT HEAD] marker"
                                    % (path, jp, t.file))
                        tally["UNRESOLVED"] += len(t.elems); continue
                else:
                    rp, how, basis, _ev = disambiguate(t, pin, s, handler)
                    if rp is None:
                        if marked:
                            tally["not-at-head(declared)"] += len(t.elems); continue
                        hard.append("%s %s: bare `:%s` -- no candidate file resolves "
                                    "it at %s" % (path, jp, t.listtext, pin[:8]))
                        tally["UNRESOLVED"] += len(t.elems); continue
                for a, b, _, _ in t.elems:
                    # 1. at its own pin -- the check that cannot rot
                    for n in (a, b):
                        if n is None:
                            continue
                        v = classify_any(pin, rp, n, how, head=WORKTREE)[0]
                        if v in ("no-file", "out-of-range"):
                            if marked:
                                break
                            hard.append("%s %s: %s:%d does not exist at the pin %s (%s)"
                                        % (path, jp, rp, n, pin[:8], v))
                            tally["AT-PIN-BROKEN"] += 1
                            break
                    else:
                        # 2. the HEAD mapping, reported
                        v = classify_any(pin, rp, a, how, head=WORKTREE)[0]
                        if b is not None:
                            v2 = classify_any(pin, rp, b, how, head=WORKTREE)[0]
                            OK = ("clean", "moved", "d14", "d14-moved")
                            v = ("clean" if v == v2 == "clean" else
                                 ("moved" if v in OK and v2 in OK else
                                  (v if v not in OK else v2)))
                        if v in ("clean",):
                            tally["clean"] += 1
                        elif v in ("moved", "d14", "d14-moved"):
                            tally["moved"] += 1
                        elif marked:
                            tally["not-at-head(declared)"] += 1
                        else:
                            hard.append("%s %s: %s:%s no longer exists at HEAD and "
                                        "carries no [NOT AT HEAD] marker"
                                        % (path, jp, rp, t.listtext))
                            tally["GONE"] += 1
                        continue
                    if marked:
                        tally["not-at-head(declared)"] += 1
    return tally, hard

def main():
    tally, hard = run()
    lines = ["# anchor_gate record -- counts, not a silencer (README, Verification).",
             "# Edited deliberately: a count that moves means the ENGINE moved under",
             "# the grammar's citations, and somebody has to look at what.",
             "# Regenerate with: python3 tests/anchor_gate.py --record"]
    for k in sorted(tally):
        lines.append("%-24s %d" % (k, tally[k]))
    got = "\n".join(lines) + "\n"
    if "--record" in sys.argv:
        open(EXPECTED, "w").write(got)
        print(got, end="")
        return 0
    for h in hard:
        print("FAIL  " + h)
    want = open(EXPECTED).read() if os.path.exists(EXPECTED) else ""
    if got != want:
        print("FAIL  counts differ from tests/anchor-expected.txt")
        print("--- recorded ---\n%s--- got ---\n%s" % (want, got))
        return 1
    if hard:
        return 1
    print(got, end="")
    print("anchor gate: every reference resolves at its pin, and its HEAD state is the recorded one")
    return 0

if __name__ == "__main__":
    sys.exit(main())
