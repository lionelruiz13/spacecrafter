#!/usr/bin/env python3
"""F27 / INTENT §5.47 — where a `get`'s reply goes, measured on the wire.

    cd claude/harness && DISPLAY=:2 ./f27_reply.py <outdir> --bin <binary> \
        --expect pre|post [--legs A,B,C,D,E]

WHAT §5.47 RECORDS. `get status position` resolves, calls
`ServerSocket::setOutput`, the string is queued — and nothing arrived on the
connection that asked in a 6 s poll, twice, in a session where `timerate`,
`date`, `flag`, `select`, `body action dual_dump` and `body action screenshot`
all worked on that same connection.

WHAT THE SOURCE SAYS (io.cpp at code d9de42ac, the three hypotheses the row
names). The queue IS drained: `run()` calls `checkDataToSend()` on EVERY pass
of its loop (io.cpp:388, the loop's own wait is a 1 ms `SDLNet_CheckSockets`),
and `checkDataToSend` empties the whole queue each time (io.cpp:630-639). It is
drained to `broadcast()` (io.cpp:641-655), which sends to every client whose
`clientBroadcastTab` entry is true — a table that is set true in exactly one
place, the `$LOGON` command (io.cpp:614), and false at init and at close. So
the reply goes to the FEEDBACK SUBSCRIBERS, not to the issuer, and with no
subscriber `broadcast` returns 0, the entry is popped, and the string is gone.
Not "only when a second client is connected": the client COUNT is irrelevant,
the subscription is the condition.

This script measures that on the wire, both ways, because a source reading is
not an observation:

  A  plain driving socket ..... the §5.47 scenario reproduced: `get status
     position`, 6 s poll, twice. PRE: silence. POST: the reply on the socket
     that asked. Then the six commands the row records as working, on that
     same connection, and the reply CONTENT against `body action dual_dump`'s
     `control` object — in a scene where the two path authorities DIFFER, so
     "the readout agrees with the path that draws" is a real test (B33's bar).
  B  the same socket after `$LOGON` ..... PRE: the reply ARRIVES. That is the
     positive map — the queue is drained, the subscription is the condition,
     and the pre-fix silence is not an absent instrument. POST: it must still
     arrive, exactly ONCE (an addressed copy must not double a subscriber's).
  C  two clients ..... a subscribed listener and a plain issuer. PRE: the
     listener receives the issuer's reply and the issuer receives nothing —
     the row's "drained to a different socket", measured. POST: both receive
     exactly one copy. Then the issuer DISCONNECTS and a new client takes its
     place (same slot) and asks again: the reply must follow the CONNECTION,
     not the slot number. The pair is entered twice.
  D  a `get` issued by a SCRIPT ..... nobody asked on a socket. The reply must
     go where it goes today (the subscribers) and must NOT be attributed to
     whichever client last spoke on the socket. PRE and POST identical.
  E  the heading pin, READ-ONLY (§11.118(f)'s recorded cost: the only live
     heading channel writes both authorities after reading, so two
     `experimental_path` pins needed two launches) ..... three heading samples
     in ONE launch across two toggles of the pin, with no command that writes
     anything. PRE: zero samples.

Every launch is fresh, carries the §11.121(m) concurrent-instance assert (the
/proc/<pid>/comm probe — a `pgrep -f` pattern that can see the binary also
matches this script's own command line), and asserts the frozen config/ssystem
md5 in == out.
"""

import argparse, glob, hashlib, json, os, re, select, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24   # the NaN-tolerant dump reader (I2)

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
PORT = 7805
JD = "2461233.5"
POLL = 6.0          # §11.118(i)'s own poll length, kept to the digit
SRC_HOME = Path.home() / ".spacecrafter"
FROZEN = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
          "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}

