#!/usr/bin/env python3
"""F61 — the two 2026-08-31 engine commits, confirmed on a RUNNING engine.

    cd claude/harness && DISPLAY=:2 ./f61_live_rulings.py [absOutdir]   # default artifacts/f61

Both commits were compiled on GCC 11 and NOT run when they landed (no display
session for this user at the time — scedit INTENT journal 2026-08-31b; parent
§11.181 / §11.182 record the live confirmation as OWED). This driver pays it.

WHAT IS MEASURED, and against which counterfactual (the pre-`3d9179d2` engine,
whose behaviour the corpus runs of 2026-08-30 measured on 408 scripts):

  comment rule (`3d9179d2`, parseCommand: a `#` outside a "…" run ends the
  command, cut BEFORE the leading-blank strip) — every clause once, on BOTH
  channels that reach parseCommand from outside (script file, TCP line):
    P1/T1  trailing comment after a command WITH a key/value pair
           `flag stars on # …`. PRE: args = {'#':…, 'stars':'on', …}, the
           `flag` handler applies only the alphabetically-first pair (B38 row,
           app_command_interface.cpp:1180-1198) — `#` is unknown, `stars`
           never applied. POST: stars applied. Observable = the flag's STATE,
           read back through `session action save` (the only read channel for
           a flag: §11.129's readFlag feeds the session file), as a
           TRANSITION (off→on) so the initial state cannot fake it.
    P3     glued comment `flag stars off#P3` — the on→off transition.
    P2/P2b/T2/T3  indented whole-line comments (spaces, tab; file and TCP).
           PRE: the line reaches parseCommand (script.cpp:114 is column-0
           only), command = `#`, "Unrecognized or malformed command name" +
           a did-you-mean line, per line. POST: the cut empties the line,
           executeCommand returns before its own `Execute_command` log line.
           Observable = the count of "Unrecognized" lines, which must equal
           the number of POSITIVE CONTROLS (one genuinely unknown command per
           channel) and nothing else; and no `Execute_command` line for any
           comment-only line.
    P4/T5  a `#` INSIDE quotes is text: `script action play filename
           "<farm>/quoted # name.sts"` on a file that does not exist, so the
           engine ECHOES the value it parsed: `Unable to execute script :
           <value>` (commandScript, :2829). POST: the echo carries ` # `.
    P5     the unquoted twin `…/glued#name.sts` — the echo must stop at
           `glued` (the cut point), never carry `#name.sts`.

  stacktrace probe (`a3437670`): this binary was configured on GCC 11, where
  all three link probes fail (CMakeCache: SC_STACKTRACE_LINKS_* empty), so
  SPACECRAFTER_HAVE_STACKTRACE is OFF and the SIGUSR1 handler's job is to say
  WHY no stack follows. `kill -USR1` on the live process; the 50 ms watchdog
  (fps.cpp:131-139) must write the WARNING naming the missing facility into
  vulkan.log, and the process must SURVIVE the signal (SIGUSR1's default
  disposition is termination — survival proves the handler is installed).

Every launch is fresh, on a temp-HOME farm (b3_farm.sh) with `sessions/` made
a REAL dir (the farm would symlink it into the field, and a session save would
then write the real tree); the real config/ssystem md5 are asserted in == out;
the concurrent-instance check reads /proc/<pid>/comm.

Exit 0 = every assertion holds; 1 = at least one failed (the table says which);
2 = the run could not be made (no display, no port, an instance already up).
"""
import hashlib, json, os, re, signal, socket, subprocess, sys, time
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f61"
FARM = Path("/tmp/sc-farm-f61")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805
STACK_WARN = "SIGUSR1 stall trace requested, but this build has no std::stacktrace"

def md5(p): return hashlib.md5(Path(p).read_bytes()).hexdigest()

def instance_pids():
    out = []
    for pid in os.listdir('/proc'):
        if not pid.isdigit(): continue
        try: comm = open(f'/proc/{pid}/comm').read().strip()
        except OSError: continue
        if comm == 'spacecrafter': out.append(int(pid))
    return out

def die(msg):
    print("FATAL:", msg); sys.exit(2)

results = []
def check(name, ok, detail=""):
    results.append({"check": name, "ok": bool(ok), "detail": detail})
    print(("PASS " if ok else "FAIL ") + name + ("  -- " + detail if detail else ""))

# ---------------------------------------------------------------- preconditions
OUT.mkdir(parents=True, exist_ok=True)
if instance_pids(): die("a spacecrafter instance is already running: %s" % instance_pids())
if not Path(BIN).exists(): die("binary not found: " + BIN)
md5_in = {k: md5(REAL / k) for k in PRISTINE}
for k, v in PRISTINE.items():
    if md5_in[k] != v: die("field %s is not the recorded pristine one (%s)" % (k, md5_in[k]))
cache = Path(BIN).resolve().parents[1] / "CMakeCache.txt"
probe = {}
if cache.exists():
    for line in cache.read_text(errors="replace").splitlines():
        m = re.match(r'(SC_STACKTRACE_LINKS_\w+):INTERNAL=(.*)$', line)
        if m: probe[m.group(1)] = m.group(2)
