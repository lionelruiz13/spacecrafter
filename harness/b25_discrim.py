#!/usr/bin/env python3
"""B25-emit discrimination re-proof: the equivalence gate still CATCHES a broken
composition post-change. Mutation = delete the [Moon:MESH] module section from
the enabled twin; expect the composed Moon to lose its MESH module + a near-route
(the §11.89(d) mutation, re-run against the B25-emit binary)."""
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
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


def run(enabled_bytes):
    dump = OUT / "b25cf_moonmesh.json"
    dump.unlink(missing_ok=True)
    ENABLED.write_bytes(enabled_bytes)
    try:
        proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                stdout=open(OUT / "b25cf_moonmesh.applog", "w"),
                                stderr=subprocess.STDOUT,
                                env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        sock = wait_port(); time.sleep(10)
        send(sock, "timerate rate 0"); send(sock, f"date jday {JD}"); time.sleep(3)
        send(sock, f"body action dual_dump filename {dump}"); time.sleep(2)
        send(sock, "shutdown action now"); sock.close()
        try:
            proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        ENABLED.unlink(missing_ok=True)
    return dump


def moon(path):
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "body" and o.get("name") == "Moon" and o.get("new"):
            return o["new"]
    return {}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    text = TWIN.read_bytes().decode("latin-1")
    # delete the [Moon:MESH] section (header + its body lines up to the next header)
    mutated = re.sub(r"(?ms)^\[Moon:MESH\]\r?\n(?:(?!^\[).*\r?\n?)*", "", text)
    if mutated == text:
        raise RuntimeError("[Moon:MESH] section not found")
    dump = run(mutated.encode("latin-1"))
    leg = moon(OUT / "b24_legacy.json")
    mut = moon(dump)
    print("legacy   Moon.modules =", leg.get("modules"), " routing.near =", leg.get("routing", {}).get("near"))
    print("mutated  Moon.modules =", mut.get("modules"), " routing.near =", mut.get("routing", {}).get("near"))
    lost_mesh = "MESH" in (leg.get("modules") or []) and "MESH" not in (mut.get("modules") or [])
    near_dropped = (mut.get("routing", {}).get("near") or 0) < (leg.get("routing", {}).get("near") or 0)
    ok = lost_mesh and near_dropped
    print("DISCRIMINATION HOLDS: the [Moon:MESH] deletion is caught (MESH dropped + near-route dropped)"
          if ok else "DISCRIMINATION LOST")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
