#!/usr/bin/env python3
# F126: merge the census matrix (method_matrix.tsv) with the hand annotations (method_annotations.tsv);
# FAIL if a census method is unannotated or an annotation names no census method. Prints the totals.
import collections, sys
A = '/home/claude/spacecrafter/claude/harness/artifacts/f126/'
ann = {}
for l in open(A + 'method_annotations.tsv'):
    if l.startswith('#') or not l.strip(): continue
    p = l.rstrip('\n').split('\t'); p += [''] * (6 - len(p))
    assert (p[0], p[1]) not in ann, ('dup', p[:2])
    ann[(p[0], p[1])] = p[2:6]
mat = {}
for i, l in enumerate(open(A + 'method_matrix.tsv')):
    if i == 0: continue
    p = l.rstrip('\n').split('\t'); mat[(p[0], p[1])] = p[2:]
missing, extra = set(mat) - set(ann), set(ann) - set(mat)
print('annotated', len(ann), '| census', len(mat), '| missing', sorted(missing), '| extra', sorted(extra))
if missing or extra: sys.exit('FAIL')
kinds, calls, ans, anscalls, ans_sh = (collections.Counter() for _ in range(5))
shared_methods = collections.Counter(); shared_calls = collections.Counter()
for k, (kind, info, a, note) in ann.items():
    tot, T, G, W, X, S = map(int, mat[k][:6])
    kinds[kind] += 1; calls[kind] += tot
    if tot - T > 0: shared_methods[kind] += 1; shared_calls[kind] += tot - T
    if kind == 'Q':
        ans[a] += 1; anscalls[a] += tot; ans_sh[a] += tot - T
print('methods by kind', dict(kinds), 'sum', sum(kinds.values()))
print('calls by kind', dict(calls), 'sum', sum(calls.values()))
print('methods with >= 1 non-T caller, by kind', dict(shared_methods), 'sum', sum(shared_methods.values()))
print('non-T calls by kind', dict(shared_calls), 'sum', sum(shared_calls.values()))
print('QUERY methods by answer', dict(ans), 'sum', sum(ans.values()))
print('QUERY calls by answer', dict(anscalls), 'sum', sum(anscalls.values()))
print('QUERY calls by answer, non-T callers', dict(ans_sh), 'sum', sum(ans_sh.values()))
info_q = collections.defaultdict(lambda: [0, 0, 0, collections.Counter()])
for k, (kind, info, a, note) in ann.items():
    if kind != 'Q': continue
    tot, T = int(mat[k][0]), int(mat[k][1])
    fam = info.split(' (')[0]
    r = info_q[fam]; r[0] += 1; r[1] += tot; r[2] += tot - T; r[3][a] += 1
print('QUERY families: family | methods | calls | non-T calls | answers')
for fam, r in sorted(info_q.items(), key=lambda kv: -kv[1][1]):
    print(f'  {fam:28s} {r[0]:3d} {r[1]:4d} {r[2]:4d}  {dict(r[3])}')
with open(A + 'query_table.tsv', 'w') as o:
    o.write('class\tmethod\tkind\tinformation\tanswer\tmember_or_note\ttotal\tT\tG\tW\tX\tS\n')
    for k in sorted(ann, key=lambda k: (k[0], ann[k][0], -int(mat[k][0]), k[1])):
        o.write('\t'.join(list(k) + ann[k] + mat[k][:6]) + '\n')
