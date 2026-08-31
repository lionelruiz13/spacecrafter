#!/usr/bin/env python3
"""F68 - the engine knows a TCP line IS a TCP line, and says so where it matters.

    cd claude/harness && DISPLAY=:2 ./f68_provenance.py [absOutdir]  # default artifacts/f68

MANDATE [vixy 2026-08-31]: "spacecrafter script engine must carry the provenance
(file/tcp + line)". INTENT 11.184 built the file half and stopped at the
two-argument overloads; 11.187 is the tcp half. This instrument runs the SAME
battery three times - twice on the PRE-change binary and once on the delivered
one - so that every claim below is measured against what the engine did before.

  phase pre   the binary at master-beta e2c8477b (SC_BIN_PRE)
  phase pre2  the same binary again: the A/A control that says whether this
              battery's WIRE is deterministic at all. Without it, "the wire did
              not change" is a sentence about one sample.
  phase post  the delivered binary (SC_BIN)

WHAT IS ASSERTED, and what each check could have found instead:

  ii  PROVENANCE ON THE WIRE'S OWN COMMANDS. The identical fault
      (`struct if end`, nothing open) is sent five ways in one run: twice by
      client P (once cold, once right after a `clear`, which nests some thirty
      commands), once by a SECOND client Q, once through the HTTP `?command=`
      door, and once from a script FILE. PREDICTED tag sequence, post:
        tcp#<idP> | tcp#<idP> | tcp#<idQ> | (none) | <file>:3
      PRE predicts (none) for the first four - which is the whole point.
      The Q line is what makes `tcp#<id>` mean the CONNECTION and not "tcp":
      if the id were a constant, idP == idQ and this check fails.
  iii NESTING. `media action play audioname nosuch.ogg` nests
      `audio filename ... action play loop `, which fails. PREDICTED: the
      nested refusal carries NO tag on either binary - a nested call passes no
      origin (11.184's rule, unchanged) - and the NEXT fault from the same
      connection carries tcp#<idP> again, so nothing was lost on the way back.
      Had the nested call inherited the outer origin, the first half fails.
  i   THE FILE HALF IS UNCHANGED (control). The file-origin fault still logs
      `script <file>:3: <what> [<line>]` and still gets its `#!` tail; and the
      file-origin FUNNEL refusal (`get status nonsense` on line 5) carries NO
      tag - the generic channel stays where 11.184 left it, Vixy's to open.
      The full 11.184 control is f63_annotations.py on the same binary.
  v   THE `#!` WRITER STILL WRITES FOR FILE ORIGINS ONLY. Every farm file is
      md5'd after the TCP battery and before the play: a TCP-origin fault must
      not have touched one byte of one file. Then exactly one tail, on line 3.
  iv  THE WIRE IS FROZEN (F69's baseline, and this task's hard boundary). Every
      byte received by a $LOGON subscriber, by two plain clients and by the
      HTTP request is recorded per phase and compared: pre == pre2 (the battery
      is deterministic) and pre == post (nothing new is sent). The POSITIVE
      control is asserted too - each of those wires is NON-EMPTY - because
      "identical" over two empty recordings is not evidence of anything.

Every launch: /proc/<pid>/comm instance probe (the pre binary is installed
UNDER THE NAME `spacecrafter` so the probe sees it - 11.134(b)), a temp-HOME
farm (never the field), config/ssystem md5 in == out, plain `timeout`
discipline from the caller. The screensaver state is read and recorded, and
woken if active (a MITIGATION, owner veto item, 11.174(h) / 11.186(a)).
Exit 0 all green, 1 a check failed, 2 no run.
"""
import gzip, hashlib, json, os, re, socket, subprocess, sys, threading, time
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
PRE = os.environ.get("SC_BIN_PRE", "/tmp/f68-pre/spacecrafter")
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f68"
FARM = Path("/tmp/sc-farm-f68")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805
FAULT = "struct if end"
MSG_FAULT = "this 'struct if end' closes nothing"

