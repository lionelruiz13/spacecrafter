#!/usr/bin/env python3
"""F60 -- enumeration of the §5 -> §13 (and §13 -> §5) reference axis.
THE BOUNDARY, committed BEFORE any verdict (the F58/F59 form).

WHAT THIS AXIS IS.  §11.163(c) named it and §11.163(k)(1) left it unswept:

    "the §5 row is stale against the §13 row that owns its work, while the
     stub<->entry pair is internally consistent -- which is precisely why F42's
     pair sweep could not reach it (both homes carry the same stale text).
     The class F42 closed was stub-vs-entry; this one is row-vs-row, and it is
     not swept by anything today."

So the unit of this sweep is a REFERENCE: a §5 row's text names a §13 row, and the
question is whether that §13 row's CURRENT state still supports what the §5 text
assumes about it.  Neither existing instrument reaches it: `intent_pair_check.py`
compares a stub against its own entry file (one id, two homes); the frozen
`intent_backmarker_scan.py` walks supersession events between §-ids and only ever
resolves §N.M citations -- a bare `B32` or `A33` token is invisible to both.

--------------------------------------------------------------------------------------
SOURCE UNIVERSE (column `srcclass`), stated so every exclusion is challengeable:

  LIVE-STUB     the 95 numbered rows of the §5 register in INTENT.md (the live rows)
  LIVE-ENTRY    INTENT/5.N.md where N HAS a live register row (10 files)
  INPLACE-ENTRY INTENT/5.N.md where N has NO live register row (6 files: 4,16,17,22,
                23,24 -- "archived in place", the index line retired 2026-07-31 while
                the ENTRY stayed live under INTENT/).  IN SCOPE: the file is live text
                and §5.24 is still marked OPEN in it.
  ARCHIVE-ENTRY INTENT/archive/5.N.md (18 files).  ENUMERATED AND REPORTED, NOT
                ADJUDICATED and never annotated: an archived entry is retired history,
                and the archival convention's own rule is that references into it are
                never rewritten (fable-dispatch.md archival note; F59's ruling that the
                23 retired numbers live only at archive).  Counted so the exclusion is
                sized rather than assumed empty.

TARGET UNIVERSE: every §13 row id ever allocated, resolved to its CURRENT home ---
  LIVE-A        a live row of §13.A (18)
  LIVE-B        a live row of §13.B (25)
  ARCHIVED      INTENT/archive/<id>.md exists (15 B-rows)
  CLOSED-PTR    named only in §13.C's closed lists (pointer-only, no body)
  UNKNOWN       an [AB]\\d+ token matching no allocated row id  -> a detector miss or
                a foreign namespace; reported, never silently dropped.

DETECTOR (forward): (?<![A-Za-z0-9_])([AB]\\d{1,2})(?![0-9]) over the source text.
  The negative lookbehind is what keeps `RGBA8`, `BMT_RGBA8_SELF_SHADOW` and `0xA0`
  out.  The detector OVER-generates by construction and the over-generation is a
  MEASURED namespace collision, not a hypothetical:

    * §11.73's B27 hardcode-key design labels its keys A1..A9 / Tier-A / Tier-B.
      §5.5's entry file carries nine of them.  They are NOT §13.A row ids.
    * §11.163's own verdict table numbers its members 1..13.

  Therefore NO verdict in the delivered table is produced by this script.  Every
  enumerated reference is adjudicated by READING both texts; the script's job is to
  make the universe reproducible and the boundary explicit.

DETECTOR (backward): §5\\.(\\d+) over the live §13 row lines -- a §13 row claiming
  ownership of a §5 fix.  Same adjudication rule.

KNOWN AND ACCEPTED INCOMPLETENESS, stated so the bound is honest:
  - a reference made in prose without the id token ("the ledger row that owns the
    composition grammar") is invisible;
  - a reference to a §12 row (S4/S6/S7/S8) or a §6 decision (D9/D15/D21/D23) is OUT
    of the stated axis (§5 -> §13) -- enumerated separately by `--adjacent` so the
    adjacent axis is sized, never verdicted here;
  - the target-state resolver reads the row's CURRENT text; a row whose own text is
    stale would propagate that staleness (the axis this sweep exists to check, one
    level up -- named, not solved).
"""
import os, re, sys, json
from collections import Counter, defaultdict

ROOT = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("-") \
       else "/home/claude/spacecrafter/claude"
ADJACENT = "--adjacent" in sys.argv
SELFTEST = "--selftest" in sys.argv

REF = re.compile(r'(?<![A-Za-z0-9_])([AB]\d{1,2})(?![0-9])')
BACKREF = re.compile(r'§5\.(\d+)')
CTX = 200


