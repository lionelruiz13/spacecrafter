#!/usr/bin/env python3
"""intent_pair_check.py — the stub<->entry-file cross-check instrument (F42, §11.156).

WHAT IT IS FOR
    The 2026-07-23 granularity split (INTENT.md header, "Document structure") gave every
    expanded §5/§11 entry two homes: the authority `INTENT/<id>.md` and a derived index
    stub in `INTENT.md`.  Nothing checked that a correction appended to one home reached
    the other.  §11.149(h) found `INTENT/5.27.md` carrying a refuted root for a month
    while the derived stub carried the correction — an AUTHORITY INVERSION.  This is the
    class detector.

WHAT IT IS NOT
    It is a FILTER, not a verdict.  It over-flags by construction (see the adjudication
    rules in §11.156) and every flag is adjudicated by reading both texts.  A clean run
    is not a proof of consistency; a run with every flag adjudicated is.

WHAT IS COMPARED (status-bearing content only)
    S1 state declarations about the entry/row itself, with their strike state
    S2 supersession/correction markers about the entry's own content
    S3 authority routing (what gates it, which task carries the fix, what closed it)
    S4 the date stamp attached to any of S1-S3
WHAT IS DELIBERATELY NOT COMPARED (the stub is a derived summary; body detail differs by design)
    N1 measured values, magnitudes, file:line citations, tables, formulas
    N2 discovery provenance (who found it, by which chain)
    N3 supporting-argument citations that neither declare nor route status
    N4 wording, ordering, level of detail, sub-item lettering
    N5 status statements about OTHER rows carried inside an entry — those are compared
       at that other row's own pair

THE FOUR TESTS
    D  (dangerous direction, stub -> entry) every status atom present in the stub must be
       present somewhere in the entry file.  This is the §5.27 shape.
    D2 (dangerous, structural)  every ~~struck~~ span of the stub, and every dated
       correction-marker span of the stub, must be represented in the entry file.
    I  (inverse direction, entry header -> stub) state atoms of the entry's own header
       absent from the stub.
    I2 (inverse, structural) every dated correction/annotation/addendum block inside the
       entry file whose date the stub does not carry.

DISCRIMINATING CHECK (must be re-run whenever this file changes)
    python3 intent_pair_check.py <tree-at-458a71f>   -> §5.27 MUST flag under D
    python3 intent_pair_check.py <current tree>      -> §5.27 MUST pass under D
    (458a71f = d43abec^, the commit before F37's §5.27 repair.)

USAGE
    python3 intent_pair_check.py [ROOT] [--json]
"""
import os, re, sys, json

STATE_WORDS = [
 'OPEN','CLOSED','CLOSES','RESOLVED','FIXED','SUSPENDED','BLOCKED','UNBLOCKED','DELIVERED',
 'LANDED','STALE','SUPERSEDED','SUPERSEDES','REFUTED','CORRECTED','CORRECTION','RETRACTED',
 'WITHDRAWN','DISSOLVED','DISSOLVE','RATIFIED','ANSWERED','VERIFIED','REOPENED','PARTIAL',
 'OWED','PENDING','DESCOPED','ARCHIVED','RETIRED','DEFERRED','IMPLEMENTED','AUDITED',
 'CONFIRMED','NOT DONE','DONE','MOOT','DEAD','ABANDONED','REPLACED','OBSOLETE','INERT',
 'NEGATIVE','GATED','WONTFIX','SPLIT','RENAMED','PROMOTED','DEMOTED','ACCEPTED','REJECTED',
]
# uppercase-only on purpose: in this ledger's grammar an uppercase state word IS a status
# declaration, lowercase prose is narration.  Cost: case-variant false positives, which the
# adjudication step absorbs.  A case-insensitive matcher would drown the run in "the open set".
STATE_RE = re.compile(r'(?<![A-Za-z])(' + '|'.join(w.replace(' ', r'\s+') for w in STATE_WORDS) + r')(?![a-z])')
DATE_RE  = re.compile(r'\b(20\d\d-\d\d-\d\d)\b')
SEC_RE   = re.compile(r'§(\d+)\.(\d+[a-z]?)')
LID_RE   = re.compile(r'(?<![A-Za-z0-9_])([ABDFTQS])(\d{1,3})(?![0-9A-Za-z])')
SS_RE    = re.compile(r'\bSS-(\d+)\b')
SHA_RE   = re.compile(r'`([0-9a-f]{7,8})`')
STRIKE_RE= re.compile(r'~~(.+?)~~', re.S)
MARK_RE  = re.compile(r'(ANNOTATION|ANNOTATED|ADDENDUM|SUPERSED\w*|CORRECTION|CORRECTED|REFUTED|'
                      r'RETRACTED|RATIFIED|REOPEN\w*|UPDATED?|WITHDRAWN|DELIVERED|PERFORMED|'
                      r'DISCHARGED|UNBLOCK\w*|ATTRIBUTED|CLOSED)', re.I)
