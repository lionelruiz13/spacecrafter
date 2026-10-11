# Emit the KEY(x) rows of key_binding.cpp from SDL_scancode.h: every identifier with a value, UNKNOWN excepted.
import re
hdr = open('/usr/include/SDL2/SDL_scancode.h').read()
ids = [m.group(1) for m in re.finditer(r'^\s*SDL_SCANCODE_([A-Z0-9_]+)\s*=\s*(\d+)', hdr, re.M) if m.group(1) != 'UNKNOWN']
assert len(ids) == len(set(ids))
rows, cur = [], '\t'
for x in ids:
    item = 'KEY(%s), ' % x
    if len(cur.expandtabs(4)) + len(item) > 112:
        rows.append(cur.rstrip())
        cur = '\t'
    cur += item
rows.append(cur.rstrip().rstrip(','))
print('\n'.join(rows))
import sys; print(len(ids), 'identifiers,', len(rows), 'lines', file=sys.stderr)
