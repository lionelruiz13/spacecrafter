#!/usr/bin/env python3
"""f70_ascii.py - the standing ASCII census for the spacecrafter CODE repo (D14).

D14 (INTENT.md 2.0, vixy 2026-08-31): "Every source file must be in ASCII, accents
are to be removed for this purpose."  This instrument is the census that makes the
constraint checkable, and the gate that keeps it true after F70's one-shot sweep.

Three things it does, deliberately separated so the boundary stays challengeable:

  census    enumerate EVERY tracked file of the code repo; per file report the
            encoding class (ascii / utf8 / mixed / iso8859 / binary), the byte
            and line counts, and - for source files - where those bytes SIT
            (comment / string / char literal / code / text).
  literals  dump every non-ASCII occurrence that sits inside a STRING or CHAR
            literal, with file:line and the enclosing literal.  This is the input
            to the per-literal disposition list: a literal can reach a socket, a
            gettext catalogue or a data comparison, and converting it there is a
            SEMANTIC change, not a transliteration.
  gate      read the CONVERT/EXCLUDE partition and assert ZERO non-ASCII bytes in
            every CONVERT member.  Non-zero exit = D14 violated.  Also asserts the
            partition still COVERS the tree: a newly added non-ASCII file that is
            in neither column is a gate failure, not a silent pass.

MEASURED HAZARDS this instrument exists to route around (both cost real time):

  (1) The Bash-tool `grep` is ugrep with `-I`: any file holding non-UTF-8 bytes is
      classed BINARY and skipped SILENTLY (CLAUDE.md rule; 11.188(k)).  So a plain
      grep sweep CANNOT see the ISO-8859 members at all.
  (2) `/usr/bin/grep -P '[\x80-\xff]'` under a UTF-8 locale matches CODE POINTS
      U+0080..U+00FF, not bytes: it finds the accents and misses every box-drawing
      / Greek / arrow character above U+00FF.  Measured 2026-08-31 (F70): it
      reports 0 hits on ftxui's border.cpp, which is made of box drawing.
      LC_ALL=C is mandatory for a byte-class grep.  This script reads bytes.

Usage:
  python3 f70_ascii.py census   [--root DIR] [--tsv OUT] [--summary]
  python3 f70_ascii.py literals [--root DIR] [--convert-only]
  python3 f70_ascii.py chars    [--root DIR]
  python3 f70_ascii.py gate     [--root DIR] [--partition FILE]
"""

import argparse
import collections
import os
import subprocess
import sys
import unicodedata

DEFAULT_ROOT = "/home/claude/spacecrafter"
DEFAULT_PARTITION = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                 "f70_partition.tsv")

# ---------------------------------------------------------------- enumeration

def tracked_files(root):
    """Every tracked path of the code repo that is a real regular file.

    git ls-files also lists the EntityCore gitlink and any symlink; both are
    excluded here - a submodule is its own repo (read-only by standing rule) and
    a symlink has no bytes of its own.
    """
    out = subprocess.run(["git", "-C", root, "ls-files", "-z"],
                         capture_output=True, check=True).stdout
    paths = [p.decode("utf-8", "surrogateescape") for p in out.split(b"\0") if p]
    keep = []
    for p in paths:
        full = os.path.join(root, p)
        if os.path.islink(full) or not os.path.isfile(full):
            continue
        keep.append(p)
    return keep


def decode_bytes(data):
    """Decode a text blob the way the tree is actually written, and say how.

    Returns (text, n_utf8_multibyte, n_latin1_fallback).

    MEASURED 2026-08-31 (F70), and it corrects the standing rule: the two
    `src/interfaceModule/app_command_interface.*` files are NOT ISO-8859 files.
    Each is valid UTF-8 except for ONE stray 0xA7 byte (a Latin-1 section sign,
    both written by code commit 2b8ec034) - which is enough to make the whole
    file undecodable and so to make every UTF-8-aware tool call it binary.
    Decoding such a file wholesale as Latin-1 - which the task section proposed -
    turns its REAL accents into mojibake: the UTF-8 pair C3 A9 ('e-acute') would
    come out as two characters 'A-tilde' + 'copyright'.  So: greedy UTF-8, and
    only the bytes that cannot start a valid UTF-8 sequence fall back to Latin-1.
    A pure ISO-8859 file (planetsephems/*, whose single 0xE9 bytes never form
    valid UTF-8) decodes correctly under the same rule, so ONE decoder serves
    every class and the census and the converter cannot disagree (I2).
    """
    out = []
    n_multi = 0
    n_fallback = 0
    i = 0
    n = len(data)
    while i < n:
        b = data[i]
        if b < 0x80:
            out.append(chr(b))
            i += 1
            continue
        for width in (2, 3, 4):
            chunk = data[i:i + width]
            if len(chunk) < width:
                continue
            try:
                ch = chunk.decode("utf-8")
            except UnicodeDecodeError:
                continue
            out.append(ch)
            n_multi += 1
            i += width
            break
        else:
            out.append(chr(b))          # Latin-1 value of the stray byte
            n_fallback += 1
            i += 1
    return "".join(out), n_multi, n_fallback