PREDICTIONS = {
    "ii": "post: the five identical faults are tagged tcp#idP | tcp#idP | tcp#idQ | (none, HTTP) "
          "| <file>:3 ; pre: (none) for the first four, <file>:3 for the last",
    "ii-id": "idP != idQ, and the ids follow the connection order (probe 1, S 2, P 3, Q 4, HTTP 5)",
    "ii-funnel": "post: 'tcp#idP: Could not execute: get status nonsense' AND "
                 "'tcp#idP: command 'get': unknown status value'; pre: both untagged",
    "ii-306": "post: 'tcp#idP: Unrecognized or malformed command name' for `flagg stars on`, "
              "with no 'Could not execute' companion (that emitter bypasses the funnel)",
    "iii": "the NESTED audio refusal is untagged on BOTH binaries; the fault sent after `clear` "
           "is tagged tcp#idP again",
    "i": "the file-origin fault logs `script <file>:3: ... [struct if end]` on both binaries, and "
         "the file-origin funnel refusal (line 5) is untagged on both",
    "v": "no farm file changes during the TCP battery; after the play exactly one `#!` tail, on "
         "line 3, every other byte identical",
    "iv": "S/P/Q/HTTP wires: pre == pre2 (deterministic) and pre == post (frozen), all non-empty",
}

FILE_LINES = [
    "# f68: the same battery, from a file",
    "clear",
    FAULT,
    "media action play audioname nosuch.ogg",
    "get status nonsense",
    "flag stars on",
]

results = []
notes = {}


def check(name, ok, detail=""):
    results.append({"check": name, "ok": bool(ok), "detail": str(detail)[:500]})
    print(("PASS " if ok else "FAIL ") + name + ("  -- " + str(detail)[:400] if detail else ""),
          flush=True)


def die(msg):
    print("FATAL:", msg, flush=True)
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


class Client:
    """A connection that records every byte the engine sends it."""

    def __init__(self, name):
        self.name = name
        self.buf = bytearray()
        self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=5)
        self.sock.settimeout(0.5)
        self.stop = threading.Event()
        self.th = threading.Thread(target=self._read, daemon=True)
        self.th.start()

    def _read(self):
        while not self.stop.is_set():
            try:
                b = self.sock.recv(4096)
            except socket.timeout:
                continue
            except OSError:
                return
            if not b:
                return
            self.buf += b

    def send(self, line, pause=0.6):
        self.sock.sendall((line + "\n").encode("latin-1"))
        time.sleep(pause)

    def close(self):
        self.stop.set()
        time.sleep(0.6)
        try:
            self.sock.close()
        except OSError:
            pass


def http_command(command):
    """The HTTP `?command=` door: same input queue, connection closed at once."""
    s = socket.create_connection(("127.0.0.1", PORT), timeout=5)
    s.settimeout(5)
    s.sendall(("GET /?command=" + command.replace(" ", "+") + " HTTP/1.0\r\n\r\n").encode("latin-1"))
    got = bytearray()
    try:
        while True:
            b = s.recv(4096)
            if not b:
                break
            got += b
    except (socket.timeout, OSError):
        pass
    s.close()
    return bytes(got)


