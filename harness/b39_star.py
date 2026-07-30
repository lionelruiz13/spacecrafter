#!/usr/bin/env python3
# B39 - the ONE surface D23's general wording reaches but the ledger row does NOT
# enumerate: a hidden body that is the system's LIGHT SOURCE.  Measured rather
# than argued, so the question recorded for Vixy is about a fact.
#
# `body name Sun hidden true` parks the star. Two things are then observable:
#   (1) does the rest of the system still get LIT by it (i.e. does "as if it
#       didn't exist" reach illumination, which the row's list - orbit line,
#       trail, hints, axis, grid, pointer, shadow cast/receive, occlusion,
#       click-pick - does not mention)?
#   (2) does the whole subtree parking under the star behave (Earth is the
#       observer's reference AND a child of the parked node: the deepest rare
#       path this change has), reversibly, entered twice?
# Also measured: the barrier on the light source (updateSystem calls useNow on
# the star before reading its position), by checking the star's evalCount grows
# per USE and not per frame.

import socket, time, json, sys, os
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "b39_star")
os.makedirs(OUT, exist_ok=True)
THR = 32
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
rep = {}


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=1.8):
    send(f"body action screenshot filename {OUT}/t_{tag}.png", pause); return tag


def dump(tag, pause=2.2):
    send(f"body action dual_dump filename {OUT}/t_{tag}.json", pause); return tag


def load(tag):
    hdr, out = None, {}
    for line in open(os.path.join(OUT, f"t_{tag}.json")):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "header":
            hdr = d
        elif d.get("type") == "body" and d.get("new"):
            out[d["name"]] = d["new"]
    return hdr, out


def img(tag):
    return np.asarray(Image.open(os.path.join(OUT, f"t_{tag}.png"))
                      .convert("RGB")).astype(np.int32)


def lit(tag):
    return int((img(tag).max(axis=2) > 16).sum())


def dpx(a, b):
    d = np.abs(img(a) - img(b)).max(axis=2)
    return int((d > THR).sum()), int(d.max())


# Frame the MOON from Earth: a lit body whose brightness comes entirely from the
# star, big enough on screen to read.
send("flag experimental_path on", 1)
send("timerate rate 0", 1)
send("date jday 2461233.5", 1)
send("flag landscape off", 1)
send("flag atmosphere off", 1)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
send("select planet Moon pointer off", 1)
send("flag track_object on", 5)
send("flag track_object off", 1.5)
send("zoom fov 30 duration 0", 3)
shot("base_a"); shot("base_b")
floor, floor_max = dpx("base_a", "base_b")
print(f"floor {floor} px (max {floor_max}); lit(base) = {lit('base_a')}")
rep["floor_px"] = floor
rep["lit_base"] = lit("base_a")

states = []
for entry in (1, 2):
    send("body name Sun hidden true", 2.0)
    h = dump(f"star_hidden{entry}"); shot(f"star_hidden{entry}")
    _, B = load(h)
    time.sleep(6)
    h2 = dump(f"star_hidden{entry}b")
    _, B2 = load(h2)
    sun_grow = B2["Sun"]["evalCount"] - B["Sun"]["evalCount"]
    earth_grow = B2["Earth"]["evalCount"] - B["Earth"]["evalCount"]
    moon_grow = B2["Moon"]["evalCount"] - B["Moon"]["evalCount"]
    send("body name Sun hidden false", 2.0)
    s = dump(f"star_shown{entry}"); shot(f"star_shown{entry}")
    px, mx = dpx(f"star_hidden{entry}", f"star_shown{entry}")
    st = {"entry": entry, "sun_relation_hidden": B["Sun"]["relation"],
          "earth_relation_hidden": B["Earth"]["relation"],
          "lit_hidden": lit(f"star_hidden{entry}"),
          "lit_shown": lit(f"star_shown{entry}"),
          "px_hidden_vs_shown": px, "max_hidden_vs_shown": mx,
          "sun_eval_growth_over_6s": sun_grow,
          "earth_eval_growth_over_6s": earth_grow,
          "moon_eval_growth_over_6s": moon_grow}
    states.append(st)
    print(f"entry {entry}: Sun relation={st['sun_relation_hidden']} (hidden), "
          f"Earth declared relation={st['earth_relation_hidden']} (UNCHANGED - "
          f"nesting does not touch it)")
    print(f"  lit px: hidden={st['lit_hidden']}  shown={st['lit_shown']}  "
          f"diff {px} px>{THR} (max {mx})")
    print(f"  evalCount over 6 s while the star is parked: Sun +{sun_grow}, "
          f"Earth +{earth_grow} (the reference - still walked), Moon +{moon_grow}")

rep["entries"] = states
px_hh, _ = dpx("star_hidden1", "star_hidden2")
px_ss, _ = dpx("star_shown1", "star_shown2")
print(f"reversible: hidden1-vs-hidden2 {px_hh} px  |  shown1-vs-shown2 {px_ss} px "
      f"(floor {floor})")
rep["hidden_vs_hidden_px"] = px_hh
rep["shown_vs_shown_px"] = px_ss
sock.close()
with open(os.path.join(OUT, "b39_star_result.json"), "w") as f:
    json.dump(rep, f, indent=1)
print(json.dumps(rep, indent=1))