def classify_encoding(data):
    """ascii | utf8 | mixed | iso8859 | binary.

    'binary' is decided by a NUL byte, not by a heuristic: every text format in
    this tree is NUL-free, and every compiled artifact (.spv, .png, .pdf, .vsix,
    .blend) carries NULs.  The three text classes are told apart by HOW the
    non-ASCII bytes decode, because that is the distinction that matters to a
    tool: 'utf8' = all of them are UTF-8; 'iso8859' = none of them are;
    'mixed' = some are and some are not, which is the worst case and the one
    D14 exists to eliminate (it makes the file undecodable AND its accents
    unrecoverable by any single-encoding read).
    """
    if not any(b >= 0x80 for b in data):
        return "ascii"
    if b"\0" in data:
        return "binary"
    _, n_multi, n_fallback = decode_bytes(data)
    if n_fallback == 0:
        return "utf8"
    if n_multi == 0:
        return "iso8859"
    return "mixed"


def decode(data, enc=None):
    return decode_bytes(data)[0]


# ------------------------------------------------------------ context scanner

C_LIKE = {".c", ".cpp", ".cc", ".cxx", ".h", ".hpp", ".hh", ".hxx",
          ".frag", ".vert", ".geom", ".glsl", ".comp", ".tesc", ".tese",
          ".ts", ".js", ".php", ".css"}
HASH_LIKE = {".py", ".sh", ".cmake", ".bash"}
JSON_LIKE = {".json"}
# Files whose whole body is prose/data: no literal structure to speak of.
TEXT_LIKE = {".md", ".txt", ".html", ".htm", ".sts", ".fab", ".ini", ".fs",
             ".gitignore", ".code-search", ".AUTHORS"}


def family(path):
    base = os.path.basename(path)
    ext = os.path.splitext(base)[1].lower()
    if base in ("CMakeLists.txt",):
        return "hash"
    if ext in C_LIKE:
        return "c"
    if ext in HASH_LIKE:
        return "hash"
    if ext in JSON_LIKE:
        return "json"
    if ext in TEXT_LIKE or ext == "":
        return "text"
    return "text"


def scan_c(text):
    """Per-character context for a C-family source.

    Returns a list the same length as `text`, each entry one of
    comment / string / char / code.  Handles // and /* */ comments, "..." and
    '...' literals with backslash escapes, and R"delim( ... )delim" raw strings.
    Approximate by design (it is a census, not a compiler) - its one job is to
    tell a comment from a literal so the literal gets a disposition instead of a
    blind rewrite.  Any misclassification therefore fails SAFE only if it calls
    code 'string'; the literals dump is reviewed by hand for exactly that reason.
    """
    n = len(text)
    ctx = ["code"] * n
    i = 0
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                ctx[k] = "comment"
            i = j
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                ctx[k] = "comment"
            i = j
            continue
        if c == "R" and i + 1 < n and text[i + 1] == '"':
            k = text.find("(", i + 2)
            if 0 <= k <= i + 18:
                delim = text[i + 2:k]
                end = text.find(")" + delim + '"', k)
                j = n if end < 0 else end + len(delim) + 2
                for q in range(i, j):
                    ctx[q] = "string"
                i = j
                continue
        if c in ('"', "'"):
            kind = "string" if c == '"' else "char"
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == c:
                    j += 1
                    break
                if text[j] == "\n":       # unterminated - do not run away
                    break
                j += 1
            for k in range(i, min(j, n)):
                ctx[k] = kind
            i = max(j, i + 1)
            continue
        i += 1
    return ctx


def scan_hash(text):
    n = len(text)
    ctx = ["code"] * n
    i = 0
    while i < n:
        c = text[i]
        if c == "#":
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                ctx[k] = "comment"
            i = j
            continue
        if c in ('"', "'"):
            trip = text[i:i + 3]
            if trip in ('"""', "'''"):
                end = text.find(trip, i + 3)
                j = n if end < 0 else end + 3
                for k in range(i, j):
                    ctx[k] = "string"
                i = j
                continue
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == c:
                    j += 1
                    break
                if text[j] == "\n":
                    break
                j += 1
            for k in range(i, min(j, n)):
                ctx[k] = "string"
            i = max(j, i + 1)
            continue
        i += 1
    return ctx


