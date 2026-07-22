#!/usr/bin/env python3
# Quick probe: does `select planet <new-path body>` reach getSelected(), and
# does the camera dump's selDist reflect it? (de-risk before the full run)
import socket, time, json, sys, os, math
OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b21p"
os.makedirs(OUT, exist_ok=True)
def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout: pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)
def hdr(tag):
    p=f"{OUT}/{tag}.json"; send(s,f"body action dual_dump filename {p}",1.2)
    for ln in open(p):
        ln=ln.strip().replace('-nan','null').replace('nan','null')
        try:o=json.loads(ln)
        except:continue
        if o.get("type")=="header": return o["camera"]
    return {}
s=socket.create_connection(("127.0.0.1",7805),timeout=10)
send(s,"flag experimental_path on",1); send(s,"date jday 2461233.5",1); send(s,"timerate rate 0",1)
send(s,'body action load name FarA radius 6000 parent Sun type Planet coord_func still_orbit '
       'tex_map bodies/moon.png orbit_x 0 orbit_y 12000 orbit_z 0',2)
send(s,"select planet Earth",1); c=hdr("selEarth")
print("after select Earth: selected=%r selDist=%s ref=%s"%(c.get("selected"),c.get("selDist"),c.get("reference")),flush=True)
send(s,"select planet FarA",1); c=hdr("selFarA")
print("after select FarA:  selected=%r selDist=%s ref=%s"%(c.get("selected"),c.get("selDist"),c.get("reference")),flush=True)
s.close(); print("probe done",flush=True)
