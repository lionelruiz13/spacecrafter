#!/usr/bin/env python3
# Print the SHAPE block of one delivery <base>..<head>: the numbers an acceptance pastes, from the shape instruments.
#   usage (from anywhere; it works at the CODE repo root):
#     delivery.py <base> [<head>|WORKTREE] [--class Class:hpp:cpp]...
#     delivery.py --self-test     the two recorded deliveries (INTENT 11.317 / 11.316) as oracles; PASS* = the record
#                                 reproduced under the record-side rule printed on its line; exit 1 on any FAIL
#   <head> defaults to WORKTREE: the files on disk - the tracked files changed against <base> and the untracked files
#   under src/. --class (repeatable) replaces the default set: every class a touched header defines whose .cpp of the
#   same stem is also touched.
# The block, in this fixed order; each section names the instrument that computes it:
#   (1) lines    per file +code -code +comment -comment +blank -blank: ratio.py's classify on both ends, read on the
#                lines of git's -U0 hunks (a trailing comment counts as code); the totals beside git's numstat; net code
#   (2) names    per touched header: names.py BEFORE -> AFTER, the names added and removed
#   (3) writers  per class: writers.py's (field, writers) pairs at both ends, each field whose writer set changed
#                BEFORE -> AFTER, and how many pairs are unchanged
#   (4) form     form.py's measure over the ADDED lines, the rest of each file read as blank (its lines are classed
#                by ratio.py's classify, block state included): full comment lines, trailing, blocks 1 / 2 / 3+ (a
#                block is a run of consecutive added comment lines), median and p90 length; the blocks' first words
#                and the verb-first share - an UPPER bound: a first word counts unless it is not alphabetic, carries a
#                capital past its first letter, or is a closed-class English word (NOT_VERB); nouns pass
#   (5) ledger   the ADDED lines matching LEDGER, as file:line and the match, or 0
#   (6) d14      f70_ascii.py gate's verdict - it reads the code repo's TRACKED files ON DISK, so the line says whether
#                that tree is <head> - then the delivery's own files read at <head> (untracked included) by the
#                gate's partition rule
#   (7) ratio    the module's files / code / comment / ratio at both ends by ratio.py's rule (WORKTREE: the files on
#                disk, untracked included)
#   (8) scope    the touched files in review.sh's header scope (new / modified / deleted) and the other touched files
#                of its ALL=1 scope
# Exit 0 whatever the numbers are (the supervisor judges them); 2 when an instrument could not produce its line (the
# line says ERROR and why) or the working tree moved during a WORKTREE run.
import collections, os, re, subprocess, sys

SHAPE = os.path.dirname(os.path.abspath(__file__))
HARNESS = os.path.dirname(SHAPE)
ROOT = os.path.dirname(os.path.dirname(HARNESS))
sys.path[:0] = [SHAPE, HARNESS]
import names as namespy, ratio, form, f70_ascii

WT = 'WORKTREE'
LEDGER = re.compile(r'INTENT|11\.[0-9]|F1[0-9][0-9]|\[observed|\[measured|\[vixy')
SOURCE = re.compile(r'\.(hpp|cpp|h|c|glsl|frag|vert|comp|geom|tesc|tese)$')
HUNK = re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', re.M)
CLASS = re.compile(r'\b(enum\s+)?class\s+(\w+)\s*(?:final\s*)?(?::[^;{]*)?\{')
NOT_VERB = set('''a an the this that these those each every all any some no none it its we our they their he his she
    her i you your one for with in on at of to by from as per into onto over under about after before between during
    without within than via and or but if when while because since unless until whether so then also only not maybe
    must should can could may might will would shall is are was were be been being has have had do does did here
    there now still just note todo fixme xxx hack why how what which where who whose see'''.split())


class InstrumentError(Exception):
    pass


def git(*args):  # Return git's stdout bytes; a failing git is an InstrumentError
    r = subprocess.run(['git'] + list(args), capture_output=True)
    if r.returncode:
        raise InstrumentError('git %s: %s' % (' '.join(args), r.stderr.decode('utf-8', 'replace').strip()))
    return r.stdout


def zsplit(raw):
    return [p.decode('utf-8', 'surrogateescape') for p in raw.split(b'\0') if p]


def read(end, path):  # Return the file's bytes at a rev or on disk, None where it does not exist
    if path is None:
        return None
    if end == WT:   # a symlink's content is its target's path, as git stores it
        return (os.readlink(path).encode('utf-8', 'surrogateescape') if os.path.islink(path)
                else open(path, 'rb').read() if os.path.isfile(path) else None)
    r = subprocess.run(['git', 'cat-file', 'blob', '%s:%s' % (end, path)], capture_output=True)
    return r.stdout if r.returncode == 0 else None


def text(raw):  # Decode as ratio.py and names.py do
    return raw.decode('utf-8', 'replace')


def nlines(raw):
    return raw.count(b'\n') + (1 if raw and not raw.endswith(b'\n') else 0)


def touched(base, head):  # Return the delivery's files: git's numstat, plus the untracked src/ files for WORKTREE
    tok = zsplit(git('diff', '--numstat', '-z', base, *([] if head == WT else [head])))
    out, i = [], 0
    while i < len(tok):
        a, d, p = tok[i].split('\t', 2)
        i += 1
        if p == '':
            old, new = tok[i], tok[i + 1]
            i += 2
        else:
            old = new = p
        out.append(dict(old=old, new=new, adds=a, dels=d))
    if head == WT:
        for p in zsplit(git('ls-files', '--others', '--exclude-standard', '-z', '--', 'src')):
            raw = read(WT, p)
            if raw is not None:   # git's numstat rule: a NUL in the first 8000 bytes makes the file binary
                binary = b'\0' in raw[:8000]
                out.append(dict(old=None, new=p, adds='-' if binary else str(nlines(raw)), dels='-' if binary else '0'))
    for f in out:
        f['a'], f['b'] = read(base, f['old']), read(head, f['new'])
        f['status'] = ('U' if f['old'] is None else 'R' if f['old'] != f['new'] else
                       'A' if f['a'] is None else 'D' if f['b'] is None else 'M')
        f['binary'] = f['adds'] == '-'
        if f['a'] is None:
            f['old'] = None
        if f['b'] is None:
            f['new'] = None
    return sorted(out, key=lambda f: f['new'] or f['old'])


