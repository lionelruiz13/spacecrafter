#!/usr/bin/env python3
"""The editor's live keys, pressed on a real terminal.

    tests/pty_gate.py <scedit binary> <grammar file>

WHAT THIS CLOSES. Every other gate reaches live mode from one side or the other:
`ui_selftest` pins what the editor DRAWS, and the `live_*` legs pin what the core
and the client DO. Between them sat a seam nothing measured — that F6 is bound to
connect, F7 to send the caret's line and F8 to play the file — read from
`src/sc_tui.cpp` and believed. This runs the real binary on a pseudo-terminal,
presses the keys, and asserts on the STAND-IN ENGINE'S side what arrived, so a
key bound to nothing cannot pass.

The control codes are pressed rather than the function keys: F6 and friends are
terminal-dependent escape sequences, the control twins exist precisely for
terminals that eat them, and this gate is not the place to encode one terminal's
table. The function keys and the control codes are the same branch in
`sc_tui.cpp` — that much IS read rather than measured, and it is one `||`.

Stdlib only. Exit 0 all green, 1 a check failed.
"""
import fcntl, os, pty, signal, struct, subprocess, sys, termios, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fake_engine import FakeEngine

CTRL_T = b"\x14"   # connect / disconnect
CTRL_L = b"\x0c"   # send the caret's line
CTRL_R = b"\x12"   # play this file
CTRL_W = b"\x17"   # show / hide the feed
CTRL_Q = b"\x11"   # quit

failures = []
checks = 0


def check(ok, what):
    global checks
    checks += 1
    if not ok:
        failures.append(what)
    print(("  ok      " if ok else "  FAIL    ") + what)


class Editor:
    """`scedit --tcp` on a pseudo-terminal: keys in, screen out."""

    def __init__(self, binary, grammar, endpoint, path, cols=100, rows=30):
        self.master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, 0, 0))
        env = dict(os.environ)
        env["TERM"] = "xterm-256color"
        env["LINES"], env["COLUMNS"] = str(rows), str(cols)
        self.p = subprocess.Popen(
            [binary, "--grammar", grammar, "--tcp", endpoint, path],
            stdin=slave, stdout=slave, stderr=slave, env=env, close_fds=True)
        os.close(slave)
        self.screen = b""

    def press(self, keys, settle=0.6):
        os.write(self.master, keys)
        self.drain(settle)

    def drain(self, seconds):
        end = time.time() + seconds
        while time.time() < end:
            r = select_read(self.master, 0.1)
            if r:
                try:
                    self.screen += os.read(self.master, 65536)
                except OSError:
                    break

    def quit(self, timeout=10):
        os.write(self.master, CTRL_Q)
        end = time.time() + timeout
        while time.time() < end and self.p.poll() is None:
            self.drain(0.2)
        if self.p.poll() is None:
            self.p.terminate()
        try:
            self.p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.p.kill()
        os.close(self.master)
        return self.p.returncode


def select_read(fd, timeout):
    import select
    r, _, _ = select.select([fd], [], [], timeout)
    return bool(r)


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    binary, grammar = sys.argv[1], sys.argv[2]
    signal.alarm(120)

    import tempfile
    d = tempfile.mkdtemp(prefix="scedit-pty-")
    path = os.path.join(d, "show.sts")
    with open(path, "wb") as f:
        f.write(b"flag stars on\n# a comment, which is not a command\nbody action clear\n")

    with FakeEngine() as eng:
        ed = Editor(binary, grammar, eng.endpoint(), path)
        ed.drain(2.0)
        check(b"scedit" in ed.screen, "the editor drew its title row")
        check(b"live off" in ed.screen,
              "with --tcp given and nothing connected, the title says `live off`")
        check(eng.connection_count() == 0,
              "and NOTHING has been sent: --tcp does not connect by itself")

        ed.press(CTRL_T, 1.5)                       # F6's twin: connect
        check(eng.wait_for("$LOGON", 5), "Ctrl-T connected and subscribed with $LOGON")
        check(b"127.0.0.1" in ed.screen, "the endpoint is on the screen")

        ed.press(CTRL_L, 1.5)                       # F7's twin: send the caret's line
        check(eng.wait_for("flag stars on", 5),
              "Ctrl-L sent the line under the caret, verbatim: %s" % eng.lines())

        # The caret on a COMMENT line: the engine would run nothing, so nothing
        # is sent — the refusal is scedit's own reading of the engine's rule.
        before = len(eng.lines())
        ed.press(b"\x1b[B", 0.4)                    # arrow down, onto line 2
        ed.press(CTRL_L, 1.2)
        check(len(eng.lines()) == before,
              "on a comment line, Ctrl-L sends NOTHING: %s" % eng.lines()[before:])

        ed.press(CTRL_R, 2.0)                       # F8's twin: play this file
        check(eng.wait_for("script action play filename " + path, 8),
              "Ctrl-R played the open file, by absolute path: %s" % eng.lines())

        ed.press(CTRL_W, 0.8)                       # F9's twin: hide the feed
        ed.press(CTRL_W, 0.8)                       # and show it again
        check(b"Vous receverez" in ed.screen,
              "the engine's own words reached the feed pane on the screen")

        rc = ed.quit()
        check(rc == 0, "the editor exited 0 on Ctrl-Q (got %s)" % rc)
        check("$LOGOFF" in eng.lines(), "and unsubscribed on the way out")

    os.remove(path)
    os.rmdir(d)
    print("%d checks, %d failures" % (checks, len(failures)))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
