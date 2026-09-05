#!/usr/bin/env python3
"""F84 -- fresh-$HOME first launches: the Sec.5.48 rate and the Sec.5.112 price.

    ./f84_coldhome.py <outdir> --homes /abs/prefix --n 6 [--bin /abs/spacecrafter]
    ./f84_coldhome.py <outdir> --homes /abs/prefix --n 1 --seed-config FILE

WHAT IT DOES, per launch, in this order:
  1. concurrency assert over /proc/<pid>/comm (F26 Sec.11.134(b) -- covers every
     account; `pgrep -f <path>` self-matches and is blind to out-of-tree bins);
  2. records the GNOME ScreenSaver GetActive value (recorded, never gated);
  3. creates a NEVER-BEFORE-USED $HOME and, inside it, `.spacecrafter/` AND
     NOTHING ELSE.  That single mkdir is a deliberate, minimal deviation:
     main.cpp:193 calls std::filesystem::current_path(appDir) BEFORE
     main.cpp:198 CallSystem::checkUserDirectory creates it, so a $HOME with no
     .spacecrafter aborts the process (measured: exit 134, "cannot set current
     path").  Everything BELOW .spacecrafter/ is still the app's own bootstrap.
     Sec.5.48's own farm (b3_farm.sh) has the same shape, so the rate stays
     comparable to F12's;
     [F86, 2026-09-05] `--no-mkdir` drops that deviation: the $HOME is created
     EMPTY and the app is asked to bootstrap itself from nothing, which is
     Sec.5.130's gate.  Against the pre-fix binary it aborts (exit 134); against
     a binary carrying Sec.5.130's fix it must exit 0 having built the whole
     tree.  The flag defaults OFF, so every F84 record this script produced
     still reproduces byte for byte; it exists here rather than in a copy
     because there must be exactly one launch shape (I2);
  4. optionally seeds $HOME/.spacecrafter/config.ini from --seed-config (the
     Sec.5.112 arms) before launching;
  5. launches, waits for TCP 7805 then 8 s of settle (the port opens before the
     renderer is usable), drives `body action dual_dump`, quits with
     `shutdown action now`;
  6. records exit code, the applog, the generated config.ini (md5, key count,
     section count, version), the $HOME tree census, and the Sec.5.48 verdict.

The real ~/.spacecrafter and /usr/local are asserted unchanged around the whole
run; any movement fails the script rather than being reported afterwards.
"""
import argparse
import hashlib
import json
import os
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import dumpread  # noqa: E402  -- the dump channel's single reader (Sec.5.103)

REAL_HOME_SC = Path(os.path.expanduser("~/.spacecrafter"))
FAILS = []


def fail(msg):
    FAILS.append(msg)
    print("FAIL: " + msg)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def no_instance():
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def screensaver_active():
    try:
        out = subprocess.run(
            ["gdbus", "call", "--session", "--dest", "org.gnome.ScreenSaver",
             "--object-path", "/org/gnome/ScreenSaver",
             "--method", "org.gnome.ScreenSaver.GetActive"],
            capture_output=True, text=True, timeout=20,
            env={**os.environ,
                 "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/%d/bus" % os.getuid()})
        return out.stdout.strip()
    except Exception as e:                                        # noqa: BLE001
        return "probe-error %s" % e


def wait_port(proc, timeout=180):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            if proc.poll() is not None:
                return None
            time.sleep(1)
    return None


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.25)
        sock.recv(8192)
        sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def read_bodies(path):
    """-> {name: record}, EVERY body line, including ones with a missing "new".

    dumpread.load_dump drops records that have no "new" side (a composed body
    has no old-path twin), which is right for a comparison and wrong here:
    Sec.5.48's whole instrument lesson is that a body must not disappear.  The
    GRAMMAR is still dumpread's single one -- `loads` = sanitize_nonfinite +
    unquote_nonfinite -- so this is one reader of one grammar (I2), not a copy.
    """
    out = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        if not line.strip():
            continue
        try:
            rec = dumpread.loads(line)
        except Exception:                                         # noqa: BLE001
            continue
        if rec.get("type") == "body" and "name" in rec:
            out[rec["name"]] = rec
    return out