def hunks(base, head, f):  # Return the removed old line numbers and the added new line numbers of one file
    if f['status'] == 'U':
        return [], list(range(1, int(f['adds']) + 1))
    paths = [p for p in dict.fromkeys([f['old'], f['new']]) if p]
    out = text(git('diff', '-U0', '--no-color', '--no-ext-diff', base, *([] if head == WT else [head]), '--', *paths))
    rem, add = [], []
    for m in HUNK.finditer(out):
        b = 1 if m.group(2) is None else int(m.group(2))
        d = 1 if m.group(4) is None else int(m.group(4))
        rem += range(int(m.group(1)), int(m.group(1)) + b)
        add += range(int(m.group(3)), int(m.group(3)) + d)
    return rem, add


def split_lines(base, head, f):  # Count each added / removed line by ratio.py's classify; check the counts against numstat
    c = collections.Counter()
    f['added'] = []
    if f['binary']:
        return c
    rem, add = hunks(base, head, f)
    if (len(add), len(rem)) != (int(f['adds']), int(f['dels'])):
        raise InstrumentError('%s: hunks +%d -%d against numstat +%s -%s' % (f['new'] or f['old'], len(add), len(rem), f['adds'], f['dels']))
    old = ratio.classify(text(f['a'])) if f['a'] is not None else []
    new = ratio.classify(text(f['b'])) if f['b'] is not None else []
    lines = text(f['b']).split('\n') if f['b'] is not None else []
    for n in rem:
        c['-' + old[n - 1]] += 1
    for n in add:
        c['+' + new[n - 1]] += 1
        f['added'].append((n, new[n - 1], lines[n - 1]))
    return c


def names_at(end, path):  # Return names.py's names of a header at an end, None where it does not exist
    if read(end, path) is None:
        return None
    return namespy.names(namespy.text_of(path, None if end == WT else end))


def classes_at(end, path):  # Return the classes a header defines at an end (enum class and struct excluded)
    if read(end, path) is None:
        return []
    t = namespy.text_of(path, None if end == WT else end)
    return [m.group(2) for m in CLASS.finditer(t) if not m.group(1)]


def writers_at(end, cls, hpp, cpp):  # Return writers.py's {field: writers} for a class, None where it is not defined
    if read(end, cpp) is None or cls not in classes_at(end, hpp):
        return None
    r = subprocess.run([sys.executable, os.path.join(SHAPE, 'writers.py'), end, cls, hpp, cpp], capture_output=True, text=True)
    if r.returncode:
        raise InstrumentError('writers.py %s %s: %s' % (end, cls, (r.stderr.strip().splitlines() or ['exit %d' % r.returncode])[-1]))
    out, on = {}, False
    for l in r.stdout.splitlines():
        if l.startswith('--'):
            on = l.startswith('-- managers per field')
        elif on:
            t = l.split()
            out[t[1]] = t[2:]
    return out


def default_classes(files):  # Return (class, hpp, cpp) for every class of a touched header whose same-stem .cpp is touched
    paths = {p for f in files for p in (f['old'], f['new']) if p}
    out = []
    for h in sorted(p for p in paths if re.search(r'\.(hpp|h)$', p)):
        cpp = re.sub(r'\.(hpp|h)$', '.cpp', h)
        if cpp in paths:
            out += [(c, h, cpp) for c in dict.fromkeys(classes_at(BASE, h) + classes_at(HEAD, h))]
    return out


def verbish(w):
    return w.isalpha() and w[1:] == w[1:].lower() and w.lower() not in NOT_VERB


def comment_form(files):  # Measure the added lines' comments with form.py, every other line of the file read as blank
    src = {f['new']: f for f in files if f['b'] is not None and not f['binary'] and SOURCE.search(f['new'])}
    return form.measure(sorted(src), lambda p: text(src[p]['b']), lambda p: {n for n, _, _ in src[p]['added']})


def module(end):  # Return (files, code, comment) of ratio.PATH's .hpp / .cpp at an end by ratio.py's classify
    if end == WT:
        paths = zsplit(git('ls-files', '-z', '--', ratio.PATH)) + zsplit(git('ls-files', '--others', '--exclude-standard', '-z', '--', ratio.PATH))
    else:
        paths = zsplit(git('ls-tree', '-r', '--name-only', '-z', end, '--', ratio.PATH))
    n = code = comment = 0
    for p in paths:
        raw = read(end, p)
        if not p.endswith(('.hpp', '.cpp')) or raw is None:
            continue
        cls = ratio.classify(text(raw))
        n, code, comment = n + 1, code + cls.count('code'), comment + cls.count('comment')
    return n, code, comment


def review_scope():  # Return review.sh's header scope and its ALL=1 scope, evaluated by bash from review.sh's own line
    line = [l for l in open(os.path.join(SHAPE, 'review.sh')).read().splitlines()
            if l.startswith('if [ "${ALL:-0}" = 1 ]; then SCOPE=')]
    if len(line) != 1:
        raise InstrumentError('review.sh: %d SCOPE lines where 1 was expected' % len(line))
    out = []
    for v in ('0', '1'):
        r = subprocess.run(['bash', '-c', line[0] + '; printf "%s\\0" "${SCOPE[@]}"'], capture_output=True, env=dict(os.environ, ALL=v))
        out.append(zsplit(r.stdout))
    return out


