#!/usr/bin/env python3
"""F38 — the config-leg probe: dump the camera state AS INITIALIZED, nothing else.

Deliberately minimal. The question D15(d) asks is what init/reinit LEAVES the
camera in, so the probe must not send a single command that could change it —
no scene setup, no time freeze, no moveto. One dump, then out.

Usage: f38_config_probe.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
s.sendall(f"body action dual_dump filename {OUT}/startup.json\n".encode())
time.sleep(3.0)
print("dumped", flush=True)
s.close()