def scan_json(text):
    n = len(text)
    ctx = ["code"] * n
    i = 0
    while i < n:
        if text[i] == '"':
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == '"':
                    j += 1
                    break
                j += 1
            for k in range(i, min(j, n)):
                ctx[k] = "string"
            i = max(j, i + 1)
            continue
        i += 1
    return ctx


def contexts(path, text):
    fam = family(path)
    if fam == "c":
        return scan_c(text)
    if fam == "hash":
        return scan_hash(text)
    if fam == "json":
        return scan_json(text)
    return ["text"] * len(text)


# ------------------------------------------------------------------ partition

def read_partition(path):
    """TSV: decision <TAB> pathspec <TAB> reason.

    decision is CONVERT or EXCLUDE; pathspec is either an exact tracked path or a
    prefix ending in '/'.  Order does not matter: the LONGEST matching pathspec
    wins, so a CONVERT exception inside an EXCLUDE subtree is expressible and
    reviewable as one row.
    """
    rows = []
    with open(path, encoding="utf-8") as fh:
        for lineno, line in enumerate(fh, 1):
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 3:
                raise SystemExit("%s:%d: need 3 tab-separated fields" % (path, lineno))
            decision, spec, reason = parts[0].strip(), parts[1].strip(), parts[2].strip()
            if decision not in ("CONVERT", "EXCLUDE"):
                raise SystemExit("%s:%d: decision must be CONVERT|EXCLUDE" % (path, lineno))
            if not reason:
                raise SystemExit("%s:%d: every row needs a reason" % (path, lineno))
            rows.append((decision, spec, reason))
    return rows


def decide(rows, path):
    best = None
    for decision, spec, reason in rows:
        if spec.endswith("/"):
            hit = path.startswith(spec)
        else:
            hit = (path == spec)
        if hit and (best is None or len(spec) > len(best[1])):
            best = (decision, spec, reason)
    return best


# --------------------------------------------------------------------- report

def collect(root):
    """One pass over the tree; every later view is derived from this."""
    recs = []
    for path in tracked_files(root):
        with open(os.path.join(root, path), "rb") as fh:
            data = fh.read()
        enc = classify_encoding(data)
        if enc == "ascii":
            recs.append(dict(path=path, enc=enc, nbytes=0, nlines=0, ctx={}, hits=[]))
            continue
        nbytes = sum(1 for b in data if b >= 0x80)
        if enc == "binary":
            recs.append(dict(path=path, enc=enc, nbytes=nbytes, nlines=0,
                             ctx={}, hits=[]))
            continue
        text = decode(data, enc)
        ctx = contexts(path, text)
        counter = collections.Counter()
        hits = []
        line = 1
        for idx, ch in enumerate(text):
            if ch == "\n":
                line += 1
                continue
            if ord(ch) < 0x80:
                continue
            kind = ctx[idx]
            counter[kind] += 1
            hits.append((line, idx, ch, kind))
        nlines = len({h[0] for h in hits})
        recs.append(dict(path=path, enc=enc, nbytes=nbytes, nlines=nlines,
                         ctx=counter, hits=hits, text=text))
    return recs


KINDS = ("comment", "string", "char", "code", "text")


def cmd_census(args):
    recs = collect(args.root)
    rows = [r for r in recs if r["enc"] != "ascii"]
    rows.sort(key=lambda r: (-r["nbytes"], r["path"]))
    part = read_partition(args.partition) if os.path.exists(args.partition) else None
    lines = ["\t".join(["path", "class", "decision", "nonascii_bytes",
                        "nonascii_lines"] + list(KINDS))]
    for r in rows:
        d = "-"
        if part:
            hit = decide(part, r["path"])
            d = hit[0] if hit else "UNCLASSIFIED"
        lines.append("\t".join([r["path"], r["enc"], d, str(r["nbytes"]),
                                str(r["nlines"])] +
                               [str(r["ctx"].get(k, 0)) for k in KINDS]))
    body = "\n".join(lines) + "\n"
    if args.tsv:
        with open(args.tsv, "w", encoding="utf-8") as fh:
            fh.write(body)
        print("wrote %s (%d non-ASCII files)" % (args.tsv, len(rows)))
    else:
        sys.stdout.write(body)

    if args.summary or args.tsv:
        by_enc = collections.Counter(r["enc"] for r in recs)
        tot = collections.Counter()
        for r in rows:
            tot.update(r["ctx"])
        sys.stderr.write("tracked regular files: %d\n" % len(recs))
        for k in ("ascii", "utf8", "mixed", "iso8859", "binary"):
            sys.stderr.write("  %-8s %5d\n" % (k, by_enc.get(k, 0)))
        sys.stderr.write("non-ASCII files: %d (text %d, binary %d)\n" % (
            len(rows), sum(1 for r in rows if r["enc"] != "binary"),
            sum(1 for r in rows if r["enc"] == "binary")))
        sys.stderr.write("non-ASCII chars by context (text files): %s\n" %
                         ", ".join("%s=%d" % (k, tot.get(k, 0)) for k in KINDS))
    return 0


