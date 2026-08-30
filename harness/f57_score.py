#!/usr/bin/env python3
"""f57_score.py — the F57 corpus scorer (task F57, §11.173(d) / Q-55).

READ-ONLY over the corpus. Extracts each answered item from the two question
files and each SS entry from SCRIPT_SURFACE.md, joins them against the
PRE-COMMITTED classification in harness/artifacts/f57/f57_predictions.json
(md5 4d5b2486…), and reports the per-metric aggregates.

The per-part coverage judgments are NOT computed here — they are a reading, not
a regex. They live in harness/artifacts/f57/f57_coverage.tsv (one row per
pre-committed part, score 1.0/0.5/0.0 with a reason) and this script only
aggregates them, so the judgment and the arithmetic are separable and both
auditable.

Usage:  python3 harness/f57_score.py [--json]      (run from the harness repo root)
"""

import json
import os
import re
import statistics
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "harness", "artifacts", "f57")

UQ = os.path.join(ROOT, "USER_QUESTIONS.md")
R2 = os.path.join(ROOT, "USER_QUESTIONS_ROUND2.md")
SS = os.path.join(ROOT, "SCRIPT_SURFACE.md")
PRED = os.path.join(ART, "f57_predictions.json")
COV = os.path.join(ART, "f57_coverage.tsv")

HDR = re.compile(r"^\*\*(?P<id>[QR]\d+)\.")
REF = re.compile(r"^\*\(ref:")


def parse_questions(path):
    """-> {id: {'question': str, 'answer': str}}. An item runs from its **Qn./**Rn.
    header to the next header; inside it, the answer is the run of non-empty lines
    after the LAST *(ref: …)* line, up to the next blank line."""
    out = {}
    cur, buf = None, []
    with open(path, encoding="utf-8") as fh:
        lines = fh.read().split("\n")
    for ln in lines:
        m = HDR.match(ln)
        if m:
            if cur:
                out[cur] = buf
            cur, buf = m.group("id"), [ln]
        elif cur is not None:
            if ln.startswith("## ") or ln.startswith("# ") or ln.startswith("---"):
                out[cur] = buf
                cur, buf = None, []
            else:
                buf.append(ln)
    if cur:
        out[cur] = buf

    parsed = {}
    for k, buf in out.items():
        ref_idx = max((i for i, l in enumerate(buf) if REF.match(l)), default=None)
        if ref_idx is None:
            parsed[k] = {"question": "\n".join(buf).strip(), "answer": ""}
            continue
        ans = []
        for l in buf[ref_idx + 1:]:
            if l.strip() == "":
                if ans:
                    break
                continue
            ans.append(l.strip())
        parsed[k] = {
            "question": "\n".join(buf[:ref_idx]).strip(),
            "answer": " ".join(ans).strip(),
        }
    return parsed


def words(s):
    return len([t for t in s.split() if t.strip()])


def parse_ss(path):
    """-> {SS-n: entry text}. Entries are '- **SS-n** …' bullets running to the
    next SS bullet or section header."""
    out, cur, buf = {}, None, []
    with open(path, encoding="utf-8") as fh:
        for ln in fh.read().split("\n"):
            m = re.match(r"^- \*\*(SS-\d+)\*\*", ln)
            if m:
                if cur:
                    out[cur] = "\n".join(buf)
                cur, buf = m.group(1), [ln]
            elif cur is not None:
                if ln.startswith("## "):
                    out[cur] = "\n".join(buf)
                    cur, buf = None, []
                else:
                    buf.append(ln)
    if cur:
        out[cur] = "\n".join(buf)
    return out


def load_coverage():
    if not os.path.exists(COV):
        return {}
    cov = {}
    with open(COV, encoding="utf-8") as fh:
        for ln in fh:
            ln = ln.rstrip("\n")
            if not ln or ln.startswith("#"):
                continue
            f = ln.split("\t")
            if len(f) < 4:
                raise SystemExit("bad coverage row: %r" % ln)
            cov[(f[0], f[1])] = (float(f[2]), f[3])
    return cov


