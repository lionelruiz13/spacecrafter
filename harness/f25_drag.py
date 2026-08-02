#!/usr/bin/env python3
# F25 / INTENT §11.133 — `Core::dragView`, the OTHER caller of `Camera::lookRel`.
#
# Why this exists: making `lookRel` the exact counterpart of
# `Navigator::updateMove` changes what a mouse drag does on the DRAWN path,
# because `dragView` has passed old's own two numbers to it since the merge that
# added the mirror (`da858612`) and one of them was being read in the opposite
# sign. That is a user-visible change and it has to be measured.
#
# Why gdb rather than a real drag: on this host XTEST pointer MOTION does not
# move the pointer at all — the root window is 0x0 and `XQueryPointer` reports
# (0,0) after every `XTestFakeMotionEvent` (MEASURED, `harness/xdrag.c`'s own
# step report: 8 fake motions, pointer at (0,0) each time, Button1Mask held). A
# fake BUTTON event still lands (that is why `xclick.c` works), but a drag is
# press + MOTION + release, so the drag channel is undrivable here. Same class
# and same remedy as §11.108(d)'s joypad button: stop the live process and call
# the exact function the UI calls, one layer below SDL. The layer above
# (`UI::handleClic` sets `is_dragging`, `UI::handleMove` calls `dragView` for
# every motion after it) is source-verified, not injected — stated, not hidden.
#
# Usage: f25_drag.py <outdir> <gdb_pid> <gdb_fifo> [--pre]
import json, math, os, re, signal, socket, sys, time

OUT, GDBPID, FIFO = sys.argv[1], int(sys.argv[2]), sys.argv[3]
PRE = "--pre" in sys.argv[4:]
os.makedirs(OUT, exist_ok=True)
GDBLOG = f"{OUT}/gdb.log"
FAILS = []
_NONFINITE = re.compile(r'([:\[,]\s*)(-?(?:nan|inf))')


def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok:
        FAILS.append(name)


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.3)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


def dump(sock, tag):
    p = f"{OUT}/{tag}.json"
    if os.path.exists(p):
        os.unlink(p)
    send(sock, f"body action dual_dump filename {p}", 3.0)
    for _ in range(30):
        if os.path.exists(p) and os.path.getsize(p):
            break
        time.sleep(0.4)
    for line in open(p, encoding="utf-8", errors="replace"):
        line = line.strip()
        if not line:
            continue
        d = json.loads(_NONFINITE.sub(
            lambda m: m.group(1) + ("NaN" if "nan" in m.group(2) else "Infinity"), line))
        if d.get("type") == "header":
            return d
    raise RuntimeError("no header in " + p)


def view_pair(h):
    """(view azimuth, view altitude) on each path, in OLD's convention."""
    cam = h["camera"]
    nav = h["oldView"]["nav"]
    v = nav["equVision"] if cam["mount"] == "equatorial" else nav["localVision"]
    n = math.sqrt(sum(c * c for c in v))
    return (math.atan2(v[1], v[0]), math.asin(v[2] / n), -cam["az"], -cam["alt"])


def wrap_pi(x):
    return (x + math.pi) % (2 * math.pi) - math.pi


def gdb_marker_count(mark):
    try:
        with open(GDBLOG, "rb") as f:
            return f.read().count(mark.encode())
    except FileNotFoundError:
        return 0


def gdb_inject(cmds, mark, timeout=120):
    before = gdb_marker_count(mark)
    os.kill(GDBPID, signal.SIGINT)
    time.sleep(3)
    with open(FIFO, "w") as f:
        for c in cmds:
            f.write(c + "\n")
        f.write(f'printf "{mark}\\n"\n')
        f.write("continue\n")
        f.flush()
    t0 = time.time()
    while time.time() - t0 < timeout:
        if gdb_marker_count(mark) > before:
            time.sleep(2)
            return True
        time.sleep(1)
    return False


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)
send(s, "timerate rate 0", 1)
send(s, "date jday 2461233.5", 1)
send(s, "meteors zhr 0", 0.6)
for f in ("atmosphere", "fog", "landscape", "milky_way", "nebulae", "stars", "show_fps",
          "planet_names", "constellation_drawing", "cardinal_points", "equatorial_grid"):
    send(s, f"flag {f} off")
