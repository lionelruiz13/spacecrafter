#!/usr/bin/env python3
"""F68/F72 - the engine knows where a command line came from, and says so where it matters.

    cd claude/harness && DISPLAY=:0 ./f68_provenance.py [absOutdir]  # default artifacts/f68

MANDATE [vixy 2026-08-31]: "spacecrafter script engine must carry the provenance
(file/tcp + line)". INTENT 11.184 built the file half and stopped at the
two-argument overloads; 11.187 is the tcp half. 11.191(b) - *"good idea"*, the
same owner, later the same day - reverses the one asymmetry 11.187(d) left: the
diagnostic funnel and its bypassing sibling now name a FILE origin too. This
instrument runs the SAME battery three times - twice on the PRE-change binary
and once on the delivered one - so that every claim below is measured against
what the engine did before.

  phase pre   the PRE-change binary (SC_BIN_PRE)
  phase pre2  the same binary again: the A/A control that says whether this
              battery's WIRE is deterministic at all. Without it, "the wire did
              not change" is a sentence about one sample.
  phase post  the delivered binary (SC_BIN)

THE PRE BINARY'S ERA IS DECLARED, NOT GUESSED (SC_PRE_TAGS, added at F72).
Which origins the pre binary tags is half of what this instrument checks, so it
is an INPUT: inferring it from the run would make every pre-side leg assert
whatever it happened to measure, which is a green that cannot fail.

  SC_PRE_TAGS=none  pre predates F68 (master-beta e2c8477b): no origin tag at
                    all. The era F68 was delivered against.
  SC_PRE_TAGS=tcp   pre is F68..F71 (423cbe23 .. 96cfc352): `tcp#<id>: ` on a
                    control-line refusal and NOTHING on a file one. The era
                    F72 was delivered against.
  SC_PRE_TAGS=all   pre is F72 (1014e5a5): BOTH origins tagged, and the funnel
                    still writes its LEGACY TWO LINES - `<tag>Could not
                    execute: <command line>` then `<tag><message>`. THE
                    DEFAULT, and the era F73 was delivered against.

A wrong declaration turns the pre-side legs RED; it cannot make a run pass.

WHAT F73 CHANGED, and why the legs below could not stay as they were: the two
lines are ONE now, and that one line is the INTENT-MODIFIED LINE - the raw
line as read, the author's comment kept, with ` #! <message>` appended and any
tail already there replaced - behind `Error executing <origin>: `. So the
needle `Could not execute` no longer exists for an origin that has a name, and
every post-side expectation here is rebuilt around a line this instrument
COMPOSES ITSELF from the script's own bytes (INTENT 11.193, 11.194).

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
  r   THE RENDERING ITSELF, F73's own leg and the sharpest one here. The gate
      RECOMPUTES the line it expects from the SCRIPT'S OWN BYTES and the
      message, through f73_line.py's re-implementation of 11.184's tail rule
      - never the engine's function, because a gate that asks the engine what
      it should have printed measures nothing. Four cases, one
      read-only script: an author's comment KEPT with the tail after it, a
      stale `#!` tail REPLACED and not doubled, a `#!` inside quotes left as
      text (the parser's own quote toggle), and the file byte-UNTOUCHED while
      the log shows the line anyway - the owner's read-only clause, measured.
  x   THE SAME FAULT, THE SAME LINE, TWICE - the owner's own discriminator
      [vixy 2026-09-01]: *"where the script with the error is re-executed,
      [it] would make the same error being logged differently on the first
      execution vs the next ones"*. A WRITABLE script whose fault IS annotated
      (the ruled class) is played twice through a natural end, so the second
      execution reads the line the first execution wrote a tail onto.
      PREDICTED: post logs the same bytes both times; PRE logs a longer line
      the second time, because it quoted the tail it had just written. That
      opposition is what makes this leg discriminate rather than pass.
  i   THE FILE HALF - a CHANGE leg since F72, and a different change now.
      The file-origin structure fault still logs `script <file>:3: <what>
      [<line>]` and still gets its `#!` tail; what F73 moved there is only that
      the quoted line is shown WITHOUT its machine tail, which is leg x's
      subject. What MOVED here is the funnel (`get status nonsense`, line 5)
      and its bypassing sibling (`flagg stars on`, line 7, added to the played
      file at F72 so the second emitter's file half is measured rather than
      assumed): each is now ONE rendered line where the pre binary wrote a
      tagged PAIR. The sharpest form of the claim is asserted directly - the
      rendered line equals this instrument's own recomposition, and separately
      equals the pre binary's two lines joined at ` #! `, so nothing was
      invented and nothing was dropped. What is NOT widened: the `#!` channel
      (leg v md5s every farm file; the full 11.184 control is
      f63_annotations.py on the same binary) and the wire (leg iv).
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
from f73_line import error_lines, rendered, script_line

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
UNREC = "Unrecognized or malformed command name"
# Which origins the PRE binary tags, and how it renders a refusal. Declared by
# the caller, never inferred from the run - see the docstring. "all" is F72
# (both origins tagged, legacy TWO lines), "tcp" is F68..F71, "none" is older.
PRE_TAGS = os.environ.get("SC_PRE_TAGS", "all")
PRE_ERAS = ("none", "tcp", "all")
# Every pre era wrote the funnel's TWO lines; they differ only in which origins
# they prefixed. F73 is the first era to write ONE.
PRE_TAGS_TCP = PRE_TAGS in ("tcp", "all")
PRE_TAGS_FILE = PRE_TAGS == "all"

PREDICTIONS = {
    "ii": "post: the five identical faults are tagged tcp#idP | tcp#idP | tcp#idQ | (none, HTTP) "
          "| <file>:3 ; pre: per SC_PRE_TAGS for the first four, <file>:3 for the last either way",
    "ii-id": "idP != idQ, and the ids follow the connection order (probe 1, S 2, P 3, Q 4, HTTP 5)",
    "ii-funnel": "post: 'tcp#idP: Could not execute: get status nonsense' AND "
                 "'tcp#idP: command 'get': unknown status value'; pre: tagged iff SC_PRE_TAGS=tcp",
    "ii-306": "post: 'tcp#idP: Unrecognized or malformed command name' for `flagg stars on`, "
              "with no 'Could not execute' companion (that emitter bypasses the funnel)",
    "iii": "the NESTED audio refusal is untagged on BOTH binaries - no tcp# and no file path, "
           "which is what says F72 did not widen 11.184's nesting rule; the fault sent after "
           "`clear` is tagged tcp#idP again",
    "i": "the file-origin structure fault logs `script <file>:3: ... [struct if end]` on both "
         "binaries (reportScriptError is unmoved by F72)",
    "i-f73": "post: the file-origin FUNNEL refusal on line 5 is ONE line reading 'Error "
             "executing <file>:5: get status nonsense #! command ...: unknown status value', "
             "and the bypassing sibling on line 7 is one line of the same shape; the needle "
             "'Could not execute' does not occur for either. PRE (declared era): TWO lines "
             "each, tagged per SC_PRE_TAGS",
    "r": "every refusal line the post binary logs for a named origin equals this instrument's "
         "OWN recomposition of it from the script bytes: raw line + ' #! ' + message, an "
         "existing tail replaced, a quoted '#!' left as text, an author's comment kept",
    "r-ro": "the read-only script's three refusals are logged as intent-modified lines while "
            "the file stays byte-identical and the annotator says it could not write",
    "x": "post: the twice-played script's fault logs the SAME BYTES on both executions; PRE: "
         "the second execution's line is LONGER, because it quotes the tail the first wrote; "
         "both binaries leave the file identical after the second execution",
    "v": "no farm file changes during the TCP battery; after the play exactly one `#!` tail, on "
         "line 3, every other byte identical - F72 adds a refusing line to the played file and "
         "the annotator must still write nothing for it",
    "iv": "S/P/Q/HTTP wires: pre == pre2 (deterministic) and pre == post (frozen), all non-empty",
}

FILE_LINES = [
    "# f68: the same battery, from a file",
    "clear",
    FAULT,
    "media action play audioname nosuch.ogg",
    "get status nonsense",
    "flag stars on",
    # F72: the emitter that BYPASSES the funnel, reached from a FILE this time.
    # Appended rather than inserted so lines 3 and 5 keep their numbers and every
    # expectation F68 recorded against this file still reads the same line.
    "flagg stars on",
]

# F73's own scripts. They are SEPARATE files rather than more lines in the one
# above, and that is deliberate three times over: the file above keeps every
# byte and every line number F68 and F72 measured against; playing a file adds
# not one byte to any wire (a file-origin refusal routes nowhere, and `script
# action play` is answered by nothing), so leg iv still compares what it always
# did; and the faults chosen below produce messages NO existing leg counts, so
# the counting legs above cannot be disturbed by their presence.
#
# The twice-played one carries a RULED-class fault - the only class the writer
# annotates - because the owner's argument is about a script whose tail LANDS:
# the second execution reads a line the first execution changed.
TWICE_LINES = [
    "# f73: the same fault, played twice - the log line may not move",
    "struct loop end",
]
# The read-only one carries the rendering cases. `get nonsense value` refuses
# with "command 'get': unknown argument", a message no other leg here counts.
RO_LINES = [
    "# f73: the rendering cases, in a directory that cannot be written",
    "get nonsense value # an author's own note",
    "get nonsense value #! a stale tail from an older run",
    'get nonsense "a #! inside quotes"',
    "struct loop end",
]
RO_DIR = Path("/tmp/f68-ro")
UNKNOWN_ARG = "command 'get': unknown argument"
LOOP_FAULT = "this 'struct loop end' closes nothing"

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
    twice_file = S / "f73_twice.sts"
    twice_file.write_bytes(("\n".join(TWICE_LINES) + "\n").encode("latin-1"))
    twice_original = twice_file.read_bytes()
    # A read-only DIRECTORY, not a read-only file: 11.184 measured that the
    # file's own mode does not stop a rename into it, so the mode that makes
    # the write fail is the directory's.
    subprocess.run(["chmod", "-R", "u+w", str(RO_DIR)], capture_output=True)
    subprocess.run(["rm", "-rf", str(RO_DIR)], check=True)
    RO_DIR.mkdir(parents=True)
    ro_file = RO_DIR / "f73_ro.sts"
    ro_file.write_bytes(("\n".join(RO_LINES) + "\n").encode("latin-1"))
    ro_original = ro_file.read_bytes()
    os.chmod(RO_DIR, 0o555)

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

    def play(path, since):
        """One play, waited out to its NATURAL end - the annotator flushes
        there and nowhere else, so a leg about what landed in a file needs
        this wait and not a sleep."""
        p.send("script action play filename %s" % path, pause=0.2)
        t = time.time()
        while time.time() - t < 90:
            txt = script_log()[since:]
            if ("ScriptMgr: load %s" % path) in txt and \
               "ScriptMgr: script end" in txt.split("ScriptMgr: load %s" % path, 1)[1]:
                break
            time.sleep(0.2)
        time.sleep(1.5)

    play(play_file, mark)

    # F73: the same fault, twice, on a WRITABLE script - the tail the first
    # execution writes is what the second execution reads.
    twice_mark_1 = len(script_log())
    play(twice_file, twice_mark_1)
    twice_after_1 = twice_file.read_bytes()
    twice_mark_2 = len(script_log())
    play(twice_file, twice_mark_2)
    twice_after_2 = twice_file.read_bytes()
    twice_log_1 = script_log()[twice_mark_1:twice_mark_2]
    twice_log_2 = script_log()[twice_mark_2:]
    # F73: the rendering cases, where the write cannot land.
    ro_mark = len(script_log())
    play(ro_file, ro_mark)
    ro_after = ro_file.read_bytes()
    ro_log = script_log()[ro_mark:]

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

    os.chmod(RO_DIR, 0o755)            # so the next phase can rebuild it
    (OUT / ("script.%s.log.gz" % phase)).write_bytes(gzip.compress(log.encode("latin-1")))
    (OUT / ("played.%s.sts" % phase)).write_bytes(played)
    (OUT / ("twice.%s.sts" % phase)).write_bytes(twice_after_2)
    (OUT / ("ro.%s.sts" % phase)).write_bytes(ro_after)
    for k, v in wires.items():
        (OUT / ("wire.%s.%s.bin" % (phase, k))).write_bytes(v)
    return {"log": log, "played": played, "original": original, "wires": wires,
            "twice_file": str(twice_file), "twice_original": twice_original,
            "twice_after_1": twice_after_1, "twice_after_2": twice_after_2,
            "twice_log_1": twice_log_1, "twice_log_2": twice_log_2,
            "ro_file": str(ro_file), "ro_original": ro_original, "ro_after": ro_after,
            "ro_log": ro_log,
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
if PRE_TAGS not in PRE_ERAS:
    die("SC_PRE_TAGS must be one of %s (see the docstring), not %r" % (PRE_ERAS, PRE_TAGS))
notes["pre_tags_declared"] = PRE_TAGS
print("PRE binary era, DECLARED: SC_PRE_TAGS=%s\n" % PRE_TAGS, flush=True)
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


def pre_tcp_line(line, body):
    """What the PRE binary is DECLARED to have written for a TCP-origin
    diagnostic whose body is `body` - read off SC_PRE_TAGS, never off the run."""
    if not PRE_TAGS_TCP:
        return line == body
    return re.fullmatch(r"tcp#\d+: " + re.escape(body), line) is not None


def pre_file_line(line, body, origin):
    """The same question for a FILE origin: tagged only in the `all` era."""
    return line == ((origin + ": " if PRE_TAGS_FILE else "") + body)


# ---- the intent-modified line: recomposed by f73_line.py, from 11.184's stated
# contract rather than from the engine's own function - and living in ONE file
# because f69_feedback.py asserts against the same line (I2, which is F73's own
# subject: a second copy of this composition inside a second gate would be the
# defect the shape it checks exists to remove).


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
if PRE_TAGS == "none":
    pre_wire_ok = seq_pre[:4] == ["", "", "", ""]
    pre_wire_what = "name NOTHING at all"
else:
    pre_wire_ok = (len(seq_pre) > 3
                   and re.fullmatch(r"tcp#\d+", seq_pre[0]) is not None
                   and seq_pre[1] == seq_pre[0]
                   and re.fullmatch(r"tcp#\d+", seq_pre[2]) is not None
                   and seq_pre[2] != seq_pre[0] and seq_pre[3] == "")
    pre_wire_what = "ALREADY name their connection (tcp#idP | tcp#idP | tcp#idQ | none, HTTP)"
check("ii   PRE (declared SC_PRE_TAGS=%s): the four wire faults %s" % (PRE_TAGS, pre_wire_what),
      pre_wire_ok, json.dumps(seq_pre))
check("ii   PRE: and its FILE fault names <file>:3 exactly as post does",
      len(seq_pre) > 4 and seq_pre[4] == pre["play_file"] + ":3", json.dumps(seq_pre[4:]))
check("ii   post: the tagged diagnostic quotes the offending line, as the file one does",
      any((idP + ": " + MSG_FAULT) in l and ("[" + FAULT + "]") in l
          for l in post["log"].splitlines()),
      [l for l in post["log"].splitlines() if idP + ": " + MSG_FAULT in l][:1])

# ------------------------------------------------- ii: the funnel and the :306 emitter
MSG_STATUS = "command 'get': unknown status value"
err_post = error_lines(post["log"])
notes["error_lines_post"] = err_post
gs_post = [l for l in err_post if "get status nonsense" in l]
cne_pre = lines_with(pre["log"], "Could not execute: get status nonsense")
msg_pre = lines_with(pre["log"], "unknown status value")
check("ii   post: the funnel writes ONE line for a TCP refusal and it IS the line that was "
      "sent, with the message as its `#!` tail (11.193(a): one rendering, and the log is one "
      "of its sinks)",
      len(gs_post) == 2 and gs_post[0] == rendered(idP, "get status nonsense", MSG_STATUS),
      json.dumps(gs_post))
check("ii   post: the two-line form is GONE for every refusal that has an origin - no "
      "`Could not execute` companion survives for one",
      not any("get status nonsense" in l for l in lines_with(post["log"], "Could not execute")),
      json.dumps(lines_with(post["log"], "Could not execute")))
check("ii   PRE (declared SC_PRE_TAGS=%s): TWO lines for the same refusal, tagged per its era "
      "- the control that says the collapse is this change and not this battery" % PRE_TAGS,
      len(cne_pre) == 2 and len(msg_pre) == 2
      and pre_tcp_line(cne_pre[0], "Could not execute: get status nonsense")
      and pre_tcp_line(msg_pre[0], MSG_STATUS)
      and pre_file_line(cne_pre[1], "Could not execute: get status nonsense",
                        pre["play_file"] + ":5")
      and pre_file_line(msg_pre[1], MSG_STATUS, pre["play_file"] + ":5"),
      json.dumps([cne_pre, msg_pre]))
unrec_post = [l for l in err_post if UNREC in l]
unrec_pre = lines_with(pre["log"], UNREC)
check("ii-306 post: the emitter that BYPASSES the funnel renders the same way - same function, "
      "so the two sites cannot drift apart",
      len(unrec_post) == 2 and unrec_post[0] == rendered(idP, "flagg stars on", UNREC),
      json.dumps(unrec_post))
check("ii-306 PRE (declared %s): the TCP one per its era, the FILE one per its era" % PRE_TAGS,
      len(unrec_pre) == 2 and pre_tcp_line(unrec_pre[0], UNREC)
      and pre_file_line(unrec_pre[1], UNREC, pre["play_file"] + ":7"),
      json.dumps(unrec_pre))

# ------------------------------------------------- iii: nesting
nested_post = lines_with(post["log"], "Could not execute: audio filename")
nested_pre = lines_with(pre["log"], "Could not execute: audio filename")
check("iii  post: a NESTED command's refusal carries NO tag - not tcp#, and since F72 not a "
      "file path either (11.184's nesting rule, NOT widened by the reversal)",
      len(nested_post) == 2
      and not any(l.startswith("tcp#") or l.startswith(pf) for l in nested_post),
      json.dumps(nested_post))
check("iii  PRE: the same two lines, identically untagged",
      [l for l in nested_pre] == [l for l in nested_post], json.dumps(nested_pre))
check("iii  post: the fault sent AFTER `clear` (thirty nested calls) still names tcp#idP",
      len(seq_post) > 1 and seq_post[1] == idP, json.dumps(seq_post[:2]))

# ------------------------------------------------- i: the file half, F73's shape, both ways
# The pre binary's FILE tag, per the DECLARED era (empty before F72).
PRE_FILE_TAG5 = (pre["play_file"] + ":5: ") if PRE_TAGS_FILE else ""
raw5 = script_line(post["original"], 5)
raw7 = script_line(post["original"], 7)
check("i-f73 post: the FILE funnel refusal is ONE line, and it is line 5 OF THAT FILE with the "
      "message on it - recomposed here from the script's own bytes, never read back from the "
      "engine's own composition",
      len(gs_post) == 2 and gs_post[1] == rendered(pf + ":5", raw5, MSG_STATUS),
      json.dumps({"expected": rendered(pf + ":5", raw5, MSG_STATUS),
                  "observed": gs_post[1] if len(gs_post) > 1 else None,
                  "raw line 5 read from the file": raw5}))
check("i-f73 post: the bypassing sibling's FILE half likewise, on line 7 - the SECOND emitter, "
      "measured and not inferred from the funnel's behaviour",
      len(unrec_post) == 2 and unrec_post[1] == rendered(pf + ":7", raw7, UNREC),
      json.dumps({"expected": rendered(pf + ":7", raw7, UNREC),
                  "observed": unrec_post[1] if len(unrec_post) > 1 else None}))
check("i-f73 post: FOUR refusals with an origin in the battery proper, FOUR rendered lines - "
      "two emitters, two channels, one shape (the read-only script's three are leg r's)",
      len([l for l in err_post if post["ro_file"] not in l]) == 4,
      json.dumps([l for l in err_post if post["ro_file"] not in l]))
check("i-f73 the collapse is exactly the pre PAIR joined: the rendered line's raw-line part is "
      "what the pre binary's first line said after `Could not execute: `, and its tail is what "
      "the pre binary's second line said - nothing was invented and nothing was dropped",
      len(gs_post) == 2 and len(cne_pre) == 2 and len(msg_pre) == 2
      and gs_post[1] == rendered(
          pf + ":5",
          cne_pre[1][len(PRE_FILE_TAG5):].split("Could not execute: ", 1)[1],
          msg_pre[1][len(PRE_FILE_TAG5):]),
      json.dumps({"pre pair": [cne_pre[1], msg_pre[1]],
                  "post one line": gs_post[1] if len(gs_post) > 1 else None}))
tp, tpre = tails(post["played"]), tails(pre["played"])
check("i    post: exactly one `#!` tail, on line 3", sorted(tp) == [3], json.dumps(sorted(tp)))
check("i    post: the tail is the engine's diagnosis", MSG_FAULT in tp.get(3, ""), tp.get(3, ""))
check("i    PRE: the same one tail on the same line (the file half did not move)",
      sorted(tpre) == [3] and MSG_FAULT in tpre.get(3, ""), json.dumps(sorted(tpre)))
stripped = b"\n".join(l.split(b" #! ")[0] for l in post["played"].split(b"\n"))
check("i    post: every other byte of the played file is identical",
      stripped == post["original"])

# ------------------------------------------------- r: the rendering, recomposed independently
rof = post["ro_file"]
ro_err = error_lines(post["ro_log"])
notes["ro_error_lines_post"] = ro_err
notes["ro_error_lines_pre"] = error_lines(pre["ro_log"])
ro_expected = [rendered(rof + ":" + str(n), script_line(post["ro_original"], n), UNKNOWN_ARG)
               for n in (2, 3, 4)]
check("r    post: the three refusals of the read-only script are EXACTLY the three lines this "
      "instrument composed from the file's bytes - an author's comment kept, a stale `#!` tail "
      "replaced, a quoted `#!` left as text",
      ro_err == ro_expected,
      json.dumps({"expected": ro_expected, "observed": ro_err}))
check("r    post: the author's own comment survives INSIDE the rendered line, with the machine "
      "tail after it (the owner's clause: the line stays recognisable)",
      len(ro_err) > 0 and "# an author's own note #! " in ro_err[0], json.dumps(ro_err[:1]))
check("r    post: the stale tail was REPLACED, not doubled - one ` #! ` in the line and the old "
      "text gone (the writer's idempotency rule, in the log, from the same function)",
      len(ro_err) > 1 and ro_err[1].count(" #! ") == 1
      and "a stale tail from an older run" not in ro_err[1], json.dumps(ro_err[1:2]))
check("r    post: a `#!` INSIDE QUOTES is text - the tail went to the end of the line, and the "
      "quoted one is untouched (the parser's own toggle, not a substring search)",
      len(ro_err) > 2 and ro_err[2].endswith(' "a #! inside quotes" #! ' + UNKNOWN_ARG),
      json.dumps(ro_err[2:3]))
check("r-ro post: the file the log just quoted is byte-IDENTICAL - the rendering exists "
      "independently of the write, which is the owner's read-only clause measured",
      post["ro_after"] == post["ro_original"],
      "%d vs %d bytes" % (len(post["ro_after"]), len(post["ro_original"])))
check("r-ro post: and the write was ATTEMPTED and refused - the annotator says so, so the "
      "identity above is a write that failed and not a write that never came",
      any("cannot write" in l and rof in l for l in post["ro_log"].splitlines()),
      json.dumps([l.split("): ", 1)[-1] for l in post["ro_log"].splitlines()
                  if "script annotation" in l]))
check("r-ro PRE: the same three refusals, in the two-line form of its era - the rendering is "
      "what changed, not the reachability of these lines",
      len(error_lines(pre["ro_log"])) == 0
      and len(lines_with(pre["ro_log"], UNKNOWN_ARG)) == 3,
      json.dumps(lines_with(pre["ro_log"], UNKNOWN_ARG)))
check("r-ro PRE: its read-only file is byte-identical too (the control for the leg above)",
      pre["ro_after"] == pre["ro_original"])

# ------------------------------------------------- x: the owner's re-execution discriminator
twf = post["twice_file"]


def loop_fault_lines(log, path):
    return [l.split("(Error): ", 1)[-1] for l in log.splitlines()
            if LOOP_FAULT in l and path in l]


x_post_1 = loop_fault_lines(post["twice_log_1"], twf)
x_post_2 = loop_fault_lines(post["twice_log_2"], twf)
x_pre_1 = loop_fault_lines(pre["twice_log_1"], pre["twice_file"])
x_pre_2 = loop_fault_lines(pre["twice_log_2"], pre["twice_file"])
notes["twice_lines_post"] = [x_post_1, x_post_2]
notes["twice_lines_pre"] = [x_pre_1, x_pre_2]
check("x    post: the SAME fault, the same line, played twice - the log line is BYTE-IDENTICAL "
      "on the second execution, though the first execution wrote a `#!` tail onto that line "
      "[vixy 2026-09-01: the same error may not be logged differently the second time]",
      len(x_post_1) == 1 and x_post_1 == x_post_2, json.dumps([x_post_1, x_post_2]))
check("x    post: and the tail DID land between the two executions - the file changed, so the "
      "stability above was tested against a real second reading",
      post["twice_after_1"] != post["twice_original"]
      and b"#!" in post["twice_after_1"],
      repr(post["twice_after_1"])[:200])
check("x    PRE: on the pre binary the second execution logs a DIFFERENT, longer line - it "
      "quotes the tail it had just written. This opposition is what makes the leg above "
      "discriminate rather than pass by luck",
      len(x_pre_1) == 1 and len(x_pre_2) == 1 and x_pre_1 != x_pre_2
      and len(x_pre_2[0]) > len(x_pre_1[0]), json.dumps([x_pre_1, x_pre_2]))
check("x    both binaries: the file after the SECOND execution is identical to after the first "
      "- the writer's own idempotency, unmoved by the rendering change (f63's property)",
      post["twice_after_2"] == post["twice_after_1"]
      and pre["twice_after_2"] == pre["twice_after_1"],
      "post %s / pre %s" % (post["twice_after_2"] == post["twice_after_1"],
                            pre["twice_after_2"] == pre["twice_after_1"]))
check("x    both binaries: exactly one tail landed, on the faulty line, and every other byte of "
      "the twice-played file is what it was written as",
      b"\n".join(l.split(b" #! ")[0] for l in post["twice_after_2"].split(b"\n"))
      == post["twice_original"]
      and b"\n".join(l.split(b" #! ")[0] for l in pre["twice_after_2"].split(b"\n"))
      == pre["twice_original"],
      repr(post["twice_after_2"])[:200])

# ------------------------------------------------- v: the annotator writes files only for files
DECOY = b"# f68: never played, never annotated\nflag stars on\n"
farmdir = Path(post["play_file"]).parent
expect_before = {str(farmdir / "f68_file.sts"): hashlib.md5(post["original"]).hexdigest(),
                 str(farmdir / "f73_twice.sts"): hashlib.md5(post["twice_original"]).hexdigest(),
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
