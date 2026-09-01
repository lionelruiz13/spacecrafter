#!/usr/bin/env python3
"""F78 -- both-ways discrimination for every strict-credit v2 behaviour change.

A green that cannot fail is not evidence.  Each case below builds a SCRATCH copy of
the ledger (INTENT.md + INTENT/), injects one decoy, runs BOTH instrument versions
against it, and asserts what each must say.  A case passes only when v1 and v2
DISAGREE in the predicted direction (or, for the additive-test cases, when the
counter that must move is the only one that moves).

Usage: python3 harness/f78_discriminate.py <ledger-root> [--keep]
Exit 0 = every case as predicted.
"""
import os, re, shutil, subprocess, sys, tempfile

ROOT = os.path.abspath(sys.argv[1])
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
KEEP = "--keep" in sys.argv

SCAN_V1 = os.path.join(REPO, "intent_backmarker_scan.v1.py")
SCAN_V2 = os.path.join(REPO, "intent_backmarker_scan.py")
PAIR_V1 = os.path.join(REPO, "intent_pair_check.v1.py")
PAIR_V2 = os.path.join(REPO, "intent_pair_check.py")


def scratch():
    d = tempfile.mkdtemp(prefix="f78_")
    shutil.copy2(os.path.join(ROOT, "INTENT.md"), d)
    shutil.copytree(os.path.join(ROOT, "INTENT"), os.path.join(d, "INTENT"))
    return d


def scan(script, root):
    """-> (events, pairs, unmarked, frozenset(pairs))"""
    out = subprocess.run([sys.executable, script, root], capture_output=True, text=True)
    if out.returncode:
        raise SystemExit("scan failed: %s" % out.stderr[:400])
    ls = out.stdout.rstrip("\n").split("\n")
    nums = [int(l.rsplit(":", 1)[1]) for l in ls[:3]]
    got = frozenset(tuple(x.strip().lstrip("§") for x in l.split("->")) for l in ls[3:] if "->" in l)
    return nums[0], nums[1], nums[2], got


def pair(script, root):
    """-> dict of counter name -> int"""
    out = subprocess.run([sys.executable, script, root], capture_output=True, text=True)
    if out.returncode:
        raise SystemExit("pair failed: %s" % out.stderr[:400])
    r = {}
    m = re.search(r"entry files (\d+) \| live pairs (\d+) \| archived-in-place \(no live stub\) (\d+) \| inline stubs \(no file\) (\d+)", out.stdout)
    r["files"], r["pairs"], r["arch"], r["inline"] = map(int, m.groups())
    for t, n in re.findall(r"=== test (\w+): (\d+) pair\(s\) flagged ===", out.stdout):
        r[t] = int(n)
    return r


def edit_intent(d, old, new, count=1):
    p = os.path.join(d, "INTENT.md")
    s = open(p, encoding="utf-8").read()
    assert s.count(old) >= 1, "anchor absent: %r" % old[:70]
    open(p, "w", encoding="utf-8").write(s.replace(old, new, count))


def edit_entry(d, eid, old, new, count=1):
    p = os.path.join(d, "INTENT", eid + ".md")
    s = open(p, encoding="utf-8").read()
    assert old in s, "anchor absent in %s: %r" % (eid, old[:70])
    open(p, "w", encoding="utf-8").write(s.replace(old, new, count))


def row_line(d, sec, num):
    """the numbered register line for S<sec>.<num>, and its index"""
    L = open(os.path.join(d, "INTENT.md"), encoding="utf-8").read().split("\n")
    SEC = re.compile(r"^## (\d+)\.")
    heads = [(i, m.group(1)) for i, l in enumerate(L) for m in [SEC.match(l)] if m]
    spans = {n: (s, heads[k+1][0] if k+1 < len(heads) else len(L)) for k, (s, n) in enumerate(heads)}
    a, b = spans[sec]
    for i in range(a, b):
        if re.match(r"^%s\.\s" % re.escape(num), L[i]):
            return i, L[i]
    raise AssertionError("no row S%s.%s" % (sec, num))


