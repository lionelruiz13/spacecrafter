#!/usr/bin/env python3
# B17 - view_offset ARMING lifecycle + reversibility (INTENT 11.63(b)/11.79(c)).
# Reproduces the old view_offset_transition (navigator.cpp:73-78): INERT at a
# fresh un-moved view, ARMED (sticky) by a commanded view move, DISARMED by a
# zoom-out-to-init, re-armable. The state channel is the dumped viewOffsetEff
# (= viewOffset * viewOffsetTransition, the effective offset the render pitch
# uses). Reversible pairs (scalar 0<->0.3, arm<->disarm) traversed TWICE, the
# second entry from the first exit's state (INTENT reversibility rule).
#
# Usage: b17_arming.py <outdir>   (app fresh-launched, FISHEYE, enable_tcp)
import socket, sys, time, os, json
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
FAILS = []
def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok: FAILS.append(name)
def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192)
    except socket.timeout: pass
    sock.settimeout(None); print(">>", cmd, flush=True)
def eff(sock, name, pause=1.5):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    c = json.loads(open(f"{OUT}/{name}.json").readline())["camera"]
    e = c["viewOffsetEff"]
    print(f"   {name}: vo={c['viewOffset']} trans={c['viewOffsetTransition']:.4f} eff={e:.4f}", flush=True)
    return e

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
send(s, "flag atmosphere off"); send(s, "flag landscape off"); send(s, "flag show_fps off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "date jday 2461233.5", 1); send(s, "timerate rate 0", 1)

send(s, "set zoom_offset 0.3", 1)
e_fresh = eff(s, "fresh")                     # scalar set, NO move yet -> INERT
check("inert_at_fresh_view", abs(e_fresh) < 1e-3, f"eff={e_fresh:.4f} (offset set but no commanded move)")

send(s, "look_at azimuth 60 altitude 30 duration 1", 2); time.sleep(2)
e_arm = eff(s, "armed")
check("arms_on_commanded_move", abs(e_arm - 0.3) < 1e-3, f"eff={e_arm:.4f}")

# scalar reversibility 0.3 -> 0 -> 0.3 -> 0 -> 0.3 (armed throughout)
send(s, "set zoom_offset 0", 1);   a0 = eff(s, "scal0a")
send(s, "set zoom_offset 0.3", 1); a3 = eff(s, "scal3a")
send(s, "set zoom_offset 0", 1);   b0 = eff(s, "scal0b")
send(s, "set zoom_offset 0.3", 1); b3 = eff(s, "scal3b")
check("scalar_reversible", abs(a0) < 1e-3 and abs(a3 - 0.3) < 1e-3 and abs(b0) < 1e-3 and abs(b3 - 0.3) < 1e-3,
      f"0->{a0:.3f} 0.3->{a3:.3f} 0->{b0:.3f} 0.3->{b3:.3f}")

# arm<->disarm reversible pair, TWICE (second from the first's exit state)
send(s, "zoom auto out duration 1", 3); time.sleep(2); d1 = eff(s, "disarm1")
send(s, "look_at azimuth 120 altitude 40 duration 1", 2); time.sleep(2); r1 = eff(s, "rearm1")
send(s, "zoom auto out duration 1", 3); time.sleep(2); d2 = eff(s, "disarm2")
send(s, "look_at azimuth 60 altitude 30 duration 1", 2); time.sleep(2); r2 = eff(s, "rearm2")
check("disarm_rearm_x2", abs(d1) < 1e-3 and abs(r1 - 0.3) < 1e-3 and abs(d2) < 1e-3 and abs(r2 - 0.3) < 1e-3,
      f"disarm1={d1:.3f} rearm1={r1:.3f} disarm2={d2:.3f} rearm2={r2:.3f}")

# swallow-guard: bogus spelling is a no-op (the offset scalar must not change)
send(s, "set zoom_ofset 0.4", 1)
bg = eff(s, "bogus")
check("bogus_spelling_noop", abs(bg - 0.3) < 1e-3, f"eff stays {bg:.4f} after `set zoom_ofset 0.4`")

print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
