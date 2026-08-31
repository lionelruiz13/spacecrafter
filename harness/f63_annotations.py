#!/usr/bin/env python3
"""F63 — the `#!` channel: the engine writes the diagnosis on the faulty line.

    cd claude/harness && DISPLAY=:2 ./f63_annotations.py [absOutdir]   # default artifacts/f63

FEATURE_REQUESTS [2026-08-30] "`#!` — the engine annotates the faulty script
line in place" [vixy]: a comment starting with `#!` at the END of the faulty
line, added once, replaced when different; for an unclosed `struct if` on the
OPENER; three-part content (what / consequence / action, §11.169). This slice
wires the BLOCK-STRUCTURE producers (the ruled class): unclosed `struct if`
and `struct loop` at their opener (reported when the queue runs out —
terminateScript audits before `script action end` discards the stack),
`end`/`else` without `if`, `loop end` without `loop`, at their own line.
ScriptAnnotator (scriptModule/script_annotator.hpp) is the writer: per-file
batch at script end, sibling temp + rename, byte comparison before any write,
stale tails cleared at a natural end, log-only when the file is unwritable.

One fresh launch on a temp-HOME farm; scripts played over TCP one after the
other, each waited to its end through the script log. What is asserted:

  A   five faults in one file: L3 `struct if end`, L4 `struct if else`, L5
      `struct loop end` (closers with nothing open), L6 `struct loop 2` never
      closed, L8 `struct if 1 equal 1` never closed → exactly those five lines
      gain a ` #! …` tail carrying the three parts, every other byte identical;
      the log carries `script <file>:<line>: <what> [<line>]` for each.
  A2  the same file played again → byte-identical afterwards (the tails already
      say this; no rewrite), and the commands still execute (the tail is a
      comment by the 3d9179d2 rule) — the same five diagnostics again.
  B   an unclosed `if` alone; fixed by APPENDING `struct if end` → the opener's
      tail is CLEARED at the next natural end (the fix path that keeps every
      other line number).
  C   `end` without `if`; fixed by editing the line above into an opener →
      the closer's tail cleared.
  D   A's content with CRLF endings → tails land BEFORE the CR, every ending
      preserved, otherwise as A.
  E   A's content in a READ-ONLY directory → file untouched, one WARNING naming
      the file and the count, the diagnostics still in the log.
  F   `#!` inside a quoted value on a clean line → not a tail: file untouched.
  G   a fault on a line the driver EDITS while the script waits → the
      annotation is skipped with a "changed since the script was loaded"
      warning and the driver's edit survives.
  H+I a script that plays another whose `if` is never closed → the opener is
      annotated in I (its own file), H untouched: provenance survives the splice.
  J   `struct if end` inside a `struct loop 2` → the diagnostic fires on the
      first pass AND on the replay (two log lines, the replayed line still
      carries its origin), ONE tail.

Every farm script lives under the farm (never the field); real config/ssystem
md5 in == out; /proc/<pid>/comm instance check. Exit 0 all green, 1 a check
failed, 2 no run.
"""
import hashlib, json, os, re, socket, stat, subprocess, sys, time
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f63"
FARM = Path("/tmp/sc-farm-f63")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805
MSG = {
    "unclosed_if": "this 'struct if' is never closed",
    "end_wo_if": "this 'struct if end' closes nothing",
    "else_wo_if": "this 'struct if else' flips nothing",
    "unclosed_loop": "this 'struct loop' is never closed",
    "loop_end_wo": "this 'struct loop end' closes nothing",
}

def md5(p): return hashlib.md5(Path(p).read_bytes()).hexdigest()
def instance_pids():
    out = []
    for pid in os.listdir('/proc'):
        if not pid.isdigit(): continue
        try: comm = open(f'/proc/{pid}/comm').read().strip()
        except OSError: continue
        if comm == 'spacecrafter': out.append(int(pid))
    return out
