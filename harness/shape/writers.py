#!/usr/bin/env python3
# Duplication in the owner's sense (S11.249 update): two functions doing the same
# thing, one possibly a parametric subset of the other. Text does not show it; the
# WRITER SET does: for one class, which member functions write which field.
#   usage: writers.py <rev> <Class> <header path> <source path>
# Heuristic, no compiler: regex on comment-stripped text. Blind to writes through
# a non-member call, through a reference/pointer alias, and to fields written only
# by another class. Positive control: at 8e065ce4, Camera `target` must list both
# trackBody and rebindTarget.
import re, subprocess, sys, collections

rev, cls, hdr, src = sys.argv[1:5]

def show(path):
    t = subprocess.run(['git', 'show', f'{rev}:{path}'], capture_output=True).stdout.decode('utf-8', 'replace')
    t = re.sub(r'/\*.*?\*/', ' ', t, flags=re.S)
    t = re.sub(r'//[^\n]*', '', t)
    return re.sub(r'"(\\.|[^"\\])*"', '""', t)

def block(t, i):  # t[i] == '{' -> index just past its matching '}'
    d = 0
    for j in range(i, len(t)):
        d += (t[j] == '{') - (t[j] == '}')
        if d == 0:
            return j + 1
    return len(t)

def params(sig):
    return {m.group(1) for p in sig.split(',') for m in [re.search(r'(\w+)\s*(?:=.*)?$', p.strip())] if m}

H, S = show(hdr), show(src)
m = re.search(r'\bclass\s+' + cls + r'\b[^;{]*\{', H)
body = H[m.end():block(H, m.end() - 1) - 1]

funcs, fields, i, stmt = [], [], 0, ''   # funcs: (name, params, body)
while i < len(body):
    c = body[i]
    if c == '{':
        j = block(body, i)
        head = stmt.strip()
        f = re.search(r'(\w+)\s*\(([^()]*)\)\s*(?:const)?\s*(?:override)?\s*$', head)
        if f and not re.match(r'(class|struct|enum|union)\b', head):
            funcs.append((f.group(1), params(f.group(2)), body[i:j]))
            stmt = ''
        elif re.match(r'(class|struct|enum|union)\b', head):
            stmt = 'NESTED '      # `} name;` after a nested type declares a field
        else:
            stmt = head + ' '     # brace initializer: the declaration continues to its ';'
        i = j
        continue
    if c == ';':
        s = re.sub(r'^(public|private|protected)\s*:', '', stmt.strip()).strip()
        s = re.sub(r'^NESTED\s*', '', s)
        if s and '(' not in s and not re.match(r'(using|friend|typedef|enum|class|struct)\b', s):
            for d in s.split(','):
                n = re.search(r'(\w+)\s*(?:\[[^\]]*\])?\s*(?:=.*|\{.*)?$', d.strip())
                if n:
                    fields.append(n.group(1))
        stmt = ''
    elif c == ':' and re.search(r'\b(public|private|protected)\s*$', stmt):
        stmt = ''
    else:
        stmt += c
    i += 1

for f in re.finditer(r'\b' + cls + r'::(~?\w+)\s*\(', S):
    d, j = 1, f.end()
    while d:
        d += (S[j] == '(') - (S[j] == ')'); j += 1
    k = S.find('{', j)
    if k < 0 or ';' in S[j:k] or f.group(1) in (cls, '~' + cls):
        continue
    funcs.append((f.group(1), params(S[f.end():j - 1]), S[k:block(S, k)]))

names = {n for n, _, _ in funcs}
W = r'(?:\s*(?:\.\s*\w+|\[[^\]]*\]))*\s*(?:=(?!=)|[-+*/|&^]=|<<=|>>=|\+\+|--)'
direct, calls = collections.defaultdict(set), collections.defaultdict(set)
for n, ps, b in funcs:
    b = b.replace('this->', '\x01')
    for fl in fields:
        local = any(d.group(1) not in ('else', 'return', 'case', 'do')   # a local of the same name shadows the field
                    for d in re.finditer(r'(\w+|>)\s+[*&]?' + fl + r'\s*(?:=(?!=)|;|\{)', b))
        own = r'\x01' if (fl in ps or local) else r'(?:\x01|(?<![\w.>\x01]))'
        if re.search(own + fl + r'\b' + W, b) or re.search(r'(?:\+\+|--)\s*\x01?' + fl + r'\b', b):
            direct[n].add(fl)
    for o in names - {n}:
        if re.search(r'(?:\x01|(?<![\w.>:]))' + o + r'\s*\(', b):
            calls[n].add(o)

eff = {n: set(direct[n]) for n in names}
changed = True
while changed:
    changed = False
    for n in names:
        for o in calls[n]:
            if not eff[o] <= eff[n]:
                eff[n] |= eff[o]; changed = True

print(f'{cls} @ {rev}: {len(fields)} fields, {len(names)} member functions (overloads merged by name)')
print('-- managers per field: functions that write it DIRECTLY --')
by = collections.defaultdict(list)
for n in sorted(names):
    for fl in direct[n]:
        by[fl].append(n)
for fl, ws in sorted(by.items(), key=lambda kv: (-len(kv[1]), kv[0])):
    print(f'  {len(ws):2d}  {fl:22s} {" ".join(ws)}')
print('-- same direct writer set (>= 2 functions) --')
groups = collections.defaultdict(list)
for n in sorted(names):
    if direct[n]:
        groups[frozenset(direct[n])].append(n)
for k, g in sorted(groups.items(), key=lambda kv: (len(kv[0]), sorted(kv[0]))):
    if len(g) > 1:
        print(f'  {{{", ".join(sorted(k))}}}: {" ".join(g)}')
print('-- forwarders: one statement, its effect is entirely another member\'s --')
for n, ps, b in sorted(funcs, key=lambda f: f[0]):
    if b.count(';') == 1 and not direct[n] and calls[n]:
        print(f'  {n} -> {" ".join(sorted(calls[n]))}')
