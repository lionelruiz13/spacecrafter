#!/usr/bin/env python3
# F126: render query_table.tsv as the markdown fragments of claude/b42-wiring-table.md
import collections
A = '/home/claude/spacecrafter/claude/harness/artifacts/f126/'
rows = [l.rstrip('\n').split('\t') for l in open(A + 'query_table.tsv')][1:]
ANS = {'C': 'Camera', 'M': 'module', 'P': 'partly', 'N': 'NONE', '-': ''}
# family table (queries only)
fam = collections.OrderedDict()
for c, m, kind, info, a, note, tot, T, G, W, X, S in rows:
    if kind != 'Q': continue
    f = info.split(' (')[0]
    r = fam.setdefault(f, [0, 0, 0, collections.Counter(), []])
    r[0] += 1; r[1] += int(tot); r[2] += int(tot) - int(T); r[3][a] += 1; r[4].append(m)
with open(A + 'frag_family.md', 'w') as o:
    o.write('| information | methods | calls | of which shared | Camera / module / partly / NONE | the methods |\n|---|---|---|---|---|---|\n')
    for f, r in sorted(fam.items(), key=lambda kv: -kv[1][2]):
        a = r[3]
        o.write(f"| {f} | {r[0]} | {r[1]} | {r[2]} | {a['C']} / {a['M']} / {a['P']} / {a['N']} | {', '.join('`'+m+'`' for m in r[4])} |\n")
    t = [sum(r[i] for r in fam.values()) for i in (0, 1, 2)]
    a = sum((r[3] for r in fam.values()), collections.Counter())
    o.write(f"| **total** | **{t[0]}** | **{t[1]}** | **{t[2]}** | **{a['C']} / {a['M']} / {a['P']} / {a['N']}** | |\n")
# full table
KIND = {'Q': 'query', 'R': 'request', 'A': 'frame step', 'I': 'instrument', 'F': 'not a method'}
with open(A + 'frag_full.md', 'w') as o:
    for cls in ('Projector', 'Navigator', 'Observer'):
        sub = [r for r in rows if r[0] == cls]
        o.write(f"\n**{cls}** - {len(sub)} names, {sum(int(r[6]) for r in sub)} calls\n\n")
        o.write('| method | kind | calls | old path proper | shared | information | answered today by / Camera counterpart |\n|---|---|---|---|---|---|---|\n')
        for c, m, kind, info, a, note, tot, T, G, W, X, S in sorted(sub, key=lambda r: ('QRAIF'.index(r[2]), -int(r[6]), r[1])):
            sh = int(tot) - int(T)
            ans = (ANS[a] + (': ' if ANS[a] and note else '') + note) if kind == 'Q' else note
            o.write(f"| `{m}` | {KIND[kind]} | {tot} | {T} | {sh} | {info} | {ans} |\n")
print(open(A + 'frag_family.md').read())