def battery(phase, binary):
    """One launch, one full battery. Returns everything measured."""
    print("\n=== phase %s: %s" % (phase, binary), flush=True)
    if instance_pids():
        die("a spacecrafter instance is already running: %s" % instance_pids())
    subprocess.run([str(HARNESS / "b3_farm.sh"), str(FARM)], check=True, stdout=subprocess.DEVNULL)
    sc = FARM / ".spacecrafter"
    env["HOME"] = str(FARM)
    S = FARM / "f68"
    S.mkdir(parents=True, exist_ok=True)
    play_file = S / "f68_file.sts"
    play_file.write_bytes(("\n".join(FILE_LINES) + "\n").encode("latin-1"))
    decoy = S / "untouched.sts"
    decoy.write_bytes(b"# f68: never played, never annotated\nflag stars on\n")
    original = play_file.read_bytes()

    app_out = open(OUT / ("app.%s.out" % phase), "wb")
    t0 = time.time()
    proc = subprocess.Popen([binary], env=env, stdout=app_out, stderr=subprocess.STDOUT)
    probe = None
    while time.time() - t0 < 240:
        if proc.poll() is not None:
            die("engine exited during init rc=%s" % proc.returncode)
        try:
            probe = socket.create_connection(("127.0.0.1", PORT), timeout=2)
            break
        except OSError:
            time.sleep(0.5)
    if probe is None:
        die("port never opened")
    probe.close()                      # connection id 1, and gone
    print("port up after %.1f s" % (time.time() - t0), flush=True)

    def script_log():
        logs = sorted((sc / "log").glob("script-*.log"))
        return logs[-1].read_text(encoding="latin-1", errors="replace") if logs else ""

    t = time.time()
    while "ScriptMgr: script end" not in script_log() and time.time() - t < 120:
        time.sleep(0.2)
    time.sleep(2.0)
    mark = len(script_log())           # everything below is the battery's own

    sub = Client("S")                  # id 2: the $LOGON subscriber
    sub.send("$LOGON")
    p = Client("P")                    # id 3: the plain client that drives
    p.send(FAULT)                      # 1  cold TCP fault
    p.send("get status nonsense")      # 2  the funnel
    p.send("flagg stars on")           # 3  the emitter that bypasses the funnel
    p.send("media action play audioname nosuch.ogg")   # 4  a NESTED refusal
    p.send("clear", pause=3.0)         # 5  thirty nested commands
    p.send(FAULT)                      # 6  and the origin is still P's
    p.send("get status object")        # 7  an answer, on the wire, both ways
    q = Client("Q")                    # id 4: a second connection, another id
    q.send(FAULT)
    http_wire = http_command(FAULT)    # id 5: the HTTP door, mapped not wired
    time.sleep(1.5)

    farm_before_play = {str(f): md5(f) for f in sorted(S.glob("*.sts"))}
    log_before_play = script_log()

    p.send("script action play filename %s" % play_file, pause=0.2)
    t = time.time()
    while time.time() - t < 90:
        txt = script_log()[mark:]
        if ("ScriptMgr: load %s" % play_file) in txt and \
           "ScriptMgr: script end" in txt.split("ScriptMgr: load %s" % play_file, 1)[1]:
            break
        time.sleep(0.2)
    time.sleep(1.5)

    log = script_log()[mark:]
    played = play_file.read_bytes()
    farm_after = {str(f): md5(f) for f in sorted(S.glob("*.sts"))}

    wires = {"S": bytes(sub.buf), "P": bytes(p.buf), "Q": bytes(q.buf), "HTTP": http_wire}
    p.send("shutdown action now", pause=0.2)
    for c in (sub, p, q):
        c.close()
    t = time.time()
    while proc.poll() is None and time.time() - t < 60:
        time.sleep(0.5)
    if proc.poll() is None:
        proc.terminate()
        proc.wait(10)
    app_out.close()
    check("%-5s engine exited on `shutdown action now`" % phase, proc.returncode == 0,
          "rc=%s" % proc.returncode)

    (OUT / ("script.%s.log.gz" % phase)).write_bytes(gzip.compress(log.encode("latin-1")))
    (OUT / ("played.%s.sts" % phase)).write_bytes(played)
    for k, v in wires.items():
        (OUT / ("wire.%s.%s.bin" % (phase, k))).write_bytes(v)
    return {"log": log, "played": played, "original": original, "wires": wires,
            "farm_before_play": farm_before_play, "farm_after": farm_after,
            "log_before_play_len": len(log_before_play),
            "frame_stalls": ((sc / "log" / "vulkan.log").read_text(encoding="latin-1",
                             errors="replace").count("Frame stall detected")
                             if (sc / "log" / "vulkan.log").exists() else "no vulkan.log"),
            "play_file": str(play_file)}


# --------------------------------------------------------------- preflight
print(__doc__.split("Every launch:")[0])
print("PREDICTIONS, stated before the run:")
for k, v in PREDICTIONS.items():
    print("  %-10s %s" % (k, v))
print(flush=True)

OUT.mkdir(parents=True, exist_ok=True)
for f in (BIN, PRE):
    if not Path(f).exists():
        die("not found: " + f)