def load_intent():
    return open(os.path.join(ROOT, 'INTENT.md'), encoding='utf-8').read().split('\n')


def sections(lines):
    idx = {}
    for i, l in enumerate(lines):
        if l.startswith('## 5. Defects'):      idx['s5'] = i
        elif l.startswith('## 6. '):           idx['s6'] = i
        elif l.startswith('### 13.A'):         idx['13A'] = i
        elif l.startswith('### 13.B'):         idx['13B'] = i
        elif l.startswith('### 13.C'):         idx['13C'] = i
        elif l.startswith('### 13.D'):         idx['13D'] = i
    return idx


def live_rows_5(lines, idx):
    out = {}
    for i in range(idx['s5'], idx['s6']):
        m = re.match(r'^(\d+)\. \*\*', lines[i])
        if m:
            out[int(m.group(1))] = (i + 1, lines[i])
    return out


def rows_13(lines, idx):
    """id -> (kind, lineno, text)"""
    out = {}
    for kind, a, b in (('LIVE-A', idx['13A'], idx['13B']),
                       ('LIVE-B', idx['13B'], idx['13C'])):
        for i in range(a, b):
            m = re.match(r'^\|\s*~*([AB]\d+)~*\s*\|', lines[i])
            if m:
                out[m.group(1)] = (kind, i + 1, lines[i])
    # archived rows: a body file exists
    adir = os.path.join(ROOT, 'INTENT', 'archive')
    for f in sorted(os.listdir(adir)):
        m = re.match(r'^([AB]\d+)\.md$', f)
        if m and m.group(1) not in out:
            out[m.group(1)] = ('ARCHIVED', 0,
                               open(os.path.join(adir, f), encoding='utf-8').read()[:400])
    # closed pointers named in 13.C
    cseg = '\n'.join(lines[idx['13C']:idx['13D']])
    for tid in sorted(set(REF.findall(cseg))):
        if tid not in out:
            out[tid] = ('CLOSED-PTR', idx['13C'] + 1, '(pointer-only in §13.C)')
    return out


def sources(lines, idx):
    """list of (srcclass, srcid, home, text)"""
    out = []
    live5 = live_rows_5(lines, idx)
    for n, (ln, txt) in sorted(live5.items()):
        out.append(('LIVE-STUB', f'5.{n}', f'INTENT.md:{ln}', txt))
    edir = os.path.join(ROOT, 'INTENT')
    for f in sorted(os.listdir(edir)):
        m = re.match(r'^5\.(\d+)\.md$', f)
        if not m:
            continue
        n = int(m.group(1))
        cls = 'LIVE-ENTRY' if n in live5 else 'INPLACE-ENTRY'
        out.append((cls, f'5.{n}', f'INTENT/{f}',
                    open(os.path.join(edir, f), encoding='utf-8').read()))
    adir = os.path.join(edir, 'archive')
    for f in sorted(os.listdir(adir)):
        m = re.match(r'^5\.(\d+)\.md$', f)
        if not m:
            continue
        out.append(('ARCHIVE-ENTRY', f'5.{m.group(1)}', f'INTENT/archive/{f}',
                    open(os.path.join(adir, f), encoding='utf-8').read()))
    return out


def enumerate_forward(lines, idx):
    r13 = rows_13(lines, idx)
    recs = []
    for cls, sid, home, txt in sources(lines, idx):
        flat = txt.replace('\n', ' ')
        for m in REF.finditer(flat):
            tid = m.group(1)
            kind = r13[tid][0] if tid in r13 else 'UNKNOWN'
            a, b = max(0, m.start() - CTX), min(len(flat), m.end() + CTX)
            recs.append(dict(srcclass=cls, src=sid, home=home, target=tid,
                             tgtclass=kind, pos=m.start(),
                             context=flat[a:b]))
    return recs, r13


def enumerate_backward(lines, idx):
    recs = []
    for kind, a, b in (('LIVE-A', idx['13A'], idx['13B']),
                       ('LIVE-B', idx['13B'], idx['13C'])):
        for i in range(a, b):
            m = re.match(r'^\|\s*~*([AB]\d+)~*\s*\|', lines[i])
            if not m:
                continue
            for mm in BACKREF.finditer(lines[i]):
                s, e = max(0, mm.start() - CTX), min(len(lines[i]), mm.end() + CTX)
                recs.append(dict(src=m.group(1), srcclass=kind,
                                 home=f'INTENT.md:{i+1}', target=f'5.{mm.group(1)}',
                                 pos=mm.start(), context=lines[i][s:e]))
    return recs