def put_line(d, idx, text):
    p = os.path.join(d, "INTENT.md")
    L = open(p, encoding="utf-8").read().split("\n")
    L[idx] = text
    open(p, "w", encoding="utf-8").write("\n".join(L))


def insert_line(d, idx, text):
    p = os.path.join(d, "INTENT.md")
    L = open(p, encoding="utf-8").read().split("\n")
    L.insert(idx, text)
    open(p, "w", encoding="utf-8").write("\n".join(L))


BASE = {}   # filled in main(): the two versions' numbers on the PRISTINE tree


CASES = []
def case(fn):
    CASES.append(fn); return fn


# ---------------------------------------------------------------- member 1 (scan)
@case
def m1_own_stub_credited():
    """A marker in the S11.N target's OWN stub, absent from its S5.N twin.
    v1 reads S5.N and misses it (pair stays unmarked); v2 credits it."""
    d = scratch()
    # S11.100 -> S11.97 is an unmarked pair in the live corpus.  Put a canonical
    # back-marker naming S11.100 into S11.97's OWN stub line.
    i, line = row_line(d, "11", "97")
    put_line(d, i, line + " **[ANNOTATION 2099-01-01, §11.100 — CORRECTED here for the decoy.]**")
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[2] == BASE["v1"][2] and ("11.100", "11.97") in a[3] and
          b[2] == BASE["v2"][2] - 1 and ("11.100", "11.97") not in b[3])
    if not KEEP: shutil.rmtree(d)
    return "m1-A  marker in the target's OWN stub", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


@case
def m1_twin_stub_refused():
    """The inverse: a qualifying span in the COLLIDING S5.N line only.
    v1 credits it (wrong register); v2 must refuse."""
    d = scratch()
    i, line = row_line(d, "5", "97")
    put_line(d, i, line + " **[ANNOTATION 2099-01-01, §11.100 — CORRECTED here for the decoy.]**")
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[2] == BASE["v1"][2] - 1 and ("11.100", "11.97") not in a[3] and
          b[2] == BASE["v2"][2] and ("11.100", "11.97") in b[3])
    if not KEEP: shutil.rmtree(d)
    return "m1-A  marker in the COLLIDING S5.N line only", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


@case
def m1_continuation_credited():
    """A marker in a CONTINUATION line of the target's stub block.
    v1 returns the numbered line only and misses it; v2 reads the block."""
    d = scratch()
    i, _ = row_line(d, "11", "97")
    insert_line(d, i + 1, "    **[ANNOTATION 2099-01-01, §11.100 — CORRECTED in a continuation line.]**")
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[2] == BASE["v1"][2] and ("11.100", "11.97") in a[3] and
          b[2] == BASE["v2"][2] - 1 and ("11.100", "11.97") not in b[3])
    if not KEEP: shutil.rmtree(d)
    return "m1-B  marker in a CONTINUATION line", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


# ------------------------------------------------------------- member 1b (pair-check)
@case
def m1b_continuation_seen_by_D():
    """A dated marker in a CONTINUATION line of a pair's stub, absent from the entry.
    v1 reads the numbered line only and sees nothing; v2 reads the block and D fires."""
    d = scratch()
    i, _ = row_line(d, "5", "18")     # §5.18 carries NO D flag in either version
    insert_line(d, i + 1, "    **[ANNOTATION 2099-01-01, §11.999 — a decoy the entry file does not carry.]**")
    a = pair(PAIR_V1, d); b = pair(PAIR_V2, d)
    ok = (a["D"] == BASE["p1"]["D"] and b["D"] == BASE["p2"]["D"] + 1 and
          a["files"] == b["files"] and a["pairs"] == b["pairs"])
    if not KEEP: shutil.rmtree(d)
    return "m1b   stub continuation reaches test D", ok, "v1 D %d (base %d) / v2 D %d (base %d)" % (
        a["D"], BASE["p1"]["D"], b["D"], BASE["p2"]["D"])


