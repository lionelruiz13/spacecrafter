#!/usr/bin/env python3
"""F112 -- THE POSITIVE MAP OF THE INSTANCE PROBE, both ways, with a live decoy.

F26 (Sec.11.134(b)) set the standard a concurrent-instance probe has to meet:
"positively mapped both ways before use: 1 with a decoy binary named
`spacecrafter` running, 0 without".  F109 (Sec.11.231(j2)) then measured that
same probe reading 0 with two RENAMED engines live -- so the 2026-08-02 map was
taken with a decoy that could not distinguish the criterion from its blind spot.

This driver takes the map again, with a decoy that CAN:
  * the decoy is a byte copy of the delivered binary under a name whose comm is
    NOT "spacecrafter" (sc_f112_decoy, 14 bytes, so truncation is not the reason);
  * it runs on a FARM home, so the real ~/.spacecrafter is never opened;
  * every reading is taken by FOUR instruments at the same moment: the OLD comm
    form (inline, exactly as the 44 files spell it), the new probe's python
    authority, the new probe's shell front end, and the old form's shell spelling;
  * it is killed BY PID from its own recorded pid -- never pkill -f, which
    matches its own caller's command line (Sec.11.231(j)(1)).

  python3 f112_decoymap.py <outdir> [--hold SECONDS]

exit 0 = the map came out as the criterion says it must; non-zero = it did not.
"""

import glob
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import sc_instances                                          # noqa: E402
from f96_offset import build_farm                            # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
PRISTINE = {"config.ini": "03fbee59bc3ec506c58f0a3f1e1d73df",
            "ssystem.ini": "545a51ef76294891579a1fc2fe13792b"}
DECOY_DIR = Path("/home/claude/sc-f112")
DECOY = DECOY_DIR / "sc_f112_decoy"
PORT = 7805


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def old_comm_form_python():
    """The pre-F112 probe, verbatim from f96_offset.py:192-200 as it stood at
    harness 5bf9f85.  Copied here ON PURPOSE: this is the thing being mapped."""
    hits = []
    for p in Path("/proc").glob("[0-9]*/comm"):
        try:
            if p.read_text().strip() == "spacecrafter":
                hits.append(str(p.parent))
        except OSError:
            pass
    return hits


def old_comm_form_shell():
    """The pre-F112 probe, verbatim from f90_rehearsal_run.sh:51-55."""
    script = (
        'HITS=""; for p in /proc/[0-9]*/comm; do [ -r "$p" ] || continue; '
        'if [ "$(cat "$p" 2>/dev/null)" = "spacecrafter" ]; then '
        'HITS="$HITS ${p%/comm}"; fi; done; echo "$HITS"')
    r = subprocess.run(["bash", "-c", script], capture_output=True, text=True)
    return [h for h in r.stdout.split() if h]


def new_probe_python():
    return sc_instances.scan()


def new_probe_shell():
    r = subprocess.run(["bash", str(HERE / "sc_instances.sh"), "--json",
                        "/dev/stdout", "--quiet"],
                       capture_output=True, text=True)
    try:
        return json.loads(r.stdout), r.returncode
    except ValueError:
        return [], r.returncode


