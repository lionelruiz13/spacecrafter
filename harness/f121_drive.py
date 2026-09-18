#!/usr/bin/env python3
"""F121 drivers -- the legs of the dual-camera seam measurement.

One leg per launch (F121_LEG in the environment, default `rest`). Every leg
ends with ONE dual dump, which carries the whole per-frame seam ring recorded
since app start (Core::recordSeamStep, INTENT S11.245) -- so the dump's own
195-714 frame latency (S11.241(d)) bounds only when the history is read.

Marks: a leg writes `marks.txt` as `<frame> <label>` lines. The frame index is
read from a CHEAP marker dump's own `seam.frames` field, which is exact: it is
the recorder's call counter at the frame the dump ran. Marker dumps go to a
scratch path and are deleted; only the final dump is kept.

  rest    settle and dump -- the at-rest floors and the frame-at-rest question
  script  the tester's act: select the Moon, play internal/fly_to_selected.sts
          UNMODIFIED, screenshot on each drawn path at the end
  census  the isolated transitions, one command per window
  mutant  the same as census but short -- used to score a perturbed binary
"""
import socket, sys, os, time, json, gzip

OUT = sys.argv[1]
LEG = os.environ.get("F121_LEG", "rest")
SCRATCH = "/home/claude/sc-f121/marks"
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
    """Read only the header line -- the body lines are not needed for a mark."""
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


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
print("leg=%s out=%s" % (LEG, OUT), flush=True)

# Every leg starts from the same declared state: the clock stopped, so that a
# body's own motion is not mistaken for a seam divergence, and nothing
# selected. `timerate rate 0` is asserted rather than assumed by the trace's
# own jd column (constant => the clock is stopped).
send(s, "timerate rate 0")
send(s, "deselect")
time.sleep(2)

if LEG == "rest":
    mark(s, "at_rest_start")
    time.sleep(4)
    mark(s, "at_rest_settled")
    # A reference switch and back, at rest: the frame-at-rest question of
    # mandate step 5 -- a non-zero delta AT REST is a state error, not an
    # interpolation one.
    send(s, "select planet Moon pointer off", 1.5)
    mark(s, "after_select_moon")
    time.sleep(3)
    send(s, "set home_planet Moon", 2.5)
    mark(s, "after_home_moon")
    time.sleep(3)
    send(s, "set home_planet Earth", 2.5)
    mark(s, "after_home_earth")
    time.sleep(3)
    mark(s, "end")

elif LEG == "script":
    send(s, "select planet Moon pointer off", 1.5)
    time.sleep(2)
    mark(s, "moon_selected")
    shot(s, "before.png")
    # UNMODIFIED, by name, exactly as WIN+^ runs it (ui.cpp, the
    # ScriptEvent(IDIR+"internal/fly_to_selected.sts") binding).
    send(s, "script action play filename internal/fly_to_selected.sts", 1.0)
    mark(s, "script_started")
    time.sleep(22)      # the script is 1 + 3 + 3 s of waits plus its transitions
    mark(s, "script_done")
    time.sleep(6)       # old's 5 s heading tail lands inside this window
    mark(s, "after_heading_tail")
    # THE MOON ON SCREEN OR NOT, one grab per DRAWN path at the same state.
    send(s, "flag experimental_path off", 2.0)
    shot(s, "end_oldpath.png")
    send(s, "flag experimental_path on", 2.0)
    shot(s, "end_newpath.png")
    mark(s, "end")

elif LEG == "mutant":
    # THE INSTRUMENT'S OWN POSITIVE CONTROL. The place channel is PREDICTED to
    # sit at its float32 floor on an unperturbed binary (prediction.txt P2/L4:
    # both observer-move laws are linear over the same wall time). This leg is
    # run TWICE -- on the delivered binary and on one whose Camera::update
    # integrates the move at 1.3x -- and the instrument is only trusted if the
    # same leg reads floor on the first and RED on the second. A channel that
    # cannot be made to fail has not been shown to be reading anything.
    send(s, "select planet Mars pointer off", 1.5)
    time.sleep(2)
    mark(s, "base")
    mark(s, "moveto_up_d3")
    send(s, "moveto altitude 20000000 duration 3", 0.2)
    time.sleep(5)
    mark(s, "moveto_up_done")
    mark(s, "moveto_down_d3")
    send(s, "moveto altitude 100 duration 3", 0.2)
    time.sleep(5)
    mark(s, "moveto_down_done")
    mark(s, "end")

elif LEG == "census":
    send(s, "select planet Mars pointer off", 1.5)
    time.sleep(2)
    mark(s, "base")

    # --- FOV: the auto-zoom law (census row `zoom auto in`) ---------------
    send(s, "zoom auto in duration 4", 0.2)
    mark(s, "zoom_auto_in")
    time.sleep(6)
    mark(s, "zoom_auto_in_done")

    send(s, "zoom auto initial duration 4", 0.2)
    mark(s, "zoom_auto_initial")
    time.sleep(6)
    mark(s, "zoom_auto_initial_done")

    # --- FOV: the explicit ramp (census row `zoom fov`) -------------------
    send(s, "zoom fov 30 duration 3", 0.2)
    mark(s, "zoom_fov_30_d3")
    time.sleep(5)
    mark(s, "zoom_fov_30_done")
    send(s, "zoom fov 180 duration 3", 0.2)
    mark(s, "zoom_fov_180_d3")
    time.sleep(5)

    # --- FOV: the DEFAULTED duration (coreLink.hpp setFov, 0.5 s new / 0
    #     old) -- `zoom delta_fov` is the reachable token ------------------
    mark(s, "zoom_delta_fov")
    send(s, "zoom delta_fov -60", 0.2)
    time.sleep(3)
    mark(s, "zoom_delta_fov_done")
    send(s, "zoom fov 180 duration 0", 1.0)

    if True:
        # --- TRACKING: old snaps, the camera eases ------------------------
        mark(s, "track_on")
        send(s, "flag track_object on", 0.2)
        time.sleep(6)
        mark(s, "track_on_done")
        send(s, "flag track_object off", 1.0)
        time.sleep(2)

        # --- THE PLACE: moveto with a duration ---------------------------
        mark(s, "moveto_alt_d3")
        send(s, "moveto altitude 20000000 duration 3", 0.2)
        time.sleep(5)
        mark(s, "moveto_alt_done")
        mark(s, "moveto_latlon_d3")
        send(s, "moveto lat 45 lon 30 duration 3", 0.2)
        time.sleep(5)
        mark(s, "moveto_latlon_done")

        # --- THE VIEW: look_at with a duration ---------------------------
        mark(s, "look_at_d3")
        send(s, "look_at azimuth 90 altitude 30 duration 3", 0.2)
        time.sleep(5)
        mark(s, "look_at_done")

        # --- OLD-ONLY view move (census: Core::panView) ------------------
        mark(s, "panview_d3")
        send(s, "look_at delta_azimuth 40 delta_altitude 10 duration 3", 0.2)
        time.sleep(5)
        mark(s, "panview_done")

        # --- HEADING with a duration -------------------------------------
        mark(s, "heading_d3")
        send(s, "heading azimuth 40 duration 3", 0.2)
        time.sleep(5)
        mark(s, "heading_done")
        send(s, "heading azimuth 0 duration 0", 1.0)

    mark(s, "end")

else:
    raise SystemExit("unknown leg %r" % LEG)

time.sleep(1)
dumpto(s, "%s/final.json" % OUT, 6.0)
writemarks()
print("done", flush=True)
