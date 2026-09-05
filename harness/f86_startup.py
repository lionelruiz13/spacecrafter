#!/usr/bin/env python3
"""F86 -- one launch on an EXISTING $HOME farm, for the three startup faults.

    ./f86_startup.py <absOutdir> --bin /abs/spacecrafter --farm /abs/farmdir \\
        --tag NAME [--cmd 'camera action switch name orbit_autour_lune'] \\
        [--asan] [--ssystem-insert FILE] [--expect-crash]

WHY IT IS NOT f84_coldhome.py: that script's subject is a $HOME that has never
existed (Sec.5.48 / Sec.5.112 / Sec.5.130).  This one's subject is a $HOME that is
already populated -- Sec.5.127's two creator faults are reached by LOADING the
shipped anchor.ini and ssystem.ini, so the farm (b3_farm.sh, symlinks + real
config/ssystem copies) is the right shape and a fresh HOME is the wrong one.
Both scripts share the same three asserts, deliberately re-implemented from the
same three sources rather than imported: /proc/<pid>/comm concurrency (F26,
Sec.11.134(b)), the real ~/.spacecrafter md5 in==out, and DISPLAY per HOST-EVENTS.

WHAT IT RECORDS, per launch: exit code (and the SIGNAL if it died of one), the
stderr/stdout head, the applog's first N lines with their md5 (the as-if control
of Sec.5.130's move compares exactly this), every ASan report file, and, when
--cmd is given, the command's own reply.  Nothing here judges: the verdicts live
in the caller's prediction file.

--ssystem-insert inserts a scratch section into the FARM's ssystem.ini (a real
copy, never the field file) immediately before its final `[end]` marker; the
file's md5 before and after the insert is recorded, and the field ssystem.ini
is asserted unmoved around the whole run.
"""
import argparse
import hashlib
import json
import os
import socket
import subprocess
import sys
import time
from pathlib import Path

REAL_HOME_SC = Path(os.path.expanduser("~/.spacecrafter"))
HERE = Path(__file__).resolve().parent
FAILS = []


def fail(msg):
    FAILS.append(msg)
    print("FAIL: " + msg)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def running_instances():
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def wait_port(proc, timeout):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            if proc.poll() is not None:
                return None
            time.sleep(1)
    return None


