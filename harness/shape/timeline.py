#!/usr/bin/env python3
"""timeline.py [--by week|day|commit] [--top N] [path ...]

WHEN the shape moved: added / deleted code and comment lines under the given paths
(default src/experimentalModule), dated by AUTHOR date, per period and per author,
with the running total. Same line classification as authors.py (comment = a line
starting // /* or *; trailing comments count as code; blank lines ignored).

Run from the CODE repo root. --by commit lists every commit; --top N lists the N
commits that added the most code and the N that added the most comment lines.
Proves: where in time the growth sits and who carried it. Cannot see: moves between
files (a move counts as a delete plus an add), or WHAT was added (a second manager
and a first one weigh the same).
"""
import subprocess, sys, collections, datetime

args = sys.argv[1:]
by, top = 'week', 0
paths = []
while args:
    a = args.pop(0)
    if a == '--by': by = args.pop(0)
    elif a == '--top': top = int(args.pop(0))
    else: paths.append(a)
paths = paths or ['src/experimentalModule']

out = subprocess.run(['git', 'log', '--no-merges', '--reverse', '-p',
                      '--format=@@@C %h|%an|%aI|%s', '--'] + paths,
                     capture_output=True).stdout.decode('utf-8', 'replace')
commits = []
cur = None
for line in out.split('\n'):
    if line.startswith('@@@C '):
        h, an, ad, s = line[5:].split('|', 3)
        cur = dict(h=h, an=an, ad=datetime.datetime.fromisoformat(ad), s=s,
                   c=collections.Counter())
        commits.append(cur)
        continue
    if cur is None or not line or line[0] not in '+-':
        continue
    if line.startswith('+++') or line.startswith('---'):
        continue
    t = line[1:].strip()
    if not t:
        continue
    kind = 'cmt' if (t.startswith('//') or t.startswith('/*') or t.startswith('*')) else 'code'
    cur['c'][line[0] + kind] += 1

def key(c):
    d = c['ad']
    if by == 'day': return d.strftime('%Y-%m-%d')
    if by == 'commit': return c['h']
    iso = d.isocalendar()
    monday = d.date() - datetime.timedelta(days=d.weekday())
    return f"{iso[0]}-W{iso[1]:02d} ({monday.strftime('%m-%d')})"

def short(an):
    return an.replace('Claude ', '').replace('Calvin Ruiz', 'owner').replace('Calvin-Ruiz', 'owner')

if by == 'commit':
    print(f"{'commit':9} {'date':16} {'author':10} {'+code':>6} {'-code':>6} {'+cmt':>6} {'-cmt':>6}  subject")
    for c in commits:
        k = c['c']
        print(f"{c['h']:9} {c['ad'].strftime('%Y-%m-%d %H:%M'):16} {short(c['an'])[:10]:10} "
              f"{k['+code']:6} {k['-code']:6} {k['+cmt']:6} {k['-cmt']:6}  {c['s'][:90]}")
else:
    rows = collections.OrderedDict()
    for c in commits:
        r = rows.setdefault(key(c), dict(n=0, c=collections.Counter(), who=collections.Counter()))
        r['n'] += 1
        r['c'] += c['c']
        r['who'][short(c['an'])] += c['c']['+code']
    code = cmt = 0
    print(f"{'period':18} {'n':>4} {'+code':>6} {'-code':>6} {'+cmt':>6} {'-cmt':>6} {'del/add':>7} "
          f"{'code':>6} {'cmt':>6} {'ratio':>5}  +code by author")
    for p, r in rows.items():
        k = r['c']
        code += k['+code'] - k['-code']
        cmt += k['+cmt'] - k['-cmt']
        who = ' '.join(f"{a}:{n}" for a, n in r['who'].most_common() if n)
        print(f"{p:18} {r['n']:4} {k['+code']:6} {k['-code']:6} {k['+cmt']:6} {k['-cmt']:6} "
              f"{k['-code'] / max(1, k['+code']):7.2f} {code:6} {cmt:6} {cmt / max(1, code):5.2f}  {who}")

if top:
    for field, label in (('+code', 'added the most CODE'), ('+cmt', 'added the most COMMENT lines')):
        print(f"\n-- the {top} commits that {label}")
        for c in sorted(commits, key=lambda c: -c['c'][field])[:top]:
            k = c['c']
            print(f"{c['h']:9} {c['ad'].strftime('%Y-%m-%d %H:%M'):16} {short(c['an'])[:10]:10} "
                  f"+{k['+code']:<5} -{k['-code']:<5} cmt +{k['+cmt']:<5} -{k['-cmt']:<5} {c['s'][:80]}")