FAILS = []
NOTES = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def note(msg):
    NOTES.append(msg)
    print(f"      {msg}", flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def concurrent_instances():
    """§11.121(m), F26's replacement probe: the executable's own name from
    /proc/<pid>/comm — world-readable (so it covers every account) and carrying
    no command-line text (so it cannot match this script)."""
    out = []
    for c in glob.glob("/proc/[0-9]*/comm"):
        try:
            if open(c).read().strip() == "spacecrafter":
                out.append(c)
        except OSError:
            pass
    return out


# --------------------------------------------------------------- the wire
# `ServerSocket::send` writes strlen(buffer)+1 bytes, i.e. the payload AND its
# terminating NUL, and `checkDataToSend` appends '\n' to each queued string. So
# a message on the wire is NUL-terminated and the reply payloads end in '\n'.
POS_RE = re.compile(r"^\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);"
                    r"\s*(-?\d+\.\d+);\s*(-?\d+\.\d+);\s*$")


def messages(raw: bytes):
    return [m.decode("latin-1") for m in raw.split(b"\x00") if m.strip()]


def positions(raw: bytes):
    """Every `get status position` reply in what was received, parsed."""
    out = []
    for m in messages(raw):
        g = POS_RE.match(m.strip("\n"))
        if g:
            out.append(dict(zip(("lat", "lon", "alt", "jday", "heading"),
                                (float(x) for x in g.groups()))))
    return out


class Client:
    """A socket that KEEPS what it receives. Every driving helper in the
    harness recv()s into the void after each command (`b25_galactic.run_phase`,
    `f23_b33_control.App.send`), which is exactly why a missing reply has never
    been visible from a harness script."""

    def __init__(self, name):
        self.name = name
        self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=5)
        self.sock.setblocking(False)
        self.all = b""

    def read(self, seconds):
        """Read for `seconds`; return only what arrived during this call."""
        got, t0 = b"", time.time()
        while time.time() - t0 < seconds:
            r, _, _ = select.select([self.sock], [], [], 0.1)
            if r:
                try:
                    d = self.sock.recv(65536)
                except (BlockingIOError, ConnectionResetError):
                    break
                if not d:
                    break
                got += d
        self.all += got
        return got

    def poll_for_reply(self, seconds=POLL):
        """Read up to `seconds`, stopping at the first parsed position reply.
        Returns (reply|None, latency_s, raw)."""
        got, t0 = b"", time.time()
        while time.time() - t0 < seconds:
            r, _, _ = select.select([self.sock], [], [], 0.1)
            if r:
                try:
                    d = self.sock.recv(65536)
                except (BlockingIOError, ConnectionResetError):
                    break
                if not d:
                    break
                got += d
                if positions(got):
                    break
        self.all += got
        p = positions(got)
        return (p[0] if p else None), time.time() - t0, got

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        # the pause is spent READING, so nothing the app says is dropped
        return self.read(pause)

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


