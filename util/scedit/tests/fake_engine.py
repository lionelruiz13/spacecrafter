#!/usr/bin/env python3
"""A stand-in for spacecrafter's control socket, framing bytes the way the
engine frames them — so `sc_tcpclient` can be gated without a dome.

WHY A FAKE AND NOT THE ENGINE. Two different questions. Whether scedit's client
speaks the protocol correctly is answerable at a desk, deterministically, in
milliseconds, on any machine: that is this file, and `tests/tcp_gate.py` +
`tests/mcp_gate.py` are its callers. Whether the ENGINE behaves as this file
claims is a live measurement and is made against the real binary by
`claude/harness/f67_tcp_live.py`. A fake that is never checked against the thing
it imitates is a way of testing one's own assumptions twice, so the live
instrument asserts the same framing facts on the wire.

WHAT IS IMITATED, and where each rule was read (code `116f6d19`):
  - every record is written as `payload` + a terminating NUL, because
    `ServerSocket::send` writes `strlen(data) + 1` bytes [io.cpp:743-756];
  - a command ANSWER is `answer + '\n'` before that NUL [io.cpp:~707, deliver];
  - `$NOTICE` is answered `"$NOTICE $LOGON $LOGOFF"` with NO trailing newline,
    `$LOGON` with `"Vous receverez maintenant les logs\n"`, `$LOGOFF` with
    `"Vous receverez maintenant PLUS les logs\n"`, and anything else beginning
    with `$LOG` with `"REQUEST ERROR"` [io.cpp:640-663];
  - ONLY `get status …` and `search name …` produce an answer at all; every
    other command is executed in silence [app_command_interface.cpp:1284-1301,
    1415 are the only callers of setOutput in the tree];
  - an answer goes to the connection that asked AND to every $LOGON subscriber,
    the addressee excluded so a client that is both gets one copy
    [io.cpp:694-726, INTENT §11.135].

Everything it receives is recorded, in order, per connection: a gate asserts
what scedit SENT as well as what scedit did with what came back.

    python3 fake_engine.py [--port N] [--transcript FILE]   # standalone
    from fake_engine import FakeEngine                      # in a gate
"""
import argparse, json, os, socket, threading, time

# The canned answers. Shapes are the engine's own (five ';'-separated floats for
# `get status position` — src/coreModule/coreLink.cpp tcpGetPosition), but the
# VALUES are this file's: a fake must never be mistaken for a measurement.
POSITION = " 45.00; 3.00;  75.00;2461233.500000;  12.500000;"
CONSTELLATION = "UMa"
OBJECT = "EOL"
MEDIA = "NMF"
SEARCH = "M1|M10|M100"


