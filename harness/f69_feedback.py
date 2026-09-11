#!/usr/bin/env python3
"""F69 - a TCP refusal comes back on a link nobody else is on.

    cd claude/harness && DISPLAY=:2 ./f69_feedback.py [absOutdir]  # default artifacts/f69

MANDATE [vixy 2026-08-31]: "feedback about tcp sent back (note: an existing tcp
path exists, used by masterput (which is closed-source), do not modify this
channel) - and sent it back through the tcp link dedicated for scedit."

That sentence has two halves and they pull in opposite directions, so this
instrument measures both in ONE battery, three times:

  phase pre   the PRE-change binary (SC_BIN_PRE)
  phase pre2  the same binary again: the A/A control that says whether this
              battery's WIRE is deterministic at all. Without it, "the wire did
              not change" is a sentence about one sample.
  phase post  the delivered binary (SC_BIN)

THE PRE BINARY'S ERA IS DECLARED, NOT GUESSED (SC_PRE_ERA, added at F72). This
instrument is re-run by every later task that touches a socket, and by then the
"pre" binary is not the one F69 was written against. Which engine the pre
binary IS decides what several legs must assert, so it is an INPUT: inferring
it from the run would make those legs assert whatever they measured.

  SC_PRE_ERA=f68  pre is master-beta 423cbe23 or earlier: no diagnostic
                  channel at all. `$DIAGON` is an ordinary (unrecognised)
                  command to it, D's wire is empty, and scedit's live leg is
                  given the opposite expectation. The era F69 delivered against.
  SC_PRE_ERA=f69  pre is be2ddd81..96cfc352: the channel exists, and file-origin
                  refusals are NOT tagged in the log. The era F72 delivered
                  against - where D's wire on the pre binary must be non-empty
                  and BYTE-IDENTICAL to post's, which is how "a file-origin
                  refusal adds zero wire bytes" gets measured inside one run
                  instead of across two.
  SC_PRE_ERA=f72  pre is 1014e5a5: the channel exists AND both origins are
                  tagged, and the funnel still writes its LEGACY TWO LINES.
                  THE DEFAULT, and the era F73 delivered against. F73 collapses
                  those two into ONE - the origin-prefixed INTENT-MODIFIED line
                  (raw line + ` #! <message>`) - so the needle `Could not
                  execute` no longer exists for a refusal that has an origin,
                  and leg viii is rebuilt around the new shape while the WIRE
                  legs stay exactly where they were (INTENT 11.194).

A wrong declaration turns those legs RED; it cannot make a run pass. (Before
F72 there was no such input: F70 ran this instrument against an f69-era pre and
took three known-false reds, which is the failure mode 11.191(c) rules out for
a gating instrument - a false positive is a defect to remove at its root.)

WHAT IS ASSERTED, and what each check could have found instead:

  iv  THE UNSUBSCRIBED WIRE IS FROZEN - the hard boundary, and a delta here is
      a defect and not a judgment call. A $LOGON subscriber (S), the client
      that drives (P), a second client that only sends (Q) and an HTTP request
      record every byte they receive, and those recordings must be identical
      pre == pre2 == post. They are also compared against F68's COMMITTED
      wires (artifacts/f68/wire.pre.*): a different task, a different run, a
      different day - 71 B to the subscriber, 5 B to the asking client, 85 B
      to HTTP, 0 B to Q. If adding a subscriber to the run changed what the
      others hear, this is where it shows.
      The positive control is asserted too - three of those four wires are
      NON-EMPTY - because "identical" over empty recordings proves nothing.
      And S, who subscribed to the OTHER channel, must receive no `$DIAG`
      record at all: byte-identity says nothing arrived, this says the right
      nothing arrived.
  vi  THE SUBSCRIBED WIRE CARRIES IT. D sends `$DIAGON` and records what it
      gets. PREDICTED, post: the confirmation, then one `$DIAG|tcp#<idP>|...`
      record per refusal P causes, one tagged with Q's DIFFERENT id, and
      NOTHING for the HTTP-origin fault or for the file script's own faults.
      PRE predicts an EMPTY wire for D - on the pre binary `$DIAGON` is not a
      verb, it is an unrecognised command, and the log says so. That log line
      appearing on pre and NOT on post is the verb's own both-ways control.
  vii THE SUBSCRIPTION CAN BE DROPPED. After `$DIAGOFF`, one more fault from
      P: D's wire must grow by the confirmation and by nothing else. A channel
      that cannot be left is not opt-in.
  viii THE LOG KEEPS EVERYTHING. The wire adds a copy, it never diverts one -
      and since F73 the log says it ONCE: where the pre binary wrote a tagged
      pair (subject line + message line) the delivered one writes the single
      intent-modified line, recomposed here from the script's own bytes
      (f73_line.py). The five structure faults are in the log in the same
      numbers on both, and the NESTED refusal keeps the two-line form on both,
      which is where the collapse stops.
  ix  SCEDIT ITSELF, on the same launch. `scedit_tcpclient_test live_diag`
      connects with its own client, sends a command the engine refuses, and
      reads it back off the wire - on post expecting a diagnostic, on pre
      expecting NONE. The same leg, the same binary, the opposite expectation:
      that is what says the post green measured the engine and not the client's
      own hopes. It runs AFTER the wires above are snapshotted, so nothing it
      asks can reach the recordings leg iv compares.
  v   NO FILE IS TOUCHED by any of it (F68's leg v, re-run): the `#!` writer
      still writes only for file origins, and a TCP fault touches no byte of
      any script.

Every launch: /proc/<pid>/comm instance probe (both binaries are named
`spacecrafter` so the probe sees them - 11.134(b)), a temp-HOME farm (never
the field), config/ssystem md5 in == out, plain `timeout` discipline from the
caller. The screensaver state is read and recorded at each phase (both arms of
the blank hazard were removed by the owner mid-round, 11.186(a) - this is the
zero-cost check that says so per run). The X server's start epoch is recorded
per phase too: it restarted under F68's runs and that is not yet explained.
Exit 0 all green, 1 a check failed, 2 no run.
"""
import gzip, hashlib, json, os, re, socket, subprocess, sys, threading, time
from pathlib import Path
from f73_line import error_lines, rendered, script_line
import logread

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
BIN = os.environ.get("SC_BIN", str(REPO / "build-claude/src/spacecrafter"))
PRE = os.environ.get("SC_BIN_PRE", "/tmp/f69-pre/spacecrafter")
OUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f69"
F68 = HARNESS / "artifacts/f68"
FARM = Path("/tmp/sc-farm-f69")
REAL = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
PORT = 7805
SCEDIT_TEST = os.environ.get("SCEDIT_TCP_TEST",
                             str(REPO / "util/scedit/build-lovely/scedit_tcpclient_test"))
