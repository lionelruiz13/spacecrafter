#!/usr/bin/env python3
"""B14-sat6 W0 meridian discriminator (INTENT §11.75(b)/§11.86 pattern).

Same math as b14_w0_analyze.py (imported verbatim - single source), with the
TEST set = the two B14-sat6 moons added to dumpHops: Janus (periodic pole+W,
prograde) and Prometheus (pure secular, prograde). Verifies the rendered prime
meridian == the IAU formula at J2000 (0.0000 deg) => rot_pole_w0 was converted,
not written raw. period_hours = the LOADED-file rot_periode (informational; the
J2000 criterion res[0] is period-independent).

pck00011.tpc BODY_PM constants (fetched md5 3c0bdc01; == pck00010; == Archinal
report text). Poles are the J2000-evaluated file values (eval_sat6 reproducible).

Usage: b14_sat6_w0.py <d1.json> <d2.json>
"""
import sys
import b14_w0_analyze as W

# name: (pole_ra_file, pole_de_file, W0_pck, Wdot_pck, period_hours_file[loaded])
# period_hours_file updated to the B14-periode-corrected rot_periode (=8640/Wdot;
# INTENT §11.88). Pre-periode garbage (Janus 0.1 placeholder, Prometheus
# -14.711769912 wrong-sign) recorded in §11.87(e); merid_d2 now collapses to ~0.
W.TEST = {
 "Janus":      (39.8195, 83.3601, 58.83,  518.2359876,  16.671941367894302),
 "Prometheus": (40.58,   83.53,   296.14, 587.2890000,  14.711666658152971),
}

if __name__ == "__main__":
    sys.exit(W.main(sys.argv[1], sys.argv[2]))