def die(msg): print("FATAL:", msg); sys.exit(2)
results = []
def check(name, ok, detail=""):
    results.append({"check": name, "ok": bool(ok), "detail": detail})
    print(("PASS " if ok else "FAIL ") + name + ("  -- " + detail if detail else ""))

OUT.mkdir(parents=True, exist_ok=True)
if instance_pids(): die("a spacecrafter instance is already running: %s" % instance_pids())
if not Path(BIN).exists(): die("binary not found: " + BIN)
md5_in = {k: md5(REAL / k) for k in PRISTINE}
for k, v in PRISTINE.items():
    if md5_in[k] != v: die("field %s is not the recorded pristine one" % k)
env = dict(os.environ); env["DISPLAY"] = os.environ.get("DISPLAY", ":2")
auth = sorted(Path(f"/run/user/{os.getuid()}").glob(".mutter-Xwaylandauth.*"))
if auth: env["XAUTHORITY"] = str(auth[0])
if subprocess.run(["xdpyinfo"], env=env, capture_output=True).returncode != 0: die("display not usable")
subprocess.run([str(HARNESS / "b3_farm.sh"), str(FARM)], check=True, stdout=subprocess.DEVNULL)
sc = FARM / ".spacecrafter"; env["HOME"] = str(FARM)
S = FARM / "f63"; S.mkdir()

# ---------------------------------------------------------------- the scripts
A_LINES = [
    "# A: one of each block-structure fault",
    "flag stars on",
    "struct if end",
    "struct if else",
    "struct loop end",
    "struct loop 2",
    "flag stars off",
    "struct if 1 equal 1",
    "flag stars on",
]
def write(path, lines, eol="\n"):
    path.write_bytes((eol.join(lines) + eol).encode("latin-1"))
A = S / "A.sts"; write(A, A_LINES)
B = S / "B.sts"; write(B, ["# B: an unclosed if, alone", "struct if 1 equal 1", "flag stars on"])
C = S / "C.sts"; write(C, ["# C: an end with nothing open", "flag stars on", "struct if end"])
D = S / "D.sts"; write(D, A_LINES, eol="\r\n")
RO = S / "ro"; RO.mkdir(); E = RO / "E.sts"; write(E, A_LINES)
F = S / "F.sts"; write(F, ["# F: a #! inside quotes is text", 'text name f63 string "a #! b" altitude 10', "flag stars on"])
G = S / "G.sts"; write(G, ["# G: edited while it runs", "struct if end", "wait duration 4", "flag stars on"])
I = S / "I.sts"; write(I, ["struct if 1 equal 1", "flag stars off"])
H = S / "H.sts"; write(H, ["# H: plays I, whose if is never closed", "flag stars on", f"script action play filename {I}", "flag stars off"])
J = S / "J.sts"; write(J, ["# J: a fault inside a loop body, replayed", "struct loop 2", "struct if end", "flag stars on", "struct loop end"])
before = {p: p.read_bytes() for p in (A, B, C, D, E, F, G, H, I, J)}
os.chmod(RO, stat.S_IRUSR | stat.S_IXUSR)   # the DIRECTORY is what a sibling-temp write needs

# ---------------------------------------------------------------- launch
app_out = open(OUT / "app.out", "wb"); t0 = time.time()
proc = subprocess.Popen([BIN], env=env, stdout=app_out, stderr=subprocess.STDOUT)
sock = None
while time.time() - t0 < 180:
    if proc.poll() is not None: die("engine exited during init rc=%s" % proc.returncode)
    try: sock = socket.create_connection(("127.0.0.1", PORT), timeout=2); break
    except OSError: time.sleep(0.5)
if sock is None: die("port never opened")
print("port up after %.1f s" % (time.time() - t0))
def script_log():
    logs = sorted((sc / "log").glob("script-*.log"))
    return logs[-1].read_text(encoding="latin-1", errors="replace") if logs else ""
t = time.time()
while "ScriptMgr: script end" not in script_log() and time.time() - t < 60: time.sleep(0.2)
time.sleep(2.0)
def send(cmd, pause=0.3):
    sock.sendall((cmd + "\n").encode("latin-1")); time.sleep(pause)