class Session:
    def __init__(self, outdir, tag, binary, launch_prefix=(), port_wait=90,
                 prepare=None, env_extra=None):
        """`launch_prefix` is prepended to the argv, so a caller can run the same
        launch UNDER another program without re-deriving the farm, the
        concurrent-instance assert or the frozen-md5 pair (I2). F31 passes
        ("gdb", "-q", "-batch", "-x", <script>, "--args") — ptrace_scope=1 blocks
        attaching, so a probe has to be there from the first instruction.

        `prepare(dst)` runs on the freshly built farm BEFORE the launch, so a
        caller can author farm-local data the app must find at startup (F32
        writes `modularSystem/SolarSystem.ini`: a composed body exists only if
        it is on disc when the system loads). `env_extra` is merged into the
        child environment (F32 passes ASAN_OPTIONS). Both default to inert, so
        every existing caller's launch is byte-identical."""
        self.tag, self.outdir, self.binary = tag, outdir, Path(binary)
        insts = concurrent_instances()
        if insts:
            raise RuntimeError(f"{tag}: concurrent spacecrafter instance(s): {insts}")
        self.md5_in = {n: md5(SRC_HOME / n) for n in FROZEN}
        for n, m in self.md5_in.items():
            if m != FROZEN[n]:
                fail(f"{tag}: {n} md5 in {m} != frozen {FROZEN[n]}")
        self.farm = outdir / f"farm_{tag}"
        self.dst = b25g.build_farm(farm=self.farm, dotted=False, corpus=None)
        if prepare is not None:
            prepare(self.dst)
        self.applog = outdir / f"{tag}.applog"
        self.proc = subprocess.Popen(
            [*launch_prefix, str(self.binary)], cwd=str(self.dst),
            stdout=open(self.applog, "w"), stderr=subprocess.STDOUT,
            env={**os.environ, "HOME": str(self.farm),
                 "DISPLAY": os.environ.get("DISPLAY", ":2"),
                 **(env_extra or {})})
        t0 = time.time()
        while time.time() - t0 < port_wait:
            if self.proc.poll() is not None:
                raise RuntimeError(f"{tag}: app died before opening its port")
            try:
                socket.create_connection(("127.0.0.1", PORT), timeout=1).close()
                break
            except OSError:
                time.sleep(1)
        else:
            self.proc.kill()
            raise RuntimeError(f"{tag}: port {PORT} never opened")
        time.sleep(10)
        self.clients = []

    def client(self, name):
        c = Client(f"{self.tag}:{name}")
        self.clients.append(c)
        return c

    def logtexts(self):
        """PER FILE, because the app writes into six of them at once. The first
        version of this reader concatenated them and marked a position by total
        length, so "what was logged since the mark" was the tail of the LAST
        file and the warning this leg looks for — written to spacecrafter.log,
        the fourth — was invisible. It reported a diagnostic missing that was
        in fact present, i.e. an instrument that turns a working channel into a
        FAIL."""
        out = {}
        for p in sorted((self.farm / ".spacecrafter" / "log").glob("*")):
            try:
                out[p.name] = p.read_text(errors="replace")
            except OSError:
                pass
        return out

    def scriptlog(self):
        return "".join(self.logtexts().values())

    def logmark(self):
        return {k: len(v) for k, v in self.logtexts().items()}

    def lognew(self, mark):
        return "".join(v[mark.get(k, 0):] for k, v in self.logtexts().items())

    def refused(self):
        """The app's OWN report that it did not execute a command (§2(f)).
        F12's lesson: a harness that ignores this measures nothing for a run."""
        return [l for l in self.scriptlog().splitlines() if "Could not execute" in l]

    def stop(self, driver, exit_wait=40):
        """`exit_wait` is a parameter because an ASan/LSan build spends real time
        in the leak check after main returns (F32); 40 s stays the default, so
        every existing caller is unchanged."""
        driver.send("shutdown action now", 1)
        for c in self.clients:
            c.close()
        try:
            rc = self.proc.wait(timeout=exit_wait)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            rc = -1
            fail(f"{self.tag}: app did not exit within {exit_wait} s")
        for n in FROZEN:
            m = md5(SRC_HOME / n)
            if m != self.md5_in[n]:
                fail(f"{self.tag}: {n} md5 in != out ({self.md5_in[n]} -> {m})")
        return rc


# --------------------------------------------------------------- the scene
def scene_split(c):
    """f12_b33_heading's scene: home Earth, look at the Moon, then move home to
    the Moon. After the reference switch the two heading authorities differ
    permanently BY DESIGN (D28), so a readout that agrees with the drawn path
    is saying something."""
    c.send("timerate rate 0", 1)
    c.send("flag experimental_path on", 1)
    c.send("set home_planet Earth", 3)
    c.send(f"date jday {JD}", 1)
    c.send("select planet Moon pointer off", 1)
    c.send("flag track_object on", 3)
    c.send("zoom fov 10 duration 0", 2)
    # the switch
    c.send("deselect", 0.6)
    c.send("flag track_object off", 1)
    c.send("set home_planet Moon", 4)
    c.send("select planet Earth pointer off", 1)
    c.send("flag track_object on", 3)
    c.send("zoom fov 10 duration 0", 2)
    c.send("timerate rate 0", 1)


def dump(sess, c, tag):
    p = sess.outdir / f"{sess.tag}_{tag}.json"
    p.unlink(missing_ok=True)
    c.send(f"body action dual_dump filename {p}", 2.5)
    if not p.exists():
        fail(f"{sess.tag}/{tag}: dual_dump wrote nothing")
        return None
    line = b24._NONFINITE.sub(
        lambda m: m.group(1) + ("NaN" if m.group(2) == "nan" else "Infinity"),
        open(p).readline())
    return json.loads(line)


