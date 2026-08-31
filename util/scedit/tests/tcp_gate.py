#!/usr/bin/env python3
"""scedit's TCP client, gated from BOTH ends without an engine.

    tests/tcp_gate.py <tcpclient_test binary>

WHAT THIS PROVES. That `sc_tcpclient` speaks the line protocol spacecrafter's
`ServerSocket` speaks — the NUL-terminated records, the `$LOGON` subscription,
an answer that arrives on the asking connection and on the subscribers, a
command that is answered with silence, ISO-8859 bytes that survive the wire —
and that what it SENDS is what the caller asked it to send, asserted on the
server side rather than taken from the client's own account of itself.

Each leg is one process: this gate starts a stand-in engine
(`tests/fake_engine.py` — its framing rules are read from the engine's source,
file by file, line by line), runs the leg, and then checks the transcript the
stand-in recorded. A leg that passes its own checks while having sent nothing
fails here.

The stand-in is not the engine, and the two are held together on purpose: the
same facts are asserted against the REAL binary by
`claude/harness/f67_tcp_live.py`, on a fresh launch, over port 7805.

Stdlib only. Exit 0 all green, 1 a check failed.
"""
import os, subprocess, sys, threading, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fake_engine import FakeEngine, SEARCH

failures = []
checks = 0


def check(ok, what):
    global checks
    checks += 1
    if not ok:
        failures.append(what)
    print(("  ok      " if ok else "  FAIL    ") + what)


def run_leg(binary, leg, endpoint=None, timeout=60):
    argv = [binary, leg] + ([endpoint] if endpoint else [])
    p = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
    sys.stdout.write(p.stdout)
    if p.stderr.strip():
        sys.stdout.write("  [stderr] " + p.stderr.strip() + "\n")
    return p.returncode


def free_port():
    import socket
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    p = s.getsockname()[1]
    s.close()
    return p


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    binary = sys.argv[1]

    print("A. parsing an endpoint (no socket involved)")
    check(run_leg(binary, "parse") == 0, "the `parse` leg is green")

    print("B. nothing is listening")
    port = free_port()   # bound and released: nothing is listening on it now
    check(run_leg(binary, "refused", "127.0.0.1:%d" % port) == 0,
          "connecting where nothing listens fails with a sentence, not a crash")

    print("C. connect, subscribe, ask, be answered")
    with FakeEngine() as eng:
        rc = run_leg(binary, "basic", eng.endpoint())
        check(rc == 0, "the `basic` leg is green")
        lines = eng.lines()
        # SERVER SIDE: what the client actually put on the wire, in order.
        check(lines[:2] == ["$LOGON", "get status position"],
              "the client subscribes FIRST and then asks: %s" % lines[:2])
        check("$NOTICE" in lines, "the $NOTICE probe was sent")
        check("flag stars on" in lines, "the ordinary command was sent verbatim")
        check("$LOGOFF" in lines, "disconnect sends $LOGOFF before closing")
        # Two of each: the leg connects, disconnects, reconnects and
        # disconnects again, and every connection both subscribes and
        # unsubscribes. (This assertion read `1 $LOGOFF` when it was first
        # written — my miscount of my own leg, caught by the gate on its first
        # run. Kept as written down rather than quietly corrected.)
        check(lines.count("$LOGON") == 2 and lines.count("$LOGOFF") == 2,
              "each of the two connections subscribed and unsubscribed: %d $LOGON, %d $LOGOFF"
              % (lines.count("$LOGON"), lines.count("$LOGOFF")))
        # Two connections were used, one after the other, and the second one
        # carried the second question.
        check(eng.lines_of(1)[0] == "$LOGON" and eng.lines_of(2)[0] == "$LOGON",
              "each connection subscribed on its own")
        check("get status constellation" in eng.lines_of(2),
              "the second question went out on the second connection")

    print("D. a command is ONE line")
    with FakeEngine() as eng:
        check(run_leg(binary, "newline", eng.endpoint()) == 0, "the `newline` leg is green")
        check(not any("flag planets on" in l for l in eng.lines()),
              "the refused two-line command reached the engine in NO form: %s" % eng.lines())
        check("get status media" in eng.lines(), "the following command still went out")

    print("E. ISO-8859 bytes, unchanged in both directions")
    with FakeEngine() as eng:
        check(run_leg(binary, "latin1", eng.endpoint()) == 0, "the `latin1` leg is green")
        sent = [l for l in eng.lines() if l.startswith("text ")]
        check(len(sent) == 1, "one text command arrived")
        raw = sent[0].encode("latin-1")
        check(b"\xe9" in raw and raw.count(b"\xe9") == 2,
              "the 0xE9 bytes arrived as bytes, not transcoded: %r" % raw)
        check(b"\xa0" in raw,
              "the 0xA0 no-break space survived too — the byte `invisible-separator` is about")

    print("F. the feed is bounded and says what it dropped")
    with FakeEngine() as eng:
        check(run_leg(binary, "bound", eng.endpoint()) == 0, "the `bound` leg is green")
        check(eng.lines().count("get status position") == 10,
              "all ten questions were asked: %d" % eng.lines().count("get status position"))

    print("G. another client's answer arrives because we subscribed ($LOGON semantics)")
    with FakeEngine() as eng:
        def other_client():
            # Wait for the leg's marker, then ask a question on a SECOND
            # connection: its answer must reach the subscribed one.
            if not eng.wait_for("get status object", timeout=20):
                return
            import socket
            s = socket.create_connection((eng.host, eng.port), timeout=5)
            s.sendall(b"search name m1\n")
            time.sleep(1.0)
            s.close()
        t = threading.Thread(target=other_client, daemon=True)
        t.start()
        rc = run_leg(binary, "feed", eng.endpoint())
        t.join(timeout=25)
        check(rc == 0, "the `feed` leg is green")
        check("search name m1" in eng.lines(), "the second client did ask")
        check(len({c for c, _ in eng.received}) == 2, "two connections were used")

    print("H. the engine goes away")
    with FakeEngine() as eng:
        def stopper():
            if eng.wait_for("get status position", timeout=20):
                time.sleep(0.5)
                eng.stop()
        t = threading.Thread(target=stopper, daemon=True)
        t.start()
        rc = run_leg(binary, "closed", eng.endpoint())
        t.join(timeout=25)
        check(rc == 0, "the `closed` leg is green")

    print("%d checks, %d failures" % (checks, len(failures)))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