def cmd_literals(args):
    recs = collect(args.root)
    part = read_partition(args.partition) if os.path.exists(args.partition) else None
    out = []
    for r in recs:
        if r["enc"] in ("ascii", "binary"):
            continue
        if args.convert_only and part:
            hit = decide(part, r["path"])
            if not hit or hit[0] != "CONVERT":
                continue
        lits = [h for h in r["hits"] if h[3] in ("string", "char")]
        if not lits:
            continue
        text = r["text"]
        bylines = collections.defaultdict(list)
        for line, idx, ch, kind in lits:
            bylines[line].append(ch)
        for line in sorted(bylines):
            start = 0
            cur = 1
            for i, c in enumerate(text):
                if cur == line:
                    start = i
                    break
                if c == "\n":
                    cur += 1
            end = text.find("\n", start)
            end = len(text) if end < 0 else end
            src = text[start:end].strip()
            chars = "".join(sorted(set(bylines[line])))
            out.append("%s:%d\t%s\t%s" % (r["path"], line, chars, src))
    print("\n".join(out))
    sys.stderr.write("%d literal-bearing lines\n" % len(out))
    return 0


def cmd_chars(args):
    recs = collect(args.root)
    part = read_partition(args.partition) if os.path.exists(args.partition) else None
    hist = collections.Counter()
    for r in recs:
        if r["enc"] in ("ascii", "binary"):
            continue
        if part:
            hit = decide(part, r["path"])
            if args.convert_only and (not hit or hit[0] != "CONVERT"):
                continue
        for _, _, ch, _ in r["hits"]:
            hist[ch] += 1
    for ch, n in hist.most_common():
        print("U+%04X\t%d\t%s" % (ord(ch), n, unicodedata.name(ch, "<unnamed>")))
    sys.stderr.write("%d distinct non-ASCII characters, %d occurrences\n" %
                     (len(hist), sum(hist.values())))
    return 0


def cmd_gate(args):
    part = read_partition(args.partition)
    recs = collect(args.root)
    bad = []
    unclassified = []
    for r in recs:
        if r["enc"] == "ascii":
            continue
        hit = decide(part, r["path"])
        if hit is None:
            unclassified.append(r["path"])
            continue
        if hit[0] == "CONVERT":
            bad.append((r["path"], r["enc"], r["nbytes"]))
    # A CONVERT row that matches nothing is stale: report it, it is how a
    # partition rots without anyone noticing.
    stale = []
    tracked = set(tracked_files(args.root))
    for decision, spec, _ in part:
        if spec.endswith("/"):
            if not any(p.startswith(spec) for p in tracked):
                stale.append(spec)
        elif spec not in tracked:
            stale.append(spec)
    rc = 0
    if bad:
        rc = 1
        print("FAIL: %d CONVERT file(s) still carry non-ASCII bytes:" % len(bad))
        for p, e, n in sorted(bad):
            print("  %-70s %-8s %d bytes" % (p, e, n))
    if unclassified:
        rc = 1
        print("FAIL: %d non-ASCII file(s) matched no partition row "
              "(the boundary must cover the tree):" % len(unclassified))
        for p in sorted(unclassified):
            print("  %s" % p)
    if stale:
        print("WARN: %d partition row(s) match no tracked path: %s" %
              (len(stale), ", ".join(sorted(stale))))
    if rc == 0:
        nconv = sum(1 for r in recs
                    if (decide(part, r["path"]) or ("EXCLUDE",))[0] == "CONVERT")
        nexcl_na = sum(1 for r in recs if r["enc"] != "ascii"
                       and (decide(part, r["path"]) or ("EXCLUDE",))[0] == "EXCLUDE")
        print("PASS: 0 non-ASCII bytes across the CONVERT set "
              "(%d files matched by CONVERT rows); %d EXCLUDE files carry "
              "non-ASCII by design." % (nconv, nexcl_na))
    return rc


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["census", "literals", "chars", "gate"])
    ap.add_argument("--root", default=DEFAULT_ROOT)
    ap.add_argument("--partition", default=DEFAULT_PARTITION)
    ap.add_argument("--tsv")
    ap.add_argument("--summary", action="store_true")
    ap.add_argument("--convert-only", action="store_true")
    args = ap.parse_args()
    return {"census": cmd_census, "literals": cmd_literals,
            "chars": cmd_chars, "gate": cmd_gate}[args.mode](args)


if __name__ == "__main__":
    sys.exit(main())
