#!/usr/bin/env python3
"""F122 drivers -- the ARM LADDER for the go-to's 156 000 km (INTENT S5.157).

One arm per launch (F122_LEG in the environment). Every arm ends with ONE dual
dump, which carries the whole per-frame seam ring recorded since app start AND
-- since S11.246 -- `seam.travels`, one record per travel INSTALL per registry.

THE LADDER. Each rung adds exactly ONE variable to the one below it, so a
difference between two rungs names its own cause:

  armA   the travel ALONE from a SETTLED point anchor   (the shipped preamble,
         then transition_to point, `wait duration 2`, then move_to)
  armB   = armA with that one `wait` REMOVED            -> the ADJACENCY
  armC   = armB with the third camera line added        -> transition_to body
                                                           issued in flight
  armD   internal/fly_to_selected.sts BY NAME, unmodified -> the tester's act
         (differs from armC only by the `struct if body_selected` that picks
          the altitude)

armA/B/C are played from this harness by ABSOLUTE path (FilePath takes an
absolute name as-is, file_path.cpp:108), so the frozen field install is not
written to; armD is played by the name WIN+^ uses (ui.cpp:2429).

Marks: a leg writes `marks.txt` as `<frame> <label>` lines, the frame index
read from a CHEAP marker dump's own `seam.frames` field. Marker dumps go to a
scratch path and are deleted; only the final dump is kept. Every arm MARKS
BEFORE it sends (S11.245(k)5: the F121 legs that sent first split a
transition's max across two windows).
"""
import socket, sys, os, time, json

OUT = sys.argv[1]
LEG = os.environ.get("F122_LEG", "armA")
SCRATCH = "/home/claude/sc-f122/marks"
SCRIPTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "f122_scripts")
os.makedirs(SCRATCH, exist_ok=True)
MARKS = []


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print("%.3f >> %s" % (time.time(), cmd), flush=True)


def dumpto(sock, path, pause=3.0):
    send(sock, "body action dual_dump filename %s" % path, pause)


def frames_of(path):
    with open(path) as f:
        head = json.loads(f.readline())
    return head["seam"]["frames"]


def mark(sock, label):
    p = "%s/mark.json" % SCRATCH
    try:
        os.remove(p)
    except OSError:
        pass
    dumpto(sock, p, 3.0)
    for _ in range(20):
        if os.path.exists(p):
            break
        time.sleep(0.5)
    try:
        n = frames_of(p)
    except Exception as e:
        print("mark '%s' unreadable: %s" % (label, e), flush=True)
        return
    MARKS.append((n, label))
    print("MARK %d %s" % (n, label), flush=True)
    try:
        os.remove(p)
    except OSError:
        pass


def writemarks():
    with open(os.path.join(OUT, "marks.txt"), "w") as f:
        for n, label in MARKS:
            f.write("%d %s\n" % (n, label))


def shot(sock, name, pause=3.0):
    send(sock, "body action screenshot filename %s/%s" % (OUT, name), pause)


def run_arm(sock, script_arg, shots):
    """The four arms differ ONLY in which script is played (and whether the two
    end frames are grabbed), so they share this body: same selection, same
    settle, same marks, same windows."""
    send(sock, "select planet Moon pointer off", 1.5)
    time.sleep(2)
    mark(sock, "moon_selected")
    mark(sock, "script_start")
    send(sock, "script action play filename %s" % script_arg, 1.0)
    time.sleep(22)          # 1 + 3 s of waits, a 3 s travel, and slack
    mark(sock, "script_done")
    time.sleep(6)           # old's 5 s heading tail lands inside this window
    mark(sock, "after_heading_tail")
    if shots:
        send(sock, "flag experimental_path off", 2.0)
        shot(sock, "end_oldpath.png")
        send(sock, "flag experimental_path on", 2.0)
        shot(sock, "end_newpath.png")
    mark(sock, "end")


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
print("leg=%s out=%s" % (LEG, OUT), flush=True)

# The same declared start state F121's legs used: clock stopped (each arm's
# script sets `timerate rate 1` itself, exactly as the tester's does) and
# nothing selected.
send(s, "timerate rate 0")
send(s, "deselect")
time.sleep(2)

if LEG in ("armA", "armB", "armC"):
    run_arm(s, os.path.join(SCRIPTS, "%s.sts" % LEG), shots=(LEG == "armC"))
elif LEG == "armD":
    run_arm(s, "internal/fly_to_selected.sts", shots=True)
else:
    raise SystemExit("unknown leg %r" % LEG)

time.sleep(1)
dumpto(s, "%s/final.json" % OUT, 6.0)
writemarks()
print("done", flush=True)