if Path(BIN).name != "spacecrafter" or Path(PRE).name != "spacecrafter":
    die("both binaries must be named `spacecrafter` so the comm probe sees them (11.134(b))")
md5_in = {k: md5(REAL / k) for k in PRISTINE}
for k, v in PRISTINE.items():
    if md5_in[k] != v:
        die("field %s is not the recorded pristine one" % k)
notes["binaries"] = {"post": BIN, "post_md5": md5(BIN), "pre": PRE, "pre_md5": md5(PRE)}
if notes["binaries"]["post_md5"] == notes["binaries"]["pre_md5"]:
    die("pre and post are the same binary: nothing would be discriminated")

env = dict(os.environ)
env["DISPLAY"] = os.environ.get("DISPLAY", ":2")
auth = sorted(Path("/run/user/%d" % os.getuid()).glob(".mutter-Xwaylandauth.*"))
if auth:
    env["XAUTHORITY"] = str(auth[0])
env.setdefault("DBUS_SESSION_BUS_ADDRESS", "unix:path=/run/user/%d/bus" % os.getuid())
if subprocess.run(["xdpyinfo"], env=env, capture_output=True).returncode != 0:
    die("display %s not usable" % env["DISPLAY"])

notes["screensaver_before"] = gdbus("GetActive")
print("screensaver GetActive before the runs: %s" % notes["screensaver_before"], flush=True)
if "true" in notes["screensaver_before"]:
    notes["mitigation"] = gdbus("SetActive", "false")
    print("MITIGATION APPLIED (owner veto item, 11.174(h)): SetActive false -> %s"
          % notes["mitigation"], flush=True)
keep_awake = threading.Event()


def awake():
    while not keep_awake.wait(60):
        gdbus("SimulateUserActivity")


threading.Thread(target=awake, daemon=True).start()   # the blank half is still live (11.186(a))
notes["wake_thread"] = "SimulateUserActivity every 60 s for the whole run"

# ------------------------------------------------------------------- runs
runs = {}
for phase, binary in (("pre", PRE), ("pre2", PRE), ("post", BIN)):
    runs[phase] = battery(phase, binary)
    notes["frame_stalls_" + phase] = runs[phase]["frame_stalls"]
keep_awake.set()
notes["screensaver_at_end"] = gdbus("GetActive")

# --------------------------------------------------------------- readers
def first_diff(a, b):
    """Where two wires part, said usefully: a red run should not need a second one."""
    if a == b:
        return "identical"
    n = min(len(a), len(b))
    i = next((k for k in range(n) if a[k] != b[k]), n)
    return "first difference at byte %d: %r vs %r" % (i, a[i:i + 60], b[i:i + 60])


def fault_origins(log):
    """The origin each `end without if` diagnostic named, in order. `script: <what>`
    (no origin) is read as ''."""
    out = []
    for line in log.splitlines():
        if MSG_FAULT not in line or "(Error)" not in line:
            continue
        body = line.split("(Error): ", 1)[1]
        if body.startswith("script: "):
            out.append("")
        else:
            out.append(body[len("script "):].split(": ", 1)[0])
    return out


def lines_with(log, needle):
    return [l.split("(Debug): ", 1)[-1] for l in log.splitlines() if needle in l]


def tails(raw):
    out = {}
    for i, line in enumerate(raw.split(b"\n"), 1):
        if b"#!" in line:
            out[i] = line.split(b"#!", 1)[1].decode("latin-1").strip()
    return out


post, pre, pre2 = runs["post"], runs["pre"], runs["pre2"]
pf = post["play_file"]

# ------------------------------------------------- ii: the tag sequence
seq_post = fault_origins(post["log"])
seq_pre = fault_origins(pre["log"])
notes["fault_origins_post"] = seq_post
notes["fault_origins_pre"] = seq_pre
check("ii   post: five identical faults, five diagnostics", len(seq_post) == 5, json.dumps(seq_post))
check("ii   pre : five identical faults, five diagnostics", len(seq_pre) == 5, json.dumps(seq_pre))
idP = seq_post[0] if seq_post else ""
idQ = seq_post[2] if len(seq_post) > 2 else ""
check("ii   post: the first fault names the CONNECTION it came from (tcp#<id>)",
      idP.startswith("tcp#") and idP[4:].isdigit(), idP)
