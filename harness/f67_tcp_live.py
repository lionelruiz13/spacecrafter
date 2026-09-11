#!/usr/bin/env python3
"""F67 — scedit's live TCP mode, measured against a running spacecrafter.

    cd claude/harness && DISPLAY=:2 ./f67_tcp_live.py [absOutdir]   # default artifacts/f67

WHAT THIS IS FOR. `util/scedit/tests/tcp_gate.py` proves scedit's client speaks
the protocol a STAND-IN engine speaks. That is a check against my own reading of
`src/tools/io.cpp`. This one runs the real binary, on a fresh temp-HOME farm, and
asks the engine itself — and every claim is read through a channel scedit's
client does not touch: the session file, the file on disk, `scedit --history`.

PREDICTIONS ARE STATED BEFORE THE RUN (printed first, and written into the
result JSON before a single leg runs), and each leg names its observable:

  A  A COMMAND SENT BY SCEDIT IS EXECUTED. scedit's client sends
     `flag stars off` / save / `flag stars on` / save. Observable: the `stars`
     line of the two session files (§11.129's readFlag is what a session save
     writes; the only read channel a flag has), as a TRANSITION off -> on, so
     the initial state cannot fake it. PREDICTED: a0 off, a1 on.
     Also predicted, and the reason the rest of this design is what it is: the
     engine sends NOTHING back for those four commands.
  B  A `get` REPLY REACHES THE CONNECTION THAT ASKED (§5.47, closed by F27).
     Observable: the reply on scedit's own socket, five ';'-separated fields.
     PREDICTED: exactly ONE copy — scedit is both the issuer and a $LOGON
     subscriber, and `deliver` excludes the addressee from the broadcast.
  C  THE $LOGON FEED CARRIES WHAT THE ENGINE BROADCASTS. A SECOND, plain client
     asks `get status position` while scedit's client only listens. Observable:
     that answer arriving on scedit's feed. PREDICTED: it arrives (§5.72 — the
     channel's actual semantics, whatever its greeting says).
  D  THE `#!` WRITE-BACK, INTO A CLEAN BUFFER. scedit plays the open file; the
     engine annotates the faulty line at the natural end of the run; scedit sees
     the file change and reloads. Observables: the file on disk, the reloaded
     buffer's error history, and `scedit --history` on the same file from a
     separate process. PREDICTED: one `#!` tail on the `struct if end` line,
     every other byte unchanged, one `spacecrafter` row in --history.
  E  THE DIRTY BUFFER IS REFUSED, and the engine's tail survives byte-exact.
     Same play, then an edit typed into the buffer, then a save. PREDICTED:
     refused, with both choices named, and the file still exactly what the
     engine left.
  E' THE SAME DRIVER, FORCED (`live_dirty_force`). PREDICTED: the tail is GONE
     and the edit is on disk — which is what makes E's refusal a fact about the
     code rather than a hope about the situation.
  F  DISCONNECT AND RECONNECT. PREDICTED: both connections answered.

HOST CONTROL, DISCLOSED (§11.174(h), an owner veto item): with the claude
session's screen LOCKED the compositor throttles the engine to a 1 Hz frame
clock, so this script READS `org.gnome.ScreenSaver.GetActive` before the launch,
RECORDS it, and — if active — wakes the session and keeps it awake for the run.
That is a mitigation, not a fix; it is in the result JSON and in the delivery.

Every launch: the /proc/<pid>/comm instance probe, a temp-HOME farm (never the
field), the frozen config/ssystem md5 asserted in == out. Exit 0 all green,
1 a check failed, 2 no run.
"""
import hashlib, json, os, re, socket, subprocess, sys, threading, time
from pathlib import Path