def main():
    pred = json.load(open(PRED, encoding="utf-8"))
    items = pred["items"]
    q1 = parse_questions(UQ)
    q2 = parse_questions(R2)
    corpus = dict(q1)
    corpus.update(q2)
    cov = load_coverage()
    ss = parse_ss(SS)

    rows = []
    for iid, meta in items.items():
        c = corpus.get(iid)
        if c is None:
            raise SystemExit("item %s in predictions but not found in corpus" % iid)
        kinds = [p["kind"] for p in meta["parts"]]
        rows.append({
            "id": iid,
            "ref": meta["ref"],
            "shape": meta["shape"],
            "nparts": len(meta["parts"]),
            "nE": kinds.count("E"),
            "qwords": words(c["question"]),
            "awords": words(c["answer"]),
            "aq": c["answer"].count("?"),
            "answer": c["answer"],
        })
    rows.sort(key=lambda r: (r["id"][0], int(r["id"][1:])))

    print("== PER-ITEM (answer word counts; question shapes from the pre-committed table) ==")
    print("%-5s %-9s %-5s %5s %3s %6s %6s %3s" %
          ("id", "ref", "shape", "parts", "E", "qwords", "awords", "?"))
    for r in rows:
        print("%-5s %-9s %-5s %5d %3d %6d %6d %3d" %
              (r["id"], r["ref"], r["shape"], r["nparts"], r["nE"],
               r["qwords"], r["awords"], r["aq"]))

    r1 = [r for r in rows if r["id"].startswith("Q")]
    ebearing = [r for r in rows if r["nE"] > 0]
    pured = [r for r in rows if r["nE"] == 0 and r["nparts"] > 0]

    def stat(name, rs):
        if not rs:
            print("  %-28s n=0" % name)
            return
        w = [r["awords"] for r in rs]
        print("  %-28s n=%2d  median=%5.1f  mean=%5.1f  min=%d max=%d"
              % (name, len(rs), statistics.median(w), statistics.mean(w), min(w), max(w)))

    print("\n== m1 LENGTH ==")
    stat("round 1 (Q1-27)", r1)
    stat("all items with a question", [r for r in rows if r["nparts"] > 0])
    stat("E-bearing items", ebearing)
    stat("pure-D items", pured)
    if ebearing and pured:
        ratio = statistics.median([r["awords"] for r in ebearing]) / \
            statistics.median([r["awords"] for r in pured])
        print("  E-bearing / pure-D median ratio = %.3f   [P1.2 threshold 1.5]" % ratio)

    print("\n== m1 COVERAGE (from f57_coverage.tsv; blank if not yet scored) ==")
    if cov:
        byk = {"D": [], "E": []}
        missing = []
        for iid, meta in items.items():
            for p in meta["parts"]:
                key = (iid, p["id"])
                if key not in cov:
                    missing.append(key)
                else:
                    byk[p["kind"]].append(cov[key][0])
        if missing:
            print("  MISSING coverage rows: %s" % missing)
        for k in ("D", "E"):
            v = byk[k]
            if v:
                print("  %s-parts  n=%2d  coverage=%.3f  (dropped=%d partial=%d full=%d)"
                      % (k, len(v), sum(v) / len(v), v.count(0.0), v.count(0.5), v.count(1.0)))
        if byk["D"] and byk["E"]:
            gap = (sum(byk["D"]) / len(byk["D"]) - sum(byk["E"]) / len(byk["E"])) * 100
            print("  D−E coverage gap = %.1f pp   [P1.3 threshold 20 pp]" % gap)
    else:
        print("  (no coverage file yet)")

    if cov:
        def cvg(pred_ids=None, kinds=("D", "E"), rnd=None):
            num = den = 0.0
            for iid, meta in items.items():
                if rnd and not iid.startswith(rnd):
                    continue
                if pred_ids is not None and iid not in pred_ids:
                    continue
                for p in meta["parts"]:
                    if p["kind"] in kinds:
                        num += cov[(iid, p["id"])][0]
                        den += 1
            return num, den

        print("\n== m1 COVERAGE, conditioned ==")
        for label, rnd in (("round 1 (Q)", "Q"), ("round 2 (R)", "R")):
            for k in (("D",), ("E",), ("D", "E")):
                n_, d_ = cvg(kinds=k, rnd=rnd)
                if d_:
                    print("  %-12s %-4s  %5.1f/%-3d = %.3f"
                          % (label, "+".join(k), n_, d_, n_ / d_))
        print("\n== m1 COVERAGE by PARTS-PER-ITEM (round 1 — the budget law) ==")
        buckets = {}
        for iid, meta in items.items():
            if not iid.startswith("Q") or not meta["parts"]:
                continue
            k = len(meta["parts"])
            k = "1" if k == 1 else ("2" if k == 2 else "3-4")
            s = sum(cov[(iid, p["id"])][0] for p in meta["parts"])
            a, b = buckets.get(k, (0.0, 0))
            buckets[k] = (a + s, b + len(meta["parts"]))
        for k in ("1", "2", "3-4"):
            if k in buckets:
                a, b = buckets[k]
                print("  %-4s part(s)/item: %5.1f/%-3d = %.3f" % (k, a, b, a / b))
        print("\n== m1 COVERAGE by QUESTION LENGTH (round 1, terciles) ==")
        qs = sorted([r for r in rows if r["id"].startswith("Q")], key=lambda r: r["qwords"])
        for i, name in ((0, "short"), (1, "mid"), (2, "long")):
            chunk = qs[i * 9:(i + 1) * 9]
            a = sum(cov[(r["id"], p["id"])][0] for r in chunk
                    for p in items[r["id"]]["parts"])
            b = sum(len(items[r["id"]]["parts"]) for r in chunk)
            print("  %-6s qwords %3d-%3d: %5.1f/%-3d = %.3f"
                  % (name, chunk[0]["qwords"], chunk[-1]["qwords"], a, b, a / b))
        print("\n== m3 HISTORY-CLASS parts inside the Q/R corpus ==")
        hist = [("Q16", "b")]
        a = sum(cov[k][0] for k in hist)
        print("  Q/R history parts: %.1f/%d = %.3f (SCRIPT_SURFACE history entries scored separately)"
              % (a, len(hist), a / len(hist)))

    print("\n== m4 QUESTIONS ASKED BACK (mechanical: '?' inside an answer) ==")
    tot = 0
    for r in rows:
        if r["aq"]:
            tot += r["aq"]
            print("  %s: %d  -> %s" % (r["id"], r["aq"], r["answer"]))
    print("  total '?' in answers = %d" % tot)

    print("\n== m2 SCRIPT_SURFACE entries ==")
    print("  parsed %d SS entries: %s" % (len(ss), " ".join(sorted(ss, key=lambda s: int(s[3:])))))

    if "--json" in sys.argv:
        out = {"rows": rows, "coverage": {"%s.%s" % k: v for k, v in cov.items()}}
        with open(os.path.join(ART, "f57_scores.json"), "w", encoding="utf-8") as fh:
            json.dump(out, fh, indent=1, ensure_ascii=False)
        print("\nwrote %s" % os.path.join(ART, "f57_scores.json"))


if __name__ == "__main__":
    main()
