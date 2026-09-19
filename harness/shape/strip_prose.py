#!/usr/bin/env python3
# Mechanical half of the comment pass (INTENT S2.1 G12 (S6)): delete every full-line
# comment BLOCK of >= MIN lines that an AGENT wrote: every line blamed to a 'Claude*' author
# AND absent from the file at BASE (blame names the last toucher, not the writer). A block
# holding one human line, a licence, or a tool directive is left whole. Blank lines go only where a deleted block leaves a hole.
# Files must be unmodified against <rev> (blame lines are matched by index).
#   usage (code repo root): strip_prose.py <rev> <file ...>      MIN=3 by default (env)
# What it deletes stays in git at <rev>; codeident.py proves no code token moved.
import os, re, subprocess, sys, unicodedata

MIN = int(os.environ.get('MIN', '3'))

BASE = os.environ.get('BASE', '4dfe7bb3')      # the owner's last state before any agent commit

def skel(line):                                 # accent- and spacing-insensitive identity of a comment line
    return re.sub(r'[^A-Za-z0-9]', '', unicodedata.normalize('NFKD', line).encode('ascii', 'ignore').decode())

def legacy_skeletons(path):                     # comment text that pre-dates the agents, whoever touched it since
    raw = subprocess.run(['git', 'show', f'{BASE}:{path}'], capture_output=True).stdout
    out = set()
    for l in raw.split(b'\n'):
        try:
            s = l.decode('utf-8')
        except UnicodeDecodeError:
            s = l.decode('latin-1')
        if len(skel(s)) >= 8:
            out.add(skel(s))
    return out

def agent_wrote(author, line, legacy):          # blame names the LAST TOUCHER: an agent's re-encoding pass is not authorship
    return (author or '').startswith('Claude') and not (len(skel(line)) >= 8 and skel(line) in legacy)
KEEP = re.compile(r'copyright|licen[cs]e|fall.?through|NOLINT|clang-format|IWYU|TODO|FIXME', re.I)
rev = sys.argv[1]
total = 0
for p in sys.argv[2:]:
    bl = subprocess.run(['git', 'blame', '--line-porcelain', rev, '--', p], capture_output=True).stdout.decode('utf-8', 'surrogateescape')
    authors, a = [], None
    for l in bl.split('\n'):
        if l.startswith('author '):
            a = l[7:]
        elif l.startswith('\t'):
            authors.append(a)
    lines = open(p, encoding='utf-8', errors='surrogateescape', newline='').read().split('\n')   # newline='': CRLF files stay CRLF
    if len(lines) - 1 != len(authors) and len(lines) != len(authors):
        print(f'SKIP {p}: modified against {rev} ({len(lines)} lines vs {len(authors)} blamed)'); continue
    legacy = legacy_skeletons(p)
    is_c, inblk = [], False
    for l in lines:
        s = l.strip()
        c = inblk or s.startswith('//') or s.startswith('/*')
        if inblk:
            inblk = '*/' not in s
        elif s.startswith('/*') and '*/' not in s:
            inblk = True
        elif s.startswith('/*') and not s.endswith('*/'):
            c = False                      # `/* x */ code` is a code line
        is_c.append(c)
    drop, i = set(), 0
    while i < len(lines):
        if not is_c[i]:
            i += 1; continue
        j = i
        while j < len(lines) and is_c[j]:
            j += 1
        blk = range(i, j)
        if (j - i >= MIN and all(k < len(authors) and agent_wrote(authors[k], lines[k], legacy) for k in blk)
                and not any(KEEP.search(lines[k]) for k in blk)):
            drop.update(blk)
        i = j
    out, after_drop = [], False     # spacing is touched ONLY where a deleted block leaves a hole
    for k, l in enumerate(lines):
        if k in drop:
            after_drop = True
            continue
        if after_drop:
            if l.strip() == '' and out and (out[-1].strip() == '' or out[-1].rstrip().endswith('{')):
                continue
            if l.strip().startswith('}') and out and out[-1].strip() == '':
                out.pop()
            after_drop = False
        out.append(l)
    open(p, 'w', encoding='utf-8', errors='surrogateescape', newline='').write('\n'.join(out))
    total += len(lines) - len(out)
    print(f'{p}: {len(lines)} -> {len(out)} lines ({len(drop)} comment lines in blocks >= {MIN})')
print(f'total lines removed: {total}')
