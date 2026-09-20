#!/usr/bin/env python3
"""F125 - INTENT 5.162 against B10's own gate: WHEN does `body name <X> ground_radius <km>`
fail to move the scaled ground radius? (INTENT 11.263 (F))

    bash f124_run.sh ../f125/<label> f125_adapt.py f125_radius.py

b10_cmd.py (command issued from altitude >= 0) is ALL PASS on the same binary on which F124's
leg M read a stale radius (command issued after its descent had carried the Camera UNDER the
datum). One body, one launch, the command issued in three states, each read back from the dump
4 s later (scaled ground radius of the body record, `camera.refCached`, the Camera's altitude):
    above     anchored 10 km above the datum                      (b10_cmd's state)
    under     after three `camera action descend coef 0.99`       (F124's state: under the datum)
    back_up   after `moveto` back to 2000 km, a NEW value commanded
A state is "moved" when the dump reads the commanded value within 1 km. No expectation is
asserted on `under` and `back_up`: this driver attributes, it does not gate. Exit 0 when the
control state `above` moved (the scene is the one described), 3 otherwise.
"""
import json, os, socket, sys, time

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
AU_KM = 149597870.691
NAME = "RadT"


def send(sock, cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.25)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(">> " + cmd, flush=True)


def read(tag, pause=2.0):
    path = "%s/%s.json" % (OUT, tag)
    send(S, "body action dual_dump filename " + path, pause)
    head = rec = None
    for ln in open(path):
        ln = ln.strip().replace("-nan", "null").replace("nan", "null")
        if not ln:
            continue
        d = json.loads(ln)
        if d.get("type") == "header":
            head = d
        elif d.get("type") == "body" and d.get("name") == NAME:
            rec = d["new"]
    cam = head["camera"]
    return {"ground_km": rec["scaledGroundRadius"] * AU_KM, "datum_km": rec["scaledDatumRadius"] * AU_KM,
            "refCached": cam.get("refCached"), "reference": cam["reference"],
            "altitude_km": head["control"]["altitude"]["new"] / 1000.0,
            "relation": rec.get("relation"), "visible": rec.get("visible")}


S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(S, "flag experimental_path on", 1)
send(S, "date jday 2461233.5", 1)
send(S, "timerate rate 0", 1)
send(S, "body action load name %s radius 6000 parent Sun type Planet oblateness 0.0 albedo 0.3 "
        "halo false color 0.6,0.6,0.9 tex_map bodies/moon.png coord_func still_orbit "
        "orbit_x 900 orbit_y 0 orbit_z 0" % NAME, 2)
send(S, "set home_planet " + NAME, 3)
send(S, "moveto lat 0 lon 0 alt 10000 duration 0", 2)

ROWS = []


def commanded(state, km):
    before = read(state + "_before")
    send(S, "body name %s ground_radius %d" % (NAME, km), 4)
    after = read(state + "_after")
    moved = abs(after["ground_km"] - km) < 1.0
    ROWS.append({"state": state, "commanded_km": km, "moved": moved, "before": before, "after": after})
    print("%s %-8s commanded %d km from altitude %.4f km: scaled ground %.3f -> %.3f km, refCached %s -> %s"
          % ("MOVED" if moved else "STALE", state, km, before["altitude_km"], before["ground_km"],
             after["ground_km"], before["refCached"], after["refCached"]), flush=True)
    return moved


control = commanded("above", 6050)
for _ in range(3):
    send(S, "camera action descend coef 0.99", 1.0)
commanded("under", 6100)
send(S, "moveto lat 0 lon 0 alt 2000000 duration 0", 3)
late = read("back_up_same_value")
print("NOTE  back above the datum, no new command: scaled ground %.3f km, refCached %s, altitude %.4f km"
      % (late["ground_km"], late["refCached"], late["altitude_km"]), flush=True)
commanded("back_up", 6150)
json.dump({"rows": ROWS, "back_up_same_value": late}, open(OUT + "/f125_radius_results.json", "w"), indent=1)
print("=== control state `above` %s" % ("moved: the scene is the one described" if control
                                         else "DID NOT MOVE: scene not reached"), flush=True)
sys.exit(0 if control else 3)