@case
def m1b_continuation_clears_I2():
    """The inverse: §5.2 is I2-flagged on dates 2026-07-22 / 2026-08-30 that its stub
    lacks.  Put them in a CONTINUATION line; v1 still flags, v2 must not."""
    d = scratch()
    i, _ = row_line(d, "5", "2")
    insert_line(d, i + 1, "    (mirror 2026-07-22 / 2026-08-30)")
    a = pair(PAIR_V1, d); b = pair(PAIR_V2, d)
    ok = a["I2"] == BASE["p1"]["I2"] and b["I2"] == BASE["p2"]["I2"] - 1
    if not KEEP: shutil.rmtree(d)
    return "m1b   stub continuation clears an I2 flag", ok, "v1 I2 %d (base %d) / v2 I2 %d (base %d)" % (
        a["I2"], BASE["p1"]["I2"], b["I2"], BASE["p2"]["I2"])


# ------------------------------------------------------------------- member 2 (scan)
@case
def m2_incomplete_is_an_event():
    """An uppercase INCOMPLETE beside a citation is an event for v2 and not for v1."""
    d = scratch()
    edit_entry(d, "11.100", "\n", "\nDecoy: §11.99's clause is INCOMPLETE as stated.\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[:3] == BASE["v1"][:3] and
          b[0] == BASE["v2"][0] + 1 and b[1] == BASE["v2"][1] + 1)
    if not KEEP: shutil.rmtree(d)
    return "m2    INCOMPLETE beside a citation is an event", ok, "v1 %d/%d / v2 %d/%d (base %d/%d)" % (
        a[0], a[1], b[0], b[1], BASE["v2"][0], BASE["v2"][1])