def in_scope(base, head, specs):  # Return the touched paths a set of pathspecs matches (untracked ones for WORKTREE)
    got = zsplit(git('diff', '--name-only', '-z', base, *([] if head == WT else [head]), '--', *specs))
    if head == WT:
        got += zsplit(git('ls-files', '--others', '--exclude-standard', '-z', '--', *specs))
    return set(got)


def section(R, key, fn):  # Run one section; an instrument failure becomes its ERROR line
    try:
        R[key] = fn()
    except InstrumentError as e:
        R[key] = None
        R['errors'].append('(%s) %s' % (key, e))


def compute(base, head, classes=None):  # Return every number of the block for <base>..<head>
    global BASE, HEAD
    BASE, HEAD = base, head
    R = dict(base=base, head=head, errors=[])
    files = touched(base, head)
    R['files'] = files
    tot = collections.Counter()
    for f in files:
        try:
            f['split'] = split_lines(base, head, f)
        except InstrumentError as e:
            f['split'], f['added'] = None, []
            R['errors'].append('(1) %s' % e)
        tot.update(f['split'] or {})
    R['tot'] = tot
    section(R, 'names', lambda: [(p, names_at(base, f['old']) if f['old'] else None, names_at(head, f['new']) if f['new'] else None)
                                 for f in files for p in [f['new'] or f['old']] if re.search(r'\.(hpp|h)$', p)])
    section(R, 'classes', lambda: classes if classes is not None else default_classes(files))
    section(R, 'writers', lambda: [(c, writers_at(base, c, h, cpp), writers_at(head, c, h, cpp)) for c, h, cpp in R['classes'] or []])
    section(R, 'form', lambda: comment_form(files))
    R['ledger'] = [(f['new'], n, m.group(0)) for f in files for n, _, l in f.get('added', []) for m in [LEDGER.search(l)] if m]
    section(R, 'd14', lambda: d14(base, head, files))
    section(R, 'ratio', lambda: (module(base), module(head)))
    section(R, 'scope', lambda: scope(base, head, files))
    if head == WT:
        again = touched(base, head)
        if [(f['new'], f['adds'], f['dels'], f['b']) for f in again] != [(f['new'], f['adds'], f['dels'], f['b']) for f in files]:
            R['errors'].append('(WORKTREE) the working tree changed during the run: run it again')
    return R


def d14(base, head, files):  # Return the gate's verdict, whether its tree is <head>, and the delivery's files by its partition
    r = subprocess.run([sys.executable, os.path.join(HARNESS, 'f70_ascii.py'), 'gate', '--root', ROOT], capture_output=True, text=True)
    if r.returncode not in (0, 1):
        raise InstrumentError('f70_ascii.py gate exit %d: %s' % (r.returncode, r.stderr.strip()[-200:]))
    same = head == WT or subprocess.run(['git', 'diff', '--quiet', head]).returncode == 0
    part = f70_ascii.read_partition(f70_ascii.DEFAULT_PARTITION)
    rows = []
    for f in files:
        if f['b'] is None:
            continue
        enc = f70_ascii.classify_encoding(f['b'])
        if enc != 'ascii':
            hit = f70_ascii.decide(part, f['new'])
            rows.append((f['new'], enc, hit[0] if hit else 'unclassified'))
    return dict(rc=r.returncode, same=same, n=sum(f['b'] is not None for f in files), rows=rows)


def scope(base, head, files):  # Return the touched headers of review.sh's scope by status, and the other touched files of ALL=1
    hdr, every = review_scope()
    h, a = in_scope(base, head, hdr), in_scope(base, head, every)
    status = {f['new'] or f['old']: f['status'] for f in files}
    paths = {p: status.get(p, '?') for p in h}
    return dict(header=collections.Counter(paths.values()), paths=paths, outside=len(a - h), specs=(hdr, every))


def fmt_names(xs):
    return ' '.join(xs) if xs else '-'


