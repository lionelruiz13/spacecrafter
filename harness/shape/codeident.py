#!/usr/bin/env python3
# Proof obligations of a COMMENT-ONLY pass (INTENT S2.1 G12 (S6)), per file changed
# between <rev> and the working tree of the code repo:
#   1. the code TOKEN stream is identical (comments out, whitespace ignored outside
#      literals), and the preprocessor lines are identical in order;
#   2. every comment line an agent did NOT write is still there, in order: a line blamed to a
#      human, or present in the file at BASE whoever touched it since (re-encoding is not authorship).
#   usage (from the code repo root): codeident.py <rev> [path ...]    exit 1 on any FAIL
import os, re, subprocess, sys, unicodedata

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
rev, paths = sys.argv[1], sys.argv[2:]
if not paths:
    paths = subprocess.run(['git', 'diff', '--name-only', rev, '--'], capture_output=True, text=True).stdout.split()
paths = [p for p in paths if re.search(r'\.(hpp|cpp|h|c|glsl|frag|vert|comp|geom|tesc|tese)$', p)]

TOK = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|[A-Za-z_]\w*|\d[\w.]*|\S')

def decomment(text):   # string/char literals kept verbatim, comments -> one space
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1]); i = j + 1
        elif text.startswith('//', i):
            j = text.find('\n', i); j = n if j < 0 else j
            while text[j - 1] == '\\' and j < n:          # line-continued // comment
                j = text.find('\n', j + 1); j = n if j < 0 else j
            out.append(' '); i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2); j = n if j < 0 else j + 2
            out.append(' '); i = j
        else:
            out.append(c); i += 1
    return ''.join(out)

def is_comment(line):
    s = line.strip()
    return s.startswith('//') or s.startswith('/*') or s.startswith('*')

bad = 0
for p in paths:
    old = subprocess.run(['git', 'show', f'{rev}:{p}'], capture_output=True).stdout.decode('utf-8', 'replace')
    try:
        new = open(p, encoding='utf-8', errors='replace').read()
    except FileNotFoundError:
        print(f'FAIL {p}: deleted'); bad += 1; continue
    a, b = decomment(old), decomment(new)
    ta, tb = TOK.findall(a), TOK.findall(b)
    da = [re.sub(r'\s+', ' ', l.strip()) for l in a.split('\n') if l.lstrip().startswith('#')]
    db = [re.sub(r'\s+', ' ', l.strip()) for l in b.split('\n') if l.lstrip().startswith('#')]
    msgs = []
    if ta != tb:
        k = next((i for i, (x, y) in enumerate(zip(ta, tb)) if x != y), min(len(ta), len(tb)))
        msgs.append(f'code tokens differ at #{k}: ...{" ".join(ta[max(0,k-4):k+4])} | ...{" ".join(tb[max(0,k-4):k+4])}')
    if da != db:
        msgs.append('preprocessor lines differ')
    blame = subprocess.run(['git', 'blame', '--line-porcelain', rev, '--', p], capture_output=True).stdout.decode('utf-8', 'replace')
    legacy = legacy_skeletons(p)
    owner_cmt, author = [], None
    for l in blame.split('\n'):
        if l.startswith('author '):
            author = l[7:]
        elif l.startswith('\t'):
            if not agent_wrote(author, l[1:], legacy) and (is_comment(l[1:]) or decomment(l[1:]).rstrip() != l[1:].rstrip()):
                owner_cmt.append(l[1:].strip())      # his comment lines AND his trailing comments
    newl, pos, lost = [x.strip() for x in new.split('\n')], 0, []
    for c in owner_cmt:
        try:
            pos = newl.index(c, pos) + 1
        except ValueError:
            lost.append(c)
    if lost:
        msgs.append(f'{len(lost)} human/legacy comment line(s) lost or reordered, first: {lost[0][:80]}')
    oc = sum(is_comment(l) for l in old.split('\n')); nc = sum(is_comment(l) for l in new.split('\n'))
    print(f'{"FAIL" if msgs else "PASS"} {p}: lines {old.count(chr(10))} -> {new.count(chr(10))}, comment lines {oc} -> {nc}, human/legacy comment lines kept {len(owner_cmt) - len(lost)}/{len(owner_cmt)}')
    for m in msgs:
        print('     ' + m)
    bad += bool(msgs)
print(f'{len(paths)} file(s), {bad} FAIL')
sys.exit(1 if bad else 0)
