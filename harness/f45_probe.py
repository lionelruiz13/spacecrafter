#!/usr/bin/env python3
"""F45 — the leg driver: read the spectral readout on demand, then make it per-frame.

Phases (identical in both legs; the ONLY difference between legs is the one
stars.ini key the launcher set):

  A  baseline `get status object` with nothing selected      -> the channel's floor
  B  for each HP of the fixed sample: select, `get status object`
     -> the ON-DEMAND route (hip_star_wrapper.cpp:151), P2 + P4
  C  a deterministic star scene + a screenshot                -> P1 (predicted null)
  D  `flag show_tui_short_obj_info on` + one selected star, dwell
     -> the PER-FRAME route (ui_tuiconf.cpp:126 inside UI::drawGravityUi), P5 + P6
  E  `shutdown action now` -> a CLEAN exit, so CaptureMetrics' destructor flushes
     the tail of statistics.dat (EntityCore/Tools/CaptureMetrics.cpp:17-20) and the
     log's last lines are on disk.

The app's log is counted at each phase boundary so the on-demand lines (phase B) and
the per-frame lines (phase D) are separable: cLog flushes every write (log.cpp:152-156),
so a read at any instant is complete up to that instant.

Usage: f45_probe.py <outdir> <app-log-path>
"""
import json
import os
import socket
import sys
import time

OUT = sys.argv[1]
APPLOG = sys.argv[2]
DWELL = 30.0

os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=20)

rec = {"phases": {}, "hp": {}}


def logcount(pattern):
    """Lines in the app log containing `pattern`, right now."""
    try:
        with open(APPLOG, "rb") as f:
            data = f.read()
    except OSError:
        return -1
    return data.count(pattern.encode())


def send(cmd, pause=0.35):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)


def ask(cmd, quiet=0.5, cap=6.0):
    """Send a command that answers, and collect the reply until `quiet` seconds of
    silence (the reply is multi-line: getInfoString embeds std::endl)."""
    sock.sendall((cmd + "\n").encode())
    buf = b""
    t0 = time.time()
    sock.settimeout(quiet)
    while time.time() - t0 < cap:
        try:
            chunk = sock.recv(8192)
        except socket.timeout:
            if buf:
                break
            continue
        if not chunk:
            break
        buf += chunk
    sock.settimeout(20)
    return buf.decode("utf-8", "replace")


def spectral_of(info):
    """The value the app put after 'Spectral Type: ', or None if the line is absent.
    An EMPTY string is a DIFFERENT answer from an absent line, and P2 turns on the
    difference, so the two are never collapsed."""
    for line in info.split("\n"):
        if "Spectral Type:" in line:
            return line.split("Spectral Type:", 1)[1].strip()
    return None


def compids_of(info):
    """The component-id token, if the HP line carries one (the P4 positive control)."""
    for line in info.split("\n"):
        s = line.strip()
        if s.startswith("HP "):
            parts = s.split()
            return parts[2] if len(parts) > 2 else ""
    return None


# ---------------------------------------------------------------- phase A
rec["phases"]["A_start_badindex"] = logcount("convertToSpectralType: bad index")
base = ask("get status object")
rec["baseline_no_selection"] = base
print("A baseline reply:", repr(base[:200]), flush=True)

# ---------------------------------------------------------------- phase B
hp_list = [int(x) for x in open(os.path.join(OUT, "hp_list.txt")).read().split()]
rec["hp_list"] = hp_list
for hp in hp_list:
    send(f"select hp {hp} pointer off", pause=0.25)
    info = ask(f"get status object")
    rec["hp"][str(hp)] = {
        "raw": info,
        "spectral": spectral_of(info),
        "compids": compids_of(info),
        "is_star1": "Spectral Type:" in info or info.strip().startswith("HP "),
    }
    print(f"B hp {hp}: spectral={spectral_of(info)!r} compids={compids_of(info)!r}", flush=True)
rec["phases"]["B_end_badindex"] = logcount("convertToSpectralType: bad index")
rec["phases"]["B_end_badindex_cids"] = logcount("convertToComponentIds: bad index")

# ---------------------------------------------------------------- phase C
# A deterministic sky: same date, frozen time, no atmosphere or landscape to mask the
# star channel, no names/lines to add text. Identical script in both legs, so any pixel
# difference is attributable; P1 predicts there is none.
for c in [
    "deselect",
    "flag atmosphere off",
    "flag landscape off",
    "flag milky_way off",
    "flag nebula off",
    "flag constellation_drawing off",
    "flag star_names off",
    "flag planets off",
    "flag stars on",
    "date utc 2020-01-01T22:00:00",
    "timerate rate 0",
]:
    send(c, pause=0.3)
time.sleep(3.0)
shot = os.path.join(OUT, "sky.png")
send(f"body action screenshot filename {shot}", pause=0.2)
time.sleep(4.0)
rec["screenshot"] = shot
rec["screenshot_exists"] = os.path.exists(shot)
print("C screenshot exists:", rec["screenshot_exists"], flush=True)

# ---------------------------------------------------------------- phase D
# The per-frame route. Pick the FIRST HP of the sample that the app answered with a
# 'Spectral Type:' line - that choice is made from the app's own answer, so it is the
# same star in both legs whenever the sample is (asserted in the analyzer).
target = None
for hp in hp_list:
    if rec["hp"][str(hp)]["spectral"] is not None:
        target = hp
        break
rec["perframe_target_hp"] = target
if target is None:
    print("D SKIPPED: no sampled HP produced a Spectral Type line", flush=True)
else:
    send(f"select hp {target} pointer off", pause=0.5)
    rec["phases"]["D_start_badindex"] = logcount("convertToSpectralType: bad index")
    rec["phases"]["D_start_wall"] = time.time()
    send("flag show_tui_short_obj_info on", pause=0.5)
    time.sleep(DWELL)
    rec["phases"]["D_end_wall"] = time.time()
    rec["phases"]["D_end_badindex"] = logcount("convertToSpectralType: bad index")
    send("flag show_tui_short_obj_info off", pause=0.5)
    rec["phases"]["D_dwell_s"] = rec["phases"]["D_end_wall"] - rec["phases"]["D_start_wall"]
    print("D per-frame lines added:",
          rec["phases"]["D_end_badindex"] - rec["phases"]["D_start_badindex"],
          "over", round(rec["phases"]["D_dwell_s"], 2), "s", flush=True)

rec["phases"]["E_final_badindex"] = logcount("convertToSpectralType: bad index")

# ---------------------------------------------------------------- phase E
with open(os.path.join(OUT, "probe.json"), "w") as f:
    json.dump(rec, f, indent=1)
send("shutdown action now", pause=1.0)
time.sleep(2.0)
sock.close()
print("E shutdown requested", flush=True)