have_stacktrace = any(v not in ("", "0", "OFF", "FALSE") for v in probe.values())
print("stacktrace link probes in the binary's cache:", probe or "(no cache found)")
print("=> SPACECRAFTER_HAVE_STACKTRACE expected", "ON" if have_stacktrace else "OFF")

env = dict(os.environ)
env["DISPLAY"] = os.environ.get("DISPLAY", ":2")
auth = sorted(Path(f"/run/user/{os.getuid()}").glob(".mutter-Xwaylandauth.*"))
if auth: env["XAUTHORITY"] = str(auth[0])
r = subprocess.run(["xdpyinfo"], env=env, capture_output=True, text=True)
if r.returncode != 0: die("display %s not usable: %s" % (env["DISPLAY"], r.stderr.strip()[:200]))

# ---------------------------------------------------------------- farm
subprocess.run([str(HARNESS / "b3_farm.sh"), str(FARM)], check=True, stdout=subprocess.DEVNULL)
sc = FARM / ".spacecrafter"
sess = sc / "sessions"
if sess.is_symlink() or sess.exists():
    sess.unlink() if sess.is_symlink() else None
sess.mkdir(exist_ok=True)
env["HOME"] = str(FARM)

probe_sts = FARM / "comment_probe.sts"
QUOTED = f"{FARM}/quoted # name.sts"      # must NOT exist: the miss echoes the value
GLUED = f"{FARM}/glued#name.sts"
lines = [
    "# C0 column-0 comment: the script layer's own rule (script.cpp:114), unchanged",
    "flag zzflag_control on",
    "zzunknown_control a b",
    "flag stars off",
    "flag stars on # P1 trailing comment on a command with a pair",
    "session action save filename p1",
    "    # P2 indented whole-line comment (spaces)",
    "\t# P2b indented whole-line comment (tab)",
    f'script action play filename "{QUOTED}"',
    f"script action play filename {GLUED}",
    "flag stars off#P3 glued comment",
    "session action save filename p3",
]
probe_sts.write_bytes(("\n".join(lines) + "\n").encode("latin-1"))
EXPECT_EXEC = [l for l in lines if not l.lstrip().startswith("#")]

# ---------------------------------------------------------------- launch
app_out = open(OUT / "app.out", "wb")
t0 = time.time()
proc = subprocess.Popen([BIN], env=env, stdout=app_out, stderr=subprocess.STDOUT)
sock = None
while time.time() - t0 < 180:
    if proc.poll() is not None: die("engine exited during init, rc=%s (see %s)" % (proc.returncode, OUT / "app.out"))
    try:
        sock = socket.create_connection(("127.0.0.1", PORT), timeout=2); break
    except OSError:
        time.sleep(0.5)
if sock is None: die("port %d never opened" % PORT)
print("port up after %.1f s" % (time.time() - t0))

def script_log():
    logs = sorted((sc / "log").glob("script-*.log"))
    return logs[-1].read_text(encoding="latin-1", errors="replace") if logs else ""

def wait_log(pattern, timeout, after=""):
    """Poll the script log (a file: no notification channel exists — bounded 5 Hz)."""
    t = time.time()
    while time.time() - t < timeout:
        txt = script_log()
        idx = txt.find(after) if after else 0
        if idx >= 0 and re.search(pattern, txt[idx + len(after):], re.M): return txt
        time.sleep(0.2)
    return None

# startup.sts autoplays; let it finish before the probe (the README's race note)
if wait_log(r"ScriptMgr: script end", 60) is None: die("startup script never ended")
time.sleep(2.0)

def send(cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode("latin-1")); time.sleep(pause)

# ---------------------------------------------------------------- script channel
LOAD = f"ScriptMgr: load {probe_sts}"
send(f"script action play filename {probe_sts}", 0.5)
txt = wait_log(r"ScriptMgr: script end", 60, after=LOAD)
if txt is None: die("probe script never ended")
time.sleep(1.0)

# ---------------------------------------------------------------- tcp channel
send("flag stars on # T1 trailing comment over tcp")
send("session action save filename t1")
send("# T2 whole-line comment over tcp")
send("    # T3 indented comment over tcp")
send("zzunknown_tcp x y")
send(f'script action play filename "{FARM}/tcpq # name.sts"')
time.sleep(1.0)

# ---------------------------------------------------------------- SIGUSR1
vk = sc / "log" / "vulkan.log"
before = vk.read_text(encoding="latin-1", errors="replace").count(STACK_WARN) if vk.exists() else 0
os.kill(proc.pid, signal.SIGUSR1)
time.sleep(1.5)
alive = proc.poll() is None
after_txt = vk.read_text(encoding="latin-1", errors="replace") if vk.exists() else ""
warn_count = after_txt.count(STACK_WARN) - before
stack_dump = bool(re.search(r"^\S+: \(Layer\)", after_txt, re.M)) if have_stacktrace else None

