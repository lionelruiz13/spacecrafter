#!/usr/bin/env python3
"""Forward view: for each commit N by the author, which share of the lines N ADDED is removed by
the very next commit touching the same file (M), and how long after. Prints the distribution and the
rows for the newest K commits. Usage: supersede2.py <repo> <ref> <author-regex> [--since D] [--show K]"""
import subprocess, sys, re, statistics, datetime, collections

repo, ref, author_re = sys.argv[1], sys.argv[2], sys.argv[3]
since = None; show = 12
a = sys.argv[4:]
while a:
    if a[0] == '--since': since = a[1]; a = a[2:]
    elif a[0] == '--show': show = int(a[1]); a = a[2:]
    else: a = a[1:]
paths = ['*.cpp', '*.hpp', '*.h', '*.c', '*.frag', '*.vert', '*.glsl']

def git(*args):
    return subprocess.run(['git', '-C', repo, *args], capture_output=True, text=True, errors='replace').stdout

log_args = ['log', '--no-merges', '--format=%H%x09%an%x09%at%x09%s']
if since: log_args.append('--since=' + since)
log_args.append(ref)
commits = [l.split('\t', 3) for l in git(*log_args).splitlines()]
mine = [(h, an, int(at), s) for h, an, at, s in commits if re.search(author_re, an)]
hunk_re = re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')

def next_commit(h, path):
    # commits after h on the lineage ref that touch path, oldest first
    out = git('rev-list', '--reverse', '--format=%H%x09%at', f'{h}..{ref}', '--', path).splitlines()
    for l in out:
        if '\t' in l and not l.startswith('commit'):
            m, mat = l.split('\t'); return m, int(mat)
    return None, None

rows = []
for h, an, at, s in mine:
    files = []
    for l in git('show', '--format=', '--numstat', h, '--', *paths).splitlines():
        p = l.split('\t')
        if len(p) == 3 and p[0] != '-' and int(p[0]) > 0:
            files.append((int(p[0]), p[2]))
    if not files: continue
    added = sum(f[0] for f in files); killed = 0; dts = []; nexts = set()
    for ad, path in files:
        m, mat = next_commit(h, path)
        if not m: continue
        nexts.add(m[:8]); dts.append((mat - at) / 3600)
        for l in git('diff', '-U0', m + '^', m, '--', path).splitlines():
            mm = hunk_re.match(l)
            if not mm: continue
            a0 = int(mm.group(1)); n = int(mm.group(2)) if mm.group(2) is not None else 1
            if n == 0: continue
            for bl in git('blame', '-l', '-s', '-L', f'{a0},{a0 + n - 1}', m + '^', '--', path).splitlines():
                tok = bl.split(' ', 1)[0].lstrip('^')
                if h.startswith(tok[:8]): killed += 1
    rows.append(dict(h=h[:8], at=at, s=s, added=added, killed=killed, dt=min(dts) if dts else None, nexts=sorted(nexts)))

def fmt_t(at): return datetime.datetime.fromtimestamp(at).strftime('%Y-%m-%d %H:%M')
print(f'repo={repo} ref={ref} author~/{author_re}/ since={since}  commits adding code={len(rows)}')
sized = [r for r in rows if r['added'] >= 5 and r['dt'] is not None]
fr = [r['killed'] / r['added'] for r in sized]
print(f'commits adding >=5 lines with a later commit on the file: {len(sized)}; share of their added lines removed by the NEXT commit on the file: '
      f'median={statistics.median(fr):.2f} mean={statistics.mean(fr):.2f}  ==0: {sum(f == 0 for f in fr)}  >=0.25: {sum(f >= 0.25 for f in fr)}  >=0.5: {sum(f >= 0.5 for f in fr)}')
short = [r for r in sized if r['killed'] / r['added'] >= 0.25 and r['dt'] <= 48]
print(f'SHORT-LIVED commits (>=25% of added lines removed by the next commit within 48h): {len(short)} = {100*len(short)/max(len(sized),1):.0f}%')
if short:
    print('  by year:', dict(sorted(collections.Counter(datetime.datetime.fromtimestamp(r["at"]).year for r in short).items())))
    print('  dt (h): median=%.1f, <=2h: %d' % (statistics.median([r['dt'] for r in short]), sum(r['dt'] <= 2 for r in short)))
print('commits adding code, by year:', dict(sorted(collections.Counter(datetime.datetime.fromtimestamp(r["at"]).year for r in rows).items())))
print(f'--- newest {show} commits (killed/added by next | dt to next | next | subject) ---')
for r in rows[:show]:
    dt = f'{r["dt"]:6.1f}h' if r['dt'] is not None else '   none'
    print(f'{r["h"]} {fmt_t(r["at"])} {r["killed"]:4d}/{r["added"]:<4d} {dt}  {",".join(r["nexts"])[:17]:17s} {r["s"][:70]}')
