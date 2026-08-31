#!/usr/bin/env python3
"""F64 — can a small local model route a request to the right command page?

    cd claude/harness && ./f64_doc_router.py [model ...]      # default: the present gemma3:4b
    ./f64_doc_router.py --limit 60 gemma3:1b llama3.2:3b     # quick pass

The documentation-helper role Vixy named [2026-08-31]: "gate which
documentation page and command to show given what the user asked". Measured
as a routing task over the command surface scedit already holds as data
(util/scedit/grammar/sc-grammar.json: 65 accepted commands, each with its
zero-knowledge one-liner).

QUESTION SET — not written for this test: doc/superscript.sts's own comment
lines, the ones an author wrote directly above a command to say what it does
("Increment audio volume" → `audio`, "Back to today but keeping hour" →
`date`). 340 pairs at HEAD, usage-shaped (flag 91, set 31, body 25, image 22,
media 21 …). A hit is the target command or one of its aliases (`div`/`divide`
are the same page).

CONTROL — a no-model baseline: bag-of-words overlap between the request and
each command's name + doc line, best match wins. If the model does not beat
it clearly, the role does not need a model; if it does, the delta is what the
model buys. Both are scored on the same pairs.

The model sees the catalogue (name: doc) in the system prompt and the request
as the user turn; temperature 0; it must answer with the command name only.
Per model: hit rate, per-target hit rate for the frequent targets, the
confusions, mean latency (ollama's total_duration) — served by the local
ollama (http://localhost:11434, native /api/chat).

Hardware this ran on [measured]: TravellingFoxDev, 15 GiB RAM, GTX 1660 Ti
6144 MiB VRAM — a 4B model at Q4 fits; a 12B one does not fit whole.

IMPORTABLE [2026-08-31, F66]: the witness pairs and the baseline scorer are
module level and side-effect free, so f66_search_parity.py can IMPORT them
rather than carry a second copy of the parser and the formula (I2). Everything
that reads argv, prints, calls a model or writes an artifact lives in main() and
runs only when this file is executed. The numbers are unchanged: importing and
calling baseline() over `pairs` reproduces 80/340.
"""
import collections, functools, json, re, sys, time, urllib.error, urllib.request
print = functools.partial(print, flush=True)
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
OUT = HARNESS / "artifacts/f64"
OLLAMA = "http://localhost:11434"

# ---- the catalogue, from the contract file (one authority)
g = json.load(open(REPO / "util/scedit/grammar/sc-grammar.json"))
cmds = {k: v for k, v in g["families"]["commands"].items() if not k.startswith("_")}
alias_of = {k: v.get("alias_of") for k, v in cmds.items() if v.get("alias_of")}
def canon(name): return alias_of.get(name, name)
fam_of = {"flag": "flags", "set": "set_names", "color": "color_names"}
def family_names(fam):
    names = g["families"][fam].get("names", [])
    out = []
    for n in names:
        if isinstance(n, str): out.append((n, ""))
        elif isinstance(n, dict) and n.get("name"): out.append((n["name"], n.get("doc") or ""))
    return out
def build_prompt(MEMBERS):
  """The system prompt for one catalogue level. Verbatim as it was measured."""
  if not MEMBERS:
    catalogue = "\n".join(f"{k}: {v['doc']}" for k, v in cmds.items())
    SYSTEM = ("You are the documentation router of spacecrafter, a planetarium program driven by scripts. "
              "The user says what they want to do; you name the ONE command whose documentation page to show. "
              "Commands, one per line as `name: what it does`:\n\n" + catalogue +
              "\n\nAnswer with the command name only - one word, nothing else.")
  else:
      # Two levels: a page is a command, or a command + the family member it
      # names (`flag constellation_drawing`, `set planet_scale`, `color
      # property nebula_names`). Member names are the engine's own; a doc line
      # is shown where the file has one (set_names is the v2 family today).
      lines_ = []
      for k, v in cmds.items():
          lines_.append(f"{k}: {v['doc']}")
          if k in fam_of:
              for name, doc in family_names(fam_of[k]):
                  lines_.append(f"  {k} {name}" + (f": {doc}" if doc else ""))
      catalogue = "\n".join(lines_)
      SYSTEM = ("You are the documentation router of spacecrafter, a planetarium program driven by scripts. "
                "The user says what they want to do; you name the ONE documentation page to show. A page is a "
                "command, or a command followed by one of the names listed under it (indented). Pages:\n\n"
                + catalogue + "\n\nAnswer with the page only: the command name, or the command name and the "
                "member name separated by a space. Nothing else.")
  return SYSTEM

# ---- the pairs, from the witness
lines = open(REPO / "doc/superscript.sts", "rb").read().decode("latin-1").splitlines()
pairs, block = [], []
for i, raw in enumerate(lines, 1):
    s = raw.rstrip("\r").strip()
    if not s: block = []; continue
    if s.startswith("#"):
        t = s.lstrip("#").strip()
        if t and not set(t) <= set("-=*_ "): block.append(t)
        continue
    cmd = s.split()[0].lower()
    if block and cmd in cmds:
        toks = s.split(); member = ""
        if cmd in ("flag", "set") and len(toks) > 1: member = toks[1].lower()
        if cmd == "color" and "property" in toks: member = toks[toks.index("property") + 1].lower() if toks.index("property") + 1 < len(toks) else ""
        pairs.append({"line": i, "q": " ".join(block), "target": cmd, "member": member})
    block = []