def render(R):  # Print the block in the fixed order of the header
    base, head = R['base'], R['head']
    out = ['```', 'SHAPE %s..%s  (harness/shape/delivery.py)' % (base, head)]
    files, tot = R['files'], R['tot']
    out.append('(1) lines [ratio.py classify on the -U0 hunks]  +code -code +comment -comment +blank -blank  status file')
    for f in files:
        s = f['split']
        cells = 'binary' if f['binary'] else 'ERROR' if s is None else ' '.join(
            '%5d' % s[k] for k in ('+code', '-code', '+comment', '-comment', '+blank', '-blank'))
        name = f['new'] or f['old']
        if f['status'] == 'R':
            name = '%s <- %s' % (f['new'], f['old'])
        out.append('  %s  %s %s' % (cells, f['status'], name))
    adds = sum(int(f['adds']) for f in files if not f['binary'])
    dels = sum(int(f['dels']) for f in files if not f['binary'])
    out.append('  total %d files: +%d -%d (numstat); code +%d -%d, comment +%d -%d, blank +%d -%d; net code %+d, net lines %+d'
               % (len(files), adds, dels, tot['+code'], tot['-code'], tot['+comment'], tot['-comment'],
                  tot['+blank'], tot['-blank'], tot['+code'] - tot['-code'], adds - dels))
    out.append('(2) names [names.py]  BEFORE -> AFTER  +added  -removed')
    if R['names'] is not None:
        ta = tr = 0
        for p, b, a in R['names']:
            bs, as_ = set(b or []), set(a or [])
            add, rem = [n for n in (a or []) if n not in bs], [n for n in (b or []) if n not in as_]
            ta, tr = ta + len(add), tr + len(rem)
            out.append('  %s %s -> %s  +[%s]  -[%s]' % (p, 'absent' if b is None else len(b), 'absent' if a is None else len(a), fmt_names(add), fmt_names(rem)))
        out.append('  total %d headers: names +%d -%d' % (len(R['names']), ta, tr))
    out.append('(3) writers [writers.py]  class: pairs BEFORE -> AFTER, unchanged; changed field: writers BEFORE -> AFTER')
    if R['writers'] is not None:
        if not R['writers']:
            out.append('  no class (no touched header with its same-stem .cpp touched; --class names one)')
        for c, b, a in R['writers']:
            pb = {(fl, w) for fl, ws in (b or {}).items() for w in ws}
            pa = {(fl, w) for fl, ws in (a or {}).items() for w in ws}
            out.append('  %s: pairs %s -> %s, unchanged %d' % (c, 'absent' if b is None else len(pb), 'absent' if a is None else len(pa), len(pb & pa)))
            if b is not None and a is not None:
                for fl in sorted(set(b) | set(a)):
                    if b.get(fl) != a.get(fl):
                        out.append('    %s: %s -> %s' % (fl, fmt_names(b.get(fl)), fmt_names(a.get(fl))))
    out.append('(4) form [form.py measure over the added lines]')
    m = R['form']
    if m is not None:
        blk = m['blk']
        out.append('  comment lines added: full %d, trailing %d; blocks 1:%d 2:%d 3+:%d; length median %d p90 %d'
                   % (m['full'], m['trailing'], blk.get(1, 0), blk.get(2, 0), sum(v for k, v in blk.items() if k >= 3), m['med'], m['p90']))
        firsts = m['firsts']
        words = collections.Counter(firsts)
        out.append('  verb-first (upper bound) %d / %d blocks; first words: %s'
                   % (sum(verbish(w) for w in firsts), len(firsts), ', '.join('%s %d' % (w or '(none)', k) for w, k in words.most_common()) or '-'))
    out.append('(5) ledger grammar in the added lines [%s]: %d' % (LEDGER.pattern, len(R['ledger'])))
    for p, n, s in R['ledger']:
        out.append('  %s:%d  %s' % (p, n, s))
    out.append('(6) d14 [f70_ascii.py gate]')
    d = R['d14']
    if d is not None:
        out.append('  gate %s (exit %d; it reads the code repo\'s tracked files on disk; that tree is %s: %s)'
                   % ('PASS' if d['rc'] == 0 else 'FAIL', d['rc'], head, 'yes' if d['same'] else 'no'))
        bad = [r for r in d['rows'] if r[2] != 'EXCLUDE']
        out.append('  delivery files at %s by its partition: %d read, %d non-ASCII (%d CONVERT or unclassified): %s'
                   % (head, d['n'], len(d['rows']), len(bad), 'PASS' if not bad else 'FAIL'))
        for p, enc, dec in d['rows']:
            out.append('    %s %s %s' % (p, enc, dec))
    out.append('(7) ratio [ratio.py classify, %s .hpp/.cpp]' % ratio.PATH)
    if R['ratio'] is not None:
        for end, (n, code, comment) in zip((base, head), R['ratio']):
            out.append('  %s files %d code %d comment %d ratio %.2f' % (end, n, code, comment, comment / max(1, code)))
    out.append('(8) scope [review.sh]')
    s = R['scope']
    if s is not None:
        h = s['header']
        out.append('  header scope %s: %d touched (new %d, modified %d, deleted %d, renamed %d)'
                   % (' '.join(s['specs'][0]), sum(h.values()), h['A'] + h['U'], h['M'], h['D'], h['R']))
        out.append('  outside it, in the ALL=1 scope %s: %d touched' % (' '.join(s['specs'][1]), s['outside']))
    for e in R['errors']:
        out.append('ERROR %s' % e)
    out.append('```')
    return '\n'.join(out)


def main():
    args = sys.argv[1:]
    os.chdir(ROOT)
    if args == ['--self-test']:
        sys.exit(self_test())
    classes, pos = [], []
    while args:
        a = args.pop(0)
        if a == '--class':
            c = args.pop(0).split(':')
            if len(c) != 3:
                sys.exit('--class takes Class:hpp:cpp')
            classes.append(tuple(c))
        else:
            pos.append(a)
    if not 1 <= len(pos) <= 2:
        sys.exit('usage: delivery.py <base> [<head>|WORKTREE] [--class Class:hpp:cpp]...   |   delivery.py --self-test')
    R = compute(pos[0], pos[1] if len(pos) == 2 else WT, classes or None)
    print(render(R))
    sys.exit(2 if R['errors'] else 0)


MINUS = '\u2212'
LAYERS = ('skyline skyDisplay nebula nebula_mgr constellation constellation_mgr cardinals meteor meteor_mgr object_base '
          'object s_font skyline_mgr skydisplay_mgr')
