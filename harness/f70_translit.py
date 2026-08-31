#!/usr/bin/env python3
"""f70_translit.py - D14's one-shot sweep, and the proof that it is only that.

D14 [vixy 2026-08-31]: "Every source file must be in ASCII, accents are to be
removed for this purpose."

THE SWEEP IS TWO RULES, and the split between them is the whole safety argument.

  (1) PROSE - comments, markdown, JSON documentation values, plain text, and any
      non-C-family file - is TRANSLITERATED through f70_map.tsv.  This is the
      mandate's own clause; a comment has no consumer but a reader.

  (2) A STRING LITERAL in a C-family source is NOT transliterated.  Every
      non-ASCII character in it is re-spelled as \\xNN escapes of the SAME bytes,
      so the compiled program emits byte-for-byte what it emitted before, and the
      source file is nevertheless pure ASCII.

Rule (2) exists because a literal is not prose - it is an output.  Measured, in
this tree, a literal reaches:
  * the DOME.  `oss << truncf(alt) << "\N{DEGREE SIGN}"` draws the altitude
    label; skyDisplay/skyline/skygrid do this 20 times.  Transliterating would
    put "45deg" on the screen where "45" and a degree sign are drawn today.
  * the TCP WIRE.  A `debug_message` reaches a $DIAG subscriber (11.188), so a
    refusal string's bytes are protocol-visible.
  * a PINNED FIXTURE.  scedit's lint/doc/ui expectations record the exact bytes
    of its messages; tests/ui-selftest-expected.txt is EXCLUDED from the sweep by
    the partition, and ui_gate.cmake says of it "never regenerating it blind".
Under rule (2) none of those move, and none of them had to be decided one by one.

The escape is not an invention of this sweep: the same tree already writes
`"\\xC2\\xB7 Tab complete "` (sc_tui.cpp:640) and `"...Caf\\xE9" "\\tx\\r\\n"`
(sc_tui.cpp:1215) for exactly this reason - the second one also demonstrating the
string-splitting guard this script applies when a hex escape is followed by a hex
digit, which C would otherwise swallow into the escape.

Modes:
  map              print the parsed map (the review view)
  plan  [--root R] what would change, per file, with counts and any REFUSALS
  apply [--root R] do it
  verify --from REV
                   re-derive the whole sweep from REV's blobs and assert the
                   working tree is EXACTLY that.  This is the deliverable: it
                   proves the diff contains nothing but rules (1) and (2) - no
                   whitespace, no line endings, no content, no "while I was here".
"""

import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import f70_ascii as A                                            # noqa: E402

DEFAULT_ROOT = "/home/claude/spacecrafter"
MAP_FILE = os.path.join(HERE, "f70_map.tsv")
HEX = set("0123456789abcdefABCDEF")


# ------------------------------------------------------------------- the map

def read_map(path=MAP_FILE):
    """key(str) -> replacement(str), longest key first at match time."""
    table = {}
    with open(path, encoding="utf-8") as fh:
        for lineno, line in enumerate(fh, 1):
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 3:
                raise SystemExit("%s:%d: need key<TAB>replacement<TAB>note"
                                 % (path, lineno))
            key_s, rep, note = parts[0].strip(), parts[1], parts[2].strip()
            key = "".join(chr(int(cp[2:], 16)) for cp in key_s.split())
            rep = rep.replace("<SP>", " ").replace("<NONE>", "")
            if not note:
                raise SystemExit("%s:%d: every row needs a note" % (path, lineno))
            if any(ord(c) >= 0x80 for c in rep):
                raise SystemExit("%s:%d: replacement is not ASCII" % (path, lineno))
            if key in table:
                raise SystemExit("%s:%d: duplicate key" % (path, lineno))
            table[key] = rep
    return table


def match_key(text, i, table, widths):
    """Longest map key matching at position i, or None.

    Tried BEFORE the ASCII fast path, and that ordering is load-bearing: a
    multi-codepoint key can BEGIN with an ASCII character - "Ch<U+FFFD>reau"
    does - and skipping ahead on the 'C' would silently leave the single-char
    rule to fire on the damaged byte alone.  Measured the first time this ran:
    the two repaired names came out as "Ch?reau" and "J?r?me".
    """
    for w in widths:
        seg = text[i:i + w]
        if len(seg) == w and seg in table:
            return w, table[seg]
    return None


