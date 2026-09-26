#!/usr/bin/env python3
"""Supersession probe: for each non-merge commit C, blame the lines C REMOVES at C^ and
measure the share that was ADDED by the most recent earlier commit touching the same file
(the "predecessor"). A high share within a short interval = C supersedes its predecessor.
Usage: supersede.py <repo> <ref> <author-regex> [--since DATE] [--paths glob ...]"""
import subprocess, sys, re, statistics, collections, datetime

repo, ref, author_re = sys.argv[1], sys.argv[2], sys.argv[3]
since = None; paths = ['*.cpp', '*.hpp', '*.h', '*.c', '*.frag', '*.vert', '*.glsl', '*.py', '*.txt', '*.cmake']
a = sys.argv[4:]
while a:
    if a[0] == '--since': since = a[1]; a = a[2:]
    elif a[0] == '--paths': paths = a[1:]; a = []
    else: a = a[1:]

def git(*args):
    return subprocess.run(['git', '-C', repo, *args], capture_output=True, text=True, errors='replace').stdout

log_args = ['log', '--no-merges', '--format=%H%x09%an%x09%at%x09%s']
if since: log_args.append('--since=' + since)
log_args.append(ref)
commits = []
for line in git(*log_args).splitlines():
    h, an, at, s = line.split('\t', 3)
    commits.append((h, an, int(at), s))
mine = [c for c in commits if re.search(author_re, c[1])]

hunk_re = re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')
rows = []
for h, an, at, s in mine:
    files = []
    for l in git('show', '--format=', '--numstat', h, '--', *paths).splitlines():
        p = l.split('\t')
        if len(p) == 3 and p[0] != '-':
            files.append((int(p[0]), int(p[1]), p[2]))
    if not files: continue
    added = sum(f[0] for f in files); removed = sum(f[1] for f in files)
    origins = collections.Counter(); pred_info = {}
    for ad, rm, path in files:
        if rm == 0: continue
        pred = git('log', '-1', '--format=%H%x09%at', h + '^', '--', path).strip()
        if pred:
            ph, pat = pred.split('\t'); pred_info[path] = (ph, int(pat))
        for l in git('diff', '-U0', h + '^', h, '--', path).splitlines():
            m = hunk_re.match(l)
            if not m: continue
            a0 = int(m.group(1)); n = int(m.group(2)) if m.group(2) is not None else 1
            if n == 0: continue
            for bl in git('blame', '-l', '-s', '-L', f'{a0},{a0 + n - 1}', h + '^', '--', path).splitlines():
                tok = bl.split(' ', 1)[0].lstrip('^')
                origins[(path, tok)] += 1
    from_pred = sum(cnt for (path, tok), cnt in origins.items() if path in pred_info and pred_info[path][0].startswith(tok[:8]))
    dts = [(at - pat) / 3600 for (ph, pat) in pred_info.values()]
    dt = min(dts) if dts else None
    rows.append(dict(h=h[:8], at=at, s=s, an=an, added=added, removed=removed, from_pred=from_pred,
                     dt=dt, nfiles=len(files), pred=[v[0][:8] for v in pred_info.values()]))

def fmt_t(at): return datetime.datetime.fromtimestamp(at).strftime('%Y-%m-%d %H:%M')
with_removals = [r for r in rows if r['removed'] >= 3]
pairs = [r for r in with_removals if r['from_pred'] / r['removed'] >= 0.5 and r['dt'] is not None and r['dt'] <= 48]
print(f'repo={repo} ref={ref} author~/{author_re}/ since={since}')
print(f'commits(author)={len(mine)}  with code changes={len(rows)}  removing>=3 lines={len(with_removals)}  '
      f'SUPERSESSION pairs (>=50% of removed lines from the predecessor, dt<=48h)={len(pairs)}')
if with_removals:
    shares = [r['from_pred'] / r['removed'] for r in with_removals]
    print(f'share-from-predecessor over removing commits: median={statistics.median(shares):.2f} '
          f'mean={statistics.mean(shares):.2f}  >=0.5: {sum(s >= 0.5 for s in shares)}  ==0: {sum(s == 0 for s in shares)}')
if pairs:
    dts = [r["dt"] for r in pairs]
    print(f'pair intervals (h): median={statistics.median(dts):.1f} min={min(dts):.2f} max={max(dts):.1f}  '
          f'<=2h: {sum(d <= 2 for d in dts)}  <=24h: {sum(d <= 24 for d in dts)}')
    verbs = collections.Counter(r['s'].split()[0].rstrip(':,').lower() for r in pairs)
    print('first word of the superseding message:', dict(verbs.most_common(8)))
    ratios = [r['removed'] / max(r['added'], 1) for r in pairs]
    print(f'del/add of superseding commits: median={statistics.median(ratios):.2f}; '
          f'of all removing commits: median={statistics.median([r["removed"]/max(r["added"],1) for r in with_removals]):.2f}')
    print('--- pairs, newest first (dt h | removed from-pred/total | +add -del | subject) ---')
    for r in pairs[:40]:
        print(f'{r["h"]} {fmt_t(r["at"])} dt={r["dt"]:6.1f}h  {r["from_pred"]:3d}/{r["removed"]:3d}  +{r["added"]:<4d}-{r["removed"]:<4d} {r["s"][:80]}')
# verbs over all commits, for the base rate
allverbs = collections.Counter(r['s'].split()[0].rstrip(':,').lower() for r in rows)
print('first word, all commits with code changes:', dict(allverbs.most_common(10)))
