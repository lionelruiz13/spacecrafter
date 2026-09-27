# Measure one header with form.py's own measure() (the module-level report is skipped).
import sys, re, collections, statistics, subprocess
src = open('claude/harness/shape/form.py').read()
exec(src.split('\nsh=lambda')[0])
for f in sys.argv[1:]:
    show(f, measure([f], lambda p: open(p, errors='replace').read()))