SPAN_RE  = re.compile(r'\*\*(.+?)\*\*|\[([^\]]{5,300})\]')
# [F78 member 7, 2026-09-01] test M's own marker vocabulary.  MARK_RE stays exactly as
# F42 wrote it, because D2 and I2 are the supervisor's gate and this member must leave
# them where they are.  M needs a wider one, and the widening is specimen-driven, not
# imagined: §11.187's entry opens its marker "[REVERSED 2026-08-31 -> §11.191(b) ...]"
# and MARK_RE has no REVERSED, so M reported a both-homes divergence that is not there --
# member 3's disease arriving at the sibling instrument.  The rest are the same specimens
# member 3 measured in intent_backmarker_scan.py, so the package holds ONE marker
# vocabulary rather than three (I2).  Unifying MARK_RE with it is candidate C6: it
# re-baselines D2 and I2, which is a supervisor's call, not this member's.
MARKM_RE = re.compile(MARK_RE.pattern.rstrip(')') +
                      '|REVERSED|REFRESH\w*|ROOT-CAUSED?|FIXED|TESTED|INCOMPLETE|STALE)', re.I)


def atoms(text):
    return {
      'state': set(m.group(1).upper().replace('\n', ' ') for m in STATE_RE.finditer(text)),
      'date' : set(DATE_RE.findall(text)),
      'sec'  : set('%s.%s' % m for m in SEC_RE.findall(text)),
      'lid'  : set('%s%s' % m for m in LID_RE.findall(text)),
      'ss'   : set('SS-%s' % m for m in SS_RE.findall(text)),
      'sha'  : set(SHA_RE.findall(text)),
    }


def marker_spans(line):
    """[F78 member 7] BALANCED bracketed spans plus bold spans, with NO size cap.

    SPAN_RE (:71) is kept for D2/I2 so their baselines do not move, but it cannot serve
    test M, and the reason is measured: its bracket arm is `\[([^\]]{5,300})\]`, and
    **175 of the 250 bracketed dated marker spans in this corpus exceed 300 characters**
    (70%) -- §11.192's "[DELIVERED 2026-09-01 at §11.194, F73 ...]" is 707.  Its bold arm
    is non-greedy from the FIRST `**` on the line, so on a line carrying several bold runs
    whether a given span is captured depends on PARITY.  A both-homes test built on that
    reports divergences that are not there.  D2/I2's exposure to the same cap is recorded
    as candidate C5, measured and not enacted -- moving it re-baselines two counters this
    member is required to leave alone.
    """
    out, depth, start = [], 0, None
    for i, ch in enumerate(line):
        if ch == '[':
            if depth == 0:
                start = i
            depth += 1
        elif ch == ']' and depth:
            depth -= 1
            if depth == 0:
                out.append(line[start:i + 1])
    if depth:
        # UNTERMINATED run.  Measured, not hypothetical: INTENT/11.192.md:29 opens
        # "**[SUPERSEDED IN PART 2026-09-01 -> §11.193 ..." and "[DELIVERED 2026-09-01 at
        # §11.194, F73 ..." and carries ONE closing bracket for the two -- a malformed
        # marker span in the ledger's text, routed as a finding.  Crediting what is
        # plainly there is the strict-credit direction; punishing a real marker for a
        # missing bracket would manufacture a divergence that does not exist.
        out.append(line[start:])
    out += re.findall(r'\*\*(.+?)\*\*', line)
    return out