def compare_content(sess, reply, hdr, tag):
    """(v) The reply CONTENT against the dump's `control` object, at the
    reply's own printf precision — %2.2f, %3.2f, %10.2f, %10.6f, %10.6f."""
    if reply is None or hdr is None:
        return
    ctl = hdr["control"]
    rows = [("lat", reply["lat"], ctl["latitude"], 0.01),
            ("lon", reply["lon"], ctl["longitude"], 0.01),
            ("alt", reply["alt"], ctl["altitude"], 0.01),
            ("heading", reply["heading"], ctl["heading"], 1e-6)]
    for name, got, obj, tol in rows:
        rep, old, new = obj["reported"], obj["old"], obj["new"]
        if abs(got - round(rep, 6)) > tol + 5e-7:
            fail(f"{tag}: reply {name}={got} != control.reported={rep}")
        else:
            ok(f"{tag}: reply {name} {got} == control.reported {rep}  "
               f"(old {old} / new {new})")
        if abs(old - new) > tol:
            note(f"{tag}: {name} authorities DIVERGE by {abs(old-new):.6g} — "
                 f"the reply follows {'new' if abs(got-round(new,6))<=tol+5e-7 else 'old'}")
    if abs(reply["jday"] - hdr["jd"]) > 5e-7:
        fail(f"{tag}: reply jday={reply['jday']} != dump jd={hdr['jd']}")
    else:
        ok(f"{tag}: reply jday {reply['jday']} == dump jd {hdr['jd']}")


# --------------------------------------------------------------------- legs
def leg_A(outdir, binary, expect):
    """The §5.47 scenario, then the six working commands on that connection,
    then the reply content."""
    sess = Session(outdir, "A", binary)
    c = sess.client("driver")
    r = {"leg": "A"}
    try:
        scene_split(c)

        # --- the row's own scenario: 6 s poll, twice ----------------------
        for i in (1, 2):
            c.send("get status position", 0)
            rep, lat, raw = c.poll_for_reply(POLL)
            r[f"poll{i}"] = {"reply": rep, "latency_s": round(lat, 3),
                             "raw": raw.decode("latin-1")}
            if rep is None:
                print(f"      A/poll{i}: {POLL:.0f} s of silence "
                      f"({len(raw)} bytes received)", flush=True)
            else:
                print(f"      A/poll{i}: reply after {lat:.3f} s: {rep}", flush=True)

        # --- the six commands §5.47 records as working, same connection ---
        # the flag leg drives BOTH directions from a known state, so it cannot
        # pass because the scene happened to leave the flag where it wanted it
        c.send("flag lock_sky_position off", 1.2)
        h0 = dump(sess, c, "ctl0")
        shot0 = outdir / f"A_shot0.png"
        c.send(f"body action screenshot filename {shot0}", 2.2)
        c.send("flag lock_sky_position on", 1.2)
        h1 = dump(sess, c, "ctl1")
        c.send("flag lock_sky_position off", 1.2)
        c.send("deselect", 0.6)
        c.send("flag track_object off", 1)
        c.send("select planet Sun pointer off", 1)
        c.send("flag track_object on", 3)
        shot1 = outdir / f"A_shot1.png"
        c.send(f"body action screenshot filename {shot1}", 2.2)
        h2 = dump(sess, c, "ctl2")

        six = {}
        six["timerate"] = (h0 or {}).get("timeSpeed")
        six["date"] = (h0 or {}).get("jd")
        six["flag"] = [(h0 or {}).get("control", {}).get("skyLock", {}).get("reported"),
                       (h1 or {}).get("control", {}).get("skyLock", {}).get("reported")]
        six["dual_dump"] = [bool(h0), bool(h1), bool(h2)]
        six["screenshot"] = [shot0.exists(), shot1.exists()]
        if shot0.exists() and shot1.exists():
            a = np.asarray(Image.open(shot0).convert("RGB")).astype(np.int32)
            b = np.asarray(Image.open(shot1).convert("RGB")).astype(np.int32)
            six["select_px8"] = int((np.abs(a - b).max(axis=2) > 8).sum())
            six["lit0"] = int((np.asarray(Image.open(shot0).convert("RGB")).max(axis=2) > 8).sum())
        r["six"] = six
        r["refused"] = sess.refused()

        # --- the content check, in the diverged scene ---------------------
        c.send("deselect", 0.6)
        c.send("flag track_object off", 1)
        h3 = dump(sess, c, "content")
        c.send("get status position", 0)
        rep, lat, raw = c.poll_for_reply(POLL)
        r["content_reply"] = rep
        r["content_latency_s"] = round(lat, 3)
        r["content_hdr"] = {"jd": (h3 or {}).get("jd"),
                            "control": (h3 or {}).get("control")}
        if rep is not None:
            compare_content(sess, rep, h3, "A/content")
    finally:
        r["rc"] = sess.stop(c)
    return r