FAULT = "struct if end"
MSG_FAULT = "this 'struct if end' closes nothing"
UNREC = "Unrecognized or malformed command name"
# Which engine the PRE binary is. Declared by the caller, never inferred from the
# run - see the docstring. "f72" is 1014e5a5, "f69" is be2ddd81..96cfc352, "f68"
# is 423cbe23 or older.
PRE_ERA = os.environ.get("SC_PRE_ERA", "f72")
PRE_ERAS = ("f68", "f69", "f72")
# The two things an era decides here, kept apart because they moved apart: does
# the pre engine HAVE the diagnostic channel, and does it TAG a file origin.
PRE_HAS_CHANNEL = PRE_ERA != "f68"
PRE_TAGS_FILE = PRE_ERA == "f72"

# The bytes F68 measured, on another run of another binary on another day.
F68_WIRE = {"S": 71, "P": 5, "Q": 0, "HTTP": 85}
# F70 re-measured the same four AND recorded the SUBSCRIBED wire, which F68 had
# no channel to record. 939 B is the frozen diagnostic wire; F72 must reproduce
# it byte for byte, because a file-origin refusal that leaked onto a socket
# would show up here and nowhere else.
F70_WIRE_DIR = HARNESS / "artifacts/f70/wire"
F70_D_BYTES = 939

