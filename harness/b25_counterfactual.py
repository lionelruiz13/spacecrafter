#!/usr/bin/env python3
"""B25-emit co-delivery counterfactual (INTENT §11.73(g); task row B25-emit).

Proves the co-delivery gate DISCRIMINATES: with applyHardcodedContent gated to
legacy-only (D14 §11.79(h)), the composed Earth's apparent sidereal time comes
SOLELY from the emitted `sidereal_time=earth_apparent` key. Remove that ONE line
from the enabled twin and the composed Earth must FALL BACK to the generic spin
(a measured delta) - the -49 deg / §11.15 loss the co-delivery constraint names.

Two composed launches, same frozen scene (JD, surface observer):
  variant "full"      : the twin verbatim (key present)  -> earth_apparent
  variant "suppressed": the twin minus the sidereal_time line -> generic (LOSS)
Both remove the enabled file afterwards (shipped state restored, ALWAYS).
"""
import json, os, re, socket, subprocess, sys, time
from pathlib import Path

HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
USERDIR = HOME / ".spacecrafter"
TWIN = USERDIR / "modularSystem/SolarSystem.ini.disabled"
ENABLED = USERDIR / "modularSystem/SolarSystem.ini"
OUT = Path(__file__).parent / "artifacts/b25emit"
JD = "2461233.5"


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


def run(tag, enabled_bytes):
    dump = OUT / f"b25cf_{tag}.json"
    dump.unlink(missing_ok=True)
    ENABLED.write_bytes(enabled_bytes)
    try:
        proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                stdout=open(OUT / f"b25cf_{tag}.applog", "w"),
                                stderr=subprocess.STDOUT,
                                env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        sock = wait_port()
        time.sleep(10)
        send(sock, "timerate rate 0")
        send(sock, f"date jday {JD}")
        time.sleep(3)
        send(sock, f"body action dual_dump filename {dump}")
        time.sleep(2)
        send(sock, "shutdown action now")
        sock.close()
        try:
            proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        ENABLED.unlink(missing_ok=True)
    return dump


def earth_attitude(path):
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "body" and o.get("name") == "Earth" and o.get("new"):
            return o["new"].get("attitude"), o["new"].get("lastJD")
    return None, None


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if (USERDIR / "beta_features.ini").exists():
        raise RuntimeError("beta_features.ini present - not shipped state")
    if not TWIN.exists():
        raise RuntimeError("twin absent - run b24_equivalence.py first")
    full = TWIN.read_bytes()
    # emit-suppress: drop the Earth sidereal_time line (ISO-8859 safe: latin-1)
    text = full.decode("latin-1")
    suppressed = re.sub(r"(?m)^sidereal_time *= *earth_apparent\r?\n", "", text)
    if suppressed == text:
        raise RuntimeError("no sidereal_time line found to suppress - emit missing?")
    sup_bytes = suppressed.encode("latin-1")

    d_full = run("full", full)
    d_sup = run("suppressed", sup_bytes)
    af, jf = earth_attitude(d_full)
    asup, jsup = earth_attitude(d_sup)
    print(f"full (key present)     Earth.attitude = {af}  lastJD={jf}")
    print(f"suppressed (no key)    Earth.attitude = {asup}  lastJD={jsup}")
    if af is None or asup is None:
        print("FAIL: could not read Earth.attitude from a dump"); return 1
    delta = abs(float(af) - float(asup))
    print(f"delta = {delta:.9f} rad  ({delta*180/3.141592653589793:.4f} deg)")
    same_jd = (jf == jsup)
    print(f"same lastJD (scene control): {same_jd}")
    # The suppressed composed Earth must land on the GENERIC analytic spin (the
    # LOSS). Independently predict it from Earth's data (rot_periode is the
    # SIDEREAL day 23.9344694 h, so generic already tracks mean sidereal - the
    # apparent-vs-generic gap is the small equation-of-equinoxes/offset term,
    # NOT the dramatic 24h-default error; the mechanism bounds the magnitude).
    import math, struct
    def f32(x): return struct.unpack("f", struct.pack("f", x))[0]
    jd = float(jsup); epoch = 2451545.0
    period = f32(23.9344694 / 24.0); offset = f32(280.5)
    gen = math.fmod((jd - epoch) / period * (2 * math.pi) + offset * (math.pi / 180), 2 * math.pi)
    gen_gap = abs(gen - float(asup))
    print(f"predicted generic spin = {gen:.9f}  |suppressed - generic| = {gen_gap:.2e}")
    # Discrimination: at IDENTICAL lastJD the key removal moves the result off the
    # apparent value (delta far above the 9-sig-fig float floor ~5e-8 at mag 5)
    # AND onto the predicted generic value (proves suppressed=generic, i.e. the
    # sniff did NOT re-grant). Both required.
    matches_generic = gen_gap < 1e-6
    discriminates = same_jd and delta > 1e-5 and matches_generic
    print("COUNTERFACTUAL DISCRIMINATES: key is load-bearing; the legacy sniff does NOT "
          "re-grant on composed (suppressed lands on the predicted generic spin)"
          if discriminates else "COUNTERFACTUAL FAILED TO DISCRIMINATE")
    return 0 if discriminates else 1


if __name__ == "__main__":
    sys.exit(main())
