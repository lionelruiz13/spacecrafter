#!/usr/bin/env python3
"""INTENT.md granularity split (2026-07-23).

Moves the expanded form of large ledger entries into INTENT/<id>.md files,
leaving a derived stub line (label + pointer) in INTENT.md.

Guarantees (verified, not assumed):
- Pure relocation: reconstructing INTENT.md by substituting each stub with its
  file's body must reproduce the original byte-for-byte (checked here).
- Nothing removed: every moved byte lives verbatim in exactly one INTENT/ file.

Scope of this pass: §5 (defects) and §11 (journal) — 494 of 671 KiB.
Entries <= THRESHOLD bytes stay inline (no expanded form worth a file).
"""
import re, sys, os, hashlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INTENT = os.path.join(ROOT, 'INTENT.md')
OUTDIR = os.path.join(ROOT, 'INTENT')
THRESHOLD = 500
LABEL_MAX = 300

orig = open(INTENT, encoding='utf-8').read()
lines = orig.split('\n')

# --- locate sections by '## N.' headers ---------------------------------
sec_start = {}
for i, l in enumerate(lines):
    m = re.match(r'^## (\d+)\.', l)
    if m:
        sec_start[int(m.group(1))] = i
sec_end = {n: min([s for k, s in sec_start.items() if s > sec_start[n]] + [len(lines)])
           for n in sec_start}

def parse_entries(sec):
    s, e = sec_start[sec], sec_end[sec]
    idx = [(m.group(1), i) for i in range(s, e)
           for m in [re.match(r'^(\d+[a-z]?)\. ', lines[i])] if m]
    idx.append((None, e))
    return [(f"{sec}.{n}", a, b) for (n, a), (_, b) in zip(idx, idx[1:])]

def make_label(first_line, num):
    t = first_line[len(num) + 2:]  # strip 'N. '
    if len(t) <= LABEL_MAX:
        return t, False
    closes = [m.end() for m in re.finditer(r'\*\*', t)][1::2]
    cands = [c for c in closes if c <= LABEL_MAX]
    cut = max(cands) if cands else 200
    # parity fix for ` and ~~ so the stub renders sanely
    for tok in ('`', '~~'):
        while t[:cut].count(tok) % 2 == 1 and cut < len(t):
            nxt = t.find(tok, cut)
            if nxt == -1:
                break
            cut = nxt + len(tok)
    return t[:cut].rstrip(), True

os.makedirs(OUTDIR, exist_ok=True)
if os.listdir(OUTDIR):
    sys.exit("INTENT/ not empty — refusing (idempotency guard)")

moved = []   # (id, start, end, tail_blanks, label_line)
for sec in (5, 11):
    for eid, a, b in parse_entries(sec):
        block = lines[a:b]
        size = sum(len(l) + 1 for l in block)
        if size <= THRESHOLD:
            continue
        tail = 0
        while block and block[-1] == '':
            block.pop(); tail += 1
        num = eid.split('.', 1)[1]
        label, truncated = make_label(block[0], num)
        marker = ' […]' if truncated else ''
        stub = f"{num}. {label}{marker} → INTENT/{eid}.md"
        body = '\n'.join(block) + '\n'
        with open(os.path.join(OUTDIR, f"{eid}.md"), 'w', encoding='utf-8') as f:
            f.write(f"# INTENT §{eid}\n\n{body}")
        moved.append((eid, a, b, tail, stub))

# --- rewrite INTENT.md ---------------------------------------------------
new_lines = []
consumed = {}
for eid, a, b, tail, stub in moved:
    consumed[a] = (eid, b, tail, stub)
i = 0
while i < len(lines):
    if i in consumed:
        eid, b, tail, stub = consumed[i]
        new_lines.append(stub)
        new_lines.extend([''] * tail)
        i = b
    else:
        new_lines.append(lines[i])
        i += 1
new_text = '\n'.join(new_lines)

# --- verification: byte-exact reconstruction -----------------------------
rec_lines = []
stub_map = {s: (eid, tail) for eid, _, _, tail, s in moved}
j = 0
while j < len(new_lines):
    l = new_lines[j]
    if l in stub_map:
        eid, tail = stub_map[l]
        fbody = open(os.path.join(OUTDIR, f"{eid}.md"), encoding='utf-8').read()
        head = f"# INTENT §{eid}\n\n"
        assert fbody.startswith(head), eid
        rec_lines.extend(fbody[len(head):].rstrip('\n').split('\n'))
        j += 1 + tail  # skip stub + reinserted tail blanks (body carries none)
        rec_lines.extend([''] * tail)
    else:
        rec_lines.append(l)
        j += 1
rec = '\n'.join(rec_lines)
if rec != orig:
    # locate first divergence for diagnosis
    for k, (x, y) in enumerate(zip(rec.split('\n'), orig.split('\n'))):
        if x != y:
            sys.exit(f"RECONSTRUCTION DIVERGES at line {k+1}:\nrec: {x[:120]}\norig:{y[:120]}")
    sys.exit(f"RECONSTRUCTION LENGTH MISMATCH rec={len(rec)} orig={len(orig)}")

open(INTENT, 'w', encoding='utf-8').write(new_text)
print(f"orig md5 {hashlib.md5(orig.encode()).hexdigest()}  reconstruction: EXACT")
print(f"moved {len(moved)} entries; INTENT.md {len(orig)} -> {len(new_text)} bytes "
      f"({len(new_text)/len(orig)*100:.0f}%)")
