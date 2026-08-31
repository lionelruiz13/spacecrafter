#!/usr/bin/env python3
"""F66 — does scedit's `--search` rank the way the measured baseline ranks?

    cd claude/harness && ./f66_search_parity.py [--scedit PATH] [--limit N]

WHY THIS EXISTS. F64 measured the documentation-router role on 340 real
(comment -> command) questions mined from doc/superscript.sts, against a
model-free bag-of-words control that scored 80/340. F66 moved that control INTO
scedit, in C++, over the same contract file — because a ranking that only exists
in a throwaway Python script cannot be a product surface, and a second Python
reader of the grammar is the I2 defect this task exists to remove. This script
is the check that the move changed nothing: question by question, scedit's
top-ranked command must be the command the baseline picks.

WHO ANSWERS WHAT. The Python side supplies the QUESTIONS and the CONTROL
VERDICT, both imported from f64_doc_router.py (one witness parser, one formula,
no copy). scedit answers, as a subprocess, through the surface a machine
consumer actually uses: `--search --scope commands --limit 1`. Nothing here
parses the grammar file and nothing here answers a query.

THE ONE STATED DIFFERENCE. The baseline picks a command for EVERY question,
including the 21 that share no word with any command's name or doc line: with
every score at zero the first candidate wins, so the "answer" is whichever
command the file lists first. scedit refuses to call that an answer and returns
an empty result set instead (sc_docjson.hpp). The check therefore reads:

    baseline best score > 0  ->  scedit's top-1 == the baseline's pick
    baseline best score == 0 ->  scedit returns NO result

which is a criterion every one of the 340 questions can fail, in both
directions. Both hit rates are printed: the baseline's 80/340 is the F64 number
this port must reproduce, and it does, because the 21 degenerate picks were hits
zero times.
"""
import argparse, importlib.util, json, subprocess, sys, time
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
GRAMMAR = REPO / "util/scedit/grammar/sc-grammar.json"
OUT = HARNESS / "artifacts/f66"


def load_f64():
    """Import the witness pairs and the baseline scorer (side-effect free)."""
    spec = importlib.util.spec_from_file_location("f64_doc_router", HARNESS / "f64_doc_router.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def scedit_top1(scedit, query):
    """scedit's best COMMAND page for a question, or None when it has none."""
    r = subprocess.run([scedit, "--grammar", str(GRAMMAR), "--search",
                        "--scope", "commands", "--limit", "1", query],
                       capture_output=True, text=True)
    if r.returncode != 0 or r.stderr:
        raise SystemExit(f"scedit --search failed (rc={r.returncode}) on {query!r}: {r.stderr}")
    out = json.loads(r.stdout)
    results = out["results"]
    return (results[0]["command"], results[0]["score"]) if results else (None, None)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scedit", default=str(REPO / "util/scedit/build-lovely/scedit"))
    ap.add_argument("--limit", type=int, default=0, help="first N questions only (a smoke run)")
    a = ap.parse_args()

    f64 = load_f64()
    pairs = f64.pairs[:a.limit] if a.limit else f64.pairs
    print(f"{len(pairs)} witness questions from doc/superscript.sts; scedit = {a.scedit}")

    rows, agree, degenerate, mismatches = [], 0, 0, []
    base_hits = sc_hits = 0
    t0 = time.time()
    for p in pairs:
        pick, score = f64.baseline_scored(p["q"])
        sc_pick, sc_score = scedit_top1(a.scedit, p["q"])
        if score == 0:
            degenerate += 1
            ok = sc_pick is None
        else:
            ok = sc_pick == pick
        agree += ok
        base_hits += f64.canon(pick) == f64.canon(p["target"])
        sc_hits += sc_pick is not None and f64.canon(sc_pick) == f64.canon(p["target"])
        row = {"line": p["line"], "q": p["q"], "target": p["target"],
               "baseline": pick, "baseline_score": score, "scedit": sc_pick,
               "scedit_score": sc_score, "agree": bool(ok)}
        rows.append(row)
        if not ok:
            mismatches.append(row)

    n = len(pairs)
    print(f"agreement          {agree}/{n}")
    print(f"  of which the empty-answer rule: {degenerate} questions share no word with any command")
    print(f"baseline hit rate  {base_hits}/{n} = {100*base_hits/n:.1f}%   (F64 recorded 80/340 = 23.5%)")
    print(f"scedit hit rate    {sc_hits}/{n} = {100*sc_hits/n:.1f}%")
    print(f"wall {time.time()-t0:.1f} s")
    for m in mismatches[:10]:
        print(f"  MISMATCH sts:{m['line']} {m['q'][:60]!r}\n"
              f"           baseline {m['baseline']} ({m['baseline_score']:.4f}) vs scedit {m['scedit']}")
    if len(mismatches) > 10:
        print(f"  ... and {len(mismatches)-10} more")

    OUT.mkdir(parents=True, exist_ok=True)
    summary = {"questions": n, "agreement": agree, "degenerate": degenerate,
               "baseline_hits": base_hits, "scedit_hits": sc_hits,
               "mismatches": mismatches, "scedit": a.scedit}
    (OUT / "search_parity.json").write_text(json.dumps({"summary": summary, "rows": rows}, indent=1))
    print(("PARITY GREEN" if agree == n else "PARITY RED") +
          f" — rows in {OUT/'search_parity.json'}")
    return 0 if agree == n else 1


if __name__ == "__main__":
    sys.exit(main())