PROJECTOR = ('src/coreModule/projector.hpp', 'src/coreModule/projector.cpp')
CAMERA = ('src/experimentalModule/Camera.hpp', 'src/experimentalModule/Camera.cpp')
# The recorded deliveries: (name, base, head, entry, classes, review-mark date, [(line, quote, probes)]). Every expected
# value is PARSED from its quote, and the quote is first found verbatim at its entry line: no value is typed twice.
ORACLES = [
    ('F146', 'fd4d9f01', '5bdabc7d', '11.317', dict(Projector=PROJECTOR), (148, '2026-10-11 14:3x', '2026-10-11 14:30'), [
        (121, 'new `skyView.hpp` 49, `oldSkyView.hpp` 95, `CameraSkyView.hpp` 54, `.cpp` 196',
         ['skyView.hpp:+', 'oldSkyView.hpp:+', 'CameraSkyView.hpp:+', 'CameraSkyView.cpp:+']),
        (121, '`CameraProjection.hpp` +19 / -8', ['CameraProjection.hpp:+', 'CameraProjection.hpp:-']),
        (121, '`EnvironmentManager.hpp` +6, `.cpp` +1', ['EnvironmentManager.hpp:+', 'EnvironmentManager.cpp:+']),
        (121, '`StarFieldEnv.cpp` +6 / -18', ['StarFieldEnv.cpp:+', 'StarFieldEnv.cpp:-']),
        (121, '`Renderer.hpp` +4, `.cpp` +11 / -5', ['Renderer.hpp:+', 'Renderer.cpp:+', 'Renderer.cpp:-']),
        (121, '`core.hpp` +4, `core.cpp` +7', ['core.hpp:+', 'core.cpp:+']),
        (121, '`ssystem_factory.hpp` +3, `.cpp` +5', ['ssystem_factory.hpp:+', 'ssystem_factory.cpp:+']),
        (121, '`skyline.cpp` 117 / 117, `skyDisplay.cpp` 58 / 59, `nebula.cpp` 12 / 12, `nebula_mgr.cpp` 13 / 13',
         ['skyline.cpp:+', 'skyline.cpp:-', 'skyDisplay.cpp:+', 'skyDisplay.cpp:-', 'nebula.cpp:+', 'nebula.cpp:-',
          'nebula_mgr.cpp:+', 'nebula_mgr.cpp:-']),
        (121, '`constellation.cpp` 11 / 12, `constellation_mgr.cpp` 14 / 14, `cardinals.cpp` 14 / 14, `meteor.cpp` 12 / 13',
         ['constellation.cpp:+', 'constellation.cpp:-', 'constellation_mgr.cpp:+', 'constellation_mgr.cpp:-',
          'cardinals.cpp:+', 'cardinals.cpp:-', 'meteor.cpp:+', 'meteor.cpp:-']),
        (121, '`meteor_mgr.cpp` 8 / 9, `object_base.cpp` 8 / 9, `object.cpp` 2 / 4, `s_font.cpp` 11 / 6',
         ['meteor_mgr.cpp:+', 'meteor_mgr.cpp:-', 'object_base.cpp:+', 'object_base.cpp:-', 'object.cpp:+', 'object.cpp:-',
          's_font.cpp:+', 's_font.cpp:-']),
        (121, '`skyline_mgr.cpp` 2 / 2, `skydisplay_mgr.cpp` 5 / 5, headers 1-20 each',
         ['skyline_mgr.cpp:+', 'skyline_mgr.cpp:-', 'skydisplay_mgr.cpp:+', 'skydisplay_mgr.cpp:-', 'range:' + LAYERS]),
        (121, '`solarSystemModule.cpp` 10 / 9, `stellarSystemModule.cpp` 10 / 9, `inGalaxyModule.cpp` 3 / 2, `inUniverseModule.cpp` 3 / 2',
         ['solarSystemModule.cpp:+', 'solarSystemModule.cpp:-', 'stellarSystemModule.cpp:+', 'stellarSystemModule.cpp:-',
          'inGalaxyModule.cpp:+', 'inGalaxyModule.cpp:-', 'inUniverseModule.cpp:+', 'inUniverseModule.cpp:-']),
        (121, '`skygrid.cpp` 7 / 7, `skygrid.hpp` 7 / 7, `skygrid_mgr.cpp` 2 / 2, `skygrid_mgr.hpp` 1 / 3',
         ['skygrid.cpp:+', 'skygrid.cpp:-', 'skygrid.hpp:+', 'skygrid.hpp:-', 'skygrid_mgr.cpp:+', 'skygrid_mgr.cpp:-',
          'skygrid_mgr.hpp:+', 'skygrid_mgr.hpp:-']),
        (121, 'Total +860 / -435; code +794 / -435 (net +359), comments +21, blanks +45',
         ['total:+', 'total:-', 'code:+', 'code:-', 'code:net', 'comment:+', 'blank:+']),
        (121, 'removed 0', ['names:removed']),
        (122, '`Renderer.hpp` 53 -> 54, `EnvironmentManager.hpp` 13 -> 14, `CameraProjection.hpp` 1 -> 3',
         ['Renderer.hpp:n0', 'Renderer.hpp:n1', 'EnvironmentManager.hpp:n0', 'EnvironmentManager.hpp:n1',
          'CameraProjection.hpp:n0', 'CameraProjection.hpp:n1']),
        (122, '`core.hpp` 144 -> 145, `ssystem_factory.hpp` 77 -> 78, `nebula.hpp` 36 -> 37',
         ['core.hpp:n0', 'core.hpp:n1', 'ssystem_factory.hpp:n0', 'ssystem_factory.hpp:n1', 'nebula.hpp:n0', 'nebula.hpp:n1']),
        (122, "the 13 other layer headers and the grid's unchanged", ['names:unchanged-nongrid', 'names:grid-unchanged']),
        (122, 'new: `skyView.hpp` 27, `oldSkyView.hpp` 27, `CameraSkyView.hpp` 29',
         ['skyView.hpp:n1new', 'oldSkyView.hpp:n1new', 'CameraSkyView.hpp:n1new']),
        (122, '`writers.py Projector` identical', ['Projector:identical']),
        (150, 'D14 PASS', ['d14']),
        (155, 'SHAPE: 50 files +860 / %s435 (+794 / %s435 code, net +359' % (MINUS, MINUS),
         ['files', 'total:+', 'total:-', 'code:+', 'code:-', 'code:net']),
        (155, '0 functions removed', ['names:removed']),
        (155, 'the header queue gains the 4 new headers + 20 retyped', ['scope:new', 'scope:modified']),
    ]),
    ('F140', '93b9f55e', 'fd4d9f01', '11.316', dict(Projector=PROJECTOR, Camera=CAMERA), (135, '2026-10-11 10:5x', '2026-10-11 10:50'), [
        (117, '`ssystem_factory.cpp` 7 / 0, `ssystem_factory.hpp` 3 / 0, `core.cpp` 3 / 1',
         ['ssystem_factory.cpp:+', 'ssystem_factory.cpp:-', 'ssystem_factory.hpp:+', 'ssystem_factory.hpp:-', 'core.cpp:+', 'core.cpp:-']),
        (117, '`solarSystemModule.cpp` 3 / 0', ['solarSystemModule.cpp:+', 'solarSystemModule.cpp:-']),
        (117, '`Camera.cpp` 9 / 36, `CameraProjection.hpp` 31 / 0, `EnvironmentManager.hpp` 2 / 0, `EnvironmentModule.hpp` 7 / 0',
         ['Camera.cpp:+', 'Camera.cpp:-', 'CameraProjection.hpp:+', 'CameraProjection.hpp:-', 'EnvironmentManager.hpp:+',
          'EnvironmentManager.hpp:-', 'EnvironmentModule.hpp:+', 'EnvironmentModule.hpp:-']),
        (117, '`Renderer.cpp` 8 / 6, `Renderer.hpp` 10 / 0, `MilkyWayEnv.cpp` 1 / 5, `StarFieldEnv.cpp` 33 / 0, `StarFieldEnv.hpp` 14 / 0',
         ['Renderer.cpp:+', 'Renderer.cpp:-', 'Renderer.hpp:+', 'Renderer.hpp:-', 'MilkyWayEnv.cpp:+', 'MilkyWayEnv.cpp:-',
          'StarFieldEnv.cpp:+', 'StarFieldEnv.cpp:-', 'StarFieldEnv.hpp:+', 'StarFieldEnv.hpp:-']),
        (117, '`hip_star_mgr.cpp` 26 / 12, `hip_star_mgr.hpp` 24 / 4, `zone_array.cpp` 34 / 11, `zone_array.hpp` 49 / 4',
         ['hip_star_mgr.cpp:+', 'hip_star_mgr.cpp:-', 'hip_star_mgr.hpp:+', 'hip_star_mgr.hpp:-', 'zone_array.cpp:+',
          'zone_array.cpp:-', 'zone_array.hpp:+', 'zone_array.hpp:-']),
        (117, '= **+264 / -79, net +185**', ['total:+', 'total:-', 'lines:net']),
        (118, '`EnvironmentModule.hpp` 6 -> 7 (+`rotationOf`)',
         ['EnvironmentModule.hpp:n0', 'EnvironmentModule.hpp:n1', 'EnvironmentModule.hpp:added']),
        (118, '`hip_star_mgr.hpp` 98 -> 100 (+`preDrawWith`, `sizeRolloff`)',
         ['hip_star_mgr.hpp:n0', 'hip_star_mgr.hpp:n1', 'hip_star_mgr.hpp:added']),
        (118, '`zone_array.hpp` 17 -> 22 (+`drawWith`, `toLocal`, `project`, `projectCheck`, `rolloff` - the policies)',
         ['zone_array.hpp:n0', 'zone_array.hpp:n1', 'zone_array.hpp:added']),
        (118, '`ssystem_factory.hpp` 76 -> 77 (+`getEnvironmentStarFieldInput`)',
         ['ssystem_factory.hpp:n0', 'ssystem_factory.hpp:n1', 'ssystem_factory.hpp:added']),
        (118, '`Renderer.hpp` 51 -> 53 (+`getViewportCenter`, `getMvp2D`)', ['Renderer.hpp:n0', 'Renderer.hpp:n1', 'Renderer.hpp:added']),
        (118, '`EnvironmentManager.hpp` 13, `Camera.hpp` 79, `projector.hpp` 78, `navigator.hpp` 51, `core.hpp` 144 unchanged',
         ['EnvironmentManager.hpp:both', 'Camera.hpp:both', 'projector.hpp:both', 'navigator.hpp:both', 'core.hpp:both']),
        (118, 'new `CameraProjection.hpp` 1 (`project`), `StarFieldEnv.hpp` 2', ['CameraProjection.hpp:n1new', 'StarFieldEnv.hpp:n1new']),
        (119, '`Projector` `fov` 1 (`setFov`) - unchanged', ['Projector.fov:both']),
        (119, '`zoomDuration` 3, `zoomTimer` 2, `srcHalfFov` 1, `dstHalfFov` 1 - unchanged managers',
         ['Camera.zoomDuration:both', 'Camera.zoomTimer:both', 'Camera.srcHalfFov:both', 'Camera.dstHalfFov:both']),
        (143, 'SHAPE: 17 files +264 / %s79, net +185' % MINUS, ['files', 'total:+', 'total:-', 'lines:net']),
        (143, '`names.py` +7 names', ['names:added']),
        (143, '`writers.py` Projector `fov` 1 unchanged', ['Projector.fov:both']),
        (143, '0 ledger references, D14 PASS', ['ledger', 'd14']),
        (143, '`hip_star_mgr.hpp`, `zone_array.hpp`, `EnvironmentManager.hpp`, `EnvironmentModule.hpp`, `ssystem_factory.hpp`, '
              '`Renderer.hpp` and the two new headers', ['scope:modified-set', 'scope:new']),
    ]),
]
WORDS = dict(zero=0, one=1, two=2, three=3, four=4, five=5, six=6, seven=7, eight=8, nine=9, ten=10)
NUMERIC = re.compile(r'(?<![\w.])\d+(?!\w)|\b(?:%s)\b' % '|'.join(WORDS))