# ---------------------------------------------------------------- shutdown
try: send("shutdown action now", 0.2)
except OSError: pass
t = time.time()
while proc.poll() is None and time.time() - t < 40: time.sleep(0.5)
if proc.poll() is None:
    proc.terminate(); proc.wait(10)
    check("engine exited on `shutdown action now`", False, "had to be terminated")
else:
    check("engine exited on `shutdown action now`", True, "rc=%s" % proc.returncode)
app_out.close()
sock.close()
final = script_log()
(OUT / "script.log").write_text(final, encoding="latin-1")
(OUT / "vulkan.log").write_text(after_txt, encoding="latin-1")
for f in sess.glob("*.ini"):          # the farm is rebuilt by the next run; the record lives here
    (OUT / f.name).write_bytes(f.read_bytes())

# ---------------------------------------------------------------- assertions
# (a) the probe window: what the engine executed from the file
win = final[final.find(LOAD):]
win = win[:win.find("End of script")] if "End of script" in win else win
execd = re.findall(r"Execute_command (.*)$", win, re.M)
check("file: Execute_command lines == the non-comment lines, in order",
      execd == EXPECT_EXEC, "got %d, expected %d: %s" % (len(execd), len(EXPECT_EXEC), json.dumps(execd)))
check("file: no comment-only line reached executeCommand's log",
      not any("P2" in e for e in execd))

unrec = [m.start() for m in re.finditer(r"Unrecognized or malformed command name", final)]
# which Execute_command line precedes each Unrecognized
def preceding_exec(pos):
    return re.findall(r"Execute_command (.*)$", final[:pos], re.M)[-1]
culprits = [preceding_exec(p) for p in unrec]
check("unknown-command diagnostics: exactly the two positive controls",
      culprits == ["zzunknown_control a b", "zzunknown_tcp x y"], json.dumps(culprits))

# "<x> is unknown. Did you mean …?" is printed by BOTH lookups (searchSimilarCommand
# for a command, the flag table for a flag), so the names are the three controls;
# the flag-specific line "Unrecognized or malformed flag argument" is the flag count.
flag_unk = re.findall(r"^\S+: \(\w+\.?\s*\): (\S+) is unknown\.", final, re.M)
check("did-you-mean lines: exactly the three positive controls (no '#')",
      flag_unk == ["zzflag_control", "zzunknown_control", "zzunknown_tcp"], json.dumps(flag_unk))
check("unknown-flag diagnostics: exactly the one positive control",
      final.count("Unrecognized or malformed flag argument") == 1,
      "count=%d" % final.count("Unrecognized or malformed flag argument"))

def flag_state(name):
    p = sess / (name + ".ini")
    if not p.exists():
        alt = sorted(sess.glob(name + "*"))
        if not alt: return None
        p = alt[0]
    m = re.search(r"^stars\s*=\s*(\S+)", p.read_text(encoding="latin-1", errors="replace"), re.M)
    return m.group(1) if m else "(no stars line)"
s_p1, s_p3, s_t1 = flag_state("p1"), flag_state("p3"), flag_state("t1")
check("session files written into the FARM (p1, p3, t1)",
      None not in (s_p1, s_p3, s_t1), "sessions/: %s" % sorted(x.name for x in sess.iterdir()))
on = {"true", "on", "1"}; off = {"false", "off", "0"}
check("P1  file, trailing comment: `flag stars on # …` APPLIED (off->on)", s_p1 and s_p1.lower() in on, "p1 stars=%s" % s_p1)
check("P3  file, glued comment: `flag stars off#…` APPLIED (on->off)", s_p3 and s_p3.lower() in off, "p3 stars=%s" % s_p3)
check("T1  tcp, trailing comment: `flag stars on # …` APPLIED (off->on)", s_t1 and s_t1.lower() in on, "t1 stars=%s" % s_t1)

echo = re.findall(r"Unable to execute script : (.*)$", final, re.M)
check("P4  quoted '#' kept: the miss echoes the value with ' # ' inside",
      QUOTED in echo, json.dumps(echo))
check("P5  unquoted '#' cuts: the echo stops at 'glued'",
      f"{FARM}/glued" in echo and not any("glued#" in e for e in echo), json.dumps(echo))
check("T5  tcp, quoted '#' kept", f"{FARM}/tcpq # name.sts" in echo)

check("SIGUSR1: process survived the signal (handler installed)", alive)
if have_stacktrace:
    check("SIGUSR1: a stack was written (HAVE_STACKTRACE build)", stack_dump)
else:
    check("SIGUSR1: the watchdog wrote the 'no std::stacktrace' WARNING exactly once", warn_count == 1, "count=%d" % warn_count)

md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in, json.dumps(md5_out))

ok = all(r["ok"] for r in results)
(OUT / "f61_result.json").write_text(json.dumps({
    "binary": BIN, "probe_cache": probe, "have_stacktrace": have_stacktrace,
    "sessions": {"p1": s_p1, "p3": s_p3, "t1": s_t1}, "executed": execd,
    "unrecognized_culprits": culprits, "flag_unknown": flag_unk, "echo": echo,
    "sigusr1_warn_count": warn_count, "alive_after_sigusr1": alive,
    "results": results, "all_ok": ok}, indent=1))
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED", sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