def translit(text, table, refusals, where):
    """Apply the map to PROSE.  Longest key wins; an unmapped non-ASCII
    character is a REFUSAL, never a silent drop - the map must grow instead."""
    widths = sorted({len(k) for k in table}, reverse=True)
    out = []
    i = 0
    n = len(text)
    while i < n:
        hit = match_key(text, i, table, widths)
        if hit:
            out.append(hit[1])
            i += hit[0]
            continue
        ch = text[i]
        if ord(ch) < 0x80:
            out.append(ch)
            i += 1
            continue
        refusals.append((where, i, ch))
        out.append(ch)
        i += 1
    return "".join(out)


def escape_run(chars):
    """The UTF-8 bytes of `chars`, spelled as \\xNN escapes."""
    return "".join("\\x%02x" % b for b in "".join(chars).encode("utf-8"))


# ------------------------------------------------------------- the file sweep

def convert_text(path, data, table, refusals):
    """Return the converted text for one file, or raise on a case the rules
    cannot express."""
    text = A.decode(data)
    fam = A.family(path)
    if fam != "c":
        return translit(text, table, refusals, path)

    ctx = A.scan_c(text)
    widths = sorted({len(k) for k in table}, reverse=True)
    out = []
    i = 0
    n = len(text)
    while i < n:
        ch = text[i]
        if ord(ch) < 0x80:
            # A multi-codepoint key may START here even though this character
            # is ASCII - but only outside a literal, where rule (2) owns the
            # bytes and nothing is transliterated at all.
            hit = None if ctx[i] in ("string", "char") else \
                match_key(text, i, table, widths)
            if hit and any(ord(c) >= 0x80 for c in text[i:i + hit[0]]):
                out.append(hit[1])
                i += hit[0]
                continue
            out.append(ch)
            i += 1
            continue
        if ctx[i] in ("string", "char"):
            # Rule (2): keep the bytes, change only how they are spelled.
            j = i
            run = []
            while j < n and ord(text[j]) >= 0x80 and ctx[j] == ctx[i]:
                run.append(text[j])
                j += 1
            out.append(escape_run(run))
            # A hex escape swallows a following hex digit; C string
            # concatenation is the standard guard and the tree already uses it.
            if j < n and text[j] in HEX and ctx[j] == ctx[i]:
                if not path.endswith((".cpp", ".c", ".hpp", ".h", ".cc", ".hh",
                                      ".cxx", ".hxx")):
                    raise SystemExit(
                        "%s:%d: a hex escape needs a string split, but adjacent "
                        "literals do not concatenate in this language - handle "
                        "by hand" % (path, text[:j].count("\n") + 1))
                out.append('" "')
            i = j
            continue
        # Rule (1) for the prose parts of a C-family file.
        hit = match_key(text, i, table, widths)
        if hit:
            out.append(hit[1])
            i += hit[0]
            continue
        refusals.append((path, i, ch))
        out.append(ch)
        i += 1
    return "".join(out)


def convert_blob(path, data, table, refusals):
    """bytes -> bytes for one file.  ASCII in, ASCII out, and the ONLY changes
    are the ones the two rules make."""
    new = convert_text(path, data, table, refusals)
    return new.encode("utf-8")


def targets(root, partition):
    part = A.read_partition(partition)
    out = []
    for p in A.tracked_files(root):
        d = A.decide(part, p)
        if not d or d[0] != "CONVERT":
            continue
        out.append(p)
    return out


def raw_string_check(path, text):
    """A raw string literal cannot hold an escape.  If one carries non-ASCII we
    must stop rather than corrupt it."""
    idx = 0
    while True:
        k = text.find('R"', idx)
        if k < 0:
            return None
        op = text.find("(", k + 2)
        if op < 0 or op - k > 20:
            idx = k + 2
            continue
        delim = text[k + 2:op]
        end = text.find(")" + delim + '"', op)
        if end < 0:
            idx = k + 2
            continue
        body = text[op:end]
        if any(ord(c) >= 0x80 for c in body):
            return text[:k].count("\n") + 1
        idx = end + 1


# ------------------------------------------------------------------- commands

def gather(root, partition, table, read):
    """read(path) -> bytes.  Returns (results, refusals)."""
    refusals = []
    results = []
    for p in targets(root, partition):
        data = read(p)
        if data is None or not any(b >= 0x80 for b in data):
            continue
        if b"\0" in data:
            continue
        text = A.decode(data)
        if A.family(p) == "c":
            ln = raw_string_check(p, text)
            if ln is not None:
                raise SystemExit("%s:%d: non-ASCII inside a raw string literal - "
                                 "escapes are impossible there, handle by hand"
                                 % (p, ln))
        new = convert_blob(p, data, table, refusals)
        if new != data:
            results.append((p, data, new))
    return results, refusals