import logread

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
SCEDIT_DIR = REPO / "util/scedit"
SCEDIT = os.environ.get("SCEDIT_BIN", str(SCEDIT_DIR / "build-lovely/scedit"))
DRIVER = os.environ.get("SCEDIT_DRIVER", str(SCEDIT_DIR / "build-lovely/scedit_tcpclient_test"))
GRAMMAR = str(SCEDIT_DIR / "grammar/sc-grammar.json")
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f67"
FARM = Path("/tmp/sc-farm-f67")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805
ENDPOINT = "127.0.0.1:%d" % PORT

PREDICTIONS = {
    "A": "a0 stars=off, a1 stars=on (a TRANSITION); 0 engine lines for four ordinary commands",
    "B": "exactly 1 position reply on scedit's own connection",
    "C": "the second client's answer arrives on scedit's subscribed feed",
    "D": "one #! tail on the `struct if end` line, every other byte unchanged, "
         "1 spacecrafter row in --history, 0 engine lines on the wire during the play",
    "E": "the save is refused, both choices named, the file byte-identical to what the engine wrote",
    "E'": "forced: the edit is on disk and the #! tail is gone",
    "F": "both connections answered",
}

results = []
notes = {}


def check(name, ok, detail=""):
    results.append({"check": name, "ok": bool(ok), "detail": str(detail)[:400]})
    print(("PASS " if ok else "FAIL ") + name + ("  -- " + str(detail)[:300] if detail else ""),
          flush=True)


def die(msg):
    print("FATAL:", msg)
    sys.exit(2)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def instance_pids():
    out = []
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            comm = open("/proc/%s/comm" % pid).read().strip()
        except OSError:
            continue
        if comm == "spacecrafter":
            out.append(int(pid))
    return out


def gdbus(method, *args):
    cmd = ["gdbus", "call", "--session", "--dest", "org.gnome.ScreenSaver",
           "--object-path", "/org/gnome/ScreenSaver",
           "--method", "org.gnome.ScreenSaver." + method] + list(args)
    r = subprocess.run(cmd, capture_output=True, text=True, env=env)
    return r.stdout.strip() if r.returncode == 0 else "ERROR:" + r.stderr.strip()[:120]


def run_leg(*argv, timeout=180):
    """One leg of util/scedit/tests/tcpclient_test — scedit's own client and
    core, driven headlessly. Returns (rc, stdout)."""
    p = subprocess.run([DRIVER] + list(argv), capture_output=True, text=True, timeout=timeout)
    sys.stdout.write(p.stdout)
    if p.stderr.strip():
        sys.stdout.write("  [stderr] " + p.stderr.strip() + "\n")
    return p.returncode, p.stdout


print(__doc__.split("Every launch:")[0])
print("PREDICTIONS, stated before the run:")
for k, v in PREDICTIONS.items():
    print("  %-3s %s" % (k, v))
print(flush=True)

OUT.mkdir(parents=True, exist_ok=True)
if instance_pids():
    die("a spacecrafter instance is already running: %s" % instance_pids())
for f in (BIN, SCEDIT, DRIVER, GRAMMAR):
    if not Path(f).exists():
        die("not found: " + f)
md5_in = {k: md5(REAL / k) for k in PRISTINE}
for k, v in PRISTINE.items():
    if md5_in[k] != v:
        die("field %s is not the recorded pristine one" % k)

env = dict(os.environ)
env["DISPLAY"] = os.environ.get("DISPLAY", ":2")
auth = sorted(Path("/run/user/%d" % os.getuid()).glob(".mutter-Xwaylandauth.*"))
if auth:
    env["XAUTHORITY"] = str(auth[0])
env.setdefault("DBUS_SESSION_BUS_ADDRESS", "unix:path=/run/user/%d/bus" % os.getuid())
if subprocess.run(["xdpyinfo"], env=env, capture_output=True).returncode != 0:
    die("display %s not usable" % env["DISPLAY"])