check("ii   post: the same connection's second fault names the SAME id",
      len(seq_post) > 1 and seq_post[1] == idP, json.dumps(seq_post[:2]))
check("ii   post: a SECOND connection gets a DIFFERENT id (the tag is not a constant)",
      idQ.startswith("tcp#") and idQ != idP, "P=%s Q=%s" % (idP, idQ))
notes["ids_observed"] = {"P": idP, "Q": idQ, "predicted": "tcp#3 / tcp#4"}
check("ii-id post: the ids are the connection order's own - Q is the NEXT id after P",
      idP[4:].isdigit() and idQ[4:].isdigit() and int(idQ[4:]) == int(idP[4:]) + 1
      and int(idP[4:]) >= 2,
      "P=%s Q=%s (predicted tcp#3 / tcp#4: %s)"
      % (idP, idQ, "yes" if (idP, idQ) == ("tcp#3", "tcp#4") else "NO - recorded, see notes"))
check("ii   post: the HTTP `?command=` door carries NO origin (mapped, not wired)",
      len(seq_post) > 3 and seq_post[3] == "", json.dumps(seq_post))
check("ii   post: the FILE fault still names <file>:3",
      len(seq_post) > 4 and seq_post[4] == pf + ":3", json.dumps(seq_post[4:]))
check("ii   PRE (both ways): the same four wire faults name NOTHING at all",
      seq_pre[:4] == ["", "", "", ""], json.dumps(seq_pre))
check("ii   PRE: and its FILE fault names <file>:3 exactly as post does",
      len(seq_pre) > 4 and seq_pre[4] == pre["play_file"] + ":3", json.dumps(seq_pre[4:]))
check("ii   post: the tagged diagnostic quotes the offending line, as the file one does",
      any((idP + ": " + MSG_FAULT) in l and ("[" + FAULT + "]") in l
          for l in post["log"].splitlines()),
      [l for l in post["log"].splitlines() if idP + ": " + MSG_FAULT in l][:1])

# ------------------------------------------------- ii: the funnel and the :306 emitter
cne_post = lines_with(post["log"], "Could not execute: get status nonsense")
cne_pre = lines_with(pre["log"], "Could not execute: get status nonsense")
check("ii   post: the funnel names the origin of a TCP refusal",
      len(cne_post) == 2 and cne_post[0].startswith(idP + ": "), json.dumps(cne_post))
check("ii   post: the funnel's SECOND line (the message) carries it too",
      any(l.startswith(idP + ": ") and "unknown status value" in l
          for l in lines_with(post["log"], "unknown status value")),
      json.dumps(lines_with(post["log"], "unknown status value")))
check("ii   PRE (both ways): the same refusal named no origin",
      len(cne_pre) == 2 and not any(l.startswith("tcp#") for l in cne_pre), json.dumps(cne_pre))
unrec_post = lines_with(post["log"], "Unrecognized or malformed command name")
unrec_pre = lines_with(pre["log"], "Unrecognized or malformed command name")
check("ii-306 post: the emitter that bypasses the funnel carries the origin too",
      len(unrec_post) == 1 and unrec_post[0] == idP + ": Unrecognized or malformed command name",
      json.dumps(unrec_post))
check("ii-306 PRE: it named no origin", len(unrec_pre) == 1 and unrec_pre[0].startswith("Unrec"),
      json.dumps(unrec_pre))
check("ii-306 post: and still no 'Could not execute' companion for it (5.117's map holds)",
      not any("flagg" in l for l in lines_with(post["log"], "Could not execute")),
      json.dumps(lines_with(post["log"], "Could not execute")))

# ------------------------------------------------- iii: nesting
nested_post = lines_with(post["log"], "Could not execute: audio filename")
nested_pre = lines_with(pre["log"], "Could not execute: audio filename")
check("iii  post: a NESTED command's refusal carries NO tag (11.184's rule, unchanged)",
      len(nested_post) == 2 and not any(l.startswith("tcp#") for l in nested_post),
      json.dumps(nested_post))