def adjacent(lines, idx):
    """§12 S-rows and §6 D-decisions cited from §5 texts -- the adjacent axis, SIZED."""
    pat = re.compile(r'(?<![A-Za-z0-9_])(S\d{1,2}|D\d{1,2})(?![0-9])')
    out = Counter()
    for cls, sid, home, txt in sources(lines, idx):
        if cls == 'ARCHIVE-ENTRY':
            continue
        for m in pat.finditer(txt.replace('\n', ' ')):
            out[(sid, m.group(1))] += 1
    return out


def selftest(lines, idx):
    """Both-ways mapping of the detector and the resolver, BEFORE any verdict."""
    ok = True
    r13 = rows_13(lines, idx)

    # P1 positive: a synthetic row line carrying a real id is found
    got = REF.findall("5. **x** -- fix routed to B28, rides B16's seam")
    print(f"P1 positive detect            : {got}  (expect ['B28','B16'])")
    ok &= got == ['B28', 'B16']

    # P2 negative: id-looking substrings inside words/hex are refused
    got = REF.findall("BMT_RGBA8_SELF_SHADOW, 0xA0, sub-B, SCK_A1X")
    print(f"P2 in-word / hex refused      : {got}  (expect [])")
    ok &= got == []

    # P3: an unallocated id resolves UNKNOWN, not silently dropped
    print(f"P3 unallocated id B99         : {'B99' in r13} (expect False -> UNKNOWN)")
    ok &= 'B99' not in r13

    # P4: the resolver separates a live row from an archived one, both directions
    print(f"P4a B27 (live)                : {r13.get('B27',('?',))[0]} (expect LIVE-B)")
    print(f"P4b B32 (archived)            : {r13.get('B32',('?',))[0]} (expect ARCHIVED)")
    print(f"P4c A33 (closed pointer)      : {r13.get('A33',('?',))[0]} (expect CLOSED-PTR)")
    ok &= r13.get('B27', ('?',))[0] == 'LIVE-B'
    ok &= r13.get('B32', ('?',))[0] == 'ARCHIVED'
    ok &= r13.get('A33', ('?',))[0] == 'CLOSED-PTR'

    # P5: the known namespace collision IS produced (so it must be adjudicated by
    #     reading, not by the machine) -- a positive control on the over-generation
    recs, _ = enumerate_forward(lines, idx)
    tierA = [r for r in recs if r['src'] == '5.5' and r['target'] in
             ('A1', 'A2', 'A3', 'A5', 'A6', 'A7', 'A8', 'A9')]
    print(f"P5 §5.5 Tier-A key collision  : {len(tierA)} hits (expect >0, all NOT-A-ROW)")
    ok &= len(tierA) > 0

    print("SELFTEST:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def main():
    lines = load_intent()
    idx = sections(lines)
    if SELFTEST:
        sys.exit(selftest(lines, idx))
    if ADJACENT:
        for (sid, rid), c in sorted(adjacent(lines, idx).items()):
            print(f"{sid}\t{rid}\t{c}")
        return
    fwd, r13 = enumerate_forward(lines, idx)
    bwd = enumerate_backward(lines, idx)
    outdir = os.path.join(ROOT, 'harness', 'artifacts', 'f60')
    with open(os.path.join(outdir, 'f60_refs_forward.tsv'), 'w', encoding='utf-8') as f:
        f.write("srcclass\tsrc\thome\ttarget\ttgtclass\tpos\tcontext\n")
        for r in fwd:
            f.write("\t".join(str(r[k]) for k in
                    ('srcclass', 'src', 'home', 'target', 'tgtclass', 'pos', 'context')) + "\n")
    with open(os.path.join(outdir, 'f60_refs_backward.tsv'), 'w', encoding='utf-8') as f:
        f.write("srcclass\tsrc\thome\ttarget\tpos\tcontext\n")
        for r in bwd:
            f.write("\t".join(str(r[k]) for k in
                    ('srcclass', 'src', 'home', 'target', 'pos', 'context')) + "\n")
    with open(os.path.join(outdir, 'f60_targets.tsv'), 'w', encoding='utf-8') as f:
        f.write("id\tkind\tline\thead\n")
        for tid, (kind, ln, txt) in sorted(r13.items(),
                                           key=lambda kv: (kv[0][0], int(kv[0][1:]))):
            f.write(f"{tid}\t{kind}\t{ln}\t{txt[:180].replace(chr(10),' ')}\n")
    c = Counter((r['srcclass'], r['tgtclass']) for r in fwd)
    print("FORWARD references:", len(fwd))
    for k in sorted(c):
        print(f"  {k[0]:14s} -> {k[1]:12s} {c[k]}")
    print("distinct (src,target) pairs:",
          len({(r['src'], r['target']) for r in fwd}))
    print("BACKWARD references:", len(bwd),
          "distinct:", len({(r['src'], r['target']) for r in bwd}))


if __name__ == '__main__':
    main()