def play(path, timeout=30, during=None):
    """Play `path`; return the log slice of that run. `during(offset)` runs once after the load line."""
    mark = len(script_log())
    send(f"script action play filename {path}", 0.2)
    t = time.time(); called = False
    while time.time() - t < timeout:
        txt = script_log()[mark:]
        if f"ScriptMgr: load {path}" in txt and during and not called:
            during(); called = True
        if f"ScriptMgr: load {path}" in txt and "ScriptMgr: script end" in txt.split(f"ScriptMgr: load {path}", 1)[1]:
            time.sleep(0.5)
            return script_log()[mark:]
        time.sleep(0.2)
    die("script never ended: %s" % path)

def tails(path):
    """line number -> tail text for every line carrying ` #! `."""
    out = {}
    for i, raw in enumerate(path.read_bytes().split(b"\n"), 1):
        line = raw.decode("latin-1")
        if "#!" in line: out[i] = line.split("#!", 1)[1].strip().rstrip("\r")
    return out
def diag_lines(txt, path): return re.findall(r"^\S+: \(Error\): script " + re.escape(str(path)) + r":(\d+): (.*?) \[(.*)\]$", txt, re.M)

# ---------------------------------------------------------------- A
logA = play(A)
ta = tails(A)
check("A  exactly lines 3,4,5,6,8 carry a '#!' tail", sorted(ta) == [3, 4, 5, 6, 8], json.dumps(sorted(ta)))
check("A  L3 end-without-if", MSG["end_wo_if"] in ta.get(3, ""))
check("A  L4 else-without-if", MSG["else_wo_if"] in ta.get(4, ""))
check("A  L5 loop-end-without-loop", MSG["loop_end_wo"] in ta.get(5, ""))
check("A  L6 unclosed loop, at the OPENER", MSG["unclosed_loop"] in ta.get(6, ""))
check("A  L8 unclosed if, at the OPENER", MSG["unclosed_if"] in ta.get(8, ""))
stripped = b"\n".join(l.split(b" #! ")[0] for l in A.read_bytes().split(b"\n"))
check("A  every other byte identical (tails stripped == original)", stripped == before[A])
dl = diag_lines(logA, A)
check("A  log: five 'script <file>:<line>: <what> [<line>]' lines", sorted(int(n) for n, _, _ in dl) == [3, 4, 5, 6, 8], json.dumps(dl)[:400])
check("A  log: the quoted line is the faulty line itself", all(A_LINES[int(n) - 1] == q for n, _, q in dl))
afterA = A.read_bytes()

# ---------------------------------------------------------------- A2: idempotent, and the tail is a comment
logA2 = play(A)
check("A2 played again: byte-identical (no rewrite)", A.read_bytes() == afterA)
check("A2 the tails were comments: the same five diagnostics fired again", sorted(int(n) for n, _, _ in diag_lines(logA2, A)) == [3, 4, 5, 6, 8])
check("A2 no 'rewritten' log line for A (A's first run had exactly one)",
      logA2.count(f"script annotation: '{A}' rewritten") == 0 and logA.count(f"script annotation: '{A}' rewritten") == 1)

# ---------------------------------------------------------------- B: fix by appending
play(B)
check("B  the opener (L2) is annotated", MSG["unclosed_if"] in tails(B).get(2, ""), json.dumps(tails(B)))
B.write_bytes(B.read_bytes() + b"struct if end\n")
play(B)
check("B  fixed by appending 'struct if end': the tail is CLEARED", tails(B) == {}, json.dumps(tails(B)))
check("B  ... and the file is the fixed text exactly", B.read_bytes() == before[B] + b"struct if end\n")