def leg_B(outdir, binary, expect):
    """The same socket, after `$LOGON`. The positive map of the drain."""
    sess = Session(outdir, "B", binary)
    c = sess.client("driver")
    r = {"leg": "B"}
    try:
        c.send("timerate rate 0", 1)
        c.send(f"date jday {JD}", 1)
        c.send("moveto lat 43.5 lon 5.4 alt 1234 duration 0", 3)
        ackraw = c.send("$LOGON", 1.5)
        r["logon_ack"] = ackraw.decode("latin-1")
        ok(f"B: $LOGON ack = {r['logon_ack']!r}")
        c.send("get status position", 0)
        rep, lat, raw = c.poll_for_reply(POLL)
        r["reply"] = rep
        r["latency_s"] = round(lat, 3)
        r["raw"] = raw.decode("latin-1")
        r["copies"] = len(positions(raw))
        print(f"      B: {r['copies']} copy/copies after {lat:.3f} s: {rep}", flush=True)
        # a second, quiet second: a duplicate would land late, not with the first
        extra = c.read(2.0)
        r["copies_late"] = len(positions(extra))
        h = dump(sess, c, "content")
        if rep is not None:
            compare_content(sess, rep, h, "B/content")
    finally:
        r["rc"] = sess.stop(c)
    return r


def leg_C(outdir, binary, expect):
    """Two clients: a subscribed listener and a plain issuer; then the issuer's
    connection is replaced and the pair entered a second time."""
    sess = Session(outdir, "C", binary)
    listener = sess.client("listener")
    issuer = sess.client("issuer")
    r = {"leg": "C"}
    try:
        listener.send("timerate rate 0", 1)
        listener.send(f"date jday {JD}", 1)
        listener.send("moveto lat 12.25 lon 34.5 alt 4321 duration 0", 3)
        listener.send("$LOGON", 1.5)
        listener.read(0.5)

        issuer.send("get status position", 0)
        rep_i, lat_i, raw_i = issuer.poll_for_reply(POLL)
        raw_l = listener.read(0.5)
        r["pass1"] = {"issuer": rep_i, "issuer_latency_s": round(lat_i, 3),
                      "issuer_copies": len(positions(raw_i)),
                      "listener": (positions(raw_l) or [None])[0],
                      "listener_copies": len(positions(raw_l))}
        print(f"      C/1: issuer {r['pass1']['issuer_copies']} copy, "
              f"listener {r['pass1']['listener_copies']} copy", flush=True)

        # the connection is replaced: same slot, different connection
        issuer.close()
        time.sleep(2)
        issuer2 = sess.client("issuer2")
        issuer2.send("get status position", 0)
        rep_j, lat_j, raw_j = issuer2.poll_for_reply(POLL)
        raw_l2 = listener.read(0.5)
        r["pass2"] = {"issuer": rep_j, "issuer_latency_s": round(lat_j, 3),
                      "issuer_copies": len(positions(raw_j)),
                      "listener": (positions(raw_l2) or [None])[0],
                      "listener_copies": len(positions(raw_l2))}
        print(f"      C/2 (reconnected): issuer {r['pass2']['issuer_copies']} copy, "
              f"listener {r['pass2']['listener_copies']} copy", flush=True)
        r["refused"] = sess.refused()
    finally:
        r["rc"] = sess.stop(listener)
    return r


def leg_D(outdir, binary, expect):
    """A `get` inside a SCRIPT: nobody asked on a socket. The reply must not be
    attributed to whichever client last spoke."""
    sess = Session(outdir, "D", binary)
    listener = sess.client("listener")
    driver = sess.client("driver")
    r = {"leg": "D"}
    sts = outdir / "f27_script_get.sts"
    sts.write_text("timerate rate 0\nget status position\n")
    try:
        listener.send("timerate rate 0", 1)
        listener.send(f"date jday {JD}", 1)
        listener.send("$LOGON", 1.5)
        listener.read(0.5)
        driver.read(0.5)
        driver.send(f"script action play filename {sts}", 1.0)
        raw_d = driver.read(POLL)
        raw_l = listener.read(0.5)
        r["driver_copies"] = len(positions(raw_d))
        r["listener_copies"] = len(positions(raw_l))
        r["listener_reply"] = (positions(raw_l) or [None])[0]
        r["refused"] = sess.refused()
        print(f"      D: script get -> driver {r['driver_copies']}, "
              f"listener {r['listener_copies']}", flush=True)
    finally:
        r["rc"] = sess.stop(driver)
    return r