def report(results, refusals):
    tot_before = 0
    tot_after = 0
    for p, old, new in results:
        b = sum(1 for x in old if x >= 0x80)
        a = sum(1 for x in new if x >= 0x80)
        tot_before += b
        tot_after += a
        print("%-70s %5d -> %d non-ASCII bytes%s"
              % (p, b, a, "" if a == 0 else "   *** NOT CLEAN ***"))
    print("---")
    print("%d file(s), %d non-ASCII bytes -> %d" % (len(results), tot_before,
                                                    tot_after))
    if refusals:
        print("REFUSALS - %d character(s) with no map row (the map must grow):"
              % len(refusals))
        seen = {}
        for where, _, ch in refusals:
            seen.setdefault(ch, []).append(where)
        for ch, ws in seen.items():
            print("  U+%04X in %s" % (ord(ch), ", ".join(sorted(set(ws))[:4])))
        return 1
    return 0


def cmd_map(args):
    table = read_map()
    for k, v in sorted(table.items(), key=lambda kv: (-len(kv[0]), kv[0])):
        print("%-40s -> %r" % (" ".join("U+%04X" % ord(c) for c in k), v))
    print("%d rows" % len(table))
    return 0


def cmd_plan(args):
    table = read_map()
    read = lambda p: open(os.path.join(args.root, p), "rb").read()
    results, refusals = gather(args.root, args.partition, table, read)
    return report(results, refusals)


def cmd_apply(args):
    table = read_map()
    read = lambda p: open(os.path.join(args.root, p), "rb").read()
    results, refusals = gather(args.root, args.partition, table, read)
    rc = report(results, refusals)
    if rc:
        print("nothing written - fix the refusals first")
        return rc
    for p, _, new in results:
        with open(os.path.join(args.root, p), "wb") as fh:
            fh.write(new)
    print("written: %d file(s)" % len(results))
    return 0


def cmd_verify(args):
    """The proof.  Re-derive every CONVERT file from REV's committed bytes and
    assert the working tree equals the derivation, byte for byte.

    What a PASS means: the tree differs from REV in exactly the characters the
    map and the escape rule touch, and in nothing else - not a space, not a line
    ending, not a line of content.  What it cannot mean: that a map row is the
    RIGHT replacement.  That is what the map's notes and the exclusion table are
    for, and they are reviewed by a person.
    """
    table = read_map()

    def read_at_rev(p):
        r = subprocess.run(["git", "-C", args.root, "show", "%s:%s" % (args.frm, p)],
                           capture_output=True)
        return r.stdout if r.returncode == 0 else None

    results, refusals = gather(args.root, args.partition, table, read_at_rev)
    if refusals:
        report(results, refusals)
        return 1
    bad = []
    checked = 0
    for p, old, want in results:
        got = open(os.path.join(args.root, p), "rb").read()
        checked += 1
        if got != want:
            bad.append(p)
    # A file the rules would NOT change must also be untouched.
    changed_set = {p for p, _, _ in results}
    stray = []
    for p in targets(args.root, args.partition):
        if p in changed_set:
            continue
        old = read_at_rev(p)
        if old is None:
            continue
        if open(os.path.join(args.root, p), "rb").read() != old:
            stray.append(p)
    if bad:
        print("FAIL: %d file(s) are not what the map derives from %s:"
              % (len(bad), args.frm))
        for p in bad:
            print("  %s" % p)
    if stray:
        print("FAIL: %d CONVERT file(s) changed although the rules change "
              "nothing in them (the sweep did more than it says):" % len(stray))
        for p in stray:
            print("  %s" % p)
    if bad or stray:
        return 1
    print("PASS: %d converted file(s) are EXACTLY the map+escape derivation of "
          "%s, and no other CONVERT file moved a byte." % (checked, args.frm))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["map", "plan", "apply", "verify"])
    ap.add_argument("--root", default=DEFAULT_ROOT)
    ap.add_argument("--partition", default=A.DEFAULT_PARTITION)
    ap.add_argument("--from", dest="frm", default="HEAD")
    args = ap.parse_args()
    return {"map": cmd_map, "plan": cmd_plan, "apply": cmd_apply,
            "verify": cmd_verify}[args.mode](args)


if __name__ == "__main__":
    sys.exit(main())
