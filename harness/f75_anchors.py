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
    __slots__ = ("start", "end", "kind", "head", "listtext", "elems", "file")
    def __init__(self, start, end, kind, head, listtext, elems):
        self.start, self.end, self.kind = start, end, kind
        self.head, self.listtext, self.elems = head, listtext, elems
        self.file = None
    def __repr__(self):
        return "<%s %s %s -> %s>" % (self.kind, self.head, self.listtext, self.file)

def tokenize(s):
    """Return the anchor tokens of one string, in order, with `file` filled in
    from the string's own left context.  `file` is None for a token whose file
    could not be determined -- reported, never guessed."""
    toks, cur = [], None
    for m in CAND.finditer(s):
        i = m.start()
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
            cur = head
        t.file = head if kind == "FILE" else cur
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

if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "shapes"
    {"shapes": cmd_shapes, "pins": cmd_pins}[cmd]()