def leg_E(outdir, binary, expect):
    """(vi) The heading pin through the READ-ONLY channel: three samples in one
    launch across two toggles of `flag experimental_path`, no writing command
    anywhere. §11.118(f) needed two launches for two pins because its only
    channel (`heading delta_azimuth 0`) writes both authorities after reading."""
    sess = Session(outdir, "E", binary)
    c = sess.client("driver")
    r = {"leg": "E", "samples": []}
    try:
        scene_split(c)
        c.send("deselect", 0.6)
        c.send("flag track_object off", 1)
        for i, pin in enumerate(("on", "off", "on")):
            c.send(f"flag experimental_path {pin}", 2.0)
            h = dump(sess, c, f"pin{i}_{pin}")
            c.send("get status position", 0)
            rep, lat, raw = c.poll_for_reply(POLL)
            ctl = (h or {}).get("control", {})
            r["samples"].append({
                "pin": pin, "reply": rep, "latency_s": round(lat, 3),
                "drawnPath": ctl.get("drawnPath"),
                "heading_reported": ctl.get("heading", {}).get("reported"),
                "heading_old": ctl.get("heading", {}).get("old"),
                "heading_new": ctl.get("heading", {}).get("new")})
            print(f"      E/{i} pin={pin}: reply={rep['heading'] if rep else None} "
                  f"dump reported={ctl.get('heading', {}).get('reported')} "
                  f"(old {ctl.get('heading', {}).get('old')} / "
                  f"new {ctl.get('heading', {}).get('new')})", flush=True)
        r["refused"] = sess.refused()
    finally:
        r["rc"] = sess.stop(c)
    return r


def leg_F(outdir, binary, expect):
    """The two ends of the new routing that no other leg reaches.

    F1  an answer NOBODY can receive (a script's `get`, no subscriber): the
        string used to be popped off the queue in silence, and the app said
        nothing about it — which is half of why §5.47 stayed open. It must now
        say so in its own log (§2(f)).
    F2  the answer's issuer is GONE by the time the answer exists. That is not
        a contrived case: the HTTP channel (io.cpp:522-604) pushes the command
        and closes the connection in the same pass, so every HTTP-issued `get`
        takes this path. The answer must fall back to the subscribers — where
        it went before — and must never be handed to whoever holds that slot
        next."""
    sess = Session(outdir, "F", binary)
    driver = sess.client("driver")
    r = {"leg": "F"}
    sts = outdir / "f27_script_get.sts"
    sts.write_text("timerate rate 0\nget status position\n")
    try:
        driver.send("timerate rate 0", 1)
        driver.send(f"date jday {JD}", 1)
        driver.send("moveto lat 11.5 lon 22.25 alt 3210 duration 0", 3)
        mark = sess.logmark()
        driver.send(f"script action play filename {sts}", 1.0)
        raw_d = driver.read(POLL)
        log_new = sess.lognew(mark)
        r["F1_driver_copies"] = len(positions(raw_d))
        r["F1_nobody_lines"] = [l for l in log_new.splitlines()
                                if "nobody to answer" in l]
        print(f"      F1: driver {r['F1_driver_copies']} copies, "
              f"{len(r['F1_nobody_lines'])} 'nobody to answer' line(s)", flush=True)

        # F2 — the HTTP channel: issue and hang up
        listener = sess.client("listener")
        listener.send("$LOGON", 1.5)
        listener.read(0.5)
        http = socket.create_connection(("127.0.0.1", PORT), timeout=5)
        http.sendall(b"GET /?command=get%20status%20position HTTP/1.0\r\n\r\n")
        time.sleep(1.0)
        try:
            r["F2_http_response"] = http.recv(4096).decode("latin-1")[:80]
        except OSError as e:
            r["F2_http_response"] = f"<{e}>"
        http.close()
        raw_l = listener.read(POLL)
        r["F2_listener_copies"] = len(positions(raw_l))
        r["F2_listener_reply"] = (positions(raw_l) or [None])[0]
        # a client taking the freed slot must not inherit the answer
        late = sess.client("late")
        raw_late = late.read(2.0)
        r["F2_late_copies"] = len(positions(raw_late))
        r["alive"] = sess.proc.poll() is None
        print(f"      F2: listener {r['F2_listener_copies']} copies, "
              f"next tenant of the slot {r['F2_late_copies']}, "
              f"app alive={r['alive']}", flush=True)
    finally:
        r["rc"] = sess.stop(driver)
    return r


