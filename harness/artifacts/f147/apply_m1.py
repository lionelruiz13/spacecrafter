#!/usr/bin/env python3
"""F147: M1, the one modification commit that precedes every move - applied BY CONTENT ANCHORS.

usage: python3 harness/artifacts/f147/apply_m1.py IN OUT
Reads IN (fable-dispatch.md as it stands), writes OUT; never edits in place (the supervisor decides
what OUT becomes). Each anchor must match EXACTLY ONCE, or it prints FAIL and writes nothing.
The acts (no line is removed; three single spaces become line breaks and the two lines of §0b step 3 are rewritten; everything else is insertion):
  1. the one-line form of each whole protocol body inserted on the line BEFORE that body (8 bodies);
  2. §0b step 1: the drift-assert sentence split onto its own lines (two single spaces become
     newline + the step's 3-space indent) and its one-line form inserted before it;
  3. §0b step 3: "the binding / SUPERSESSION BLOCK (template below, values refreshed), " re-pointed;
  4. §1: the regenerable sentence inserted before the derived index, and line "unanswered).* **Session-25"
     split (its single space becomes a newline);
  5. the live list (the census's section 6 minus its [s42] lines) appended at the end of the file.
Its expected output on the pinned file is plan-out/post-M1.md (md5 recorded in plan-out/units.json)."""
import hashlib, importlib.util, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("plan_text", os.path.join(HERE, "plan.py"))
src = open(os.path.join(HERE, "plan.py"), encoding="utf-8").read()
ns = {}
exec(src.split("# the protocol bodies at the PINNED state")[0].split("# ---------------------------------------------------------------- the one-line forms (Reason > Rule)")[1], ns)
REPL, INDEX_SENTENCE = ns["REPL"], ns["INDEX_SENTENCE"]
spec2 = importlib.util.spec_from_file_location("ll", os.path.join(HERE, "livelist.py")); ll = importlib.util.module_from_spec(spec2); spec2.loader.exec_module(ll)

BODY_FIRST = {  # the first line of each whole body (exact), and the id whose one-line form goes before it
 "protocol-iso8859": "   - Loaded data `~/.spacecrafter/ssystem.ini` is ISO-8859 and untracked — the Bash",
 "protocol-instance-assert": "   - **Concurrent-instance assert (2026-07-31, §11.121(m)):** before each measurement",
 "protocol-session-env": "   - **Session-environment hazards (2026-08-09, F28/F30, §11.138/§11.140(i)):**",
 "protocol-display-arch": "   - **Display architecture is part of the instrument (2026-07-31, §11.122(o) +",
 "protocol-canary": "   - **ENVIRONMENT CANARY (2026-08-30, §11.176; enumerated §0.7 precondition per",
 "protocol-backmarker": "   - **Back-marker at the write (2026-08-29, §11.161(g), RA-MODEL E18/E55):** any",
 "protocol-routing": "(a) ~~**Executors are Fable**: agent type `claude`, model `fable` — the `opus-xhigh`",
}


def fail(msg):
    print("FAIL", msg); sys.exit(1)


def main(inp, outp):
    L = open(inp, "rb").read().decode("utf-8").split("\n")
    def one(pred, what):
        hits = [i for i, x in enumerate(L) if pred(x)]
        if len(hits) != 1:
            fail(f"{what}: {len(hits)} matches")
        return hits[0]
    # 4b (split first, bottom-up so indices above stay valid): §1 line 484
    i = one(lambda x: x.startswith("unanswered).* **Session-25 round (2026-09-06):"), "§1 split anchor")
    a, b = L[i].split(" **Session-25 round", 1)
    L[i:i + 1] = [a, "**Session-25 round" + b]
    # 4a: the sentence before the derived index
    i = one(lambda x: x.startswith("*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F36 all"), "§1 index head")
    L[i:i] = [INDEX_SENTENCE]
    # 3: step 3 re-point
    i = one(lambda x: x.endswith("Every prompt carries: the binding"), "step 3 line")
    if not L[i + 1].startswith("   SUPERSESSION BLOCK (template below, values refreshed), the task-section pointer +"):
        fail("step 3 continuation")
    L[i] = L[i][: -len("the binding")] + "the per-round variables (step 1:"
    L[i + 1] = "   no supersession block - the definition is generated from its authority), the task-section pointer +"
    # 2: the drift-assert split + its one-line form
    i = one(lambda x: x.endswith(" **Ledger instruments (2026-09-06, the miss-ledger item — two") and "always fail.]**" in x, "drift end")
    a, b = L[i].split(" **Ledger instruments", 1)
    L[i:i + 1] = [a, "   **Ledger instruments" + b]
    i = one(lambda x: x.endswith(" **Definition-drift assert") and x.startswith('   "clean expected"'), "drift start")
    a, b = L[i].split(" **Definition-drift assert", 1)
    L[i:i + 1] = [a, REPL["protocol-drift-assert"], "   **Definition-drift assert" + b]
    # 1: the one-line forms before their whole bodies
    for pid, first in BODY_FIRST.items():
        i = one(lambda x, f=first: x == f, pid)
        L[i:i] = [REPL[pid]]
    # 5: the live list at the end
    if L[-1] != "":
        fail("file does not end with a newline")
    fence = ["- **The open asks of the earlier lists (sessions 4 → 40), carried by the census of 2026-10-11 (F147, §11.318) - every other item of those lists is closed at a ledger node, and its record left with its block (`fable-dispatch/archive/forvixy-*.md`; the verdict per item: `fable-dispatch/census-2026-10-11.md`):**",
             "", "  ```", "  YOURS — ONE LIST (held set per node; the entry's section in brackets)",
             "  tags: [N+] the item's own node states the ask · [N-] the node holds only the open state, the ask's text is here alone",
             "        [T] routed to the tester, through you · [D] a default that queued or running work builds unless you rule"]
    for head, items in ll.G:
        rows = [(k, t, p, tag) for k, t, p, tag in items if not k.startswith("C66 ")]
        if rows:
            fence.append("  " + head + ":")
            fence.extend(f"    {t} -> {p} [{k}] {ll.TAGS[tag]}" for k, t, p, tag in rows)
    fence.append("  ```")
    L = L[:-1] + fence + [""]
    out = "\n".join(L).encode("utf-8")
    open(outp, "wb").write(out)
    print(f"M1 written to {outp}: {len(L) - 1} lines, {len(out)} B, md5 {hashlib.md5(out).hexdigest()}")


if __name__ == "__main__":
    if len(sys.argv) != 3 or os.path.abspath(sys.argv[1]) == os.path.abspath(sys.argv[2]):
        print(__doc__); sys.exit(2)
    main(sys.argv[1], sys.argv[2])