# ---- the no-model baseline
def words(s): return set(w for w in re.findall(r"[a-z][a-z_]+", s.lower()) if len(w) > 2)
doc_words = {k: words(k.replace("_", " ") + " " + v["doc"]) for k, v in cmds.items()}
def baseline_scored(q):
    """(pick, score). The score matters to a consumer: a best score of 0 means
    no command shares a word with the request, and the pick is then whichever
    command comes first in the file rather than an answer [F66]."""
    qw = words(q); best, score = None, -1
    for k, dw in doc_words.items():
        sc = len(qw & dw) / (1 + len(dw) ** 0.5)
        if sc > score: best, score = k, sc
    return best, score
def baseline(q): return baseline_scored(q)[0]

THINK_OK = {}   # model -> whether the server accepts "think": false for it
def ask(model, q, SYSTEM):
    # A model with the "thinking" capability would spend the answer budget
    # thinking and return empty content: ask for no thinking, and fall back
    # to the plain request for a model whose server rejects the field.
    for think in ([False, None] if THINK_OK.get(model, True) else [None]):
        req_body = {"model": model, "stream": False,
                    "messages": [{"role": "system", "content": SYSTEM}, {"role": "user", "content": q}],
                    "options": {"temperature": 0, "num_predict": 12, "num_ctx": 8192}}
        if think is False: req_body["think"] = False
        req = urllib.request.Request(OLLAMA + "/api/chat", data=json.dumps(req_body).encode(),
                                     headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=600) as r:
                THINK_OK[model] = think is False
                return json.load(r)
        except urllib.error.HTTPError as e:
            if think is False and e.code == 400: THINK_OK[model] = False; continue
            raise

def main(argv):
    """Everything that reads argv, prints, calls a model or writes a file."""
    global pairs
    OUT.mkdir(parents=True, exist_ok=True)
    args = [a for a in argv[1:] if not a.startswith("--")]
    limit = None
    for a in argv[1:]:
        if a.startswith("--limit"): limit = int(a.split("=")[1]) if "=" in a else int(argv[argv.index(a) + 1])
    if limit and str(limit) in args: args.remove(str(limit))
    models = args or ["gemma3:4b"]
    MEMBERS = "--members" in argv
    if MEMBERS: args = [a for a in args if a != "--members"]; models = args or ["gemma4:latest"]
    SYSTEM = build_prompt(MEMBERS)
    if limit: pairs = pairs[:limit]
    print(f"{len(pairs)} pairs from doc/superscript.sts; catalogue {len(cmds)} commands")
    base_hits = sum(canon(baseline(p["q"])) == canon(p["target"]) for p in pairs)
    print(f"baseline (bag-of-words): {base_hits}/{len(pairs)} = {100*base_hits/len(pairs):.1f}%")
    summary = {"pairs": len(pairs), "baseline_hits": base_hits, "models": {}}
    for model in models:
        rows, t0 = [], time.time()
        try: ask(model, "warm up", SYSTEM)   # load; not scored
        except Exception as e: print(f"{model}: cannot load ({e})"); continue
        for p in pairs:
            r = ask(model, p["q"], SYSTEM)
            raw = r["message"]["content"].strip()
            toks_ = [re.sub(r"[^a-z0-9_]", "", t.lower().strip("`*'\"")) for t in raw.split()] if raw else []
            ans = toks_[0] if toks_ else ""
            ans_member = toks_[1] if len(toks_) > 1 else ""
            member_hit = bool(p.get("member")) and canon(ans) == canon(p["target"]) and ans_member == p["member"]
            rows.append({**p, "answer": ans, "answer_member": ans_member, "raw": raw[:60],
                         "hit": canon(ans) == canon(p["target"]), "member_hit": member_hit,
                         "ms": r.get("total_duration", 0) // 1_000_000})
            if len(rows) % 20 == 0:
                print(f"   {model}: {len(rows)}/{len(pairs)} hits so far {sum(x['hit'] for x in rows)}, last {rows[-1]['ms']} ms")
        hits = sum(x["hit"] for x in rows); n = len(rows)
        per = collections.defaultdict(lambda: [0, 0])
        for x in rows: per[x["target"]][1] += 1; per[x["target"]][0] += x["hit"]
        conf = collections.Counter((x["target"], x["answer"]) for x in rows if not x["hit"]).most_common(10)
        ms = sorted(x["ms"] for x in rows)
        print(f"\n== {model}: {hits}/{n} = {100*hits/n:.1f}%  (baseline {100*base_hits/len(pairs):.1f}%)  "
              f"median {ms[len(ms)//2]} ms, p90 {ms[int(len(ms)*0.9)]} ms, wall {time.time()-t0:.0f} s")
        for t, (h, c) in sorted(per.items(), key=lambda kv: -kv[1][1])[:10]:
            print(f"   {t:12s} {h:3d}/{c:<3d}")
        print("   confusions:", ", ".join(f"{t}->{a or '?'}x{c}" for (t, a), c in conf))
        if MEMBERS:
            withm = [x for x in rows if x.get("member")]
            mh = sum(x["member_hit"] for x in withm)
            print(f"   member level (command AND member right): {mh}/{len(withm)} of the pairs naming a member")
        summary["models"][model] = {"hits": hits, "n": n, "median_ms": ms[len(ms)//2], "per_target": dict(per), "confusions": conf}
        (OUT / (model.replace("/", "_").replace(":", "_") + ("_members" if MEMBERS else "") + ".json")).write_text(json.dumps(rows, indent=1))
    (OUT / ("summary_members.json" if MEMBERS else "summary.json")).write_text(json.dumps(summary, indent=1))


if __name__ == "__main__":
    main(sys.argv)