# ------------------------------------------------------------------ asserts
def check(res, expect):
    A, B, C, D, E, F = (res.get(k) for k in "ABCDEF")

    if A:
        for i in (1, 2):
            got = A[f"poll{i}"]["reply"]
            if expect == "pre":
                if got is None:
                    ok(f"A/poll{i}: {POLL:.0f} s silence on the driving socket "
                       f"(§11.118(i) reproduced)")
                else:
                    fail(f"A/poll{i}: pre-fix binary REPLIED ({got}) — "
                         f"the row's scenario did not reproduce")
            else:
                if got is None:
                    fail(f"A/poll{i}: no reply on the driving socket within {POLL:.0f} s")
                else:
                    ok(f"A/poll{i}: reply on the driving socket after "
                       f"{A[f'poll{i}']['latency_s']} s")
        six = A["six"]
        if six["timerate"] != 0:
            fail(f"A/six: `timerate rate 0` -> timeSpeed {six['timerate']}")
        else:
            ok("A/six: timerate")
        if abs((six["date"] or 0) - float(JD)) > 1e-6:
            fail(f"A/six: `date jday` -> jd {six['date']}")
        else:
            ok("A/six: date")
        if six["flag"] != [False, True]:
            fail(f"A/six: `flag lock_sky_position` readback {six['flag']}")
        else:
            ok("A/six: flag (false -> true)")
        if not all(six["dual_dump"]):
            fail(f"A/six: dual_dump {six['dual_dump']}")
        else:
            ok("A/six: body action dual_dump")
        if not all(six["screenshot"]):
            fail(f"A/six: screenshot {six['screenshot']}")
        elif six.get("lit0", 0) < 1000:
            fail(f"A/six: screenshot has {six.get('lit0')} lit px — an empty "
                 f"frame makes the select check vacuous")
        elif six.get("select_px8", 0) < 1000:
            fail(f"A/six: `select` moved {six.get('select_px8')} px>8")
        else:
            ok(f"A/six: select ({six['select_px8']} px>8) + screenshot "
               f"({six['lit0']} lit px)")
        if A["refused"]:
            fail(f"A: the app refused command(s): {A['refused']}")
        else:
            ok("A: the app refused nothing (its own §2(f) channel)")
        if expect == "post" and A["content_reply"] is None:
            fail("A/content: no reply to compare against the dump")

    if B:
        if B["reply"] is None:
            fail("B: no reply on a $LOGON-subscribed socket — the drain is NOT "
                 "the subscription channel and the source map is wrong")
        else:
            ok(f"B: the subscribed socket receives the reply after "
               f"{B['latency_s']} s ({B['copies']} copy)")
        if B["copies"] > 1 or B["copies_late"] > 0:
            fail(f"B: {B['copies']}+{B['copies_late']} copies — the reply is "
                 f"doubled on a socket that is both issuer and subscriber")
        else:
            ok("B: exactly one copy")

    if C:
        p1, p2 = C["pass1"], C["pass2"]
        for name, p in (("C/1", p1), ("C/2", p2)):
            if p["listener_copies"] != 1:
                fail(f"{name}: listener got {p['listener_copies']} copies")
            else:
                ok(f"{name}: the subscribed listener gets the reply")
            if expect == "pre":
                if p["issuer_copies"] == 0:
                    ok(f"{name}: the issuer gets NOTHING (drained to a "
                       f"different socket, measured)")
                else:
                    fail(f"{name}: pre-fix issuer received {p['issuer_copies']}")
            else:
                if p["issuer_copies"] == 1:
                    ok(f"{name}: the issuer gets exactly one copy")
                else:
                    fail(f"{name}: issuer got {p['issuer_copies']} copies")

    if D:
        if D["driver_copies"] != 0:
            fail(f"D: a script's `get` was delivered to the last client that "
                 f"spoke ({D['driver_copies']} copies) — misattribution")
        else:
            ok("D: a script's `get` is not attributed to the last speaker")
        if D["listener_copies"] != 1:
            fail(f"D: the subscriber got {D['listener_copies']} copies of the "
                 f"script's reply (today's routing must be preserved)")
        else:
            ok("D: the subscriber still receives a script-issued reply")

    if E:
        got = [s for s in E["samples"] if s["reply"] is not None]
        if expect == "pre":
            if got:
                fail(f"E: pre-fix binary answered {len(got)}/3 samples")
            else:
                ok("E: 0/3 samples — no read-only heading channel exists pre-fix")
        else:
            if len(got) != 3:
                fail(f"E: {len(got)}/3 samples answered")
            else:
                ok("E: 3/3 heading samples in ONE launch, no writing command")
            for s in E["samples"]:
                if s["reply"] is None:
                    continue
                exp = s["heading_reported"]
                if abs(s["reply"]["heading"] - round(exp, 6)) > 1e-6 + 5e-7:
                    fail(f"E/pin={s['pin']}: reply heading {s['reply']['heading']} "
                         f"!= dump reported {exp}")
            pins = [s["heading_reported"] for s in E["samples"]]
            if len(set(round(p, 6) for p in pins if p is not None)) < 2:
                fail(f"E: the two pins report the same heading {pins} — the "
                     f"demonstration cannot show the pin was entered")
            else:
                ok(f"E: the pin changes the reported heading {pins}")


    if F:
        if F["F1_driver_copies"] != 0:
            fail(f"F1: the script's answer was handed to the driving client "
                 f"({F['F1_driver_copies']} copies)")
        else:
            ok("F1: the script's answer is not handed to the last speaker")
        if expect == "pre":
            if F["F1_nobody_lines"]:
                fail(f"F1: the pre-fix binary reported {F['F1_nobody_lines']}")
            else:
                ok("F1: pre-fix, an answer nobody can receive is discarded in "
                   "silence (the app's log says nothing)")
        else:
            if not F["F1_nobody_lines"]:
                fail("F1: an answer with no recipient was discarded without a "
                     "word in the log — the §2(f) diagnostic does not fire")
            else:
                ok(f"F1: the app reports the discard: "
                   f"{F['F1_nobody_lines'][0].strip()[:110]}")
        if F["F2_listener_copies"] != 1:
            fail(f"F2: the subscriber got {F['F2_listener_copies']} copies of an "
                 f"HTTP-issued answer (the issuer is gone: the fallback must "
                 f"still be the subscription channel)")
        else:
            ok("F2: an answer whose issuer hung up still reaches the subscribers")
        if F["F2_late_copies"] != 0:
            fail(f"F2: the next client on that slot inherited {F['F2_late_copies']} "
                 f"answer(s) it never asked for")
        else:
            ok("F2: the next tenant of the slot inherits nothing")
        if not F["alive"]:
            fail("F2: the app died on the HTTP path")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--expect", choices=("pre", "post"), required=True)
    ap.add_argument("--legs", default="A,B,C,D,E,F")
    a = ap.parse_args()

    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    print(f"=== F27 §5.47 reply-path measurement ===", flush=True)
    print(f"binary   : {a.bin}", flush=True)
    print(f"md5      : {md5(a.bin)}", flush=True)
    print(f"expect   : {a.expect}", flush=True)
    print(f"outdir   : {out}", flush=True)
    print(f"wall     : {time.strftime('%F %T %Z')}", flush=True)

    legs = {"A": leg_A, "B": leg_B, "C": leg_C, "D": leg_D, "E": leg_E, "F": leg_F}
    res = {"binary": str(a.bin), "md5": md5(a.bin), "expect": a.expect,
           "wall": time.strftime("%F %T %Z")}
    for name in a.legs.split(","):
        name = name.strip()
        if not name:
            continue
        print(f"\n--- leg {name} ---", flush=True)
        res[name] = legs[name](out, a.bin, a.expect)

    print("\n--- asserts ---", flush=True)
    check(res, a.expect)
    (out / f"f27_{a.expect}.json").write_text(json.dumps(res, indent=1, default=str))
    print(f"\n{len(FAILS)} FAIL / expect={a.expect}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