def entry_line(entry, n):  # Return line n of an INTENT entry, through the archive door when it moved
    for p in ('claude/INTENT/%s.md' % entry, 'claude/INTENT/archive/%s.md' % entry):
        if os.path.isfile(p):
            lines = open(p, encoding='utf-8').read().split('\n')
            return p, lines[n - 1] if n <= len(lines) else ''
    return 'claude/INTENT/%s.md' % entry, ''


def expected(quote, probes):  # Return each probe's expected value, parsed from the quote alone
    toks = [int(WORDS.get(t, t)) for t in NUMERIC.findall(re.sub(r'`[^`]*`', '', quote))]
    ticks = [t for t in re.findall(r'`([^`]+)`', quote) if '.' not in t]
    out = []
    for p in probes:
        if p.endswith(':added') and not p.startswith('names:'):
            out.append(sorted(ticks))
        elif p == 'scope:modified-set':
            out.append(sorted(t for t in re.findall(r'`([^`]+)`', quote) if t.endswith('.hpp')))
        elif p.endswith(':identical') or p == 'names:grid-unchanged':
            out.append(True)
        elif p == 'd14':
            out.append('PASS' if 'PASS' in quote else 'FAIL')
        elif p.startswith('range:'):
            out.append((toks.pop(0), toks.pop(0)))
        else:
            out.append(toks.pop(0))
    if toks:
        raise InstrumentError('quote %r: %d number(s) no probe consumes' % (quote, len(toks)))
    return out