# ------------------------------------------------------- the host, disclosed
screensaver_before = gdbus("GetActive")
notes["screensaver_before"] = screensaver_before
print("screensaver GetActive before the launch: %s" % screensaver_before, flush=True)
woke = False
if "true" in screensaver_before:
    notes["mitigation"] = gdbus("SetActive", "false")
    woke = True
    print("MITIGATION APPLIED (owner veto item, §11.174(h)): SetActive false -> %s"
          % notes["mitigation"], flush=True)
time.sleep(2.0)
notes["screensaver_after_wake"] = gdbus("GetActive")
print("screensaver GetActive after the wake: %s" % notes["screensaver_after_wake"], flush=True)
keep_awake = threading.Event()
if woke:
    def awake():
        while not keep_awake.wait(60):
            gdbus("SimulateUserActivity")
    threading.Thread(target=awake, daemon=True).start()

# --------------------------------------------------------------------- farm
subprocess.run([str(HARNESS / "b3_farm.sh"), str(FARM)], check=True, stdout=subprocess.DEVNULL)
sc = FARM / ".spacecrafter"
sess = sc / "sessions"
if sess.is_symlink():
    sess.unlink()
sess.mkdir(exist_ok=True)
env["HOME"] = str(FARM)
S = FARM / "f67"
S.mkdir(exist_ok=True)

# The three scripts. Each carries ONE block-structure fault, which is the ruled
# class the engine's annotator writes for (§11.184).
BODY = ["# f67: the engine's write-back",
        "flag stars on",
        "struct if end",
        "flag stars off"]
D = S / "D.sts"
E = S / "E.sts"
EF = S / "Eforce.sts"
for p in (D, E, EF):
    p.write_bytes(("\n".join(BODY) + "\n").encode("latin-1"))
ORIGINAL = D.read_bytes()

# ------------------------------------------------------------------- launch
app_out = open(OUT / "app.out", "wb")
t0 = time.time()
proc = subprocess.Popen([BIN], env=env, stdout=app_out, stderr=subprocess.STDOUT)
sock = None
while time.time() - t0 < 180:
    if proc.poll() is not None:
        die("engine exited during init rc=%s" % proc.returncode)
    try:
        sock = socket.create_connection(("127.0.0.1", PORT), timeout=2)
        break
    except OSError:
        time.sleep(0.5)
if sock is None:
    die("port never opened")
print("port up after %.1f s" % (time.time() - t0), flush=True)


def script_log():
    # F108: the live script channel is `script.log` (numbered rotation at
    # open); a pre-F108 landed dir still has its dated names.  One rule,
    # in logread.py, which is selftested both ways.
    return logread.text(sc / "log", "script")


# The shipped startup.sts autoplays; wait for its end rather than sleeping.
t = time.time()
while "ScriptMgr: script end" not in script_log() and time.time() - t < 90:
    time.sleep(0.2)
time.sleep(2.0)
sock.close()

# ----------------------------------------------------------------- the legs
print("\n--- A: a command sent by scedit's client is EXECUTED", flush=True)
rcA, outA = run_leg("live_send", ENDPOINT,
                    "flag stars off", "session action save filename a0",
                    "flag stars on", "session action save filename a1")
check("A  the leg's own checks are green", rcA == 0)


def flag_state(name):
    p = sess / (name + ".ini")
    if not p.exists():
        alt = sorted(sess.glob(name + "*"))
        if not alt:
            return None
        p = alt[0]
    m = re.search(r"^stars\s*=\s*(\S+)", p.read_text(encoding="latin-1", errors="replace"), re.M)
    return m.group(1) if m else "(no stars line)"


ON = {"true", "on", "1"}
OFF = {"false", "off", "0"}
a0, a1 = flag_state("a0"), flag_state("a1")
check("A  the two session files were written into the FARM", None not in (a0, a1),
      "sessions/: %s" % sorted(x.name for x in sess.iterdir()))
check("A  TRANSITION off -> on, read through `session action save`",
      a0 is not None and a1 is not None and a0.lower() in OFF and a1.lower() in ON,
      "a0=%s a1=%s" % (a0, a1))