PREDICTIONS = {
    "iv": "S/P/Q/HTTP wires: pre == pre2 (deterministic) and pre == post (frozen), and each "
          "equal BYTE FOR BYTE to F68's committed wire - 71 / 5 / 0 / 85 B",
    "iv-S": "the $LOGON subscriber receives NO record beginning `$DIAG` in any phase",
    "iv-D": "the SUBSCRIBED wire too: D's 939 B reproduce F70's committed recording byte for "
            "byte (a file-origin refusal that leaked onto a socket would show here and nowhere "
            "else) - asserted when SC_PRE_ERA=f69, where pre and post both have the channel",
    "vi-pre": "SC_PRE_ERA=f68: on the pre binary D's wire is EMPTY (0 B), `$DIAGON` is not a verb "
              "there but a command, and the script log carries `Unrecognized or malformed command "
              "name` THREE times ($DIAGON, $DIAGOFF, flagg) where post carries it once. "
              "SC_PRE_ERA=f69: D's wire is NON-empty and byte-identical to post's, and both logs "
              "carry that line exactly once",
    "vi-post": "D's wire opens with `$DIAGON ok:` and then carries one `$DIAG|tcp#<id>|msg|subject` "
               "record per TCP-origin refusal: the cold fault, the funnel's `get status nonsense`, "
               "the bypassed `flagg stars on`, ~~the nested audio refusal~~, the fault after "
               "`clear`, and one tagged with Q's OTHER id. "
               "[CORRECTED after the first run, and the correction is the interesting part: this "
               "enumeration listed the NESTED audio refusal, and the check written beside it "
               "asserts the opposite - a nested call passes no origin (11.184's rule), so it "
               "routes nowhere. The measurement agreed with the CHECK: 5 records, none about "
               "audio. The prediction text was wrong where the check was right; both were "
               "committed before the run, so the disagreement is on the record rather than "
               "resolved by whichever one the result happened to match.]",
    "vi-none": "NO `$DIAG` record names a file path or an empty origin: the HTTP-origin fault and "
               "every fault of the played FILE route nowhere",
    "ix": "scedit's OWN client, on the same launch: `live_diag` green on post (a refused command "
          "comes back, split into origin/message/subject); on pre green WITH `none` when "
          "SC_PRE_ERA=f68 (the same leg, the opposite expectation, because that engine has no "
          "such verb) and green WITHOUT it when SC_PRE_ERA=f69 (that engine has the verb, and a "
          "green there says F72 did not break the consumer)",
    "vii": "after `$DIAGOFF` the confirmation arrives and then nothing: one more fault from P adds "
           "0 bytes to D",
    "viii": "the log keeps the error and stops repeating it: TWO refusals rendered as ONE line "
            "each on the delivered binary where the pre binary wrote a tagged PAIR each, the "
            "same number of `struct if end` faults on both, and the NESTED refusal still in "
            "the two-line form on both (the collapse reaches named origins and stops there)",
    "viii-f73": "the file-origin funnel refusal reads `Error executing <file>:5: get status "
                "nonsense #! command 'get': unknown status value` - a line this instrument "
                "recomposes from the script's own bytes - and the pre binary's two lines are "
                "exactly its two halves; D's wire does not gain one byte for it, which is "
                "F73's boundary stated as one pair of legs",
    "v": "no farm file changes during the TCP battery; after the play exactly one `#!` tail, on "
         "line 3, every other byte identical",
}

