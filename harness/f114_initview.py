#!/usr/bin/env python3
"""F114 -- the FRAME CONVERSION's own discriminating leg.

The field's `init_view_pos` is 1e-04,1e-04,1 -- the ZENITH -- and the
conversion (x,y,z)_old -> (y,-x,z)_camera is a 90 deg roll ABOUT the zenith, so
on the field config a correct conversion and no conversion at all differ by
0.0115 deg. This leg runs on a temp-HOME farm whose only changed key is

    init_view_pos = 1,0,0        (old-frame x = SOUTH)

where the three candidate conversions are 90 deg apart:
    correct   (y,-x,z)  -> (0,-1,0)_camera = South      : agrees with old
    dropped   (x, y,z)  -> (1, 0,0)_camera = East       : 90 deg from old
    unsigned  (y, x,z)  -> (0, 1,0)_camera = North      : 180 deg from old

Two witnesses in one launch, both through the same home (Camera::oldLocalToLocal):
  startup  loadCamera's own call, before any command is sent
  after    Core::autoZoomOut's call, through `zoom auto initial`, from a state
           where both paths have been moved away by `zoom auto in` on Mars

Usage: f114_initview.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
JD0 = 2461233.5


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print("%.3f >> %s" % (time.time(), cmd), flush=True)


def dump(sock, name, pause=2.5):
    send(sock, "body action dual_dump filename %s/%s.json" % (OUT, name), pause)


def settle(sock, secs=7.0):
    send(sock, "timerate rate 1", 0.3)
    time.sleep(secs)
    send(sock, "timerate rate 0", 1.5)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
# STARTUP WITNESS FIRST: nothing but the dump command is sent before it, so
# what it shows is loadCamera's conversion and old's setLocalVision(InitViewPos),
# with no command of ours in between.
send(s, "timerate rate 0", 1)
dump(s, "startup")

send(s, "flag atmosphere off")
send(s, "flag fog off")
send(s, "flag landscape off")
send(s, "flag show_fps off")
send(s, "flag stars on")
send(s, "flag star_names off")
send(s, "flag planet_names off")
send(s, "timerate rate 1", 0.3)
send(s, "date jday %.6f" % JD0, 1.0)
send(s, "timerate rate 0", 2.0)
dump(s, "before_zoom")

send(s, "select planet Mars", 1)
send(s, "zoom auto in duration 0", 1.5)
settle(s, 7.0)
dump(s, "tracked")                 # both paths on Mars, far from the init dir

send(s, "zoom auto initial duration 0", 1.5)
settle(s, 5.0)
dump(s, "after")                   # both paths back on the init direction

print("f114 initview done", flush=True)
s.close()
