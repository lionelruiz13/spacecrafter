#!/usr/bin/env python3
"""F45 rider — the spInt census, taken for free off the EMPTY leg's own error line.

Why: the data root carries exactly one spectral table, `stars_hip_sp_0v0_0.cat`
(4122 lines), while `~/.spacecrafter/stars.ini` asks for `stars_hip_sp_0v0_4.cat`
(§5.90's split-root mismatch, spectral member). Populating the array with the version
that EXISTS answers §5.88's cost question — but it leaves a second question the same
control can answer at no extra cost: does that table COVER the star catalogues this
install actually loads? Every star whose `getSpInt()` is >= the table's size would
still read "" and still log a bad index, populated or not.

The instrument is the defect itself. With the array empty, `convertToSpectralType`
logs `bad index: <N>, max: 0` for EVERY call (hip_star_mgr.cpp:76-80), where N is the
star's own `getSpInt()`. So driving the per-frame readout across many stars turns the
app into a census of spInt over the loaded catalogues, with no product code and no
guess about the .cat binary layout — the loader stays the authority on what the bytes
mean. Run this on the `empty` leg only; on `full` the line does not fire.

Usage: F45_PROBE=f45_spint.py ./f45_run.sh empty spint
"""
import json
import os
import socket
import sys
import time

OUT = sys.argv[1]
APPLOG = sys.argv[2]
DATAROOT = "/usr/local/share/spacecrafter"
STEP = 0.10          # s between selections; at 144 fps that is ~14 frames each
os.makedirs(OUT, exist_ok=True)

# every distinct HIP the installed name.fab names, in file order
hips, seen = [], set()
for line in open(os.path.join(DATAROOT, "stars", "name.fab"), "rb"):
    tok = line.split(b"|")[0].strip()
    if tok.isdigit() and tok not in seen:
        seen.add(tok)
        hips.append(int(tok))

sock = socket.create_connection(("127.0.0.1", 7805), timeout=20)


def send(cmd, pause=STEP):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)


send("flag show_tui_short_obj_info on", pause=1.0)
t0 = time.time()
for i, hp in enumerate(hips):
    send(f"select hp {hp} pointer off")
    if i % 250 == 0:
        print(f"  {i}/{len(hips)} hp={hp} t={time.time()-t0:.0f}s", flush=True)
send("flag show_tui_short_obj_info off", pause=1.0)

json.dump({"hips": hips, "step_s": STEP, "elapsed_s": time.time() - t0},
          open(os.path.join(OUT, "spint_probe.json"), "w"))
print("census drive done:", len(hips), "hips in", round(time.time() - t0, 1), "s", flush=True)
send("shutdown action now", pause=1.0)
time.sleep(2.0)
sock.close()
