#!/usr/bin/env python3
# F126: page_template.md + the two generated fragments -> claude/b42-wiring-table.md
A = '/home/claude/spacecrafter/claude/harness/artifacts/f126/'
t = open(A + 'page_template.md').read()
assert t.count('@@FAMILY_TABLE@@') == 1 and t.count('@@FULL_TABLE@@') == 1
page = t.replace('@@FAMILY_TABLE@@', open(A + 'frag_family.md').read().rstrip('\n')).replace('@@FULL_TABLE@@', open(A + 'frag_full.md').read().rstrip('\n'))
open('/home/claude/spacecrafter/claude/b42-wiring-table.md', 'w').write(page)
print('lines', page.count('\n'), 'bytes', len(page.encode()), 'non-ascii', sum(1 for c in page if ord(c) > 127),
      'appendix rows', sum(1 for l in page.split('\n') if l.startswith('| `')))