def by_name(R, name):  # Return the one touched file whose base name is name
    hit = [f for f in R['files'] if os.path.basename(f['new'] or f['old']) == name]
    if len(hit) != 1:
        raise InstrumentError('%d touched files named %s' % (len(hit), name))
    return hit[0]


def numstat_rows(base, head, *flags):
    tok = zsplit(git('diff', '--numstat', '-z', *flags, base, head))
    return [(t.split('\t')[2], int(t.split('\t')[0]), int(t.split('\t')[1])) for t in tok]


def numstat_probe(rows, p):  # Evaluate a numstat probe over (path, adds, dels) rows
    a, d = sum(r[1] for r in rows), sum(r[2] for r in rows)
    if p in ('files', 'total:+', 'total:-', 'lines:net'):
        return dict(files=len(rows), **{'total:+': a, 'total:-': d, 'lines:net': a - d})[p]
    name, side = p.rsplit(':', 1)
    hit = [r for r in rows if os.path.basename(r[0]) == name]
    return None if len(hit) != 1 else hit[0][1] if side == '+' else hit[0][2]


def header_names(R, name):  # Return (BEFORE, AFTER) names of a header, touched or not
    for p, b, a in R['names']:
        if os.path.basename(p) == name:
            return b, a
    hit = [p for p in zsplit(git('ls-tree', '-r', '--name-only', '-z', R['head'], '--', 'src')) if os.path.basename(p) == name]
    if len(hit) != 1:
        raise InstrumentError('%d headers named %s at %s' % (len(hit), name, R['head']))
    return names_at(R['base'], hit[0]), names_at(R['head'], hit[0])


def observe(R, p, classes):  # Return the value the block's run gives for one probe
    t = R['tot']
    if p in ('files', 'total:+', 'total:-', 'lines:net') or re.search(r'\.(hpp|cpp|h)?:[+-]$', p):
        return numstat_probe([(f['new'] or f['old'], int(f['adds']) if not f['binary'] else 0, int(f['dels']) if not f['binary'] else 0)
                              for f in R['files']], p)
    if p in ('code:+', 'code:-', 'comment:+', 'blank:+'):
        return t[p[-1] + p[:-2]]
    if p == 'code:net':
        return t['+code'] - t['-code']
    if p.startswith('range:'):
        v = [max(int(f['adds']), int(f['dels'])) for s in p[6:].split() for f in [by_name(R, s + '.hpp')]]
        return (min(v), max(v))
    if p.startswith('names:'):
        rows = [(os.path.basename(q), b or [], a or []) for q, b, a in R['names']]
        if p == 'names:removed':
            return sum(len(set(b) - set(a)) for _, b, a in rows)
        if p == 'names:added':
            return sum(len(set(a) - set(b)) for _, b, a in rows)
        same = [n for n, b, a in rows if b == a]
        if p == 'names:unchanged-nongrid':
            return len([n for n in same if not n.startswith('skygrid')])
        return all(n in same for n, _, _ in rows if n.startswith('skygrid'))
    if p.startswith('scope:'):
        paths = R['scope']['paths']
        if p == 'scope:new':
            return sum(s in 'AU' for s in paths.values())
        if p == 'scope:modified':
            return sum(s == 'M' for s in paths.values())
        return sorted(os.path.basename(q) for q, s in paths.items() if s == 'M')
    if p == 'ledger':
        return len(R['ledger'])
    if p == 'd14':
        return 'PASS' if not [r for r in R['d14']['rows'] if r[2] != 'EXCLUDE'] else 'FAIL'
    w = re.match(r'(\w+)\.(\w+):both$', p)
    if w and w.group(2) not in ('hpp', 'h', 'cpp') and w.group(1) in classes:
        cls, field = w.groups()
        b, a = (writers_at(e, cls, *classes[cls]) for e in (R['base'], R['head']))
        return (len(b.get(field, [])), len(a.get(field, [])))
    if p.endswith(':identical'):
        cls = p.split(':')[0]
        return writers_at(R['base'], cls, *classes[cls]) == writers_at(R['head'], cls, *classes[cls])
    name, kind = p.rsplit(':', 1)
    b, a = header_names(R, name)
    return dict(n0=None if b is None else len(b), n1=None if a is None else len(a),
                n1new=len(a) if b is None and a is not None else 'present at base',
                both=(None if b is None else len(b), None if a is None else len(a)),
                added=sorted(set(a or []) - set(b or [])))[kind]


def matches(p, obs, exp):
    if p.startswith('range:'):
        return exp[0] <= obs[0] and obs[1] <= exp[1]
    if p.endswith(':both'):
        return obs == (exp, exp)
    return obs == exp


