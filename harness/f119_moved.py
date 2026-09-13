#!/usr/bin/env python3
"""f119_moved.py -- LOOK at what the anchor gate calls `moved`, and say who moved it.

WHY THIS EXISTS.  `util/scedit/tests/anchor_gate.py` counts; `tests/anchor-expected.txt`
records the counts; and the README's contract for that record is that it is "a record,
not a silencer ... edited deliberately, never regenerated blind" (util/scedit/README.md:789).
Between the recorded state (clean 7194 / moved 2) and today the engine moved under the
grammar's citations, and re-recording without reading what moved is exactly the blind
regeneration the contract forbids.  The gate prints totals and nothing else, so the
reading it demands has no instrument.  This is that instrument.

WHAT IT DOES.  It walks the same five grammar files with the gate's OWN functions
(imported, never re-implemented -- a second copy of the tokenizer would drift from the
one the gate actually runs), records every reference's verdict with its resolved source
file and its line at the pin and at HEAD, and then attributes each moved reference to the
commits that touched its source file between that file's pin and HEAD.  Each commit is
labelled with the delivery that cites it, found by searching the harness ledger for the
abbreviated sha -- so the output answers "which delivery moved these anchors", not merely
"they moved".

THE SELF-CHECK THAT CAN FAIL.  The per-verdict counts this script collects are compared
against the counts anchor_gate.run() returns in the same process.  Any disagreement is a
drift between this reading and the gate's, and the script exits 2 saying so: a reading
that does not reproduce the gate's own tally is not a reading OF the gate.

Run: python3 claude/harness/f119_moved.py [--out <file>]
"""
import os, sys, re, subprocess, collections

HARN = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HARN, "..", ".."))
GATE_DIR = os.path.join(REPO, "util", "scedit", "tests")
sys.path.insert(0, GATE_DIR)
import anchor_gate as G           # noqa: E402  (the path has to be set first)


def git(*a, repo=REPO):
    r = subprocess.run(["git", "-C", repo] + list(a), capture_output=True)
    return r.stdout.decode("utf-8", "replace") if r.returncode == 0 else ""


# ---------------------------------------------------------------------------
# The walk.  This mirrors anchor_gate.run()'s reference loop; every decision it
# makes (file resolution, disambiguation, the submodule route, the marker) is
# taken by calling the gate's own function for it.
# ---------------------------------------------------------------------------
def collect():
    rows, tally = [], collections.Counter()
    for path in G.TARGETS:
        pin = G.file_pin(path)
        g = G.load(path)
        if path == G.MERGED:
            cmds = g["families"]["commands"]
        elif "commands" in g:
            cmds = g["commands"]
        else:
            cmds = {}
        for jp, s in G.walk(g):
            if jp.startswith("._meta"):
                tally["skipped(_meta)"] += sum(len(t.elems) for t in G.tokenize(s))
                continue
            toks = G.tokenize(s)
            if not toks:
                continue
            marked = bool(G.MARKER.search(s))
            m = re.match(r'\.families\.commands\.([A-Za-z0-9_]+)\.' if path == G.MERGED
                         else r'\.commands\.([A-Za-z0-9_]+)\.', jp)
            handler = cmds.get(m.group(1), {}).get("handler") if m else None
            for ti, t in enumerate(toks):
                if (jp, ti) in G.FILE_OVERRIDES:
                    rp, how = G.FILE_OVERRIDES[(jp, ti)], "repo-relative"
                elif t.kind == "FILE":
                    rp, how = G.resolve_file(t.file)
                    if rp is None:
                        tally["not-at-head(declared)" if marked else "UNRESOLVED"] += len(t.elems)
                        continue
                else:
                    rp, how, _basis, _ev = G.disambiguate(t, pin, s, handler)
                    if rp is None:
                        tally["not-at-head(declared)" if marked else "UNRESOLVED"] += len(t.elems)
                        continue
                for a, b, _, _ in t.elems:
                    broken = False
                    for n in (a, b):
                        if n is None:
                            continue
                        v = G.classify_any(pin, rp, n, how, head=G.WORKTREE)[0]
                        if v in ("no-file", "out-of-range"):
                            broken = True
                            if not marked:
                                tally["AT-PIN-BROKEN"] += 1
                                rows.append((path, jp, rp, pin, a, None, "AT-PIN-BROKEN", v))
                            break
                    if broken:
                        if marked:
                            tally["not-at-head(declared)"] += 1
                        continue
                    v, jline, src, headtext = G.classify_any(pin, rp, a, how, head=G.WORKTREE)
                    if b is not None:
                        v2 = G.classify_any(pin, rp, b, how, head=G.WORKTREE)[0]
                        OK = ("clean", "moved", "d14", "d14-moved")
                        v = ("clean" if v == v2 == "clean" else
                             ("moved" if v in OK and v2 in OK else
                              (v if v not in OK else v2)))
                    if v == "clean":
                        tally["clean"] += 1
                    elif v in ("moved", "d14", "d14-moved"):
                        tally["moved"] += 1
                        rows.append((path, jp, rp, pin, a, jline, v, src, b))
                    elif marked:
                        tally["not-at-head(declared)"] += 1
                    else:
                        tally["GONE"] += 1
                        rows.append((path, jp, rp, pin, a, None, "GONE", src))
    return rows, tally


def delivery_of(sha8):
    """The delivery that cites this code commit, by searching the harness ledger."""
    hits = []
    for d in ("INTENT", "INTENT/archive", "."):
        out = git("grep", "-l", sha8, "HEAD", "--", d, repo=os.path.join(REPO, "claude"))
        for line in out.splitlines():
            p = line.split(":", 1)[-1]
            if p.endswith(".md") and p not in hits:
                hits.append(p)
    return hits[:3]


