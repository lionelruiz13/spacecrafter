#!/usr/bin/env python3
# B21 FAR-CASE only (fast iteration): create off-centre far targets, select
# while NEAR (persists), fly to a SYSTEM reference, descend -> selDist *= coef.
import socket, time, json, sys, os, math
OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b21f"
os.makedirs(OUT, exist_ok=True)
AU_M = 149597870700.0
def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout: pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)
def cam(tag, pause=1.2):
    p=f"{OUT}/{tag}.json"; send(s,f"body action dual_dump filename {p}",pause)
    for ln in open(p):
        ln=ln.strip().replace('-nan','null').replace('nan','null')
        try:o=json.loads(ln)
        except:continue
        if o.get("type")=="header": return o["camera"]
    return {}
def bnew_iter(tag):
    p=f"{OUT}/{tag}.json"; d={}
    for ln in open(p):
        ln=ln.strip().replace('-nan','null').replace('nan','null')
        try:o=json.loads(ln)
        except:continue
        if o.get("type")=="body": d[o["name"]]=o
    return d
s=socket.create_connection(("127.0.0.1",7805),timeout=10)
send(s,"flag experimental_path on",1); send(s,"date jday 2461233.5",1); send(s,"timerate rate 0",1)
C=("parent Sun type Planet coord_func still_orbit tex_map bodies/moon.png halo false")
send(s,f'body action load name FarA radius 6000 {C} orbit_x 0 orbit_y 12000 orbit_z 0',2)
send(s,f'body action load name FarB radius 6000 {C} orbit_x 0 orbit_y 0 orbit_z 20000',2)
# baseline AoI
send(s,"set home_planet Earth",2); send(s,"moveto lat 48.85 lon 2.35 alt 100 duration 0",2)
send(s,"body action dual_dump filename %s/base.json"%OUT,1.5); bb=bnew_iter("base")
def bn(n): return bb.get(n,{}).get("new")
terms=[]
for nm,o in bb.items():
    nw=o.get("new")
    if nw and nw.get("parent")=="Sun":
        e=nw["ecl"]; terms.append(math.sqrt(sum(x*x for x in e))+1.1*nw["boundingRadius"])
sb=bn("Sun")["boundingRadius"]; sun_sub=1.1*max([sb]+terms); sun_aoi=max(sb*128,sun_sub*16)
FAR=sun_aoi*1.07                # small margin above sun_aoi so a 4% step stays SolarSystem
print(f"# sun_aoi={sun_aoi:.1f} fly_to={FAR:.1f} AU",flush=True)
CF="0.96"                       # small step -> reference stays SolarSystem (no de-escalation)
def fly(sel):
    send(s,"camera action free_mode state off",1); send(s,"set home_planet Earth",2)
    send(s,"moveto lat 0 lon 0 alt 100 duration 0",1.2)
    send(s,f"select planet {sel}",1.5)
    send(s,"camera action free_mode state on",1)
    send(s,f"moveto altitude {int(FAR*AU_M)} duration 0",2.5)
    for _ in range(6): send(s,"body action screenshot d",0.4)
res={}
for sel,t in [("FarA","a"),("FarB","b")]:
    fly(sel); c0=cam(f"far0{t}"); c1=None
    send(s,f"camera action descend coef {CF}",1.5); c1=cam(f"far1{t}")
    res[t]=(c0,c1)
fly("FarA"); z0=cam("far0z"); send(s,"camera action descend",1.5); z1=cam("far1z")
s.close()
print("\n==== FAR ====")
for t in ["a","b"]:
    c0,c1=res[t]
    r=c1["selDist"]/c0["selDist"] if c0["selDist"] else float('nan')
    print(f"sel={c0.get('selected'):5s} ref0={c0['reference']:12s} selDist0={c0['selDist']:.6e} "
          f"ref1={c1['reference']:12s} selDist1={c1['selDist']:.6e} ratio={r:.6f}",flush=True)
zr=z1["selDist"]/z0["selDist"] if z0["selDist"] else float('nan')
print(f"BOGUS sel={z0.get('selected'):5s} selDist0={z0['selDist']:.4e} ratio={zr:.6f} (expect 1.0)",flush=True)
ca,_=res["a"]; cb,_=res["b"]
print(f"selDist follows selection: FarA={ca['selDist']:.4e} != FarB={cb['selDist']:.4e}",flush=True)
print("far done",flush=True)