m = re.search(r"engine lines: (\d+)", outA)
check("A  after the subscription, the engine sent NOTHING back for four ordinary commands",
      m is not None and m.group(1) == "0", "engine lines=%s" % (m.group(1) if m else "?"))

print("\n--- B: the reply reaches the connection that asked (§5.47)", flush=True)
rcB, outB = run_leg("live_get", ENDPOINT)
check("B  the leg's own checks are green (exactly one copy on this connection)", rcB == 0)
check("B  a position reply was printed", "reply:" in outB, outB.strip().splitlines()[-1:])

print("\n--- C: the $LOGON feed carries what the engine broadcasts (§5.72)", flush=True)
second_client_said = []


def second_client():
    # Ask, more than once, inside the window the leg is listening for: the leg
    # is a local process and takes about a second to subscribe, and one ask
    # timed by a guess is exactly the kind of thing that makes a green run mean
    # nothing.
    ok = False
    for _ in range(3):
        time.sleep(2.5)
        try:
            s2 = socket.create_connection(("127.0.0.1", PORT), timeout=5)
            s2.sendall(b"get status position\n")
            time.sleep(0.5)
            s2.close()
            ok = True
        except OSError as e:
            second_client_said.append(str(e))
            return
    second_client_said.append(ok)


th = threading.Thread(target=second_client, daemon=True)
th.start()
rcC, outC = run_leg("live_feed", ENDPOINT, "10")
th.join(timeout=20)
check("C  the second client asked its question", second_client_said == [True],
      str(second_client_said))
feed_lines = [l for l in outC.splitlines() if l.strip().startswith("feed:")]
check("C  the leg's own checks are green", rcC == 0)
check("C  the other client's answer arrived on scedit's subscribed feed",
      any(l.count(";") == 5 for l in feed_lines), "\n".join(feed_lines))

print("\n--- D: the `#!` write-back into a CLEAN buffer", flush=True)
rcD, outD = run_leg("live_play", ENDPOINT, GRAMMAR, str(D), "90", timeout=240)
check("D  the leg's own checks are green", rcD == 0)
after = D.read_bytes()
tails = {i: l.split(b"#!", 1)[1].decode("latin-1").strip()
         for i, l in enumerate(after.split(b"\n"), 1) if b"#!" in l}
check("D  exactly line 3 carries a `#!` tail", sorted(tails) == [3], json.dumps(sorted(tails)))
check("D  the tail is the engine's diagnosis of the fault",
      "closes nothing" in tails.get(3, ""), tails.get(3, ""))
stripped = b"\n".join(l.split(b" #! ")[0] for l in after.split(b"\n"))
check("D  every other byte is identical (tails stripped == the original)", stripped == ORIGINAL)
m = re.search(r"engine tails after reload: (\d+)", outD)
check("D  the reloaded CLEAN buffer lists the engine's finding",
      m is not None and int(m.group(1)) == 1, "engine tails=%s" % (m.group(1) if m else "?"))
m = re.search(r"engine lines after the subscription: (\d+)", outD)
check("D  PROTOCOL GAP, measured: after the subscription the engine said NOTHING on the "
      "wire for the whole play — no start, no end, no diagnostic",
      m is not None and m.group(1) == "0", "engine lines=%s" % (m.group(1) if m else "?"))
h = subprocess.run([SCEDIT, "--grammar", GRAMMAR, "--history", str(D)],
                   capture_output=True, text=True)
rows = [r.split("\t") for r in h.stdout.splitlines() if r.strip()]
engine_rows = [r for r in rows if len(r) > 2 and r[2] == "spacecrafter"]
check("D  `scedit --history`, a separate process, lists the engine's row",
      len(engine_rows) == 1 and engine_rows[0][1] == "3" and engine_rows[0][3] == "#!",
      h.stdout.strip())
check("D  and relates it to scedit's own finding on that line (the C1 signal)",
      len(engine_rows) == 1 and "agrees with" in engine_rows[0][6],
      engine_rows[0][6] if engine_rows else "")

