#!/usr/bin/env python3
"""B16 - `body action reload` observable check (INTENT 11.55).

One fresh launch. Protocol:
  A. deterministic scene: fixed jd, frozen time, anchored on Earth (the
     reference body the reload DESTROYS and recreates), tracking the Moon
     (the tracked-body reference, same problem).
  B. baseline dump + shot
  C. MUTATE ~/.spacecrafter/ssystem.ini (Moon radius x2) -> reload #1 ->
     dump + shot.  The mutation is the proof the reload was not a no-op:
     a no-op reload passes every state-preservation check trivially.
  D. RESTORE the data file byte-identically (md5 asserted) -> reload #2,
     which starts from the state reload #1 produced (reversible pair, both
     entries) -> dump + shot.  The Moon must come back to its file radius.
  E. dates: the jd of every dump must be identical (a reload never speaks to
     the clock) - measured, not assumed.

Usage: b16_reload.py <outdir>
"""
import socket, sys, time, os, shutil, hashlib, subprocess

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
BAK = os.path.join(OUT, "ssystem.ini.orig")


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def dump(sock, name, pause=3):
    p = f"{OUT}/{name}.json"
    send(sock, f"body action dual_dump filename {p}", pause)
    return p


def shot(sock, name, pause=3):
    p = f"{OUT}/{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    return p


shutil.copy2(SSY, BAK)
MD5_ORIG = md5(SSY)
print(f"ssystem.ini md5 (original) = {MD5_ORIG}", flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# --- A. scene -------------------------------------------------------------
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "flag star_twinkle off")
send(s, "flag show_fps off")
send(s, "flag planet_names off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
# pointer off: the selection pointer is an ANIMATED overlay whose bracket
# radius eases toward the object's apparent size, so it moves for seconds
# after any size change - it would dominate the screen A/B with something
# that is not the reload (measured, first B16 run: 266 px>32, all of it the
# four red brackets).  Selection itself stays on (it is one of the
# references the reload must re-seat) and still drives track_object.
send(s, "select planet Moon pointer off", 1)
send(s, "flag track_object on", 15)   # tracking settle (INTENT 11.19c)
send(s, "flag moon_scaled off", 1)
send(s, "timerate rate 0", 2)

# --- B. baseline + NO-RELOAD CONTROL PAIR --------------------------------
# The control is what makes the post-reload deltas readable: it is the same
# state sampled twice with nothing done in between, i.e. the floor of this
# instrument (tracking never exactly settles - Camera.cpp:264 re-plans every
# frame - and the new path is not bit-stable on a frozen scene, B30).
shot(s, "b16_ctrl")
dump(s, "b16_ctrl")
shot(s, "b16_pre")
dump(s, "b16_pre")

# --- C. mutate + reload #1 ------------------------------------------------
with open(SSY, "rb") as f:
    data = f.read()
assert data.count(b"radius = 1737.4\n") == 1, "Moon radius marker not unique"
mut = data.replace(b"radius = 1737.4\n", b"radius = 3474.8\n")
with open(SSY, "wb") as f:
    f.write(mut)
print(f"ssystem.ini md5 (mutated)  = {md5(SSY)}", flush=True)
send(s, "body action reload", 5)
shot(s, "b16_post1")
dump(s, "b16_post1")

# --- D. restore + reload #2 (second entry of the pair) --------------------
shutil.copy2(BAK, SSY)
back = md5(SSY)
print(f"ssystem.ini md5 (restored) = {back}  MATCH={back == MD5_ORIG}", flush=True)
assert back == MD5_ORIG
send(s, "body action reload", 5)
shot(s, "b16_post2")
dump(s, "b16_post2")

# --- E. third reload, unmutated: the state reload #2 produced -------------
send(s, "body action reload", 5)
dump(s, "b16_post3")
print("b16 done", flush=True)
s.close()
