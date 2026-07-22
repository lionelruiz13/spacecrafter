#!/usr/bin/env python3
"""B26 observable check driver - dual-path default flip (INTENT 11.50(c)).

One run == one case (default / alternate / explicit-new / explicit-old).
Protocol (identical for every case):
  A. scene setup, frozen time, animated overlays off, long settle
  B. warmup shot (discarded) + freeze witness dump #1
  C. 24 stability shots, 0.25 s spacing (6 s span). The DoD pair is any
     (k, k+10) = 2.5 s apart; (00,10) is reported as THE pair.
  D. freeze witness dump #2 -> equal jd proves the freeze held over the burst
  E. path-identity references: pin OLD -> shot, pin NEW -> shot x2
     (the two pin_new shots are a same-state pair -> instrument noise floor)
Usage: drive_b26.py <case-tag> <outdir>
"""
import socket, sys, time, os

TAG = sys.argv[1]
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)

def shot(sock, name, pause=0.5):
    p = f"{OUT}/{TAG}_{name}.png"
    t = time.time()
    sock.sendall(f"body action screenshot filename {p}\n".encode())
    print(f"{t:.3f} SHOT {name}", flush=True)
    time.sleep(pause)
    return t

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# --- A. scene setup -------------------------------------------------------
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "flag star_twinkle off")
send(s, "flag show_fps off")
send(s, "flag planet_names off")
send(s, "flag subtitle off")
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "select planet Moon", 1)
send(s, "flag track_object on", 15)
send(s, "flag moon_scaled on", 1)
send(s, "set moon_scale 30", 15)          # ASmooth ease - settle
send(s, "timerate rate 0", 2)             # re-assert after any startup.sts race
extra = float(os.environ.get("SETTLE_EXTRA", "0"))
if extra:
    print(f"extra settle {extra}s", flush=True); time.sleep(extra)

# --- B. warmup + freeze witness #1 ---------------------------------------
shot(s, "warmup", 2)
send(s, f"body action dual_dump filename {OUT}/{TAG}_jd1.json", 10)

# --- C. stability burst (24 x 0.25 s = 6 s) -------------------------------
ts = []
for i in range(24):
    ts.append(shot(s, f"stab{i:02d}", 0.25))
time.sleep(3)   # last readback lands async

# --- D. freeze witness #2 -------------------------------------------------
send(s, f"body action dual_dump filename {OUT}/{TAG}_jd2.json", 3)

# --- E. path-identity references -----------------------------------------
send(s, "flag experimental_path off", 4)   # pin OLD
shot(s, "pin_old", 3)
send(s, "flag experimental_path on", 4)    # pin NEW
shot(s, "pin_new", 3)
shot(s, "pin_new2", 3)                     # same-state pair -> noise floor
time.sleep(2)

print("TIMESTAMPS " + " ".join(f"{t:.3f}" for t in ts), flush=True)
send(s, "shutdown action now", 2)
s.close()
print("driver done", flush=True)