def new_type_names(R, path):  # Return the names declared inside the classes / structs a header gains between the two ends
    def types(end):
        if read(end, path) is None:
            return {}
        t = namespy.text_of(path, None if end == WT else end)
        out = {}
        for m in re.finditer(r'\b(?:class|struct)\s+(\w+)\b[^;{()]*\{', t):
            d, j = 0, m.end() - 1
            for j in range(m.end() - 1, len(t)):
                d += (t[j] == '{') - (t[j] == '}')
                if d == 0:
                    break
            out.setdefault(m.group(1), t[m.end():j])
        return out
    base, head = types(R['base']), types(R['head'])
    return {n for k in head if k not in base for n in namespy.names(head[k])}


def cause(R, p, exp, mark, ws):  # Return the named rule under which the record's value reproduces, or None
    if re.search(r'\.(hpp|cpp|h)?:[+-]$', p):
        if numstat_probe(numstat_rows(R['base'], R['head'], '-w'), p) == exp:
            ws.add(p.rsplit(':', 1)[0])
            return 'whitespace-only lines in this file: its git diff --numstat -w row reproduces the record'
    if p in ('files', 'total:+', 'total:-', 'lines:net') and ws:
        plain, white = numstat_rows(R['base'], R['head']), numstat_rows(R['base'], R['head'], '-w')
        rows = [r for r in plain if os.path.basename(r[0]) not in ws] + [r for r in white if os.path.basename(r[0]) in ws]
        if numstat_probe(rows, p) == exp:
            return 'the whitespace-only lines of %s (named above) alone: the record precedes them' % ', '.join(sorted(ws))
    if p in ('scope:new', 'scope:modified'):
        r = subprocess.run(['git', 'rev-parse', '--short=8', 'refs/review/headers@{%s}' % mark], capture_output=True, text=True)
        if r.returncode == 0:
            sha = r.stdout.strip()
            old = [q for q in R['scope']['paths'] if read(sha, q) is None]
            v = len(old) if p == 'scope:new' else len(R['scope']['paths']) - len(old)
            if v == exp:
                return 'counted against the owner\'s review mark refs/review/headers@{%s} = %s, not against <base>' % (mark, sha)
    if p == 'names:added':
        v = sum(len(set(a) - set(b) - new_type_names(R, q)) for q, b, a in R['names'] if b is not None and a is not None)
        if v == exp:
            return 'names added to the types that existed at <base>: new headers and new classes / structs left out'
    h = re.match(r'(.+\.(?:hpp|h)):(n0|n1|both)$', p)
    if h:
        ends = {'n0': [R['base']], 'n1': [R['head']], 'both': [R['base'], R['head']]}[h.group(2)]
        if all(record_text_count(h.group(1), e) == exp for e in ends):
            return 'the record\'s names.py (%s) stripped /* */ before //, so a //* comment swallowed code - fixed in decomment.py' % RECORD_NAMES
    return None


RECORD_NAMES = '736d30e:harness/shape/names.py'


def record_text_count(name, rev):  # Count today's names on the text the records' names.py saw; None where both texts give the same names
    paths = [q for q in zsplit(git('ls-tree', '-r', '--name-only', '-z', rev, '--', 'src')) if os.path.basename(q) == name]
    if len(paths) != 1:
        return None
    ns = {'__name__': 'record_names'}
    exec(compile(git('-C', HARNESS, 'show', RECORD_NAMES), RECORD_NAMES, 'exec'), ns)
    old, new = namespy.names(ns['text_of'](paths[0], rev)), namespy.names(namespy.text_of(paths[0], rev))
    return None if old == new else len(old)


def self_test():  # Run every oracle; print one PASS / PASS* / FAIL line per probe; exit 1 on any FAIL
    passed = starred = failed = 0
    for name, base, head, entry, classes, (mline, mquote, mark), cases in ORACLES:
        R, ws = compute(base, head), set()
        for e in R['errors']:
            print('FAIL %s ERROR %s' % (name, e))
            failed += 1
        path, line = entry_line(entry, mline)
        if mquote not in line:
            print('FAIL %s %s:%d quote not found: %r' % (name, path, mline, mquote))
            failed += 1
        for end in (base, head):
            ref = subprocess.run([sys.executable, os.path.join(SHAPE, 'ratio.py'), end], capture_output=True, text=True).stdout.strip()
            n, code, comment = module(end)
            mine = '%s files %d code %d comment %d ratio %.2f' % (end, n, code, comment, comment / max(1, code))
            ok = mine == ref
            print('%s %s ratio %s: delivery.py "%s" ratio.py "%s"' % ('PASS' if ok else 'FAIL', name, end, mine, ref))
            passed, failed = passed + ok, failed + (not ok)
        for n, quote, probes in cases:
            path, line = entry_line(entry, n)
            if quote not in line:
                print('FAIL %s %s:%d quote not found: %r' % (name, path, n, quote))
                failed += 1
                continue
            try:
                exps = expected(quote, probes)
            except InstrumentError as e:
                print('FAIL %s %s:%d %s' % (name, path, n, e))
                failed += 1
                continue
            for p, exp in zip(probes, exps):
                try:
                    obs = observe(R, p, classes)
                except InstrumentError as e:
                    obs = 'ERROR %s' % e
                if matches(p, obs, exp):
                    passed += 1
                    print('PASS %s %s:%d %s observed=%s expected=%s' % (name, os.path.basename(path), n, p, obs, exp))
                    continue
                why = cause(R, p, exp, mark, ws)
                if why:
                    starred += 1
                    print('PASS* %s %s:%d %s observed=%s expected=%s rule=%s' % (name, os.path.basename(path), n, p, obs, exp, why))
                else:
                    failed += 1
                    print('FAIL %s %s:%d %s observed=%s expected=%s cause=UNNAMED' % (name, os.path.basename(path), n, p, obs, exp))
    print('self-test: %d PASS, %d PASS* (the record reproduced under the named rule on its line), %d FAIL' % (passed, starred, failed))
    return 1 if failed else 0


if __name__ == '__main__':
    main()
