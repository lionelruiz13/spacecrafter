#!/usr/bin/env python3
"""B33's two LATENT members — the view offset and the mount (INTENT §11.108(f),
delivered §11.131). Driver for `f23_b33_inject_run.sh`.

WHY INJECTION, AND WHY THAT IS NOT A WEAKER CHECK. Both members have dual
authorities that NOTHING SHIPPED SPLITS today:

  * the view offset has ONE writer, `Core::setViewOffset`, and it has handed
    the same clamped scalar to both paths since §11.92; the session restore
    joined it at §11.130(e).
  * the mount is config-only on both paths (B35) and both are initialised from
    the SAME key, [navigation] viewing_mode — `core.cpp` for the navigator,
    `ssystem_factory.cpp` for the camera. `Core::setMountMode` and
    `toggleMountMode` have zero callers in src/.

So a "natural repro" for these two does not exist, and waiting for one is how
the class stays open. The check is therefore derived from the MECHANISM — two
authorities exist and the drawn one must be reported — and the divergence is
created by writing the DRAWN path's authority directly, in the live process,
through the same gdb-FIFO instrument `b21_keypath.py` uses. The layer above
(who would call the setter) is source-verified, not injected: stated, not
hidden.

Usage: f23_b33_inject.py <outdir> <gdb_pid> <gdb_fifo>
"""
import json, os, signal, socket, sys, time

OUT, GDBPID, FIFO = sys.argv[1], int(sys.argv[2]), sys.argv[3]
os.makedirs(OUT, exist_ok=True)
GDBLOG = f"{OUT}/gdb.log"
FAILS = []


def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok:
        FAILS.append(name)


def send(sock, cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.25)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(">>", cmd, flush=True)


def _clean(ln):
    return ln.strip().replace('-nan', 'null').replace('nan', 'null') \
             .replace('-inf', '-1e308').replace('inf', '1e308')


def control(sock, tag, pause=2.0):
    send(sock, f"body action dual_dump filename {OUT}/{tag}.json", pause)
    with open(f"{OUT}/{tag}.json") as f:
        hdr = json.loads(_clean(f.readline()))
    c = hdr["control"]
    print(f"-- {tag}: offset {json.dumps(c['viewOffset'])} "
          f"mount {json.dumps(c['mount'])}", flush=True)
    return c


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
send(s, "flag atmosphere off")
send(s, "flag landscape off")

# ---- V1: the AGREED half, through the shipped channel --------------------
# `set zoom_offset` is the registered §2(c) name (B17/§11.63; `set view_offset`
# is refused - §11.130(e) measured that). It writes both paths, so both
# binaries must report the same thing here: the fix must be invisible while
# the authorities agree.
send(s, "set zoom_offset 0.3", 2)
c = control(s, "v1")
check("V1_agreed",
      abs(c["viewOffset"]["reported"] - 0.3) < 1e-9
      and abs(c["viewOffset"]["old"] - 0.3) < 1e-9
      and abs(c["viewOffset"]["new"] - 0.3) < 1e-9,
      f"`set zoom_offset 0.3` -> reported {c['viewOffset']['reported']}, "
      f"old {c['viewOffset']['old']}, new {c['viewOffset']['new']}")
check("M0_agreed", c["mount"]["reported"] == c["mount"]["old"] == c["mount"]["new"],
      f"mount reported {c['mount']['reported']}, old {c['mount']['old']}, "
      f"new {c['mount']['new']} (both from [navigation] viewing_mode)")