# ---------------------------------------------------------------------------
# EXACT attribution: which commit moved THIS line.
#
# The window's commits are walked oldest-first and the pin's line is classified
# against each one in turn (the same difflib mapping the gate uses, so the
# answer is the gate's own notion of "moved").  The first commit at which the
# line number differs from the pin's is the commit that moved it.  A
# per-file-per-commit mapping is computed once and cached inside anchor_gate,
# so this costs one diff per (file, commit) pair, not one per reference.
#
# It can fail to attribute: if the line still reads its pin number at every
# commit in the window yet the gate calls it moved, the mapping is not a
# straight line-shift (a file resolved through the submodule, say), and the row
# is reported as UNATTRIBUTED rather than assigned to a commit by guesswork.
# ---------------------------------------------------------------------------
def window_commits(pin, path):
    log = git("log", "--reverse", "--format=%H\t%h\t%ad\t%s", "--date=short",
              "%s..HEAD" % pin, "--", path)
    out = []
    for line in log.splitlines():
        full, sha, date, subj = line.split("\t", 3)
        out.append((full, sha, date, subj))
    return out


def attribute(moved):
    """-> {(sha,date,subj): count}, unattributed rows."""
    per_commit, unattributed = collections.Counter(), []
    by_file = collections.defaultdict(list)
    for r in moved:
        by_file[(r[2], r[3])].append(r)       # (source file, pin)
    for (src, pin), rows in sorted(by_file.items()):
        wins = window_commits(pin, src)
        for r in rows:
            # A range anchor `:508-512` is ONE reference whose verdict is `moved`
            # when EITHER end moved, so both ends have to be asked; testing only
            # the first left six range references unattributed (measured).
            ends = [e for e in (r[4], r[8] if len(r) > 8 else None) if e is not None]
            hit = None
            for full, sha, date, subj in wins:
                for n in ends:
                    v, j, _s, _h = G.classify(pin, src, n, head=full)
                    if v == "no-file":
                        continue
                    if j != n:
                        hit = (sha, date, subj)
                        break
                if hit:
                    break
            if hit is None:
                unattributed.append(r)
            else:
                per_commit[hit] += 1
    return per_commit, unattributed


def main():
    out = sys.stdout
    if "--out" in sys.argv:
        out = open(sys.argv[sys.argv.index("--out") + 1], "w")

    rows, tally = collect()
    gate_tally, _hard = G.run()
    if dict(tally) != dict(gate_tally):
        print("FAIL  this reading does not reproduce the gate's tally", file=sys.stderr)
        print("  mine: %s" % dict(sorted(tally.items())), file=sys.stderr)
        print("  gate: %s" % dict(sorted(gate_tally.items())), file=sys.stderr)
        return 2

    print("== f119_moved: the gate's tally, reproduced by this reading ==", file=out)
    for k in sorted(tally):
        print("   %-24s %d" % (k, tally[k]), file=out)
    print("   (self-check: identical to anchor_gate.run() in the same process)", file=out)

    moved = [r for r in rows if r[6] in ("moved", "d14", "d14-moved")]
    hard = [r for r in rows if r[6] in ("GONE", "AT-PIN-BROKEN")]

    per_src = collections.Counter(r[2] for r in moved)
    print("\n== %d moved references, by the SOURCE FILE they cite (%d files) ==\n"
          % (len(moved), len(per_src)), file=out)

    # per grammar file, too: which contract carries the drift
    per_gram = collections.Counter(r[0] for r in moved)
    for k, v in sorted(per_gram.items(), key=lambda x: -x[1]):
        print("   %-46s %4d" % (os.path.basename(k), v), file=out)

    print("\n== the cited source files, with the commit window each was read against ==\n", file=out)
    for src, n in sorted(per_src.items(), key=lambda x: -x[1]):
        pins = sorted({r[3] for r in moved if r[2] == src})
        seen = []
        print("   %-58s moved %4d" % (src, n), file=out)
        for pin in pins:
            for _full, sha, date, subj in window_commits(pin, src):
                if sha in seen:
                    continue
                seen.append(sha)
                print("      %s %s  %s" % (sha, date, subj[:86]), file=out)
        if not seen:
            print("      (no commit touches it in the window)", file=out)
        print("", file=out)

    print("== EXACT attribution: the commit at which each moved line first moved ==\n", file=out)
    per_commit, unattributed = attribute(moved)
    total = sum(per_commit.values())
    for (sha, date, subj), n in sorted(per_commit.items(), key=lambda x: -x[1]):
        cites = delivery_of(sha)
        print("   %-9s %s  moved %4d  %s" % (sha, date, n, subj[:66]), file=out)
        if cites:
            print("             cited by: %s" % ", ".join(cites), file=out)
    print("\n   attributed %d of %d moved references; unattributed %d"
          % (total, len(moved), len(unattributed)), file=out)
    for r in unattributed:
        print("      UNATTRIBUTED %s %s -> %s:%s" % (os.path.basename(r[0]), r[1], r[2], r[4]),
              file=out)

    if hard:
        print("== the rows that are NOT moved and need a human ==\n", file=out)
        for path, jp, rp, pin, a, _j, v, txt in hard:
            print("   %-8s %s %s\n      %s:%s at pin %s\n      text at pin: %s"
                  % (v, os.path.basename(path), jp, rp, a, pin[:8],
                     (txt or "").strip()[:100]), file=out)
    if out is not sys.stdout:
        out.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