def markercites(text):
    """[F78 member 7] the §-ids cited inside DATED MARKER SPANS of one home.

    A span counts only if it carries a marker word AND a date -- the §11.113(p) marker
    form.  Anything looser stops being machine-decidable, which is the §11.180(i) bound.
    """
    out = set()
    for l in text.split('\n'):
        for s in marker_spans(l):
            if MARKM_RE.search(s) and DATE_RE.search(s):
                out |= set('%s.%s' % m for m in SEC_RE.findall(s))
    return out


def enumerate_pairs(root):
    """-> (pairs, orphan_files, orphan_stubs, stub_index)

    orphan_files = entry files with no live stub.  For an ARCHIVED-in-place entry that is
    the correct state: the retired index line is appended verbatim inside the file
    (INTENT.md header, "Archival"), so there is no live pair to diverge.
    orphan_stubs = stubs with no entry file: short entries that stayed inline.
    """
    lines = open(os.path.join(root, 'INTENT.md'), encoding='utf-8').read().split('\n')
    heads = [(i, m.group(1)) for i, l in enumerate(lines)
             for m in [re.match(r'^## (\d+)\.', l)] if m]
    spans = {name: (start, heads[k + 1][0] if k + 1 < len(heads) else len(lines))
             for k, (start, name) in enumerate(heads)}
    stubs = {}
    # [F78 member 1b, 2026-09-01] A stub is its numbered line PLUS its continuation
    # lines -- markdown's own list-item rule: a following line continues the row if it
    # is blank or INDENTED, and a non-indented line ends it.  v1 took lines[i] alone,
    # so a dated marker living in a continuation block was invisible to all four
    # tests.  Extent on the 2026-09-01 tree: exactly ONE register row has a real
    # continuation block, §5.117 (7 lines, F76's and F77's annotations), and it has no
    # entry file, so no live PAIR is multi-line and no counter moves today -- the class
    # is closed prospectively and its decoys are in harness/f78_discriminate.py.
    # The indentation rule is what keeps §11.14's '---' and §11.196's trailing
    # maintenance-marker paragraph out of their stubs.
    # Same defect, same fix as member 1 in intent_backmarker_scan.py: one fact, 'the
    # stub of id X', was resolved twice and wrongly in both copies (I2).
    for sec in ('5', '11'):
        a, b = spans[sec]
        idx = [i for i in range(a, b) if re.match(r'^(\d+[a-z]?)\.\s', lines[i])]
        for k, i in enumerate(idx):
            m = re.match(r'^(\d+[a-z]?)\.\s', lines[i])
            end = idx[k + 1] if k + 1 < len(idx) else b
            j = i + 1
            while j < end and (lines[j].strip() == '' or lines[j][:1].isspace()):
                j += 1
            while j - 1 > i and lines[j - 1].strip() == '':
                j -= 1
            stubs['%s.%s' % (sec, m.group(1))] = (i + 1, '\n'.join(lines[i:j]))
    d = os.path.join(root, 'INTENT')
    files = sorted(f[:-3] for f in os.listdir(d) if f.endswith('.md'))
    pairs = [f for f in files if f in stubs]
    return (pairs, [f for f in files if f not in stubs],
            sorted(k for k in stubs if k not in files), stubs)


