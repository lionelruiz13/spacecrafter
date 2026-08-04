#!/bin/bash
# Sky-look runner (free session 2026-08-04, §11.136). NOT a measurement: no
# config mutation, no md5 dance, no parity claims — this one exists to LOOK.
# Launch pattern from b10_run.sh; screenshot via the app's own readback
# (external grabs see black — app_command_interface.cpp:4034 / INTENT 11.19a).
#
# EXPECTATION [vixy 2026-08-04]: the sky render is DEFORMED FOR FISHEYE
# projection (dome master) — the image is a fisheye disc, not a rectilinear
# window. That is the product's real face; do not read the warp as a defect.
#
# Blocked until star catalogs exist (~/.spacecrafter is a skeleton — §11.136;
# spacecrafter-data install-path divergence under investigation by Vixy).
set -u
BIN="${SC_BIN:-/usr/local/bin/spacecrafter}"
OUT="${1:-/tmp/look}"
mkdir -p "$OUT"

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -5 "$OUT/app.log"; exit 1; }
done
sleep 8

python3 - "$OUT" <<'EOF'
import socket, time, sys
OUT = sys.argv[1]
def send(sock, cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
# Atacama, a frozen August night. jday [derived, look-only]:
# 2026-08-05 03:00 UTC = 23:00 Chile (UTC-4) = JD 2461256.625
send(s, "moveto lat -23.0 lon -67.8 alt 2400 duration 0", 2)
send(s, "date jday 2461256.625", 2)
send(s, "timerate rate 0", 1)
send(s, "look_at azimuth 180 altitude 70", 2)
send(s, "zoom fov 100 duration 1", 3)
time.sleep(4)  # let the render settle
send(s, f"body action screenshot filename {OUT}/sky.png", 3)
time.sleep(3)  # async write, ~1 frame later
s.close()
EOF
DRC=$?
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }
ls -la "$OUT"/sky.png 2>/dev/null || echo "no screenshot produced"
exit $DRC