# ---- V2/M1: force the divergence on the DRAWN path -----------------------
oldmount = c["mount"]["old"]
# `CameraMount::ALTAZ` is a syntax error in a gdb expression (scoped enum);
# the enumerator's VALUE is what the call needs — ALTAZ 0, EQUATORIAL 1
# (Camera.hpp's declaration order). Found by the run, not guessed: the first
# attempt logged "A syntax error in expression, near `::ALTAZ)'" and the mount
# leg then passed VACUOUSLY on two equal values, which is why M1_reported now
# requires the divergence it is reporting about.
want_mount = "equatorial" if oldmount == "altaz" else "altaz"
ok = gdb_inject([
    "call (void) Camera::instance->setViewOffset(0.15)",
    f"call (void) Camera::instance->setMount((CameraMount){0 if want_mount == 'altaz' else 1})",
    "print Camera::instance->getViewOffset()",
    "print (int)Camera::instance->getMount()",
], "F23_INJECTED")
check("inject", ok, "gdb wrote the DRAWN path's two authorities directly")

c = control(s, "v2")
check("V2_diverged",
      abs(c["viewOffset"]["new"] - 0.15) < 1e-9
      and abs(c["viewOffset"]["old"] - 0.3) < 1e-9,
      f"the two authorities are apart: old {c['viewOffset']['old']}, "
      f"drawn {c['viewOffset']['new']}")
# F23_PRE=1 runs the SAME scene against the pre-fix binary and asserts the
# DEFECT instead of the fix — the red half, written as an assertion so it
# cannot pass by the leg silently not running.
PRE = os.environ.get("F23_PRE") == "1"
check("V2_reported",
      abs(c["viewOffset"]["reported"]
          - (c["viewOffset"]["old"] if PRE else c["viewOffset"]["new"])) < 1e-9
      and abs(c["viewOffset"]["new"] - c["viewOffset"]["old"]) > 1e-9,
      (f"PRE-FIX: the readout reports the old authority {c['viewOffset']['old']} "
       f"while the drawn path holds {c['viewOffset']['new']}" if PRE else
       f"the readout follows the path that draws: {c['viewOffset']['reported']} "
       f"(a binary that reads the old authority says {c['viewOffset']['old']})"))
check("M1_diverged", c["mount"]["new"] == want_mount and c["mount"]["old"] == oldmount,
      f"the two mounts are apart: old {c['mount']['old']}, drawn {c['mount']['new']}")
check("M1_reported",
      c["mount"]["reported"] == (c["mount"]["old"] if PRE else c["mount"]["new"])
      and c["mount"]["new"] != c["mount"]["old"],
      (f"PRE-FIX: the mount readout reports the old authority "
       f"{c['mount']['old']} while the drawn path holds {c['mount']['new']}" if PRE else
       f"the mount readout follows the path that draws: "
       f"{c['mount']['reported']} (old authority says {c['mount']['old']})"))

# ---- V3/M2: the pin, entered TWICE --------------------------------------
bad = []
for i in range(2):
    send(s, "flag experimental_path off", 2)
    c = control(s, f"v3_off{i}")
    if abs(c["viewOffset"]["reported"] - c["viewOffset"]["old"]) > 1e-9:
        bad.append(f"off{i}: offset {c['viewOffset']['reported']} != old "
                   f"{c['viewOffset']['old']}")
    if c["mount"]["reported"] != c["mount"]["old"]:
        bad.append(f"off{i}: mount {c['mount']['reported']} != old {c['mount']['old']}")
    send(s, "flag experimental_path on", 2)
    c = control(s, f"v3_on{i}")
    want_o = c["viewOffset"]["old"] if PRE else c["viewOffset"]["new"]
    want_m = c["mount"]["old"] if PRE else c["mount"]["new"]
    if abs(c["viewOffset"]["reported"] - want_o) > 1e-9:
        bad.append(f"on{i}: offset {c['viewOffset']['reported']} != {want_o}")
    if c["mount"]["reported"] != want_m:
        bad.append(f"on{i}: mount {c['mount']['reported']} != {want_m}")
check("pin_twice", not bad,
      "`flag experimental_path off/on` flips BOTH readouts back and forth, on "
      "both entries of the pair, the second from the state the first left"
      + ("" if not bad else "  " + "; ".join(bad)))

print(f"\n{'OK' if not FAILS else str(len(FAILS)) + ' FAILS'}", flush=True)
sys.exit(1 if FAILS else 0)
