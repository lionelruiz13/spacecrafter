#!/usr/bin/env python3
# S3 depth-partitioning verification scene (INTENT 11.30).
# The case per-body clearing gets WRONG and bucket merging fixes:
# 'blockin' is sorted NEARER than 'bigball' (center 800 km closer to the
# observer) but its geometry is SUBMERGED below bigball's near surface
# (800 km from center + ~700 km bounding < 2000 km radius) - correct
# rendering hides it entirely; painter's per-body clear draws it on top of
# the disc. 'blockout' sits at the same depth but 1 deg along-track
# (~6700 km lateral: outside the disc, inside the depth range) - same merged
# bucket, must stay VISIBLE (merge must not over-hide). Old path merges
# buckets too -> old is the parity reference.
#
# FRAME-INVARIANT DESIGN (2nd attempt - the 1st chased the real Moon and
# missed by 5.4 deg of ecliptic latitude): the ell_orbit frame bakes the
# parent rotation (ModularBody.hpp frame contract), so absolute placement
# from meanlongitude is unreliable; instead all three bodies share IDENTICAL
# elements except sma (radial = along-LOS from an Earth observer, <=1 deg
# LOS tilt from the surface offset -> <=14 km lateral leakage) and
# meanlongitude (lateral); tracking centers bigball wherever the frame puts
# it. ml 55 puts the trio ~74 deg elongation from the Sun (lon ~341.7 at
# this jd) -> ~64% lit disc (the real Moon direction is near-new = dark).
# Launch precondition: fresh instance, init_fov=10 (both paths read it; the
# zoom-fov command reaches only the old path - INTENT 11.15c).
import socket, time

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

ORBIT = ("rot_periode 1000 rot_rotation_offset 0 rot_obliquity 0 rot_equator_ascending_node 0 "
         "orbit_epoch 2461103.30 orbit_period 27.3 orbit_eccentricity 0.0 "
         "orbit_inclination 0.76 orbit_ascendingnode 256.70 orbit_longofpericenter 0.0 ")

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "date jday 2461103.30", 1)
send(s, "timerate rate 0", 1)
send(s, "flag atmosphere off", 1)
send(s, "flag landscape off", 1)
send(s, "flag experimental_path on", 2)
send(s, "body action load name bigball parent Earth type Planet tex_map bodies/moon.jpg "
        "radius 2000 oblateness 0.0 halo false lighting true albedo 0.3 coord_func ell_orbit "
        + ORBIT + "orbit_semimajoraxis 383867. orbit_meanlongitude 55.0", 5)
send(s, "select planet bigball pointer off", 1)
send(s, "flag track_object on", 15)  # smoothing settle (INTENT 11.19c)
# Parity frame: single-body bucket - pre/post-S3 builds must match bit-exact.
send(s, "body action screenshot filename /tmp/s3_prestation_new.png", 3)
send(s, "body action load name blockin parent Earth type Artificial model_name ISS2021 "
        "radius 400 oblateness 0.0 halo false coord_func ell_orbit albedo 0.9 "
        + ORBIT + "orbit_semimajoraxis 383067. orbit_meanlongitude 55.0", 4)
send(s, "body action load name blockout parent Earth type Artificial model_name ISS2021 "
        "radius 400 oblateness 0.0 halo false coord_func ell_orbit albedo 0.9 "
        + ORBIT + "orbit_semimajoraxis 383067. orbit_meanlongitude 56.0", 8)
# Numeric state check: both stations' freshness/positions in both paths.
send(s, "body action dual_dump filename /tmp/s3_overlap_dump.json", 2)
# Overlap frame, new path: blockin HIDDEN, blockout VISIBLE.
send(s, "body action screenshot filename /tmp/s3_overlap_new.png", 3)
# Old-path reference: same expectation (old merges buckets).
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/s3_overlap_old.png", 3)
# Path-flag re-entry: second entry must bit-restore the first (rare-path contract).
send(s, "flag experimental_path on", 3)
send(s, "body action screenshot filename /tmp/s3_overlap_new2.png", 3)
s.close()
print("s3 overlap scene done", flush=True)