def check(root):
    pairs, orphan_file, orphan_stub, stubs = enumerate_pairs(root)
    d = os.path.join(root, 'INTENT')
    out = []
    for pid in sorted(pairs, key=lambda s: (int(s.split('.')[0]),
                                            int(re.match(r'\d+', s.split('.')[1]).group()), s)):
        lineno, stub = stubs[pid]
        body = open(os.path.join(d, pid + '.md'), encoding='utf-8').read()
        hdr = next((l for l in body.split('\n') if re.match(r'^\d+[a-z]?\.\s', l)), '')
        sa, ba, ha = atoms(stub), atoms(body), atoms(hdr)
        norm = lambda t: re.sub(r'[^a-z0-9]+', ' ', t.lower())
        nbody, nstub = norm(body), norm(stub)
        r = dict(id=pid, line=lineno)
        r['D']  = {k: sorted(sa[k] - ba[k]) for k in sa if sa[k] - ba[k]}
        r['I']  = {k: sorted(ha[k] - sa[k]) for k in ha if ha[k] - sa[k]}
        r['D2'] = ([('strike', s[:90]) for s in STRIKE_RE.findall(stub)
                    if norm(s)[:40] not in nbody] +
                   [('marker', s[:90]) for sp in SPAN_RE.findall(stub)
                    for s in [sp[0] or sp[1]]
                    if MARK_RE.search(s) and (set(DATE_RE.findall(s)) - ba['date'])])
        r['I2'] = [(i, sorted(set(DATE_RE.findall(s)) - sa['date']), s[:90])
                   for i, l in enumerate(body.split('\n'), 1)
                   for sp in SPAN_RE.findall(l) for s in [sp[0] or sp[1]]
                   if MARK_RE.search(s) and (set(DATE_RE.findall(s)) - sa['date'])]
        # [F78 member 7, 2026-09-01] test M -- the BOTH-HOMES class, its own counter.
        # Three measured instances: §11.187's REVERSED marker reached the entry and not
        # the stub (F72 acceptance, executor-caught); §11.192's SUPERSEDED marker likewise
        # (F73 executor's §0.7 report); the F73 acceptance named it a CLASS and queued
        # this extension.  D2 and I2 compare marker spans by DATE, so a marker whose date
        # appears anywhere else in the other home passes them.
        # THE BOUND IS §11.180(i)'s, applied literally: the test asks both homes for the
        # SAME OBJECT -- a DATED marker span citing source §X -- which is machine-decidable.
        # "Does the stub relay the claim the entry corrected" is a READING and is not
        # implemented.  Like its four siblings this is a FILTER, not a verdict: the stub is
        # a derived summary and mirrors a marker only where it relays the superseded claim
        # (§11.156(f)), so entry_only flags are expected in bulk and are adjudicated.
        ec, sc = markercites(body) - {pid}, markercites(stub) - {pid}
        r['M'] = ([('entry_only', x) for x in sorted(ec - sc)] +
                  [('stub_only', x) for x in sorted(sc - ec)])
        out.append(r)
    return dict(n_entry_files=len(pairs) + len(orphan_file), n_pairs=len(pairs),
                orphan_file=orphan_file, orphan_stub=orphan_stub, report=out)


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    root = args[0] if args else os.path.dirname(os.path.abspath(__file__))
    res = check(root)
    if '--json' in sys.argv:
        print(json.dumps(res, ensure_ascii=False))
        raise SystemExit(0)
    print('entry files %d | live pairs %d | archived-in-place (no live stub) %d | inline stubs (no file) %d'
          % (res['n_entry_files'], res['n_pairs'], len(res['orphan_file']), len(res['orphan_stub'])))
    for t in ('D', 'D2', 'I', 'I2', 'M'):
        hits = [r for r in res['report'] if r[t]]
        print('\n=== test %s: %d pair(s) flagged ===' % (t, len(hits)))
        for r in hits:
            print('  %-8s (INTENT.md:%d)  %s' % (r['id'], r['line'],
                  json.dumps(r[t], ensure_ascii=False)[:240]))
