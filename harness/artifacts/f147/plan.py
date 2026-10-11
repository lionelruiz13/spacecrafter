#!/usr/bin/env python3
"""F147 step 7: the archive plan, SIMULATED on a copy of the pinned file.

M1 (one modification commit, no line removed - three spaces become line breaks, step 3's two lines re-pointed, the rest inserted): the extractions that must stand before any body
leaves - the one-line forms of the nine protocol bodies, the §1 replacement sentence, the two
splits (§1 line 484; §0b step 1's drift-assert sentence), the §0b step 3 re-point, the live list
appended at the end of the file. Then the units are derived on the post-M1 bytes, the moves are
replayed exactly as fd_archive_pass.py does them (removals + reconstruction check), and the result
is measured. Writes: plan-out/units.json, plan-out/post-M1.md, plan-out/post-move.md,
plan-out/commands.txt, plan-out/report.txt. Reads only the pinned copy and livelist.py."""
import hashlib, importlib.util, json, os, re

A = "/home/claude/spacecrafter/claude/harness/artifacts/f147"
OUT = f"{A}/plan-out"
os.makedirs(OUT, exist_ok=True)
PIN = open(f"{A}/fable-dispatch.at-start.md", "rb").read()
assert hashlib.md5(PIN).hexdigest().startswith("a18fdc67")
spec = importlib.util.spec_from_file_location("ll", f"{A}/livelist.py"); ll = importlib.util.module_from_spec(spec); spec.loader.exec_module(ll)

md5 = lambda b: hashlib.md5(b).hexdigest()
L = PIN.decode("utf-8").split("\n")          # L[-1] == '' (trailing newline)


def line(n):
    return L[n - 1]


# ---------------------------------------------------------------- the one-line forms (Reason > Rule)
REPL = {
 "protocol-iso8859": "   - **Encoding:** the Bash `grep` wrapper (ugrep `-I`) skips every file holding non-UTF-8 bytes in silence, so a 0-hit there proves nothing - since F70 only `~/.spacecrafter/ssystem.ini`, `doc/superscript.sts` and the EXCLUDE rows of `harness/f70_partition.tsv` hold such bytes: use `/usr/bin/grep` or Read on them, `LC_ALL=C` for a byte class (`CLAUDE.md`'s encoding hazards (1)-(5) are the authority; §11.188(k), §11.189(a)).",
 "protocol-instance-assert": "   - **Concurrent-instance assert, before each measurement launch:** `bash claude/harness/sc_instances.sh --assert <label>` (exit 2 an engine is live, 4 the probe could not run = a STOP) or `harness/f116_assert.sh <label>` (+ the GPU gate, exit 3) - because concurrent sessions share `~/.spacecrafter` and an md5 re-assert cannot see a concurrent launcher; the criterion is the UNION of `comm`, `/proc/<pid>/exe` and TCP 7805, each covering another's blind spot (residual, said and not claimed: a renamed engine of another uid with no server up); never `pkill -f` (§11.238).",
 "protocol-session-env": "   - **Session environment:** the display target, the auth path and the session's shape are PER-HOST and PER-BOOT - read `HOST-EVENTS.md`'s latest entries for this host and run `xdpyinfo` before the first launch, because an inherited `DISPLAY` / `XAUTHORITY` can name another uid's or a dead display; `timeout -s KILL` bounds nothing in this session type (measured, §11.140(i)): use plain `timeout` or a poll-and-kill watchdog.",
 "protocol-display-arch": "   - **The display stack is part of the instrument:** we render on our own session (the real `:2`, per HOST-EVENTS) while the owner's RDP relay is view-only and CPU-coupled to what we draw, and the cadence is the config cap (`maximum_fps = 144`, measured [144.0, 146.4] fps, §11.159) - so trust counter RATIOS and in-run A/B only, never absolute fps labels or cross-session cadence, and a stack change (compositor, streamer, X server, screen power) means report + re-baseline.",
 "protocol-canary": "   - **ENVIRONMENT CANARY before any measuring launch** (`harness/f56_canary.sh`: full, ~100 s, for photometric tasks; `--no-scene`, seconds, otherwise; §11.176) - because a host change that reaches the instrument reaches every number taken on it: a non-zero exit is a STOP and a report (§11.174(h)), never a mitigation or a widened band; a re-bank is one VALUES-block edit WITH its argument, and the bank is PER-BOOT on the real `:2` session, so a red after a reboot is the protocol (§11.199); the band's values live in that VALUES block, not here; §11.104(d)'s numbers are targets only on a canary-passed stack; A/A floors are per scene (374 px star field, 28 px Moon frame); the shared `~/.spacecrafter/cache` is WATCHED (`f56_manifest.py`).",
 "protocol-backmarker": "   - **Back-marker at the write (§11.161(g), RA-MODEL E18/E55):** an entry that supersedes, refutes or corrects ANOTHER entry's claim places the §11.113(p) back-marker at the superseded node IN THE SAME COMMIT, and a §13 row's state flip greps the §5 register for the references it stales in that commit - because the write is the one moment its author holds both ends (I3: the state's owner informs its dependents), and every stale reference measured sat on the non-working side (§11.180).",
 "protocol-routing": "(a) **Executors are routed by STRUCTURAL-COUPLING LOAD at the mint, never by label** [vixy 2026-10-10, §11.302; the 2026-09-27 structure / implementation split, §11.293, is its common case]: FOCUSED (the input set closed and named in the section, one concern, criteria that can fail) → `opus-xhigh` (Claude Opus 5.5 under the owner's system prompt; step 1 asserts `deploy.sh --check`), bound to report every conflict with the predictions; OPEN-COUPLING (an S4 pass over a family, a shape still to choose, modelling the owner, a PREDICTIONS mint, every acceptance, an edit of the ledger's structure) → `claude` / `fable` - because Opus is strongest on focused work and the conflicts it reports against the predictions are the signal that drives the rework [vixy]. Disjoint file sets may run in parallel in one tree when no agent commits, builds or stashes; an implementer ALONE in the tree may build and still commits nothing; the supervisor verifies, builds, commits. The return opens with `prompt-window: present` (step 4 reads it).",
 "protocol-drift-assert": "   **Definition-drift assert:** `bash claude/agents/deploy.sh --check` must exit 0 - the executor's deployed projection is GENERATED from its authority + the owner's agent prompt + the bridge and compared byte-exact, because a drifted definition silently changes every executor; on drift run `bash claude/agents/deploy.sh` before any dispatch (§11.302).",
}
INDEX_SENTENCE = "*Derived index (regenerable, never authoritative): every task section not live below has left this file - `ls fable-dispatch/archive/F*.md` lists them, each naming its §11 record in its own header line; F113 was WITHDRAWN before dispatch and keeps its section as the record; F61-F64 (the session-18 scedit round) left no section, their records are `util/scedit/INTENT.md` and `harness/README.md`; the per-round lines that grew here are `fable-dispatch/archive/index-f0-f95.md` and `index-s25-s42.md`.*"