check("iii  PRE: the same two lines, identically untagged",
      [l for l in nested_pre] == [l for l in nested_post], json.dumps(nested_pre))
check("iii  post: the fault sent AFTER `clear` (thirty nested calls) still names tcp#idP",
      len(seq_post) > 1 and seq_post[1] == idP, json.dumps(seq_post[:2]))

# ------------------------------------------------- i: the file half, unchanged
file_funnel_post = [l for l in lines_with(post["log"], "Could not execute: get status nonsense")]
check("i    post: the FILE funnel refusal is UNTAGGED - the generic channel stays Vixy's",
      len(file_funnel_post) == 2 and file_funnel_post[1] == "Could not execute: get status nonsense",
      json.dumps(file_funnel_post))
tp, tpre = tails(post["played"]), tails(pre["played"])
check("i    post: exactly one `#!` tail, on line 3", sorted(tp) == [3], json.dumps(sorted(tp)))
check("i    post: the tail is the engine's diagnosis", MSG_FAULT in tp.get(3, ""), tp.get(3, ""))
check("i    PRE: the same one tail on the same line (the file half did not move)",
      sorted(tpre) == [3] and MSG_FAULT in tpre.get(3, ""), json.dumps(sorted(tpre)))
stripped = b"\n".join(l.split(b" #! ")[0] for l in post["played"].split(b"\n"))
check("i    post: every other byte of the played file is identical",
      stripped == post["original"])

# ------------------------------------------------- v: the annotator writes files only for files
DECOY = b"# f68: never played, never annotated\nflag stars on\n"
farmdir = Path(post["play_file"]).parent
expect_before = {str(farmdir / "f68_file.sts"): hashlib.md5(post["original"]).hexdigest(),
                 str(farmdir / "untouched.sts"): hashlib.md5(DECOY).hexdigest()}
check("v    post: the TCP battery touched NO file - every farm script byte-identical "
      "after five wire faults and before the play",
      post["farm_before_play"] == expect_before,
      json.dumps({"observed": post["farm_before_play"], "expected": expect_before}))
check("v    post: the never-played file is untouched after the play too",
      post["farm_after"].get(str(farmdir / "untouched.sts")) ==
      hashlib.md5(DECOY).hexdigest(), json.dumps(post["farm_after"]))

# ------------------------------------------------- iv: the frozen wire (F69's baseline)
for who in ("S", "P", "Q", "HTTP"):
    a, b, c = pre["wires"][who], pre2["wires"][who], post["wires"][who]
    check("iv   %-4s A/A control: pre == pre2, so this wire is deterministic" % who, a == b,
          "%d vs %d bytes; %s" % (len(a), len(b), first_diff(a, b)))
    check("iv   %-4s FROZEN: post == pre, byte for byte" % who, a == c,
          "%d vs %d bytes; %s" % (len(a), len(c), first_diff(a, c)))
check("iv   the subscriber's wire is NOT empty (otherwise 'identical' proves nothing)",
      len(post["wires"]["S"]) > 0, repr(post["wires"]["S"])[:200])
check("iv   the asking client's wire is NOT empty either",
      len(post["wires"]["P"]) > 0, repr(post["wires"]["P"])[:200])
check("iv   the HTTP door answered too", len(post["wires"]["HTTP"]) > 0,
      repr(post["wires"]["HTTP"])[:120])
check("iv   the subscriber received the OTHER client's answer (5.72's broadcast, live)",
      b"EOL" in post["wires"]["S"] or b";" in post["wires"]["S"], repr(post["wires"]["S"])[:200])

md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in)

ok = all(r["ok"] for r in results)
payload = json.dumps({"predictions": PREDICTIONS, "host": notes, "results": results,
                      "run": time.strftime("%Y%m%d-%H%M%S"), "all_ok": ok}, indent=1)
(OUT / "f68_result.json").write_text(payload)
(OUT / ("f68_result-%s.json" % time.strftime("%Y%m%d-%H%M%S"))).write_text(payload)
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED",
                              sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
