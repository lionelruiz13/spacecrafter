#!/usr/bin/env python3
# The old path's query surface (INTENT S13 B42): every call into Projector / Navigator / Observer
# from outside src/experimentalModule, by source directory and by method. On the new path nothing
# may read them (S11.255): this is what the wiring class has to answer from the Camera.
# Regex on comment-stripped text, receiver names only - a call through another name is not seen.
#   usage (code repo root): trio_census.py [-v]     -v: every method with its count
import re, subprocess, collections, sys
files = [f for f in subprocess.run(['git', 'ls-files', 'src'], capture_output=True, text=True).stdout.split()
         if re.search(r'\.(cpp|hpp)$', f) and 'experimentalModule' not in f]
pat = re.compile(r'\b(prj|proj|projection|projector|nav|navigation|navigator|observatory|observer|obs)\s*(?:->|\.)\s*(\w+)\s*\(')
cls = dict(prj='Projector', proj='Projector', projection='Projector', projector='Projector', nav='Navigator',
           navigation='Navigator', navigator='Navigator', observatory='Observer', observer='Observer', obs='Observer')
per_dir, meth = collections.Counter(), collections.defaultdict(collections.Counter)
for f in files:
    t = re.sub(r'//[^\n]*', '', open(f, errors='replace').read())
    for m in pat.finditer(t):
        per_dir[f.split('/')[1]] += 1
        meth[cls[m.group(1)]][m.group(2)] += 1
print('calls into Projector / Navigator / Observer outside the module:', sum(per_dir.values()))
for d, n in per_dir.most_common():
    print(f'  {n:5d}  {d}')
for c in ('Projector', 'Navigator', 'Observer'):
    top = meth[c].most_common(None if '-v' in sys.argv else 12)
    print(f'{c}: {len(meth[c])} methods, {sum(meth[c].values())} calls:', ', '.join(f'{k} {v}' for k, v in top))