def send(sock, cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    out = b""
    try:
        sock.settimeout(0.4)
        out = sock.recv(8192)
    except socket.timeout:
        pass
    finally:
        sock.settimeout(None)
    return out.decode("latin-1").replace("\x00", "|")


def insert_section(ssystem, section_file, rec):
    txt = Path(ssystem).read_bytes().decode("latin-1")
    add = Path(section_file).read_bytes().decode("latin-1")
    rec["ssystem_md5_before"] = md5(ssystem)
    marker = "[end]"
    i = txt.rfind(marker)
    if i < 0:
        fail("farm ssystem.ini has no [end] marker -- refusing to guess")
        return
    txt = txt[:i] + add.rstrip("\n") + "\n\n" + txt[i:]
    Path(ssystem).write_bytes(txt.encode("latin-1"))
    rec["ssystem_md5_after"] = md5(ssystem)
    rec["ssystem_inserted"] = section_file


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", required=True)
    ap.add_argument("--farm", required=True)
    ap.add_argument("--tag", default="run")
    ap.add_argument("--cmd", action="append", default=[])
    ap.add_argument("--asan", action="store_true")
    ap.add_argument("--asan-opts", default=None,
                    help="replace the default ASAN_OPTIONS body (log_path is "
                         "always appended). Used to raise the quarantine so a "
                         "freed chunk is not recycled before it is read: with "
                         "the default quarantine ASan reports the dangling read "
                         "as heap-buffer-overflow into whatever took the memory, "
                         "and never names the free.")
    ap.add_argument("--ssystem-insert", default=None)
    ap.add_argument("--expect-crash", action="store_true",
                    help="documentary only: recorded in the result, never gates")
    ap.add_argument("--head-lines", type=int, default=40)
    ap.add_argument("--port-wait", type=int, default=300)
    ap.add_argument("--exit-wait", type=int, default=240)
    ap.add_argument("--settle", type=float, default=8.0)
    ap.add_argument("--keep-farm", action="store_true",
                    help="do not rebuild the farm (b3_farm.sh wipes it)")
    a = ap.parse_args()

    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    rec = {"tag": a.tag, "bin": a.bin, "bin_md5": md5(a.bin), "farm": a.farm,
           "asan": a.asan, "cmds": a.cmd, "expect_crash": a.expect_crash}

    hits = running_instances()
    rec["concurrency_pre"] = hits
    if hits:
        fail("another spacecrafter is running: %s" % hits)
        (out / ("result_%s.json" % a.tag)).write_text(json.dumps(rec, indent=1))
        return 2

    real_before = {p.name: md5(p) for p in
                   (REAL_HOME_SC / "config.ini", REAL_HOME_SC / "ssystem.ini")}
    rec["real_home_before"] = real_before

    if not a.keep_farm:
        r = subprocess.run(["bash", str(HERE / "b3_farm.sh"), a.farm],
                           capture_output=True, text=True)
        rec["farm_build"] = r.stdout.strip() + r.stderr.strip()
        if r.returncode != 0:
            fail("b3_farm.sh failed: %s" % rec["farm_build"])
            return 2
    sc = Path(a.farm) / ".spacecrafter"
    if a.ssystem_insert:
        insert_section(sc / "ssystem.ini", a.ssystem_insert, rec)

    logdir = sc / "log"
    for f in logdir.glob("*"):
        f.unlink()

    applog = out / ("%s.stderr" % a.tag)
    env = {**os.environ, "HOME": str(a.farm),
           "DISPLAY": os.environ.get("DISPLAY", ":2")}
    if a.asan:
        body = a.asan_opts or "detect_leaks=0:halt_on_error=1:malloc_context_size=30"
        env["ASAN_OPTIONS"] = "%s:log_path=%s" % (body, out / ("asan_%s" % a.tag))
        rec["asan_options"] = env["ASAN_OPTIONS"]

    t0 = time.time()
    proc = subprocess.Popen([a.bin], cwd=str(sc), stdout=open(applog, "w"),
                            stderr=subprocess.STDOUT, env=env)
    sock = wait_port(proc, a.port_wait)
    rec["tcp_wait_s"] = round(time.time() - t0, 1)
    if sock is None:
        try:
            proc.wait(timeout=60)
        except subprocess.TimeoutExpired:
            proc.kill()
        rec["tcp"] = "never opened"
        rec["exit_code"] = proc.returncode
        rec["died_of_signal"] = (-proc.returncode) if proc.returncode < 0 else None
    else:
        rec["tcp"] = "up"
        time.sleep(a.settle)
        rec["replies"] = []
        for c in a.cmd:
            rec["replies"].append({"cmd": c, "reply": send(sock, c, 2.0)})
        time.sleep(2.0)
        send(sock, "shutdown action now", 1.0)
        sock.close()
        try:
            proc.wait(timeout=a.exit_wait)
        except subprocess.TimeoutExpired:
            proc.kill()
            fail("%s: no exit on `shutdown action now` within %s s"
                 % (a.tag, a.exit_wait))
        rec["exit_code"] = proc.returncode
        rec["died_of_signal"] = (-proc.returncode) if proc.returncode < 0 else None
        time.sleep(2)
    rec["wall_s"] = round(time.time() - t0, 1)

    txt = applog.read_bytes().decode("latin-1")
    rec["stderr_lines"] = len(txt.splitlines())
    rec["stderr_head"] = [l.rstrip() for l in txt.splitlines()[:12]]
    rec["stderr_tail"] = [l.rstrip() for l in txt.splitlines()[-12:]]
    applg = logdir / "spacecrafter.log"
    if applg.exists():
        ltxt = applg.read_bytes().decode("latin-1")
        head = "\n".join(ltxt.splitlines()[:a.head_lines])
        (out / ("%s.applog_head" % a.tag)).write_text(head, encoding="latin-1")
        rec["applog_head_md5"] = hashlib.md5(head.encode("latin-1")).hexdigest()
        rec["applog_head_lines"] = len(head.splitlines())
        rec["applog_lines"] = len(ltxt.splitlines())
        rec["applog_errors"] = [l.rstrip() for l in ltxt.splitlines()
                                if "Error" in l or "error" in l
                                or "OrbitCreator" in l][:40]
        (out / ("%s.applog.gz" % a.tag)).write_bytes(
            __import__("gzip").compress(ltxt.encode("latin-1")))
    else:
        rec["applog_head_md5"] = None

    reports = sorted(out.glob("asan_%s.*" % a.tag))
    rec["asan_reports"] = [p.name for p in reports]
    rec["asan_first_lines"] = []
    for p in reports:
        rec["asan_first_lines"] += [l.rstrip() for l in
                                    p.read_text(errors="replace").splitlines()[:60]]

    real_after = {p.name: md5(p) for p in
                  (REAL_HOME_SC / "config.ini", REAL_HOME_SC / "ssystem.ini")}
    rec["real_home_after"] = real_after
    if real_before != real_after:
        fail("the REAL ~/.spacecrafter moved: %s -> %s" % (real_before, real_after))
    rec["fails"] = FAILS
    (out / ("result_%s.json" % a.tag)).write_text(json.dumps(rec, indent=1))
    print("%s: exit=%s signal=%s tcp=%s applog_head_md5=%s asan_reports=%d fails=%d"
          % (a.tag, rec.get("exit_code"), rec.get("died_of_signal"), rec.get("tcp"),
             rec.get("applog_head_md5"), len(rec["asan_reports"]), len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
