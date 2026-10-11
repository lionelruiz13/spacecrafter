#!/usr/bin/env python3
# The S4 name pass, injected: every function-like NAME a header declares at class / namespace
# depth, in declaration order, one line per header - to paste into a task's PREMISES block
# (INTENT 11.265(g)5) so the pass over the existing names is in the executor's window.
#   usage: names.py [--rev <rev>] <header> [<header>...]      (run from the CODE repo root)
# Regex, no compiler. Blind to: names produced by a macro, function-pointer members, a
# declaration split around a preprocessor branch. It lists `operator` forms as `operator`,
# skips names inside function bodies, and reports overloads ONCE (the count is of NAMES).
# It is a record that the list exists, never a substitute for reading the header.
import re, subprocess, sys
from decomment import decomment

KEYWORDS = {'if', 'for', 'while', 'switch', 'return', 'sizeof', 'static_assert', 'alignas',
            'decltype', 'catch', 'defined', 'assert', 'noexcept', 'throw', 'alignof', 'typeid'}
SCOPE = re.compile(r'\b(class|struct|namespace|union|extern)\b')

def text_of(path, rev):
    if rev:
        raw = subprocess.run(['git', 'show', '%s:%s' % (rev, path)], capture_output=True, check=True).stdout
    else:
        raw = open(path, 'rb').read()
    t = decomment(raw.decode('utf-8', 'replace'), literals=False)
    return re.sub(r'^[ \t]*#[^\n]*(\\\n[^\n]*)*', '', t, flags=re.M)

def name_of(head):
    head = re.sub(r'\b(public|private|protected)\s*:', ' ', head)
    i = head.find('(')
    if i < 0:
        return None
    before = head[:i]
    if 'operator' in before:
        return 'operator'
    if '=' in before:
        return None
    m = re.search(r'(~?\w+)\s*$', before)
    if not m or m.group(1) in KEYWORDS or m.group(1)[0].isdigit():
        return None
    return m.group(1)

def names(t):
    out, stack, stmt, paren = [], [], '', 0
    for c in t:
        in_body = 'body' in stack
        if c == '(':
            paren += 1
        elif c == ')':
            paren -= 1
        if c == '{' and paren == 0:
            head = stmt
            is_scope = SCOPE.search(head) and '(' not in head
            is_enum = re.search(r'\benum\b', head) and '(' not in head
            if not in_body and not is_scope and not is_enum:
                n = name_of(head)
                if n:
                    out.append(n)
            stack.append('scope' if is_scope and not in_body else 'body')
            stmt = ''
        elif c == '}' and paren == 0:
            if stack:
                stack.pop()
            stmt = ''
        elif c == ';' and paren == 0:
            if not in_body:
                n = name_of(stmt)
                if n:
                    out.append(n)
            stmt = ''
        else:
            stmt += c
    seen, uniq = set(), []
    for n in out:
        if n not in seen:
            seen.add(n)
            uniq.append(n)
    return uniq

def main():
    args = sys.argv[1:]
    rev = None
    if args[:1] == ['--rev']:
        rev, args = args[1], args[2:]
    if not args:
        sys.exit(__doc__ or 'usage: names.py [--rev <rev>] <header>...')
    for path in args:
        n = names(text_of(path, rev))
        print('%s %d: %s' % (path.rsplit('/', 1)[-1], len(n), ' '.join(n)))

if __name__ == '__main__':
    main()