print("\n--- E: the DIRTY buffer is refused, and the engine's tail survives", flush=True)
rcE, outE = run_leg("live_dirty", ENDPOINT, GRAMMAR, str(E), "90", timeout=240)
check("E  the leg's own checks are green (the save was REFUSED)", rcE == 0)
eb = E.read_bytes()
check("E  the engine's tail is still on disk", b"#!" in eb)
check("E  the author's edit was NOT written", b"an edit made while the show was running" not in eb)
check("E  the file is byte-exactly what the engine left (tails stripped == the original)",
      b"\n".join(l.split(b" #! ")[0] for l in eb.split(b"\n")) == ORIGINAL)
check("E  the refusal names both choices",
      "reload" in outE and "save anyway" in outE,
      [l for l in outE.splitlines() if "refusal:" in l])

print("\n--- E': the same driver FORCED — the refusal is doing work", flush=True)
rcF1, outF1 = run_leg("live_dirty_force", ENDPOINT, GRAMMAR, str(EF), "90", timeout=240)
check("E' the forced leg's own checks are green", rcF1 == 0)
fb = EF.read_bytes()
check("E' FORCED: the author's edit IS on disk", b"an edit made while the show was running" in fb)
check("E' FORCED: the engine's `#!` tail is GONE — which is what E prevents", b"#!" not in fb)

print("\n--- F: disconnect and reconnect", flush=True)
rcG, outG = run_leg("live_reconnect", ENDPOINT)
check("F  the leg's own checks are green", rcG == 0)

# --------------------------------------------------------------- shutdown
try:
    s = socket.create_connection(("127.0.0.1", PORT), timeout=5)
    s.sendall(b"shutdown action now\n")
    s.close()
except OSError:
    pass
t = time.time()
while proc.poll() is None and time.time() - t < 40:
    time.sleep(0.5)
if proc.poll() is None:
    proc.terminate()
    proc.wait(10)
check("engine exited on `shutdown action now`", proc.returncode == 0, "rc=%s" % proc.returncode)
app_out.close()
keep_awake.set()

md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in)
notes["screensaver_at_end"] = gdbus("GetActive")
# The frame clock, RECORDED not gated: a locked session throttles the engine to
# 1 Hz, and `Frame stall detected` every 1000 ms is what that looks like from
# inside (HOST-EVENTS.md 2026-08-31). It prices the mitigation above.
vk = sc / "log" / "vulkan.log"
notes["frame_stalls"] = (vk.read_text(encoding="latin-1", errors="replace")
                         .count("Frame stall detected") if vk.exists() else "no vulkan.log")
print("frame stalls this run (recorded, not gated): %s" % notes["frame_stalls"], flush=True)

(OUT / "script.log").write_text(script_log(), encoding="latin-1")
for p in (D, E, EF):
    (OUT / p.name).write_bytes(p.read_bytes())
for name in sorted(x.name for x in sess.iterdir()):
    (OUT / ("session_" + name)).write_bytes((sess / name).read_bytes())
ok = all(r["ok"] for r in results)
payload = json.dumps({"binary": BIN, "scedit": SCEDIT, "driver": DRIVER, "endpoint": ENDPOINT,
                      "run": time.strftime("%Y%m%d-%H%M%S"), "predictions": PREDICTIONS,
                      "host": notes, "results": results, "all_ok": ok}, indent=1)
# TWO names, on purpose. The stable one is what a record cites; the timestamped
# one is what keeps a second run from destroying the first run's evidence — and
# the first run's evidence is exactly where the host state that no longer exists
# was written down (this run woke the session; the next one will find it awake).
# The class is F66's `f64_doc_router.py` finding, met again here, in my own
# instrument, on its second run.
(OUT / "f67_result.json").write_text(payload)
(OUT / ("f67_result-%s.json" % time.strftime("%Y%m%d-%H%M%S"))).write_text(payload)
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED",
                              sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