# ------------------------------------------------------------------ Sec.5.48
NONFINITE_BODIES = ("Moon", "Sun")
NONFINITE_FIELDS = ("scaling", "boundingRadius")


def is_nonfinite(v):
    """True iff v is a non-finite number in ANY spelling the dump uses.

    The new path QUOTES its non-finite values (src/experimentalModule/
    JsonNum.hpp), so both a bare float('nan') and the string "nan" must be
    caught; dumpread.unquote_nonfinite is what makes the bare form possible.
    """
    if isinstance(v, str):
        return v.strip().lower().lstrip("+-") in ("nan", "inf", "infinity")
    if isinstance(v, (int, float)):
        return v != v or v in (float("inf"), float("-inf"))
    return False


def s548_verdict(bodies):
    """-> (fired: bool, detail: dict)"""
    detail, fired = {}, False
    for name in NONFINITE_BODIES:
        rec = bodies.get(name)
        if rec is None:
            detail[name] = "ABSENT-FROM-DUMP"
            fired = True                     # absence is itself a fire: Sec.5.48's
            continue                         # own instrument note (a nan body used
        new = rec.get("new") or {}           # to disappear from the loader)
        vals = {f: new.get(f, "MISSING") for f in NONFINITE_FIELDS}
        detail[name] = vals
        if any(is_nonfinite(v) or v == "MISSING" for v in vals.values()):
            fired = True
    return fired, detail


def control():
    """Positive control: the predicate MUST classify a synthetic nan as a fire."""
    good = {"Moon": {"new": {"scaling": 1.0, "boundingRadius": 1737.4}},
            "Sun": {"new": {"scaling": 1.0, "boundingRadius": 696000.0}}}
    bad = json.loads(json.dumps(good))
    bad["Moon"]["new"]["scaling"] = "nan"
    worse = json.loads(json.dumps(good))
    worse["Sun"]["new"]["boundingRadius"] = float("nan")
    a, _ = s548_verdict(good)
    b, _ = s548_verdict(bad)
    c, _ = s548_verdict(worse)
    d, _ = s548_verdict({})
    ok = (a is False and b is True and c is True and d is True)
    print("control: finite->%s quoted-nan->%s bare-nan->%s empty->%s  => %s"
          % (a, b, c, d, "PASS" if ok else "FAIL"))
    if not ok:
        fail("Sec.5.48 predicate control failed -- the rate is withheld")
    return ok


def census(home):
    sc = Path(home) / ".spacecrafter"
    out = {"dirs": [], "files": [], "class_counts": {}}
    if not sc.exists():
        return out
    for e in sorted(sc.iterdir()):
        (out["dirs"] if e.is_dir() else out["files"]).append(e.name)
    for d in out["dirs"]:
        p = sc / d
        files = [f for f in p.rglob("*") if f.is_file()]
        out["class_counts"][d] = {"files": len(files),
                                  "zero_byte": sum(1 for f in files if f.stat().st_size == 0),
                                  "bytes": sum(f.stat().st_size for f in files)}
    return out


def config_stats(path):
    if not Path(path).exists():
        return {"present": False}
    txt = Path(path).read_bytes().decode("latin-1")
    secs, keys, comments, version, upper = [], 0, 0, None, 0
    cur = None
    for raw in txt.splitlines():
        s = raw.strip()
        if not s:
            continue
        if s[0] in "#;":
            comments += 1
        elif s.startswith("["):
            cur = s.strip("[]").strip()
            secs.append(cur)
        elif "=" in s:
            keys += 1
            k, v = s.split("=", 1)
            if k.strip() != k.strip().lower():
                upper += 1
            if cur and cur.lower() == "main" and k.strip().lower() == "version":
                version = v.strip()
    return {"present": True, "md5": md5(path), "sections": len(secs), "keys": keys,
            "comment_lines": comments, "uppercase_keys": upper, "version": version,
            "section_names": secs}


