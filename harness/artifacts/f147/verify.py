#!/usr/bin/env python3
"""F147 check (d): every quoted ruling line reproduces at its pointer.

For every census table row (`| # | item | ask | verdict | pointer | quoted line |`):
  - pointer = `path:LINE` (path relative to the harness root, or ~/shared/...), quote = "..."
    -> the quote (outer double quotes stripped) must be a substring of line LINE of path,
       and `grep -n -F` semantics hold (the same line number is among the fixed-string hits);
  - quote = "-" or "(Cnn #k)" -> no check (CARRIED rows carry their searches in the pointer cell;
    a back-reference row shares the verdict of the row it names, which must exist).
Also counts verdicts. usage: verify.py FILE_OR_DIR...
"""
import glob, os, re, sys

ROOT = "/home/claude/spacecrafter/claude"
SH = "/home/claude/shared"


def resolve(p):
    if p.startswith("~/shared/"):
        return SH + "/" + p[len("~/shared/"):]
    return f"{ROOT}/{p}"


def cells(line):
    # split on | not inside backquotes
    out, cur, tick = [], "", False
    for ch in line.strip()[1:-1] if line.strip().endswith("|") else line.strip()[1:]:
        if ch == "`":
            tick = not tick
        if ch == "|" and not tick:
            out.append(cur.strip()); cur = ""
        else:
            cur += ch
    out.append(cur.strip())
    return out


def main(args):
    files = []
    for a in args:
        files += sorted(glob.glob(os.path.join(a, "C*.md"))) if os.path.isdir(a) else [a]
    ok = bad = skipped = 0
    rows_by_ref = {}
    verdicts = {}
    allrows = []
    for f in files:
        block = None
        for ln, line in enumerate(open(f, encoding="utf-8"), 1):
            m = re.match(r"### (C\d+)", line)
            if m:
                block = m.group(1)
            if not line.startswith("| ") or line.startswith("| # ") or line.startswith("|---"):
                continue
            if block is None:  # summary tables above the first block heading are not census rows
                continue
            c = cells(line)
            if len(c) < 6:
                print(f"MALFORMED {f}:{ln}: {len(c)} cells")
                bad += 1
                continue
            num, item, ask, verdict, pointer, quote = c[:6]
            rows_by_ref[f"{block} #{num}"] = verdict
            allrows.append((f, ln, block, num, verdict, pointer, quote))
    for f, ln, block, num, verdict, pointer, quote in allrows:
        v = verdict.split()[0] if verdict else "?"
        verdicts[v] = verdicts.get(v, 0) + 1
        ref = re.fullmatch(r"\((C\d+ #\d+)\)", quote)
        if quote == "-" or ref:
            if ref and ref.group(1) not in rows_by_ref:
                print(f"DANGLING REF {f}:{ln} {block} #{num} -> {ref.group(1)}")
                bad += 1
            elif ref and rows_by_ref[ref.group(1)].split()[0] != v:
                print(f"VERDICT MISMATCH {f}:{ln} {block} #{num} {v} vs {ref.group(1)} {rows_by_ref[ref.group(1)].split()[0]}")
                bad += 1
            skipped += 1
            continue
        pm = re.match(r"(~/shared/\S+?|[A-Za-z0-9_./-]+?):(\d+)$", pointer.split()[0])
        if not (quote.startswith('"') and quote.endswith('"')) or not pm:
            print(f"UNCHECKABLE {f}:{ln} {block} #{num}: pointer={pointer[:60]!r} quote={quote[:60]!r}")
            bad += 1
            continue
        path, lno = resolve(pm.group(1)), int(pm.group(2))
        q = quote[1:-1]
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
        except OSError as e:
            print(f"NOFILE {f}:{ln} {block} #{num}: {path}")
            bad += 1
            continue
        if lno < 1 or lno > len(lines) or q not in lines[lno - 1]:
            hits = [i + 1 for i, l in enumerate(lines) if q in l]
            print(f"MISS {f}:{ln} {block} #{num}: {pm.group(1)}:{lno} quote not on that line; fixed-string hits at {hits[:5]}")
            bad += 1
        else:
            ok += 1
    uniq = {}
    for f, ln, block, num, verdict, pointer, quote in allrows:
        if "(= C" in verdict:
            continue
        v = verdict.split()[0]
        uniq[v] = uniq.get(v, 0) + 1
    tot = sum(uniq.values())
    print(f"quotes verified {ok}, failed {bad}, refs/carried {skipped}; rows {len(allrows)} (occurrences) verdicts {dict(sorted(verdicts.items()))}")
    print(f"UNIQUE items {tot}: {dict(sorted(uniq.items()))}; CARRIED share {100.0*uniq.get('CARRIED',0)/max(tot,1):.1f} %")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