def reading(label):
    """All four instruments at one moment."""
    oc_py = old_comm_form_python()
    oc_sh = old_comm_form_shell()
    np_py = new_probe_python()
    np_sh, sh_rc = new_probe_shell()
    chans = {}
    for h in np_py:
        for c in h["channels"]:
            chans[c] = chans.get(c, 0) + 1
    row = {
        "label": label,
        "when": time.strftime("%F %T %Z"),
        "old_comm_python": len(oc_py),
        "old_comm_shell": len(oc_sh),
        "new_probe_python": len(np_py),
        "new_probe_shell": len(np_sh),
        "new_probe_shell_rc": sh_rc,
        "channels": chans,
        "pids": sorted(h["pid"] for h in np_py if h["pid"]),
        "lines": [sc_instances.fmt(h) for h in np_py],
    }
    print("\n--- reading: %s  (%s)" % (label, row["when"]))
    print("    OLD comm form (python) : %d" % row["old_comm_python"])
    print("    OLD comm form (shell)  : %d" % row["old_comm_shell"])
    print("    NEW probe (python)     : %d   channels %s"
          % (row["new_probe_python"], chans or "{}"))
    print("    NEW probe (shell)      : %d   exit %d"
          % (row["new_probe_shell"], sh_rc))
    for ln in row["lines"]:
        print(ln)
    return row


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 4
    out = Path(argv[1])
    out.mkdir(parents=True, exist_ok=True)
    hold = 12.0
    if "--hold" in argv:
        hold = float(argv[argv.index("--hold") + 1])

    binary = os.environ.get(
        "SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    print("=== F112 decoy map  %s" % time.strftime("%F %T %Z"))
    print("    source binary : %s  md5 %s" % (binary, md5(binary)))

    md5_in = {k: md5(REAL_HOME / k) for k in PRISTINE}
    for k, v in PRISTINE.items():
        if md5_in[k] != v:
            print("STOP: field %s is %s, not the pristine %s" % (k, md5_in[k], v))
            return 4
    print("    field md5 in  : %s" % md5_in)

    rows = [reading("before -- nothing launched")]
    if rows[0]["new_probe_python"] or rows[0]["old_comm_python"]:
        print("STOP: the host is not clear before the decoy; aborting.")
        return 4

    DECOY_DIR.mkdir(parents=True, exist_ok=True)
    shutil.copy2(binary, DECOY)
    print("\n    decoy         : %s  md5 %s  (a byte copy; comm will be %r)"
          % (DECOY, md5(DECOY), DECOY.name[:15]))
    farm = build_farm(DECOY_DIR / "farm")
    print("    farm home     : %s" % farm)

    env = {**os.environ, "HOME": str(farm.parent),
           "DISPLAY": os.environ.get("DISPLAY", ":2")}
    applog = open(out / "decoy.applog", "w")
    proc = subprocess.Popen([str(DECOY)], cwd=str(farm), stdout=applog,
                            stderr=subprocess.STDOUT, env=env)
    print("    launched pid  : %d" % proc.pid)
    t0 = time.time()
    port_up = False
    while time.time() - t0 < 180:
        if proc.poll() is not None:
            print("STOP: the decoy exited before opening %d (rc %s)"
                  % (PORT, proc.returncode))
            rows.append(reading("after the decoy died"))
            json.dump(rows, open(out / "map.json", "w"), indent=1)
            return 4
        r = subprocess.run(["bash", "-c",
                            "ss -ltn 2>/dev/null | grep -c ':%d '" % PORT],
                           capture_output=True, text=True)
        if r.stdout.strip() not in ("0", ""):
            port_up = True
            break
        time.sleep(1)
    print("    port %d up    : %s after %.1f s" % (PORT, port_up, time.time() - t0))

    rows.append(reading("DECOY LIVE"))
    gpu = subprocess.run(
        ["nvidia-smi", "--query-compute-apps=pid,process_name,used_memory",
         "--format=csv,noheader"], capture_output=True, text=True).stdout
    gpu_all = subprocess.run(["nvidia-smi", "-q", "-d", "PIDS"],
                             capture_output=True, text=True).stdout
    (out / "nvidia_compute_apps_with_decoy.txt").write_text(gpu)
    (out / "nvidia_pids_with_decoy.txt").write_text(gpu_all)
    # the decoy's OWN footprint, which is what the headroom gate is a proxy for
    own = None
    blk = gpu_all.split("Process ID")
    for b in blk[1:]:
        if b.strip().split()[1] == str(proc.pid):
            for line in b.splitlines():
                if "Used GPU Memory" in line:
                    own = line.split(":")[1].strip()
    print("    decoy's own GPU footprint (nvidia-smi -q -d PIDS): %s" % own)

    print("\n    holding for %.0f s ..." % hold)
    time.sleep(hold)
    rows.append(reading("DECOY LIVE, after the hold"))

    print("\n    killing BY PID %d (never pkill -f)" % proc.pid)
    proc.terminate()
    try:
        proc.wait(timeout=30)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=30)
    print("    decoy exit    : %s" % proc.returncode)
    applog.close()
    time.sleep(3)
    rows.append(reading("after -- killed by pid"))

    md5_out = {k: md5(REAL_HOME / k) for k in PRISTINE}
    ok_md5 = md5_in == md5_out
    print("\n    field md5 out : %s   in == out %s" % (md5_out, ok_md5))

    res = {"binary": binary, "binary_md5": md5(binary),
           "decoy": str(DECOY), "decoy_md5": md5(DECOY),
           "decoy_pid": proc.pid, "decoy_comm_would_be": DECOY.name[:15],
           "port_came_up": port_up, "decoy_gpu_mib": own,
           "field_md5_in": md5_in, "field_md5_out": md5_out,
           "field_md5_ok": ok_md5, "readings": rows}
    json.dump(res, open(out / "map.json", "w"), indent=1, sort_keys=True)

    # --------------------------------------------------------------- verdict
    before, live, live2, after = rows[0], rows[1], rows[2], rows[3]
    checks = [
        ("C1 old comm form reads 0 with the decoy LIVE (the blind spot)",
         live["old_comm_python"] == 0 and live["old_comm_shell"] == 0),
        ("C2 new probe reads >= 1 with the decoy LIVE",
         live["new_probe_python"] >= 1),
        ("C3 the new probe's hit is the decoy's own pid",
         proc.pid in live["pids"]),
        ("C4 channel exe fired", live["channels"].get("exe", 0) >= 1),
        ("C5 channel port fired (the server came up)",
         live["channels"].get("port", 0) >= 1 if port_up else True),
        ("C6 channel comm did NOT fire (the decoy is renamed)",
         live["channels"].get("comm", 0) == 0),
        ("C7 python and shell agree on the count, every reading",
         all(r["new_probe_python"] == r["new_probe_shell"] for r in rows)),
        ("C8 0 before and 0 after, all four instruments",
         all(before[k] == 0 and after[k] == 0 for k in
             ("old_comm_python", "old_comm_shell", "new_probe_python",
              "new_probe_shell"))),
        ("C9 the reading is stable across the hold",
         live["new_probe_python"] == live2["new_probe_python"]),
        ("C10 the field's config.ini and ssystem.ini are md5 in == out", ok_md5),
    ]
    print("\n=== VERDICT")
    nfail = 0
    for name, ok in checks:
        print("  %-4s %s" % ("PASS" if ok else "FAIL", name))
        nfail += 0 if ok else 1
    print("  %d check(s), %d FAIL" % (len(checks), nfail))
    return 0 if nfail == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