FILE_LINES = [
    "# f69: the same battery, from a file",
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


def xserver_start():
    """The X server's own start time: it restarted under F68's runs (11.187(i)
    recorded the note), and a stack that moves mid-battery is worth seeing."""
    try:
        pid = subprocess.run(["pgrep", "-f", "Xwayland.*:2"], capture_output=True,
                             text=True).stdout.split()
        if not pid:
            return "no Xwayland :2"
        stat = open("/proc/%s/stat" % pid[0]).read().rsplit(")", 1)[1].split()
        btime = 0
        for line in open("/proc/stat"):
            if line.startswith("btime "):
                btime = int(line.split()[1])
        hz = os.sysconf("SC_CLK_TCK")
        return int(btime + int(stat[19]) / hz)
    except (OSError, IndexError, ValueError) as e:
        return "unreadable: %s" % e


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
    notes["xserver_start_before_" + phase] = xserver_start()
    notes["screensaver_before_" + phase] = gdbus("GetActive")
    subprocess.run([str(HARNESS / "b3_farm.sh"), str(FARM)], check=True, stdout=subprocess.DEVNULL)
    sc = FARM / ".spacecrafter"
    env["HOME"] = str(FARM)
    S = FARM / "f69"
    S.mkdir(parents=True, exist_ok=True)
    play_file = S / "f69_file.sts"
    play_file.write_bytes(("\n".join(FILE_LINES) + "\n").encode("latin-1"))
    decoy = S / "untouched.sts"
    decoy.write_bytes(b"# f69: never played, never annotated\nflag stars on\n")
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
        # F108: the live script channel is `script.log` (numbered rotation at
        # open); a pre-F108 landed dir still has its dated names.  One rule,
        # in logread.py, which is selftested both ways.
        return logread.text(sc / "log", "script")

    t = time.time()
    while "ScriptMgr: script end" not in script_log() and time.time() - t < 120:
        time.sleep(0.2)
    time.sleep(2.0)
    mark = len(script_log())           # everything below is the battery's own

    sub = Client("S")                  # id 2: the $LOGON subscriber - the frozen wire
    sub.send("$LOGON")
    d = Client("D")                    # id 3: the DEDICATED link
    d.send("$DIAGON")
    p = Client("P")                    # id 4: the plain client that drives
    p.send(FAULT)                      # 1  cold TCP fault (reportScriptError)
    p.send("get status nonsense")      # 2  the funnel
    p.send("flagg stars on")           # 3  the emitter that bypasses the funnel
    p.send("media action play audioname nosuch.ogg")   # 4  a NESTED refusal
    p.send("clear", pause=3.0)         # 5  thirty nested commands
    p.send(FAULT)                      # 6  and the origin is still P's
    p.send("get status object")        # 7  an answer, on the wire, both ways
    q = Client("Q")                    # id 5: a second connection, another id
    q.send(FAULT)
    http_wire = http_command(FAULT)    # id 6: the HTTP door, mapped not wired
    time.sleep(1.5)

    farm_before_play = {str(f): md5(f) for f in sorted(S.glob("*.sts"))}

    p.send("script action play filename %s" % play_file, pause=0.2)
    t = time.time()
    while time.time() - t < 90:
        txt = script_log()[mark:]
        if ("ScriptMgr: load %s" % play_file) in txt and \
           "ScriptMgr: script end" in txt.split("ScriptMgr: load %s" % play_file, 1)[1]:
            break
        time.sleep(0.2)
    time.sleep(1.5)

    # vii: the subscription can be dropped, and dropping it is the whole of it.
    diag_before_off = bytes(d.buf)
    d.send("$DIAGOFF")
    off_confirmation = bytes(d.buf)[len(diag_before_off):]
    p.send(FAULT, pause=2.0)           # one more, with nobody subscribed
    diag_after_off = bytes(d.buf)

    log = script_log()[mark:]
    played = play_file.read_bytes()
    farm_after = {str(f): md5(f) for f in sorted(S.glob("*.sts"))}

    wires = {"S": bytes(sub.buf), "P": bytes(p.buf), "Q": bytes(q.buf), "HTTP": http_wire,
             "D": bytes(d.buf)}

    # ix: SCEDIT'S OWN CLIENT, on this engine, on this launch - and it runs
    # AFTER the wires above are snapshotted, so nothing it asks can reach the
    # recordings leg iv compares. Its `get status position` would otherwise be
    # broadcast to S, which is exactly the kind of accident leg iv exists to
    # catch and would have caught.
    # The leg is given the OPPOSITE expectation per phase: on post a refused
    # command must come back, on pre it must not. Same binary, same leg, and a
    # green that means two different things.
    scedit_leg = None
    if Path(SCEDIT_TEST).exists():
        # The expectation is the DECLARED era's, not the phase's: an f68-era pre
        # engine has no verb and must come back with nothing; an f69-era one has
        # it and must come back with a diagnostic, exactly as post does.
        expect_none = (phase != "post" and not PRE_HAS_CHANNEL)
        argv = [SCEDIT_TEST, "live_diag", "127.0.0.1:%d" % PORT] + \
               (["none"] if expect_none else [])
        try:
            r = subprocess.run(argv, capture_output=True, text=True, timeout=180)
            scedit_leg = {"rc": r.returncode, "argv": argv}
            (OUT / ("scedit.%s.txt" % phase)).write_text(r.stdout + r.stderr)
        except subprocess.TimeoutExpired:
            scedit_leg = {"rc": "timeout", "argv": argv}
    p.send("shutdown action now", pause=0.2)
    for c in (sub, d, p, q):
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
    notes["xserver_start_after_" + phase] = xserver_start()
    notes["screensaver_after_" + phase] = gdbus("GetActive")

    (OUT / ("script.%s.log.gz" % phase)).write_bytes(gzip.compress(log.encode("latin-1")))
    (OUT / ("played.%s.sts" % phase)).write_bytes(played)
    for k, v in wires.items():
        (OUT / ("wire.%s.%s.bin" % (phase, k))).write_bytes(v)
    return {"log": log, "played": played, "original": original, "wires": wires,
            "scedit_leg": scedit_leg,
            "farm_before_play": farm_before_play, "farm_after": farm_after,
            "diag_before_off": diag_before_off, "off_confirmation": off_confirmation,
            "diag_after_off": diag_after_off,
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
if PRE_ERA not in PRE_ERAS:
    die("SC_PRE_ERA must be one of %s (see the docstring), not %r" % (PRE_ERAS, PRE_ERA))
notes["pre_era_declared"] = PRE_ERA
print("PRE binary era, DECLARED: SC_PRE_ERA=%s\n" % PRE_ERA, flush=True)
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

# ------------------------------------------------------------------- runs
runs = {}
for phase, binary in (("pre", PRE), ("pre2", PRE), ("post", BIN)):
    runs[phase] = battery(phase, binary)
    notes["frame_stalls_" + phase] = runs[phase]["frame_stalls"]

# --------------------------------------------------------------- readers
def first_diff(a, b):
    """Where two wires part, said usefully: a red run should not need a second one."""
    if a == b:
        return "identical"
    n = min(len(a), len(b))
    i = next((k for k in range(n) if a[k] != b[k]), n)
    return "first difference at byte %d: %r vs %r" % (i, a[i:i + 60], b[i:i + 60])


def records(wire):
    """The wire, split the way the engine frames it: one record per NUL."""
    return [r.decode("latin-1") for r in wire.split(b"\x00") if r]


def diag_records(wire):
    """The `$DIAG|...` records, parsed into their four fields. The subject is the
    LAST field and may itself contain the separator, so the split is bounded."""
    out = []
    for r in records(wire):
        for line in r.split("\n"):
            if line.startswith("$DIAG|"):
                f = line.split("|", 3)
                out.append({"origin": f[1] if len(f) > 1 else "",
                            "message": f[2] if len(f) > 2 else "",
                            "subject": f[3] if len(f) > 3 else "",
                            "raw": line})
    return out


def fault_origins(log):
    """The origin each `end without if` diagnostic named, in order."""
    out = []
    for line in log.splitlines():
        if MSG_FAULT not in line or "(Error)" not in line:
            continue
        body = line.split("(Error): ", 1)[1]
        out.append("" if body.startswith("script: ") else body[len("script "):].split(": ", 1)[0])
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
seq_post = fault_origins(post["log"])
idP = seq_post[0] if seq_post else ""
idQ = next((o for o in seq_post if o.startswith("tcp#") and o != idP), "")
notes["ids_observed"] = {"P": idP, "Q": idQ}
notes["fault_origins_post"] = seq_post
notes["fault_origins_pre"] = fault_origins(pre["log"])

# ---------------------------------------- iv: the unsubscribed wire is FROZEN
for who in ("S", "P", "Q", "HTTP"):
    a, b, c = pre["wires"][who], pre2["wires"][who], post["wires"][who]
    check("iv   %-4s A/A control: pre == pre2, so this wire is deterministic" % who, a == b,
          "%d vs %d bytes; %s" % (len(a), len(b), first_diff(a, b)))
    check("iv   %-4s FROZEN: post == pre, byte for byte" % who, a == c,
          "%d vs %d bytes; %s" % (len(a), len(c), first_diff(a, c)))
    f68wire = F68 / ("wire.pre.%s.bin" % who)
    if f68wire.exists():
        want = f68wire.read_bytes()
        check("iv   %-4s == F68's committed wire (another run, another day, %d B)"
              % (who, F68_WIRE[who]), c == want and len(want) == F68_WIRE[who],
              "%d B now vs %d B then; %s" % (len(c), len(want), first_diff(want, c)))
    else:
        check("iv   %-4s F68's committed wire is present to compare against" % who, False,
              "missing: %s" % f68wire)
check("iv   the $LOGON subscriber's wire is NOT empty (identity over nothing proves nothing)",
      len(post["wires"]["S"]) > 0, repr(post["wires"]["S"])[:200])
check("iv   the asking client's wire is NOT empty either", len(post["wires"]["P"]) > 0,
      repr(post["wires"]["P"])[:200])
check("iv   the HTTP door answered too", len(post["wires"]["HTTP"]) > 0,
      repr(post["wires"]["HTTP"])[:120])
check("iv-S the $LOGON subscriber received NO `$DIAG` record - the right nothing, not just nothing",
      b"$DIAG" not in post["wires"]["S"], repr(post["wires"]["S"])[:200])
check("iv-S ... and neither did the plain clients P and Q",
      b"$DIAG" not in post["wires"]["P"] and b"$DIAG" not in post["wires"]["Q"],
      "%r / %r" % (post["wires"]["P"][:80], post["wires"]["Q"][:80]))
# The SUBSCRIBED wire, frozen against F70's committed recording. F68 could not
# record this one (it had no channel), so the baseline starts at F70 - and it is
# the only place a diagnostic that leaked to a socket would appear.
f70_D = F70_WIRE_DIR / "wire.post.D.bin"
if f70_D.exists():
    want_D = f70_D.read_bytes()
    check("iv-D the SUBSCRIBED wire == F70's committed %d B recording, byte for byte "
          "(another task, another binary, another day)" % F70_D_BYTES,
          post["wires"]["D"] == want_D and len(want_D) == F70_D_BYTES,
          "%d B now vs %d B then; %s"
          % (len(post["wires"]["D"]), len(want_D), first_diff(want_D, post["wires"]["D"])))
else:
    check("iv-D F70's committed subscribed wire is present to compare against", False,
          "missing: %s" % f70_D)

# ---------------------------------------- vi: the subscribed wire carries it
dpost = post["wires"]["D"]
dpre = pre["wires"]["D"]
notes["D_wire_post_len"] = len(dpost)
notes["D_wire_pre_len"] = len(dpre)
notes["D_records_post"] = records(dpost)
unrec_pre = lines_with(pre["log"], UNREC)
unrec_post = lines_with(post["log"], UNREC)
if not PRE_HAS_CHANNEL:
    check("vi-pre  D's wire on the PRE binary is EMPTY: `$DIAGON` was not a verb there",
          len(dpre) == 0 and len(pre2["wires"]["D"]) == 0,
          "pre %d B / pre2 %d B: %r" % (len(dpre), len(pre2["wires"]["D"]), dpre[:120]))
    check("vi-pre  ... and the PRE engine logged THREE unrecognised commands: $DIAGON, $DIAGOFF "
          "and `flagg` - the two verbs were ordinary commands to it", len(unrec_pre) == 3,
          json.dumps(unrec_pre))
else:
    check("vi-pre  (declared %s) D's wire on the PRE binary is NON-EMPTY and BYTE-IDENTICAL "
          % PRE_ERA +
          "to post's: this engine already had the channel, and the delivered one adds not one "
          "byte to it - a file-origin refusal routes to no socket",
          len(dpre) > 0 and dpre == dpost and dpre == pre2["wires"]["D"],
          "pre %d B / pre2 %d B / post %d B; %s"
          % (len(dpre), len(pre2["wires"]["D"]), len(dpost), first_diff(dpre, dpost)))
    check("vi-pre  ... and the PRE engine logged the unrecognised command ONCE, as post does: "
          "the two verbs were already verbs to it", len(unrec_pre) == 1, json.dumps(unrec_pre))
check("vi-post the POST engine logs exactly ONE (`flagg` alone): the socket layer consumed the "
      "two verbs before the application ever saw them", len(unrec_post) == 1,
      json.dumps(unrec_post))
check("vi-post D's wire is NOT empty", len(dpost) > 0, repr(dpost)[:120])
check("vi-post the first record is the subscription's own confirmation",
      records(dpost) and records(dpost)[0].startswith("$DIAGON ok:"),
      json.dumps(records(dpost)[:1]))
dr = diag_records(dpost)
notes["D_diag_records"] = dr
check("vi-post D received diagnostics at all", len(dr) > 0, "%d records" % len(dr))
check("vi-post every one names a TCP origin - no file path, no empty origin (the HTTP-origin "
      "fault and the played file's own faults route NOWHERE)",
      all(re.fullmatch(r"tcp#\d+", r["origin"]) for r in dr),
      json.dumps([r["origin"] for r in dr]))
check("vi-post the cold `struct if end` came back, quoted, tagged with P's id",
      any(r["origin"] == idP and MSG_FAULT in r["message"] and r["subject"] == FAULT for r in dr),
      json.dumps([r["raw"] for r in dr if MSG_FAULT in r["message"]][:2]))
check("vi-post the FUNNEL's refusal came back with its command line",
      any(r["origin"] == idP and "unknown status value" in r["message"]
          and r["subject"] == "get status nonsense" for r in dr),
      json.dumps([r["raw"] for r in dr if "unknown status" in r["message"]]))
check("vi-post the emitter that BYPASSES the funnel came back too",
      any(r["origin"] == idP and UNREC in r["message"] and r["subject"] == "flagg stars on"
          for r in dr),
      json.dumps([r["raw"] for r in dr if UNREC in r["message"]]))
check("vi-post a SECOND connection's fault came back tagged with ITS id, not P's",
      idQ and idQ != idP and any(r["origin"] == idQ for r in dr),
      "P=%s Q=%s; origins seen %s" % (idP, idQ, json.dumps(sorted({r["origin"] for r in dr}))))
check("vi-post the fault sent after `clear` (thirty nested calls) is tagged tcp#idP again",
      len([r for r in dr if r["origin"] == idP and MSG_FAULT in r["message"]]) >= 2,
      json.dumps([r["raw"] for r in dr if MSG_FAULT in r["message"]]))
# A NESTED refusal carries no origin (11.184's rule, preserved by F68 by
# measurement), so it routes NOWHERE - and that is a consequence worth asserting
# rather than discovering: a subscriber that sends
# `media action play audioname nosuch.ogg` is told nothing about why it failed,
# because the failing command is the inner one. Recorded in the entry as a named
# gap; if the rule ever changes, this check goes red.
check("vi-post the NESTED audio refusal routes NOWHERE - the inner command has no origin, so "
      "the subscriber hears nothing about it (11.184's rule, and its cost)",
      not any("audio" in r["subject"] or "audio" in r["message"] for r in dr),
      json.dumps([r["raw"] for r in dr if "audio" in r["raw"]]))
check("vi-post ... and the log DID record that nested refusal, so the silence above is the "
      "channel's and not the run's",
      len(lines_with(post["log"], "Could not execute: audio filename")) > 0,
      json.dumps(lines_with(post["log"], "Could not execute: audio filename")))
notes["D_nonascii_bytes"] = sum(1 for c in dpost if c > 127)

# ---------------------------------------- vii: the subscription can be dropped
check("vii  `$DIAGOFF` is confirmed", post["off_confirmation"].startswith(b"$DIAGOFF ok:"),
      repr(post["off_confirmation"])[:160])
check("vii  and then a further fault adds NOTHING to D's wire",
      post["diag_after_off"] == post["diag_before_off"] + post["off_confirmation"],
      "before %d + confirmation %d = %d, observed %d"
      % (len(post["diag_before_off"]), len(post["off_confirmation"]),
         len(post["diag_before_off"]) + len(post["off_confirmation"]),
         len(post["diag_after_off"])))
n_log_faults_P = len([o for o in seq_post if o == idP])
n_wire_faults_P = len([r for r in dr if r["origin"] == idP and MSG_FAULT in r["message"]])
check("vii  the fault D did not hear about DID happen: the log has THREE faults from P and D's "
      "wire carries TWO - the missing one is the one sent after $DIAGOFF",
      n_log_faults_P == 3 and n_wire_faults_P == 2,
      "log %d from %s, wire %d" % (n_log_faults_P, idP, n_wire_faults_P))

# ---------------------------------------- viii: the log keeps everything
MSG_STATUS = "command 'get': unknown status value"
err_post = error_lines(post["log"])
notes["error_lines_post"] = err_post
gs_post = [l for l in err_post if "get status nonsense" in l]
cne_pre = lines_with(pre["log"], "Could not execute: get status nonsense")
msg_pre = lines_with(pre["log"], MSG_STATUS)
check("viii the funnel writes ONE line per refusal where the pre binary wrote TWO - the log "
      "did not lose the error, it stopped saying it twice (11.193(a))",
      len(gs_post) == 2 and len(cne_pre) == 2 and len(msg_pre) == 2,
      "post %s / pre %s" % (json.dumps(gs_post), json.dumps([cne_pre, msg_pre])))
check("viii the NESTED refusal still comes in the two-line form on BOTH binaries - the "
      "collapse reaches exactly the origins that have a name, and 11.184's nesting rule is "
      "where it was",
      len(lines_with(post["log"], "Could not execute: audio filename"))
      == len(lines_with(pre["log"], "Could not execute: audio filename")) > 0,
      json.dumps(lines_with(post["log"], "Could not execute: audio filename")))
check("viii the structure faults are in the log in the same number on both",
      len(fault_origins(post["log"])) == len(fault_origins(pre["log"])) > 0,
      "post %s / pre %s" % (json.dumps(fault_origins(post["log"])),
                            json.dumps(fault_origins(pre["log"]))))
PRE_FILE_TAG5 = (pf + ":5: ") if PRE_TAGS_FILE else ""
raw5 = script_line(post["original"], 5)
check("viii-f73 the file-origin refusal IS line 5 of that file with the message on it - "
      "recomposed here from the script's own bytes, not read back from the engine "
      "(11.193(a): the log line and the file line are one rendering)",
      len(gs_post) == 2 and gs_post[1] == rendered(pf + ":5", raw5, MSG_STATUS),
      json.dumps({"expected": rendered(pf + ":5", raw5, MSG_STATUS),
                  "observed": gs_post[1] if len(gs_post) > 1 else None, "raw": raw5}))
check("viii-f73 ... and the pre binary wrote the tagged PAIR its era declares (SC_PRE_ERA=%s), "
      "whose two halves are exactly the halves of the one line above" % PRE_ERA,
      len(cne_pre) == 2 and len(msg_pre) == 2
      and cne_pre[1] == PRE_FILE_TAG5 + "Could not execute: get status nonsense"
      and msg_pre[1] == PRE_FILE_TAG5 + MSG_STATUS
      and len(gs_post) == 2
      and gs_post[1] == rendered(pf + ":5",
                                 cne_pre[1][len(PRE_FILE_TAG5):]
                                 .split("Could not execute: ", 1)[1],
                                 msg_pre[1][len(PRE_FILE_TAG5):]),
      json.dumps([cne_pre[1], msg_pre[1]]))
check("viii-f73 ... while D's wire carries NO record whose origin is that file: the log half "
      "moved AGAIN and the socket half still did not, which is F73's boundary as it was F72's",
      not any(pf in r["raw"] for r in dr) and not any(pf in r["origin"] for r in dr),
      json.dumps([r["raw"] for r in dr]))

# ---------------------------------------- v: no file is touched
DECOY = b"# f69: never played, never annotated\nflag stars on\n"
farmdir = Path(post["play_file"]).parent
expect_before = {str(farmdir / "f69_file.sts"): hashlib.md5(post["original"]).hexdigest(),
                 str(farmdir / "untouched.sts"): hashlib.md5(DECOY).hexdigest()}
check("v    the TCP battery touched NO file - every farm script byte-identical before the play",
      post["farm_before_play"] == expect_before,
      json.dumps({"observed": post["farm_before_play"], "expected": expect_before}))
tp = tails(post["played"])
check("v    exactly one `#!` tail, on line 3", sorted(tp) == [3], json.dumps(sorted(tp)))
check("v    the tail is the engine's diagnosis", MSG_FAULT in tp.get(3, ""), tp.get(3, ""))
stripped = b"\n".join(l.split(b" #! ")[0] for l in post["played"].split(b"\n"))
check("v    every other byte of the played file is identical", stripped == post["original"])
check("v    the never-played file is untouched after the play too",
      post["farm_after"].get(str(farmdir / "untouched.sts")) == hashlib.md5(DECOY).hexdigest(),
      json.dumps(post["farm_after"]))

# ---------------------------------------- ix: scedit consumes it, live
notes["scedit_legs"] = {k: runs[k]["scedit_leg"] for k in runs}
pre_what = ("the same leg, expecting NOTHING (this engine has no verb)" if not PRE_HAS_CHANNEL
            else "the same leg, same expectation (this engine has the verb): a green says the "
                 "consumer still reads what it read before")
for phase, what in (("post", "a refused command COMES BACK"), ("pre", pre_what)):
    leg = runs[phase]["scedit_leg"]
    check("ix   scedit's own client on the live engine, phase %-4s: %s" % (phase, what),
          leg is not None and leg["rc"] == 0, json.dumps(leg))
if not PRE_HAS_CHANNEL:
    check("ix   the two phases were given OPPOSITE expectations (otherwise both greens are one)",
          runs["post"]["scedit_leg"] is not None and runs["pre"]["scedit_leg"] is not None
          and "none" not in runs["post"]["scedit_leg"]["argv"]
          and "none" in runs["pre"]["scedit_leg"]["argv"],
          json.dumps([runs["post"]["scedit_leg"], runs["pre"]["scedit_leg"]]))
else:
    # No opposite expectation is available in this era, and saying so is better
    # than pretending: what discriminates HERE is that the leg has a failing
    # form at all - it goes red against an engine with no channel, which is the
    # f68 arm, and the wire-identity leg above carries the F72 claim instead.
    check("ix   both phases were given the SAME (positive) expectation, per SC_PRE_ERA=%s - "
          % PRE_ERA +
          "the discrimination for this run is iv-D/vi-pre wire identity, not this leg",
          runs["post"]["scedit_leg"] is not None and runs["pre"]["scedit_leg"] is not None
          and "none" not in runs["post"]["scedit_leg"]["argv"]
          and "none" not in runs["pre"]["scedit_leg"]["argv"],
          json.dumps([runs["post"]["scedit_leg"], runs["pre"]["scedit_leg"]]))

md5_out = {k: md5(REAL / k) for k in PRISTINE}
check("real ~/.spacecrafter config/ssystem md5 in == out", md5_out == md5_in)

ok = all(r["ok"] for r in results)
payload = json.dumps({"predictions": PREDICTIONS, "host": notes, "results": results,
                      "run": time.strftime("%Y%m%d-%H%M%S"), "all_ok": ok}, indent=1)
(OUT / "f69_result.json").write_text(payload)
(OUT / ("f69_result-%s.json" % time.strftime("%Y%m%d-%H%M%S"))).write_text(payload)
print("\n%s: %d/%d checks" % ("ALL GREEN" if ok else "RED",
                              sum(r["ok"] for r in results), len(results)))
sys.exit(0 if ok else 1)