# the protocol bodies at the PINNED state: (id, first line no, count) - whole bullets, line-aligned
WHOLE = [("protocol-iso8859", 72, 23), ("protocol-instance-assert", 117, 11), ("protocol-session-env", 128, 24),
         ("protocol-display-arch", 157, 18), ("protocol-canary", 183, 18), ("protocol-backmarker", 225, 11),
         ("protocol-routing", 288, 35), ("protocol-supersession-template", 429, 14)]
for i, s, c in WHOLE:  # assert the boundaries the plan stands on
    nxt = line(s + c)
    assert line(s).startswith(("   - ", "(a) ", "~~**Supersession")), (i, line(s)[:40])
    assert nxt.startswith(("   - ", "(b) ", "6. ")) or nxt == "", (i, nxt[:40])

# ---------------------------------------------------------------- M1, applied in memory
M = list(L)
# (1) §0b step 3 re-point (lines 362-363): the template it names leaves with protocol-supersession-template
old362, old363 = M[361], M[362]
assert old362.endswith("Every prompt carries: the binding") and old363.startswith("   SUPERSESSION BLOCK (template below, values refreshed), the task-section pointer +")
M[361] = old362[: -len("the binding")] + "the per-round variables (step 1:"
M[362] = "   no supersession block - the definition is generated from its authority), the task-section pointer +"
# (2) the drift-assert split inside §0b step 1 (lines 339 and 349): one space -> newline + the step's indent
a339 = M[338]; k = a339.index(" **Definition-drift assert");
p339 = (a339[:k], "   " + a339[k + 1:])
a349 = M[348]; k2 = a349.index(" **Ledger instruments")
p349 = (a349[:k2], "   " + a349[k2 + 1:])
# (3) §1 split at line 484: the space after 'unanswered).*' -> newline
a484 = M[483]; k3 = a484.index(" **Session-25 round")
assert a484[:k3] == "unanswered).*"
p484 = (a484[:k3], a484[k3 + 1:])
# rebuild with insertions (work bottom-up so earlier indices stay valid)
def rebuild():
    out = []
    for n, txt in enumerate(M, 1):
        if n == len(M):            # the '' after the final newline
            out.append(txt); continue
        rid = next((i for i, s, c in WHOLE if s == n), None)
        if rid and rid in REPL:
            out.append(REPL[rid])          # the one-line form stands just before its body
        if n == 448:
            out.append(INDEX_SENTENCE)     # §1's regenerable sentence before the index body
        if n == 339:
            out.extend([p339[0], REPL["protocol-drift-assert"], p339[1]]); continue
        if n == 349:
            out.extend(p349); continue
        if n == 484:
            out.extend(p484); continue
        out.append(txt)
    return out
