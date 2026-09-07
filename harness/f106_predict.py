#!/usr/bin/env python3
"""f106_predict.py -- predict, from the GRAPH alone, which commits a
supervised-by.sh run rewrites once the --commit-filter of F106 is in place.

The rule the script implements, restated here so the prediction is derived and
not copied from a run:

  * a commit is SELECTED exactly as `scan_range` selects it (the same identity
    grammar, the same wildcard resolution, the same Supervised-By deselection);
  * a selected commit is REBUILT (its message gains Supervised-By, and/or its
    author is resolved from the wildcard);
  * a harness commit whose `Code: <branch> @ <sha>` trailer prefixes a code
    commit that changed sha is REBUILT too -- transform (1) of the msg-filter
    is independent of selection;
  * ANY commit, selected or not, whose MAPPED PARENTS differ from its original
    parents is REBUILT -- so the rebuild propagates downstream through the
    range, and only there;
  * every other commit keeps its object, hence its sha and its `gpgsig`.

Usage:
    f106_predict.py code    <repo> <range>
    f106_predict.py harness <repo> <range> --code-map <old-full-sha>...

Output is a report on stdout; `--tsv <file>` also writes `<sha>\tREBUILT|KEPT`.
"""
import argparse
import re
import subprocess
import sys

MODEL_RE = re.compile(r"Claude [A-Za-z]+ [0-9]+(?:[.][0-9]+)*")
WILDCARD_AUTHOR = "Claude"
CODE_TRAILER_RE = re.compile(
    r"^Code:[ \t]+([^ \t]+)[ \t]+@[ \t]+([0-9a-f]{7,40})[ \t]*$", re.M)


def git(repo, *args):
    return subprocess.run(["git", "-C", repo] + list(args),
                          check=True, capture_output=True, text=True).stdout


def identity(s):
    m = MODEL_RE.search(s or "")
    return m.group(0) if m else ""


def resolve_wildcard(msg):
    seen = set()
    for line in msg.splitlines():
        if line.lower().startswith("co-authored-by:"):
            i = identity(line.split(":", 1)[1])
            if i:
                seen.add(i)
    return next(iter(seen)) if len(seen) == 1 else ""


def has_supervisor(msg):
    return any(l.startswith("Supervised-By:") for l in msg.splitlines())


def read_range(repo, rng):
    """[(sha, author-name, [parents], body)] in the range, in git log order."""
    raw = git(repo, "log", "-z", "--format=%H\x02%an\x02%P\x02%B", rng)
    out = []
    for rec in raw.split("\0"):
        if not rec.strip():
            continue
        sha, an, par, body = rec.split("\x02", 3)
        out.append((sha, an, par.split(), body))
    return out


def classify(commits):
    """-> {sha: (selected, reason)}"""
    verdict = {}
    for sha, an, _par, body in commits:
        wild = 0
        if an == WILDCARD_AUTHOR:
            an = resolve_wildcard(body)
            if not an:
                verdict[sha] = (False, "MANUAL: bare wildcard, unresolvable")
                continue
            wild = 1
        aid = identity(an)
        if not aid:
            verdict[sha] = (False, "not a Claude identity (%s)" % an)
            continue
        if has_supervisor(body) and not wild:
            verdict[sha] = (False, "already carries Supervised-By")
            continue
        verdict[sha] = (True, "selected (author %s%s)" % (aid, ", wildcard" if wild else ""))
    return verdict


def map_lookup_count(tok, olds):
    """how many map OLD shas the token prefixes (>1 == AMBIG in the script)."""
    return sum(1 for o in olds if o.startswith(tok))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("side", choices=["code", "harness"])
    ap.add_argument("repo")
    ap.add_argument("range")
    ap.add_argument("--code-map", nargs="*", default=[],
                    help="OLD full shas that the code map will carry")
    ap.add_argument("--tsv")
    args = ap.parse_args()

    commits = read_range(args.repo, args.range)
    verdict = classify(commits)
    in_range = {c[0] for c in commits}

    remapped = {}
    if args.side == "harness":
        for sha, _an, _par, body in commits:
            for _br, tok in CODE_TRAILER_RE.findall(body):
                n = map_lookup_count(tok, args.code_map)
                if n == 1:
                    remapped[sha] = tok
                elif n > 1:
                    remapped[sha] = "AMBIG:" + tok

    # topological closure: parents before children.
    order = subprocess.run(
        ["git", "-C", args.repo, "rev-list", "--topo-order", "--reverse", args.range],
        check=True, capture_output=True, text=True).stdout.split()
    parents = {sha: par for sha, _an, par, _b in commits}
    rebuilt, why = {}, {}
    for sha in order:
        sel = verdict[sha][0]
        rem = sha in remapped
        inherited = [p for p in parents[sha] if p in in_range and rebuilt.get(p)]
        r = sel or rem or bool(inherited)
        rebuilt[sha] = r
        if sel:
            why[sha] = verdict[sha][1]
        elif rem:
            why[sha] = "Code: trailer remapped (%s)" % remapped[sha]
        elif inherited:
            why[sha] = "parent %s rebuilt" % inherited[0][:8]
        else:
            why[sha] = "KEPT -- " + verdict[sha][1]

    n_sel = sum(1 for s in in_range if verdict[s][0])
    n_reb = sum(1 for s in in_range if rebuilt[s])
    print("== %s  %s  %s" % (args.side, args.repo, args.range))
    print("   commits in range        : %d" % len(commits))
    print("   selected (Claude author): %d" % n_sel)
    if args.side == "harness":
        print("   Code: trailer remapped  : %d (of which AMBIG %d)"
              % (len(remapped),
                 sum(1 for v in remapped.values() if str(v).startswith("AMBIG"))))
    print("   REBUILT (sha changes)   : %d" % n_reb)
    print("   KEPT    (object intact) : %d" % (len(commits) - n_reb))
    print()
    print("   unselected but REBUILT (the downstream/remap closure):")
    any_u = False
    for sha in order:
        if rebuilt[sha] and not verdict[sha][0]:
            any_u = True
            subj = git(args.repo, "log", "-1", "--format=%an | %s", sha).strip()
            print("     %s  %-34s  %s" % (sha[:8], why[sha][:34], subj[:56]))
    if not any_u:
        print("     (none)")
    print()
    print("   KEPT commits:")
    for sha in order:
        if not rebuilt[sha]:
            subj = git(args.repo, "log", "-1", "--format=%an | %s", sha).strip()
            sig = git(args.repo, "cat-file", "commit", sha).split("\n\n", 1)[0]
            gpg = "gpgsig" if "\ngpgsig" in "\n" + sig else "-"
            print("     %s  %-6s  %s" % (sha[:8], gpg, subj[:64]))
    if args.tsv:
        with open(args.tsv, "w") as fh:
            for sha in order:
                fh.write("%s\t%s\n" % (sha, "REBUILT" if rebuilt[sha] else "KEPT"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