class FakeEngine:
    """One listening socket, one thread per connection, all state guarded by
    one lock. Started and stopped by a `with` block."""

    def __init__(self, host="127.0.0.1", port=0, answer_delay=0.0):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((host, port))
        self.sock.listen(8)
        self.host, self.port = self.sock.getsockname()
        self.answer_delay = answer_delay
        self.lock = threading.Lock()
        self.conns = {}          # id -> {"sock":…, "logon":bool}
        self.received = []       # [(conn_id, line)] in arrival order
        self.stop_flag = False
        self.thread = threading.Thread(target=self._accept_loop, daemon=True)

    # -- lifecycle ---------------------------------------------------------
    def __enter__(self):
        self.thread.start()
        return self

    def __exit__(self, *a):
        self.stop()

    def stop(self):
        self.stop_flag = True
        try:
            # Same reason as the connection sockets below: a thread is blocked in
            # accept() on this fd, and close() alone leaves the open file
            # description — and therefore the LISTEN — alive, so the port goes on
            # accepting after stop() returns. Measured: a gate that stopped the
            # engine and then asked scedit to connect "where nothing listens" was
            # answered by a socket that was still there.
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        try:
            self.sock.close()
        except OSError:
            pass
        with self.lock:
            socks = [c["sock"] for c in self.conns.values()]
        for s in socks:
            # SHUTDOWN before close, and this is not a nicety: on Linux,
            # close()ing a socket another thread is blocked in recv() on does
            # NOT tear the connection down — the blocked call holds the open
            # file description, so no FIN goes out and the peer sees a live
            # socket that has simply gone quiet. shutdown() acts on the
            # connection itself, so the peer gets its end-of-stream at once.
            # Measured: without this, the `closed` leg polls for ten seconds
            # and the client correctly reports nothing, because nothing
            # happened.
            try:
                s.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                s.close()
            except OSError:
                pass

    def endpoint(self):
        return "%s:%d" % (self.host, self.port)

    # -- what a gate asks it -----------------------------------------------
    def lines(self):
        with self.lock:
            return [l for _, l in self.received]

    def lines_of(self, conn_id):
        with self.lock:
            return [l for c, l in self.received if c == conn_id]

    def wait_for(self, text, timeout=5.0):
        """Block until a line equal to `text` has arrived. Returns True/False —
        a gate never sleeps a fixed time waiting for the other side."""
        end = time.time() + timeout
        while time.time() < end:
            if text in self.lines():
                return True
            time.sleep(0.01)
        return False

    def connection_count(self):
        with self.lock:
            return len(self.conns)

    def broadcast_raw(self, payload):
        """Send a record to every $LOGON subscriber, as the engine does when
        another client's command produces an answer."""
        with self.lock:
            targets = [c["sock"] for c in self.conns.values() if c["logon"]]
        for s in targets:
            self._write(s, payload)
        return len(targets)

    # -- the wire ----------------------------------------------------------
    @staticmethod
    def _write(sock, payload):
        """One record: the bytes, then the NUL the engine's send() writes."""
        try:
            sock.sendall(payload.encode("latin-1") + b"\x00")
        except OSError:
            pass

    def _accept_loop(self):
        next_id = 0
        while not self.stop_flag:
            try:
                s, _ = self.sock.accept()
            except OSError:
                return
            next_id += 1
            with self.lock:
                self.conns[next_id] = {"sock": s, "logon": False}
            threading.Thread(target=self._serve, args=(next_id, s), daemon=True).start()

    def _answer(self, conn_id, sock, text):
        """An answer: to the asker, and to the subscribers minus the asker."""
        if self.answer_delay:
            time.sleep(self.answer_delay)
        payload = text + "\n"
        self._write(sock, payload)
        with self.lock:
            others = [c["sock"] for i, c in self.conns.items()
                      if c["logon"] and i != conn_id]
        for o in others:
            self._write(o, payload)

    def _serve(self, conn_id, sock):
        buf = b""
        while not self.stop_flag:
            try:
                data = sock.recv(4096)
            except OSError:
                break
            if not data:
                break
            buf += data
            while b"\n" in buf:
                raw, buf = buf.split(b"\n", 1)
                self._handle(conn_id, sock, raw.decode("latin-1").rstrip("\r"))
        with self.lock:
            self.conns.pop(conn_id, None)
        try:
            sock.close()
        except OSError:
            pass

    def _handle(self, conn_id, sock, line):
        with self.lock:
            self.received.append((conn_id, line))
        # The control commands, answered on the asking socket only and WITHOUT
        # going through the output queue — io.cpp answers them inline.
        if line.startswith("$NOTICE"):
            self._write(sock, "$NOTICE $LOGON $LOGOFF")
            return
        if line.startswith("$LOG"):
            with self.lock:
                on = self.conns.get(conn_id, {}).get("logon", False)
            if line[4:6] == "ON" and not on:
                with self.lock:
                    self.conns[conn_id]["logon"] = True
                self._write(sock, "Vous receverez maintenant les logs\n")
            elif line[4:7] == "OFF" and on:
                with self.lock:
                    self.conns[conn_id]["logon"] = False
                self._write(sock, "Vous receverez maintenant PLUS les logs\n")
            else:
                self._write(sock, "REQUEST ERROR")
            return
        # A command. Only these two produce anything on the wire.
        words = line.split()
        if len(words) >= 3 and words[0] == "get" and words[1] == "status":
            what = words[2]
            answer = {"position": POSITION, "constellation": CONSTELLATION,
                      "object": OBJECT, "media": MEDIA}.get(what)
            if answer is not None:
                self._answer(conn_id, sock, answer)
            return
        if len(words) >= 3 and words[0] == "search" and words[1] == "name":
            self._answer(conn_id, sock, SEARCH)
            return
        # Everything else: executed in silence, exactly like the engine.
        return


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", type=int, default=7805)
    ap.add_argument("--transcript")
    ap.add_argument("--seconds", type=float, default=0.0,
                    help="stop after this long (0 = until interrupted)")
    a = ap.parse_args()
    with FakeEngine(port=a.port) as eng:
        print("fake engine on %s" % eng.endpoint(), flush=True)
        try:
            if a.seconds:
                time.sleep(a.seconds)
            else:
                while True:
                    time.sleep(0.5)
        except KeyboardInterrupt:
            pass
        if a.transcript:
            with open(a.transcript, "w") as f:
                json.dump(eng.received, f, indent=1)


if __name__ == "__main__":
    main()