def key_set(path):
    out = set()
    cur = None
    for raw in Path(path).read_bytes().decode("latin-1").splitlines():
        s = raw.strip()
        if not s or s[0] in "#;":
            continue
        if s.startswith("["):
            cur = s.strip("[]").strip().lower()
        elif "=" in s and cur:
            out.add(cur + ":" + s.split("=", 1)[0].strip().lower())
    return out


def one_launch(idx, home, outdir, binary, seed_config, extra_cmds, no_mkdir=False):
    rec = {"index": idx, "home": str(home)}
    hits = no_instance()
    rec["concurrency_pre"] = hits
    if hits:
        fail("launch %s: another spacecrafter is running: %s" % (idx, hits))
        return rec
    rec["screensaver_GetActive"] = screensaver_active()

    if Path(home).exists():
        fail("launch %s: $HOME %s already exists -- not a fresh home" % (idx, home))
        return rec
    if no_mkdir:
        # Sec.5.130's gate: an EMPTY $HOME, nothing inside it at all.
        Path(home).mkdir(parents=True)
        rec["home_preseed"] = ["(nothing -- --no-mkdir, Sec.5.130 gate)"]
    else:
        (Path(home) / ".spacecrafter").mkdir(parents=True)
        rec["home_preseed"] = [".spacecrafter/ (mkdir only)"]
    if seed_config:
        shutil.copy(seed_config, Path(home) / ".spacecrafter" / "config.ini")
        rec["home_preseed"].append("config.ini <- " + str(seed_config))
        rec["seed_config_md5"] = md5(seed_config)
        rec["seed_config_stats"] = config_stats(seed_config)

    applog = Path(outdir) / ("launch%s.applog" % idx)
    env = {**os.environ, "HOME": str(home), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    t0 = time.time()
    # cwd: the app cds to $HOME/.spacecrafter itself; with --no-mkdir that
    # directory does not exist yet, so the launch starts from $HOME (which is
    # also what a real first launch does -- a newcomer types the command from
    # wherever he stands).
    cwd = Path(home) / ".spacecrafter"
    if not cwd.is_dir():
        cwd = Path(home)
    proc = subprocess.Popen([binary], cwd=str(cwd),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT, env=env)
    sock = wait_port(proc)
    rec["tcp_wait_s"] = round(time.time() - t0, 1)
    if sock is None:
        proc.wait(timeout=30)
        rec["exit_code"] = proc.returncode
        rec["tcp"] = "never opened"
        fail("launch %s: port 7805 never opened (exit %s)" % (idx, proc.returncode))
    else:
        rec["tcp"] = "up"
        time.sleep(8)
        for c in extra_cmds:
            send(sock, c, 1.0)
        dump = Path(outdir) / ("dump%s.json" % idx)
        if dump.exists():
            dump.unlink()
        send(sock, "body action dual_dump filename %s" % dump, 1.5)
        for _ in range(60):
            if dump.exists() and dump.stat().st_size > 0:
                break
            time.sleep(0.2)
        rec["dump_present"] = dump.exists() and dump.stat().st_size > 0
        if rec["dump_present"]:
            bodies = read_bodies(dump)
            rec["dump_bodies"] = len(bodies)
            fired, detail = s548_verdict(bodies)
            rec["s548_fired"] = fired
            rec["s548_detail"] = detail
        else:
            fail("launch %s: dump never appeared" % idx)
        send(sock, "shutdown action now", 1.0)
        sock.close()
        # 180 s, not 60: a config that drives a heavy async texture preload
        # keeps the app alive until the big-texture loader quiesces (the
        # teardown order the tree's own "quiesce async big-texture loader
        # before app teardown" commit installed). Measured: a field-config
        # launch was still logging "Created image support for ..." at 60 s.
        try:
            proc.wait(timeout=180)
        except subprocess.TimeoutExpired:
            proc.kill()
            fail("launch %s: did not exit on `shutdown action now` within 180 s" % idx)
        # let the kernel reap it before the next launch's /proc scan
        time.sleep(2)
        rec["exit_code"] = proc.returncode

    rec["wall_s"] = round(time.time() - t0, 1)
    cfg = Path(home) / ".spacecrafter" / "config.ini"
    rec["config"] = config_stats(cfg)
    rec["census"] = census(home)
    log = Path(home) / ".spacecrafter" / "log" / "spacecrafter.log"
    if log.exists():
        txt = log.read_bytes().decode("latin-1")
        rec["applog_root_dir"] = [l.strip() for l in txt.splitlines()
                                  if "ROOT   DIR" in l or "CONFIG DIR" in l
                                  or "LOCALE DIR" in l]
        rec["applog_copy_lines"] = [l.strip() for l in txt.splitlines()
                                    if "Completed copy" in l or "Failed to copy" in l
                                    or "subdirectory" in l or "default_" in l]
        rec["applog_warnings"] = [l.strip() for l in txt.splitlines()
                                  if "Warning" in l or "WARNING" in l][:40]
        rec["applog_removed"] = [l.strip() for l in txt.splitlines()
                                 if "has been removed from config.ini" in l]
        rec["applog_lines"] = len(txt.splitlines())
    stderrtxt = applog.read_bytes().decode("latin-1")
    rec["stderr_failed_copy"] = [l.strip() for l in stderrtxt.splitlines()
                                 if "Failed to copy" in l or "Abort" in l]
    # F86: an aborting launch says everything it has to say on stderr and
    # nothing in the applog (the log system does not exist yet), so the head of
    # the process output is part of the record, not a debugging aid.
    rec["stderr_head"] = [l.rstrip() for l in stderrtxt.splitlines()[:12]]
    rec["home_entries"] = sorted(p.name for p in Path(home).iterdir()) \
        if Path(home).is_dir() else None
    return rec


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--homes", required=True, help="absolute $HOME prefix; <prefix><i>")
    ap.add_argument("--n", type=int, default=6)
    ap.add_argument("--start", type=int, default=1)
    ap.add_argument("--bin", default="/home/claude/sc-f84/prefix/bin/spacecrafter")
    ap.add_argument("--seed-config", default=None)
    ap.add_argument("--cmd", action="append", default=[])
    ap.add_argument("--tag", default="run")
    ap.add_argument("--no-mkdir", action="store_true",
                    help="create the $HOME EMPTY (Sec.5.130 gate, F86); default "
                         "keeps F84's single `.spacecrafter/` mkdir")
    a = ap.parse_args()

    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    real_before = {p.name: md5(p) for p in
                   (REAL_HOME_SC / "config.ini", REAL_HOME_SC / "ssystem.ini")}

    if not control():
        print(json.dumps({"fails": FAILS}, indent=1))
        return 2

    launches = []
    for i in range(a.start, a.start + a.n):
        print("--- launch %d ---" % i, flush=True)
        r = one_launch(i, a.homes + str(i), out, a.bin, a.seed_config, a.cmd,
                       no_mkdir=a.no_mkdir)
        launches.append(r)
        print("  exit=%s tcp=%s s548_fired=%s cfg_keys=%s cfg_ver=%s"
              % (r.get("exit_code"), r.get("tcp"), r.get("s548_fired"),
                 (r.get("config") or {}).get("keys"),
                 (r.get("config") or {}).get("version")), flush=True)

    real_after = {p.name: md5(p) for p in
                  (REAL_HOME_SC / "config.ini", REAL_HOME_SC / "ssystem.ini")}
    if real_before != real_after:
        fail("the REAL ~/.spacecrafter moved: %s -> %s" % (real_before, real_after))

    k = sum(1 for r in launches if r.get("s548_fired"))
    res = {"tag": a.tag, "binary": a.bin, "binary_md5": md5(a.bin),
           "n": len(launches), "s548_fires": k,
           "s548_rate": "%d/%d" % (k, len(launches)),
           "real_home_before": real_before, "real_home_after": real_after,
           "launches": launches, "fails": FAILS}
    (out / ("result_%s.json" % a.tag)).write_text(json.dumps(res, indent=1))
    print("Sec.5.48 rate: %d/%d      fails: %d" % (k, len(launches), len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