send(s, "set home_planet Earth", 3)
send(s, "select planet Moon pointer off", 2)
send(s, "flag track_object on", 7)
send(s, "zoom fov 30 duration 0", 3)
send(s, "flag track_object off", 2)
send(s, "timerate rate 0", 1)

# The projector's viewport is 2048x2048 (render_size), so a drag through its
# CENTRE is the well-conditioned case; the UI hands `dragView` the SDL window
# coordinates, and that window/render mismatch is pre-existing and not this
# row's (both paths get the SAME two unprojected points either way, which is
# what the comparison rests on).
LEGS = [("vert", 1024, 924, 1024, 1124), ("horiz", 924, 1024, 1124, 1024)]
res = {}
for i, (tag, x1, y1, x2, y2) in enumerate(LEGS):
    h0 = dump(s, f"drag_{tag}_before")
    ok = gdb_inject(["break Core::updateMove", "continue", "set $core = this",
                     "delete breakpoints",
                     f"call $core->dragView({x1}, {y1}, {x2}, {y2})"],
                    f"F25DRAG{i}")
    check(f"{tag}_injection_ran", ok, f"gdb handshake F25DRAG{i} seen")
    time.sleep(1.5)
    h1 = dump(s, f"drag_{tag}_after")
    a0, b0, ca0, cb0 = view_pair(h0)
    a1, b1, ca1, cb1 = view_pair(h1)
    d_old = (wrap_pi(a1 - a0), b1 - b0)
    d_new = (wrap_pi(ca1 - ca0), cb1 - cb0)
    res[tag] = (d_old, d_new)
    print(f"-- drag {tag} ({x1},{y1})->({x2},{y2}): OLD view d(az,alt) = "
          f"({d_old[0]:+.6f}, {d_old[1]:+.6f}) rad ; NEW ({d_new[0]:+.6f}, {d_new[1]:+.6f})",
          flush=True)

dv_old, dv_new = res["vert"]
dh_old, dh_new = res["horiz"]
check("drag_reached_the_view", abs(dv_old[1]) > 1e-3 and abs(dh_old[0]) > 1e-3,
      f"the injected drag moved the OLD path: vertical leg d(alt) {dv_old[1]:+.6f} rad, "
      f"horizontal leg d(az) {dh_old[0]:+.6f} rad")
if PRE:
    check("drag_PRE_altitude_inverted",
          dv_new[1] * dv_old[1] < 0
          and abs(abs(dv_new[1]) - abs(dv_old[1])) < 0.02 * abs(dv_old[1]),
          f"pre-fix: a vertical drag moves the two paths in OPPOSITE directions — old "
          f"{dv_old[1]:+.6f} vs new {dv_new[1]:+.6f} rad, same magnitude, opposite sign: "
          f"`dragView` hands old's deltaAlt straight to a camera parameter that is its "
          f"negative")
else:
    check("drag_altitude_parity",
          dv_new[1] * dv_old[1] > 0
          and abs(dv_new[1] - dv_old[1]) < 0.01 * abs(dv_old[1]),
          f"delivered: a vertical drag moves both paths' view altitude TOGETHER — old "
          f"{dv_old[1]:+.6f} vs new {dv_new[1]:+.6f} rad ({abs(dv_new[1]-dv_old[1]):.2e} apart)")
check("drag_azimuth_parity",
      dh_new[0] * dh_old[0] > 0 and abs(dh_new[0] - dh_old[0]) < 0.01 * abs(dh_old[0]),
      f"the AZIMUTH half agrees on BOTH binaries (it was already right, and that is the "
      f"control saying the change moved the one sign that was wrong): old {dh_old[0]:+.6f} "
      f"vs new {dh_new[0]:+.6f} rad")

json.dump({k: dict(old=v[0], new=v[1]) for k, v in res.items()},
          open(f"{OUT}/f25_drag.json", "w"), indent=1)
print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
sys.exit(1 if FAILS else 0)