# ---------------------------------------------------------------- C: fix by editing the line above
play(C)
check("C  the closer (L3) is annotated", MSG["end_wo_if"] in tails(C).get(3, ""))
lines = C.read_bytes().split(b"\n"); lines[1] = b"struct if 1 equal 1"; C.write_bytes(b"\n".join(lines))
play(C)
check("C  fixed by giving it an opener: the tail is CLEARED", tails(C) == {}, json.dumps(tails(C)))
check("C  ... file == fixed text", C.read_bytes() == b"\n".join(before[C].split(b"\n")[:1] + [b"struct if 1 equal 1"] + before[C].split(b"\n")[2:]))

# ---------------------------------------------------------------- D: CRLF
play(D)
raw = D.read_bytes()
check("D  CRLF: every line still ends with CRLF", raw.count(b"\r\n") == len(A_LINES) and b"\n" not in raw.replace(b"\r\n", b""))
check("D  CRLF: the tails sit BEFORE the CR", all(l.endswith(b"\r") and b" #! " in l for i, l in enumerate(raw.split(b"\n"), 1) if i in (3, 4, 5, 6, 8)))
check("D  CRLF: the same five lines", sorted(tails(D)) == [3, 4, 5, 6, 8])

# ---------------------------------------------------------------- E: read-only directory
logE = play(E)
check("E  read-only dir: file untouched", E.read_bytes() == before[E])
check("E  read-only dir: one WARNING naming the file and '5 annotation(s) to place'",
      len(re.findall(r"\(Warn\.\): script annotation: cannot write '" + re.escape(str(E)) + r"' \(5 annotation", logE)) == 1)
check("E  read-only dir: the five diagnostics are in the log anyway", len(diag_lines(logE, E)) == 5)

# ---------------------------------------------------------------- F: quoted #!
logF = play(F)
check("F  a '#!' inside quotes is not a tail: file untouched", F.read_bytes() == before[F])
check("F  ... and no annotation activity for F in the log", f"script annotation: '{F}'" not in logF)

# ---------------------------------------------------------------- G: edited while running
def edit_G():
    time.sleep(0.8)   # let the engine pass L2 and sit in the wait
    ls = G.read_bytes().split(b"\n"); ls[1] = b"struct if end   "; G.write_bytes(b"\n".join(ls))
logG = play(G, during=edit_G)
check("G  a line edited while the script ran: annotation SKIPPED, the edit survives",
      G.read_bytes().split(b"\n")[1] == b"struct if end   " and "#!" not in G.read_text(encoding="latin-1"))
check("G  ... with a 'changed since the script was loaded' warning", "changed since the script was loaded" in logG)

# ---------------------------------------------------------------- H + I: the splice
logH = play(H)
check("H+I the opener in the CALLED script is annotated in ITS file (I line 1)", MSG["unclosed_if"] in tails(I).get(1, ""), json.dumps(tails(I)))
check("H+I the caller is untouched", H.read_bytes() == before[H])

# ---------------------------------------------------------------- J: replayed by a loop
logJ = play(J)
check("J  one tail on the replayed line (L3)", sorted(tails(J)) == [3] and MSG["end_wo_if"] in tails(J)[3], json.dumps(tails(J)))
check("J  two log lines: first pass and the replay both carry the origin", len(diag_lines(logJ, J)) == 2, json.dumps(diag_lines(logJ, J))[:300])

# ---------------------------------------------------------------- shutdown
try: send("shutdown action now", 0.2)
except OSError: pass
t = time.time()
while proc.poll() is None and time.time() - t < 40: time.sleep(0.5)
if proc.poll() is None: proc.terminate(); proc.wait(10)
check("engine exited on `shutdown action now`", proc.returncode == 0, "rc=%s" % proc.returncode)
app_out.close(); sock.close()
(OUT / "script.log").write_text(script_log(), encoding="latin-1")
for p in (A, B, C, D, E, F, G, H, I, J): (OUT / p.name).write_bytes(p.read_bytes())
os.chmod(RO, stat.S_IRWXU)
md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in)
ok = all(r["ok"] for r in results)
(OUT / "f63_result.json").write_text(json.dumps({"binary": BIN, "results": results, "all_ok": ok}, indent=1))
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED", sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