@case
def m2_lowercase_is_not():
    """The same sentence in lowercase must move nothing, in EITHER version --
    the ledger's grammar makes an uppercase word a declaration and prose narration."""
    d = scratch()
    edit_entry(d, "11.100", "\n", "\nDecoy: §11.99's clause is incomplete as stated.\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[:3] == BASE["v1"][:3] and b[:3] == BASE["v2"][:3]
    if not KEEP: shutil.rmtree(d)
    return "m2    the same sentence lowercase moves nothing", ok, "v1 %d/%d/%d / v2 %d/%d/%d" % (a[:3] + b[:3])


# ------------------------------------------------------------------- member 3 (scan)
@case
def m3a_bold_span_credits():
    """A dated marker in a BOLD span (not bracketed) at the target: v1 blind, v2 credits.
    Four of §11.179/M1's six specimens are exactly this shape."""
    d = scratch()
    edit_entry(d, "11.97", "\n", "\n**FIXED AND CLOSED 2099-01-01 (§11.100): the decoy's own marker.**\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[2] == BASE["v1"][2] and b[2] == BASE["v2"][2] - 1
    if not KEEP: shutil.rmtree(d)
    return "m3-a  a BOLD dated marker credits", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


@case
def m3a_vocab_credits():
    """A BRACKETED span whose opening word is in the widened vocabulary only."""
    d = scratch()
    edit_entry(d, "11.97", "\n", "\n[STUB REFRESHED 2099-01-01 — §11.100: the decoy's own marker.]\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[2] == BASE["v1"][2] and b[2] == BASE["v2"][2] - 1
    if not KEEP: shutil.rmtree(d)
    return "m3-a  the widened marker vocabulary credits", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


@case
def m3a_unmarked_bold_refused():
    """Control: a BOLD span carrying the citation but NO marker word must credit
    nothing -- the widening is span-plus-vocabulary, not bare co-occurrence."""
    d = scratch()
    edit_entry(d, "11.97", "\n", "\n**A plain bold sentence mentioning §11.100 and nothing else.**\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[2] == BASE["v1"][2] and b[2] == BASE["v2"][2]
    if not KEEP: shutil.rmtree(d)
    return "m3-a  a bold span with NO marker word credits nothing", ok, "v1 %d / v2 %d (base %d)" % (
        a[2], b[2], BASE["v2"][2])


@case
def m3b_present_tense_is_an_event():
    """SUPERSEDES beside a citation: an event for v2, invisible to v1."""
    d = scratch()
    edit_entry(d, "11.100", "\n", "\nDecoy: this clause SUPERSEDES §11.99's reading.\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[:3] == BASE["v1"][:3] and
          b[0] == BASE["v2"][0] + 1 and b[1] == BASE["v2"][1] + 1)
    if not KEEP: shutil.rmtree(d)
    return "m3-b  present-tense SUPERSEDES is an event", ok, "v1 %d/%d / v2 %d/%d (base %d/%d)" % (
        a[0], a[1], b[0], b[1], BASE["v2"][0], BASE["v2"][1])


# ------------------------------------------------------------------- member 4 (scan)
@case
def m4_stale_routing_is_an_event():
    """F60's own marker keyword: STALE ROUTING beside a citation is an event for v2."""
    d = scratch()
    edit_entry(d, "11.100", "\n", "\nDecoy: **[STALE ROUTING 2099-01-01, §11.99(a)]** the clause names a delivered row.\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = (a[:3] == BASE["v1"][:3] and
          b[0] == BASE["v2"][0] + 1 and b[1] == BASE["v2"][1] + 1)
    if not KEEP: shutil.rmtree(d)
    return "m4    STALE ROUTING is an event", ok, "v1 %d/%d / v2 %d/%d (base %d/%d)" % (
        a[0], a[1], b[0], b[1], BASE["v2"][0], BASE["v2"][1])


@case
def m4_bare_stale_is_not():
    """THE NEGATIVE CONTROL THAT DECIDED THE MEMBER'S SHAPE.  Bare STALE is a CLASS
    NAME in this ledger -- "the pre-existing distant-TNO STALE class (§11.39)" -- and
    putting it in the event lexicon manufactured 25 pairs, ~20 of them that one phrase
    and ordinary narration.  v2 must NOT fire on it."""
    d = scratch()
    edit_entry(d, "11.100", "\n", "\nDecoy: the pre-existing distant-TNO STALE class (§11.99).\n", 1)
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[:3] == BASE["v1"][:3] and b[:3] == BASE["v2"][:3]
    if not KEEP: shutil.rmtree(d)
    return "m4    bare STALE is refused (class-name control)", ok, "v1 %d/%d/%d / v2 %d/%d/%d" % (a[:3] + b[:3])


@case
def m4_stale_marker_credits():
    """The MARKER arm: a [STALE ROUTING ...] span at the target credits under v2 only.
    It moves nothing on today's corpus, so it needs this to discriminate at all."""
    d = scratch()
    i, _ = row_line(d, "11", "97")
    put_line(d, i, open(os.path.join(d, "INTENT.md"), encoding="utf-8").read().split("\n")[i] +
             " **[STALE ROUTING 2099-01-01 (§11.100): this row's routing is superseded.]**")
    a = scan(SCAN_V1, d); b = scan(SCAN_V2, d)
    ok = a[2] == BASE["v1"][2] and b[2] == BASE["v2"][2] - 1
    if not KEEP: shutil.rmtree(d)
    return "m4    a STALE ROUTING span credits", ok, "v1 %d (base %d) / v2 %d (base %d)" % (
        a[2], BASE["v1"][2], b[2], BASE["v2"][2])


def main():
    print("ledger root: %s" % ROOT)
    BASE["v1"], BASE["v2"] = scan(SCAN_V1, ROOT), scan(SCAN_V2, ROOT)
    BASE["p1"], BASE["p2"] = pair(PAIR_V1, ROOT), pair(PAIR_V2, ROOT)
    print("  base scan v1 %d/%d/%d   v2 %d/%d/%d" % (BASE["v1"][:3] + BASE["v2"][:3]))
    bad = 0
    for fn in CASES:
        name, ok, detail = fn()
        print("  %-4s %-46s %s" % ("PASS" if ok else "FAIL", name, detail))
        bad += (not ok)
    print("%d case(s), %d failed" % (len(CASES), bad))
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
