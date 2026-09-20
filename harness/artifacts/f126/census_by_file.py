#!/usr/bin/env python3
# F126: trio_census.py's exact regex and file set, extended to per-method / per-file / per-line output.
# Same blind spots as the census: receiver NAMES only, `//` comments stripped, /* */ not stripped.
#   usage (code repo root): census_by_file.py > census_sites.tsv     (class, method, file, line, text)
import re, subprocess, collections, sys
files = [f for f in subprocess.run(['git', 'ls-files', 'src'], capture_output=True, text=True).stdout.split()
         if re.search(r'\.(cpp|hpp)$', f) and 'experimentalModule' not in f]
pat = re.compile(r'\b(prj|proj|projection|projector|nav|navigation|navigator|observatory|observer|obs)\s*(?:->|\.)\s*(\w+)\s*\(')
cls = dict(prj='Projector', proj='Projector', projection='Projector', projector='Projector', nav='Navigator',
           navigation='Navigator', navigator='Navigator', observatory='Observer', observer='Observer', obs='Observer')
n = 0
for f in files:
    raw = open(f, errors='replace').read()
    t = re.sub(r'//[^\n]*', lambda m: ' ' * len(m.group(0)), raw)   # same strip, offsets kept
    lines = raw.split('\n')
    for m in pat.finditer(t):
        ln = t.count('\n', 0, m.start()) + 1
        print('\t'.join([cls[m.group(1)], m.group(2), f, str(ln), m.group(1), lines[ln - 1].strip()[:160]]))
        n += 1
print(n, file=sys.stderr)
