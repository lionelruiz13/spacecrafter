#!/usr/bin/env python3
"""F62 — `mod` / `div` / `mul` as aliases, and what a recording keeps.

    cd claude/harness && DISPLAY=:2 ./f62_aliases.py [absOutdir]   # default artifacts/f62

FEATURE_REQUESTS [2026-08-30] "Short aliases for the long math commands":
`div`, `mul`, `mod` registered on the same enum as `divide`, `multiply`,
`modulo` (app_command_init.cpp aliases block). Two claims, each with its
positive control in the same launch:

  ARITHMETIC  `define x 7` then `mod x 3` -> x = 1; `define y 8`, `div y 2`
              -> 4, `mul y 3` -> 12; the canonical `modulo`/`divide`/`multiply`
              run beside them on other variables with the same expectations.
              Readback = `struct print var`, which writes every variable as
              `name => value` into the script log (AppCommandEval::printVar).
              Control: on a PRE-alias binary `mod x 3` is "Unrecognized or
              malformed command name" and x stays 7 (SS-22's dead line).

  RECORDING   `script action record filename <farm>/rec.sts`, then `div y 2`
              and `flyto action move_to` and `modulo x 3`, then `script action
              cancel`. The file must carry the three lines AS TYPED:
              ScriptMgr::recordCommand writes the raw `commandline`
              (script_mgr.cpp:252) and the enum-to-name map has no consumer,
              so no spelling is rewritten. This is the measurement behind the
              retirement of scedit's `alias-respelled` seed and the correction
              of the "recording trap" note (FEATURE_REQUESTS, §11.182): the
              trap was in a map nothing reads. Control: `flag stars toggle` in
              the same recording IS re-serialised (`flag stars 0|1`, from
              m_flags_ToString, app_command_interface.cpp:347-349) — the one
              place the engine does rewrite, so a verbatim `div` line is a
              measured property, not an absent instrument.

Temp-HOME farm, fresh launch, real config/ssystem md5 in == out, instance
check via /proc/<pid>/comm. Exit 0 all green, 1 a check failed, 2 no run.
"""
import hashlib, json, os, re, socket, subprocess, sys, time
from pathlib import Path

import logread

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f62"
FARM = Path("/tmp/sc-farm-f62")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805

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
REC = FARM / "rec.sts"

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
    # F108: the live script channel is `script.log` (numbered rotation at
    # open); a pre-F108 landed dir still has its dated names.  One rule,
    # in logread.py, which is selftested both ways.
    return logread.text(sc / "log", "script")
t = time.time()
while "ScriptMgr: script end" not in script_log() and time.time() - t < 60: time.sleep(0.2)
time.sleep(2.0)
def send(cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode("latin-1")); time.sleep(pause)

# --- arithmetic, aliases and canonical names side by side -------------------
for c in ["define x 7", "mod x 3", "define y 8", "div y 2", "mul y 3",
          "define cx 7", "modulo cx 3", "define cy 8", "divide cy 2", "multiply cy 3",
          "struct print var"]:
    send(c)
time.sleep(1.0)
# --- recording ---------------------------------------------------------------
# A recorded line must SUCCEED to be written (executeCommandStatus records on
# success only). `flyto` is NOT exercised here: three camera forms tried in the
# 2026-08-31 run all failed for camera-state reasons (missing target / unknown
# target arg / "error move_to point"), and the claim — an alias is written as
# typed — is carried exactly by `div y 2`, so a fourth guess was not worth its
# minute. If a `flyto` leg is ever wanted, read families.commands.camera.args
# in the grammar first and pick a form the default camera state accepts.
for c in [f"script action record filename {REC}", "div y 2",
          "modulo x 3", "flag stars toggle", "script action cancel"]:
    send(c)
time.sleep(1.0)
try: send("shutdown action now", 0.2)
except OSError: pass
t = time.time()
while proc.poll() is None and time.time() - t < 40: time.sleep(0.5)
if proc.poll() is None: proc.terminate(); proc.wait(10)
check("engine exited on `shutdown action now`", proc.returncode == 0, "rc=%s" % proc.returncode)
app_out.close(); sock.close()
final = script_log(); (OUT / "script.log").write_text(final, encoding="latin-1")

# the LAST print block: each variable's final value
vals = dict(re.findall(r"^\S+: \(Info \): (\w+) => (\S+)$", final, re.M))
def num(v):
    try: return float(v)
    except (TypeError, ValueError): return None
check("alias  mod x 3   -> x = 1 (7 mod 3)", num(vals.get("x")) == 1, "x=%s" % vals.get("x"))
check("alias  div/mul   -> y = 12 (8 / 2 * 3)", num(vals.get("y")) == 12, "y=%s" % vals.get("y"))
check("control modulo   -> cx = 1", num(vals.get("cx")) == 1, "cx=%s" % vals.get("cx"))
check("control divide/multiply -> cy = 12", num(vals.get("cy")) == 12, "cy=%s" % vals.get("cy"))
unrec = re.findall(r"Execute_command (.*)\n[^\n]*\n?[^\n]*Unrecognized or malformed command name", final)
check("no alias line was 'Unrecognized' (pre-alias binaries fail here on mod/div/mul)",
      not any(u.split()[0] in ("mod", "div", "mul") for u in unrec), json.dumps(unrec))

rec = REC.read_text(encoding="latin-1") if REC.exists() else ""
(OUT / "rec.sts").write_text(rec, encoding="latin-1")
rl = rec.splitlines()
check("recording file written", bool(rl), "%d lines" % len(rl))
check("recording keeps `div y 2` AS TYPED (no respell to divide)", "div y 2" in rl and "divide y 2" not in rl)
check("recording keeps `modulo x 3` (canonical, unchanged)", "modulo x 3" in rl)
check("control: `flag stars toggle` IS re-serialised as `flag stars 0|1` (the one rewrite site)",
      any(re.fullmatch(r"flag stars [01]", l) for l in rl) and "flag stars toggle" not in rl, json.dumps([l for l in rl if l.startswith("flag stars")]))
md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in)
ok = all(r["ok"] for r in results)
(OUT / "f62_result.json").write_text(json.dumps({"binary": BIN, "vars": vals, "recording": rl, "results": results, "all_ok": ok}, indent=1))
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED", sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