POST = rebuild()
# (4) the live list appended at the end of the file (the [s42] items stay in their own block)
fence = ["- **The open asks of the earlier lists (sessions 4 → 40), carried by the census of 2026-10-11 (F147, §11.318) - every other item of those lists is closed at a ledger node, and its record left with its block (`fable-dispatch/archive/forvixy-*.md`; the verdict per item: `fable-dispatch/census-2026-10-11.md`):**",
         "", "  ```", "  YOURS — ONE LIST (held set per node; the entry's section in brackets)",
         "  tags: [N+] the item's own node states the ask · [N-] the node holds only the open state, the ask's text is here alone",
         "        [T] routed to the tester, through you · [D] a default that queued or running work builds unless you rule"]
n_live = 0
for head, items in ll.G:
    rows = [(k, t, p, tag) for k, t, p, tag in items if not k.startswith("C66 ")]
    if not rows:
        continue
    fence.append("  " + head + ":")
    for k, t, p, tag in rows:
        fence.append(f"    {t} -> {p} [{k}] {ll.TAGS[tag]}"); n_live += 1
fence.append("  ```")
assert POST[-1] == ""
POST = POST[:-1] + fence + [""]
post_bytes = "\n".join(POST).encode("utf-8")
open(f"{OUT}/post-M1.md", "wb").write(post_bytes)

# ---------------------------------------------------------------- the units on the post-M1 bytes
P = POST
def find(first, count, expect_last=None):
    hits = [i + 1 for i, x in enumerate(P[:-1]) if x == first]
    assert len(hits) == 1, (first[:60], hits)
    s = hits[0]
    if expect_last is not None:
        assert P[s + count - 2] == expect_last, (first[:50], P[s + count - 2][:60])
    return s
units = []
def add(uid, kind, s, c):
    payload = ("\n".join(P[s - 1:s - 1 + c]) + "\n").encode("utf-8")
    units.append({"id": uid, "kind": kind, "start": s, "count": c, "first": P[s - 1], "last": P[s + c - 2], "bytes": len(payload), "md5": md5(payload)})
# §3 blocks 2..66, labels from the census tables
labels = {}
import glob
for f in glob.glob(f"{A}/rows/C*.md"):
    h = open(f, encoding="utf-8").readline()
    b = int(re.search(r"file block (\d+)", h).group(1))
    m = re.search(r"- (?:unit|label) `([^`]+)`", h)
    if m: labels[b] = m.group(1)
S3 = open(f"{A}/section3.snapshot.md", "rb").read().decode("utf-8").split("\n")
starts = [i for i in range(len(S3) - 1) if S3[i].startswith("- **")]
for k, s in enumerate(starts):
    b = k + 1
    if b == 1: continue
    e = starts[k + 1] if k + 1 < len(starts) else len(S3) - 1
    c = e - s
    first, last = S3[s], S3[e - 1]
    st = find(first, c, last)
    add(labels[b], "forvixy", st, c)
# §1 index units
s1 = find(p484[0].replace(p484[0], line(448)), 0) if False else None
st = find(line(448), 37, "unanswered).*"); add("index-f0-f95", "index", st, 37)
st = find(p484[1], 22, line(505)); add("index-s25-s42", "index", st, 22)
# §0 / §0b protocol units
for i, s, c in WHOLE:
    st = find(line(s), c, line(s + c - 1)); add(i, "protocol", st, c)
st = find(p339[1], 11, p349[0]); add("protocol-drift-assert", "protocol", st, 11)
ids = [u["id"] for u in units]
assert len(ids) == len(set(ids)), "duplicate ids"
# seams: a unit whose removal leaves two blank lines adjacent
removed = set()
for u in units: removed.update(range(u["start"], u["start"] + u["count"]))
seams = []
for u in units:
    before = u["start"] - 1; after = u["start"] + u["count"]
    while after in removed: after += 1
    if P[before - 1] == "" and after <= len(P) - 1 and P[after - 1] == "" :
        seams.append({"after": u["id"], "start": u["start"] + u["count"], "count": 1, "line": P[u["start"] + u["count"] - 1]})
