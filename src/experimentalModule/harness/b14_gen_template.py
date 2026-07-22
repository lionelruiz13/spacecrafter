#!/usr/bin/env python3
"""Generate the B14-land per-body edit template from the LIVE ssystem.ini.
No real pole values - placeholders only. Emits line numbers for each cluster
body so B14-land edits the exact spot."""
PATH = "/home/claude/.spacecrafter/ssystem.ini"
lines = open(PATH, encoding="latin-1").read().split("\n")
# find sections and the two garbage keys with line numbers
sec = None; sec_name = None; parent = None
records = []   # (section, name, parent, obliq_line, ascn_line, offset_line, offset_val)
cur = None
for i, ln in enumerate(lines, 1):
    s = ln.strip()
    if s.startswith("[") and s.endswith("]"):
        if cur and cur["is_cluster"]:
            records.append(cur)
        cur = {"sec": s, "name": None, "parent": None, "secline": i,
               "obliq": None, "ascn": None, "offset": None, "offval": None,
               "coordfunc": None, "is_cluster": False}
    elif cur is not None:
        if s.startswith("name ="): cur["name"] = s.split("=",1)[1].strip()
        elif s.startswith("parent ="): cur["parent"] = s.split("=",1)[1].strip()
        elif s == "rot_obliquity = 15.5": cur["obliq"] = i; cur["is_cluster"] = True
        elif s == "rot_equator_ascending_node = 213.7": cur["ascn"] = i
        elif s.startswith("rot_rotation_offset"):
            cur["offset"] = i; cur["offval"] = s.split("=",1)[1].strip()
        elif s.startswith("coord_func"): cur["coordfunc"] = s.split("=",1)[1].strip()
if cur and cur["is_cluster"]:
    records.append(cur)

print("# B14-land per-body edit template - LOADED ~/.spacecrafter/ssystem.ini")
print("# 34 copy-paste-cluster bodies (rot_obliquity = 15.5). VALUES SUSPENDED")
print("# FOR VIXY: every <FROM REPORT ...> is an IAU/WGCCRE placeholder - NO")
print("# real pole may be written from recall (INTENT 11.51(d) red line).")
print(f"# {len(records)} bodies. Per body: REMOVE the garbage parent-relative keys,")
print("# ADD the absolute pole + W0 + explicit frame. Loader ignores the removed")
print("# keys once absolute_pole resolves (verified: ModularSystem.cpp:710-748).")
print()
byparent = {}
for r in records:
    byparent.setdefault(r["parent"], []).append(r)
for par in ["Jupiter", "Saturn", "Uranus", "Neptune"]:
    rs = byparent.get(par, [])
    print(f"# ===== {par} system ({len(rs)} cluster moons) =====")
    for r in rs:
        cf = f"  (coord_func={r['coordfunc']})" if r["coordfunc"] else ""
        print(f"[{r['sec'][1:-1]}]  name={r['name']} parent={r['parent']}{cf}")
        print(f"    # section line {r['secline']}")
        print(f"    REMOVE line {r['obliq']}: rot_obliquity = 15.5")
        if r["ascn"]:
            print(f"    REMOVE line {r['ascn']}: rot_equator_ascending_node = 213.7")
        if r["offset"]:
            print(f"    REPLACE line {r['offset']}: rot_rotation_offset = {r['offval']}"
                  f"  ->  rot_rotation_offset = <FROM REPORT - {r['name']} W0 prime-meridian, deg>")
        else:
            print(f"    ADD: rot_rotation_offset = <FROM REPORT - {r['name']} W0 prime-meridian, deg>")
        print(f"    ADD: rot_pole_ra = <FROM REPORT - {r['name']} pole RA J2000, deg>")
        print(f"    ADD: rot_pole_de = <FROM REPORT - {r['name']} pole DE J2000, deg>")
        print(f"    ADD: rot_frame = absolute_pole")
        print()