json.dump({"pinned_md5": md5(PIN), "post_M1_md5": md5(post_bytes), "units": units, "seams": seams}, open(f"{OUT}/units.json", "w"), indent=1, ensure_ascii=False)

# ---------------------------------------------------------------- replay the moves (fd_archive_pass.py's arithmetic)
lines = post_bytes.split(b"\n")
rem = sorted([(u["start"], u["count"], u["id"]) for u in units] + [(s["start"], s["count"], "seam:" + s["after"]) for s in seams])
last = 0
for s, c, i in rem:
    assert s > last, ("overlap", i); last = s + c - 1
keep, cur = [], 1
for s, c, i in rem:
    keep.extend(lines[cur - 1:s - 1]); cur = s + c
keep.extend(lines[cur - 1:])
recon = list(keep)
for s, c, i in rem:
    recon[s - 1:s - 1] = lines[s - 1:s - 1 + c]
assert md5(b"\n".join(recon)) == md5(post_bytes), "reconstruction failed"
after = b"\n".join(keep)
open(f"{OUT}/post-move.md", "wb").write(after)
# head rule of the extended tool, per unit
def head_ok(u):
    h = u["first"]
    return (u["id"].startswith("forvixy-") and h.startswith("- **")) or (u["id"].startswith("index-") and h.startswith("*")) or u["id"].startswith("protocol-")
bad_heads = [u["id"] for u in units if not head_ok(u)]
# regions after
A_ = after.decode("utf-8").split("\n")
def region(lines_, start_pat, end_pat):
    s = next(i for i, x in enumerate(lines_) if re.match(start_pat, x))
    e = next((i for i, x in enumerate(lines_[s + 1:], s + 1) if re.match(end_pat, x)), len(lines_) - 1)
    return sum(len(x.encode()) + 1 for x in lines_[s:e]), e - s
rep = []
for name, sp, ep in [("§0", r"^## 0\. Cold", r"^## 0b\."), ("§0b", r"^## 0b\.", r"^## 1\. "), ("§1", r"^## 1\. ", r"^### F|^## 2\."), ("§3", r"^## 3\. For Vixy", r"^\Z#never")]:
    b0, n0 = region(L, sp, ep); b1, n1 = region(A_, sp, ep)
    rep.append(f"{name}: before {n0} lines {b0} B -> after {n1} lines {b1} B")
rep.insert(0, f"pinned {md5(PIN)} {len(PIN)} B {len(L)-1} lines; post-M1 {md5(post_bytes)} {len(post_bytes)} B {len(P)-1} lines; post-move {md5(after)} {len(after)} B {len(A_)-1} lines")
rep.append(f"units {len(units)} (forvixy {sum(u['kind']=='forvixy' for u in units)}, index {sum(u['kind']=='index' for u in units)}, protocol {sum(u['kind']=='protocol' for u in units)}); seams {len(seams)} {[(s['after'], s['start']) for s in seams]}; live-list lines appended {n_live}; bad heads {bad_heads}")
rep.append(f"removed lines {sum(u['count'] for u in units) + sum(s['count'] for s in seams)}; reconstruction byte-exact: OK")
open(f"{OUT}/report.txt", "w").write("\n".join(rep) + "\n")
# the commands, bottom-up passes on the post-M1 numbering (valid for all three passes: each pass removes lines BELOW the next one's)
def args(kind):
    return " ".join(f"{u['id']}:{u['start']}:{u['count']}" for u in units if u["kind"] == kind)
seam_args = " ".join(f"--seam-after {s['after']}:{s['start']}:{s['count']}" for s in seams)
cmd = [f"# pass A - section 3 (65 units)\npython3 harness/fd_archive_pass.py <passA> 2026-10-11 <commit after M1> {args('forvixy')}",
       f"# pass B - section 1's derived index (2 units)\npython3 harness/fd_archive_pass.py <passB> 2026-10-11 <commit after pass A> {args('index')}",
       f"# pass C - sections 0 / 0b (9 units + {len(seams)} seam)\npython3 harness/fd_archive_pass.py <passC> 2026-10-11 <commit after pass B> {args('protocol')} {seam_args}"]
open(f"{OUT}/commands.txt", "w").write("\n\n".join(cmd) + "\n")
print("\n".join(rep))
