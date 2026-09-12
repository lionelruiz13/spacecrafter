#!/usr/bin/env python3
"""sc_instances -- THE ONE HOME OF "is a spacecrafter engine running?".

F112, INTENT Sec.11.238.  This file is the AUTHORITY for the identity criterion;
`sc_instances.sh` is a front end that delegates to it, and every LIVE harness
caller routes through one of the two.  There is exactly one implementation of the
criterion because two would be a pending silent desync (I2) -- and this corpus has
already paid for that: 44 files carried a copy-pasted `comm == "spacecrafter"`
test, all 44 blind to the same thing, and nobody could fix it in one place.

WHY THE OLD PROBE WAS BLIND (Sec.11.121(m) -> Sec.11.134(b) -> Sec.11.231(j2)).
  2026-08-02, F26: `pgrep -f <path>` was replaced because it is blind to an
  out-of-tree binary AND self-matches its own wrapper (measured: 3 reported with
  nothing running).  The replacement read /proc/<pid>/comm -- world-readable, so
  it covers every account, and carrying no command-line text, so it cannot match
  the caller.  Positively mapped 1 with a decoy / 0 without.
  2026-09-12, F109: measured 0 with TWO `sc_f109_iso` engines live and holding
  port 7805.  `comm` is the executable's basename truncated to 15 bytes, so a
  staging binary named `sc_f109_iso` or `spacecrafter-pre` NEVER equals
  "spacecrafter".  Every task that measured a staging binary ran that probe blind.

THE CRITERION (the union of three channels; a hit fires on ANY of them):

  comm   /proc/<pid>/comm == "spacecrafter"
         world-readable => covers EVERY ACCOUNT.  Blind to any rename.

  exe    readlink /proc/<pid>/exe, with a trailing " (deleted)" stripped, matching
         any of:
           E1  basename starts with "spacecrafter"   (spacecrafter, -pre, -post)
           E2  basename starts with "sc_" or "sc-"   (the staging convention:
               sc_f109_iso, sc_post_95087b68, sc_f112_decoy)
           E3  the path is under /home/claude/sc-*/  (the staging-tree convention)
           E4  the path holds a /build*/src/ component (build-claude/src/...)
           E5  the path equals realpath($SC_BIN)      (the binary under test)
         a real file, so no shell command line can ever match it -- the F109
         Sec.11.231(j)(1) self-match hazard cannot recur here.  Covers copies and
         renames.  BLIND ACROSS ACCOUNTS: under yama/ptrace_scope=1 (measured 1 on
         this host) readlink("/proc/<pid>/exe") on another uid's process returns
         EACCES -- measured 2026-09-12 on the owner's java, pid 121858, uid foxy:
         empty, rc 1, while /proc/121858/comm read "java" fine.

  port   the process holds a LISTEN socket on TCP 7805, the engine's server port.
         Read from /proc/net/tcp{,6} (state 0A), which also carries the socket's
         OWNER UID -- so a listener of ANOTHER account is still a hit, named by uid
         even when its pid cannot be attributed (fd scanning is same-uid only).
         An engine that answers is an engine whatever its file is called.

THE RESIDUAL -- what this criterion cannot see, stated so it can be argued with:

  R1  an engine of ANOTHER UID whose binary is renamed out of the comm match and
      which has not opened 7805 (a --no-scene/farm launch with no server, or one
      still inside its first seconds of init).  exe is unreadable across accounts,
      comm is name-exact: this is the true residual, and it is the one the F112
      task section did not name.
  R2  same account, binary renamed outside E1-E5, outside /home/claude/sc-*/ and
      outside any */build*/src/*, no server up.  Reachable only by deliberately
      breaking the naming convention this corpus has used since F26.
  R3  a process that rewrote its own comm via prctl() while also satisfying R2.
      The engine never calls prctl.
  R4  NOT a blind spot but a deliberate false positive: a LISTEN on 7805 held by a
      non-engine is reported.  The port is the resource a launch needs, so a
      stranger holding it is a stop whoever it is; the hit line names them.
  R5  the window between exec and listen: exe already fires there, port does not.

usage:
  sc_instances.py                       list hits (one line each), exit 0/2
  sc_instances.py --assert [LABEL]      one summary line, exit 0 clear / 2 hit
  sc_instances.py --quiet               no output, exit 0/2
  sc_instances.py --json FILE           also write the hits as json
  sc_instances.py --kill [--timeout S]  SIGTERM then SIGKILL every hit, BY PID
                                        (never pkill -f: Sec.11.231(j)(1))
  sc_instances.py --self-test [--mutant NAME]
  exit 0 clear | 2 at least one engine | 4 the probe could not run

python callers:
  from sc_instances import hits, no_instance, assert_clear, concurrent
"""

import glob
import json
import os
import re
import signal
import sys
import time

COMM_NAME = "spacecrafter"
PORT = 7805
STAGING_ROOT = "/home/claude"          # E3: <STAGING_ROOT>/sc-*/...
BUILD_RE = re.compile(r"/build[^/]*/src/")   # E4

#: set by --mutant, and ONLY by --self-test --mutant.  A mutant disables or
#: perverts one channel so the self-test can be shown able to fail.
MUTANT = None


# --------------------------------------------------------------- the sources
class ProcSource:
    """The live /proc.  Every read is best-effort: a pid can exit under us."""

    def __init__(self, root="/proc", sc_bin=None):
        self.root = root
        self.sc_bin = sc_bin if sc_bin is not None else os.environ.get("SC_BIN")
        self._sc_real = None
        if self.sc_bin:
            try:
                self._sc_real = os.path.realpath(self.sc_bin)
            except OSError:
                self._sc_real = None

    def pids(self):
        out = []
        for name in os.listdir(self.root):
            if name.isdigit():
                out.append(int(name))
        return sorted(out)

    def comm(self, pid):
        try:
            with open("%s/%d/comm" % (self.root, pid)) as fh:
                return fh.read().strip()
        except OSError:
            return None

    def exe(self, pid):
        try:
            return os.readlink("%s/%d/exe" % (self.root, pid))
        except OSError:
            return None          # EACCES across accounts, ESRCH if it exited

    def cmdline(self, pid):
        try:
            with open("%s/%d/cmdline" % (self.root, pid), "rb") as fh:
                return fh.read().replace(b"\0", b" ").decode("utf-8", "replace")
        except OSError:
            return ""

    def uid(self, pid):
        try:
            return os.stat("%s/%d" % (self.root, pid)).st_uid
        except OSError:
            return None

    def listeners(self):
        """[(inode, uid)] of every LISTEN socket on PORT, any account."""
        found = []
        hexport = "%04X" % PORT
        for net in ("net/tcp", "net/tcp6"):
            try:
                with open("%s/%s" % (self.root, net)) as fh:
                    lines = fh.read().splitlines()[1:]
            except OSError:
                continue
            for line in lines:
                f = line.split()
                if len(f) < 10:
                    continue
                if f[3] != "0A":                       # TCP_LISTEN
                    continue
                if not f[1].endswith(":" + hexport):
                    continue
                try:
                    found.append((int(f[9]), int(f[7])))   # inode, uid
                except ValueError:
                    continue
        return found

    def socket_inodes(self, pid):
        """The socket inodes pid holds.  Same-uid only; EACCES elsewhere."""
        out = set()
        try:
            fds = os.listdir("%s/%d/fd" % (self.root, pid))
        except OSError:
            return out
        for fd in fds:
            try:
                tgt = os.readlink("%s/%d/fd/%s" % (self.root, pid, fd))
            except OSError:
                continue
            if tgt.startswith("socket:["):
                try:
                    out.add(int(tgt[8:-1]))
                except ValueError:
                    pass
        return out

    def sc_bin_real(self):
        return self._sc_real


class FakeSource:
    """A fixture source for --self-test.  Same interface, no /proc at all."""

    def __init__(self, procs, listeners=(), sc_bin_real=None):
        # procs: {pid: {"comm":.., "exe":.. or None, "cmdline":.., "uid":..,
        #               "sockets": set()}}
        self.procs = procs
        self._listeners = list(listeners)     # [(inode, uid)]
        self._sc_real = sc_bin_real

    def pids(self):
        return sorted(self.procs)

    def comm(self, pid):
        return self.procs[pid].get("comm")

    def exe(self, pid):
        return self.procs[pid].get("exe")

    def cmdline(self, pid):
        return self.procs[pid].get("cmdline", "")

    def uid(self, pid):
        return self.procs[pid].get("uid", 0)

    def listeners(self):
        return list(self._listeners)

    def socket_inodes(self, pid):
        return set(self.procs[pid].get("sockets", ()))

    def sc_bin_real(self):
        return self._sc_real


# ------------------------------------------------------------- the criterion
def strip_deleted(path):
    suffix = " (deleted)"
    return path[:-len(suffix)] if path.endswith(suffix) else path


def exe_matches(path, sc_bin_real=None):
    """E1-E5 on a resolved exe path.  Returns the rule that fired, or None."""
    if not path:
        return None
    path = strip_deleted(path)
    base = path.rsplit("/", 1)[-1]
    if MUTANT == "basename_eq":
        if base == "spacecrafter":
            return "E1"
    elif base.startswith("spacecrafter"):
        return "E1"
    if base.startswith("sc_") or base.startswith("sc-"):
        return "E2"
    if path.startswith(STAGING_ROOT + "/sc-"):
        return "E3"
    if BUILD_RE.search(path):
        return "E4"
    if sc_bin_real and path == sc_bin_real:
        return "E5"
    return None


def scan(source=None):
    """Every running spacecrafter engine, by the criterion above.

    Returns a list of dicts: pid, uid, comm, exe, exe_rule, ports, channels.
    A listening socket whose pid cannot be attributed (another account) is
    reported with pid None -- it is a hit, and saying so is the point."""
    if source is None:
        source = ProcSource()

    listen = source.listeners()
    listen_inodes = {ino: uid for ino, uid in listen}
    if MUTANT == "no_port":
        listen_inodes = {}

    hits = []
    claimed = set()
    for pid in source.pids():
        comm = source.comm(pid)
        if comm is None:
            continue
        channels, rule = [], None

        if MUTANT != "comm_only_off" and comm == COMM_NAME:
            channels.append("comm")

        if MUTANT == "comm_only":
            pass                                   # the pre-F112 probe, exactly
        elif MUTANT == "cmdline":
            # the F109 Sec.11.231(j)(1) hazard, reinstated on purpose: match the
            # COMMAND LINE instead of the exe.  A shell that carries the binary's
            # path as an argument then matches, and the probe reports its caller.
            cl = source.cmdline(pid)
            for token in cl.split():
                rule = exe_matches(token, source.sc_bin_real())
                if rule:
                    channels.append("exe")
                    break
        else:
            rule = exe_matches(source.exe(pid), source.sc_bin_real())
            if rule:
                channels.append("exe")

        ports = []
        if listen_inodes:
            mine = source.socket_inodes(pid)
            for ino in mine:
                if ino in listen_inodes:
                    ports.append(PORT)
                    claimed.add(ino)
            if ports:
                channels.append("port")

        if channels:
            hits.append({
                "pid": pid,
                "uid": source.uid(pid),
                "comm": comm,
                "exe": source.exe(pid),
                "exe_rule": rule,
                "ports": sorted(set(ports)),
                "channels": channels,
            })

    for ino, uid in listen_inodes.items():
        if ino not in claimed:
            hits.append({
                "pid": None,
                "uid": uid,
                "comm": None,
                "exe": None,
                "exe_rule": None,
                "ports": [PORT],
                "channels": ["port"],
                "note": "LISTEN socket inode %d, owner uid %d -- the holding pid "
                        "is not attributable from this account" % (ino, uid),
            })
    return hits


# ------------------------------------------------------- the callers' surface
def hits(source=None):
    return scan(source)


def fmt(hit):
    """pid . uid . comm . exe . port . channels -- one line per hit."""
    pid = str(hit["pid"]) if hit["pid"] is not None else "?"
    uid = hit["uid"]
    try:
        import pwd
        user = pwd.getpwuid(uid).pw_name if uid is not None else "?"
    except (KeyError, ImportError):
        user = str(uid)
    exe = hit["exe"] or ("<unreadable: another account>" if hit["pid"]
                         else "<no pid>")
    ports = ",".join(str(p) for p in hit["ports"]) or "-"
    rule = ("/" + hit["exe_rule"]) if hit["exe_rule"] else ""
    return "  pid %-7s uid %-4s %-10s comm=%-16s exe=%-52s port=%-5s [%s%s]" % (
        pid, "?" if uid is None else uid, user, hit["comm"] or "-", exe, ports,
        "+".join(hit["channels"]), rule)


def no_instance(source=None):
    """Drop-in for the 20 copies of this function across the harness.  Returns
    the hit lines (empty list = clear), so `if no_instance(): raise` keeps
    working AND the message now says which channel fired."""
    return [fmt(h).strip() for h in scan(source)]


def concurrent(source=None):
    """Drop-in for the `concurrent()` shape, which returned a count."""
    return len(scan(source))


def assert_clear(label="", source=None):
    """Raise if an engine is live.  The message is the evidence."""
    found = scan(source)
    if found:
        raise RuntimeError(
            "%sanother spacecrafter engine is live (sc_instances, "
            "Sec.11.238):\n%s" % (label + ": " if label else "",
                                  "\n".join(fmt(h) for h in found)))


def kill_hits(timeout=10.0, source=None):
    """SIGTERM then SIGKILL every hit, BY PID.  Never pkill -f: that pattern
    matches its own caller's command line (Sec.11.231(j)(1) -- it killed an
    executor's tool-call shell).  Returns (signalled, still_alive)."""
    found = [h for h in scan(source) if h["pid"]]
    for h in found:
        try:
            os.kill(h["pid"], signal.SIGTERM)
        except OSError:
            pass
    deadline = time.time() + timeout
    while time.time() < deadline:
        alive = [h for h in found if os.path.exists("/proc/%d" % h["pid"])]
        if not alive:
            return [h["pid"] for h in found], []
        time.sleep(0.25)
    alive = [h for h in found if os.path.exists("/proc/%d" % h["pid"])]
    for h in alive:
        try:
            os.kill(h["pid"], signal.SIGKILL)
        except OSError:
            pass
    time.sleep(0.5)
    still = [h["pid"] for h in alive if os.path.exists("/proc/%d" % h["pid"])]
    return [h["pid"] for h in found], still


# ------------------------------------------------------------- the self-test
def _fixture():
    """The cases, each one a fact this corpus paid for."""
    procs = {
        101: {"comm": "spacecrafter", "exe": "/usr/local/bin/spacecrafter",
              "uid": 1003},
        102: {"comm": "sc_f109_iso",
              "exe": "/home/claude/sc-f109/sc_f109_iso", "uid": 1003},
        103: {"comm": "spacecrafter-pr",          # comm truncates at 15 bytes
              "exe": "/home/claude/sc-f116/spacecrafter-pre", "uid": 1003},
        104: {"comm": "spacecrafter", "exe": None, "uid": 1001},
        105: {"comm": "java", "exe": None, "uid": 1001},
        106: {"comm": "bash", "exe": "/usr/bin/bash", "uid": 1003,
              "cmdline": "bash -c SC_BIN=/home/claude/sc-f116/sc_post_95087b68 "
                         "./b22_live_run.sh out"},
        107: {"comm": "spacecrafter",
              "exe": "/home/claude/spacecrafter/build-claude/src/spacecrafter",
              "uid": 1003},
        108: {"comm": "firefox",
              "exe": "/snap/firefox/8863/usr/lib/firefox/firefox", "uid": 1001},
        109: {"comm": "socat", "exe": "/usr/bin/socat", "uid": 1003,
              "sockets": {900001}},
        110: {"comm": "sc_f112_decoy",
              "exe": "/home/claude/sc-f112/sc_f112_decoy (deleted)",
              "uid": 1003},
        111: {"comm": "gnome-shell", "exe": "/usr/bin/gnome-shell", "uid": 1001},
        112: {"comm": "myengine",
              "exe": "/home/claude/sc-f110/tree/build/src/spacecrafter",
              "uid": 1003},
        113: {"comm": "renamed", "exe": "/opt/elsewhere/renamed", "uid": 1003},
        114: {"comm": "python3", "exe": "/usr/bin/python3", "uid": 1003,
              "cmdline": "python3 sc_instances.py --assert"},
        # E1 is the ONLY rule that can fire here: the basename starts with
        # "spacecrafter" but is not equal to it, the path is not under
        # /home/claude/sc-*/ and holds no /build*/src/ component.  Added after
        # the --mutant basename_eq run scored 17 PASS 0 FAIL against a
        # prediction that it would fail t3: t3's path is under sc-f116/, so E3
        # caught it and the mutant was invisible.  A mutant a suite cannot see
        # is a hole in the suite, not a harmless mutant.
        115: {"comm": "spacecrafter-pr",
              "exe": "/usr/local/bin/spacecrafter-pre", "uid": 1003},
    }
    return FakeSource(procs, listeners=[(900001, 1003), (900002, 1001)],
                      sc_bin_real="/home/claude/spacecrafter/build-claude/"
                                  "src/spacecrafter")


CASES = [
    # (id, pid-or-None, must-hit, channels expected, why this case exists)
    ("t1", 101, True, ["comm", "exe"],
     "an engine installed under its own name: both channels fire"),
    ("t2", 102, True, ["exe"],
     "THE BLIND SPOT: a staging binary; comm is sc_f109_iso, not spacecrafter"),
    ("t3", 103, True, ["exe"],
     "comm truncated at 15 bytes (spacecrafter-pr) is still not a match"),
    ("t4", 104, True, ["comm"],
     "ANOTHER ACCOUNT: exe unreadable under ptrace_scope=1, comm carries it"),
    ("t5", 105, False, [], "another account's java is not an engine"),
    ("t6", 106, False, [],
     "Sec.11.231(j)(1): a shell whose COMMAND LINE holds the binary path is NOT "
     "a hit -- the probe reads exe, a real file"),
    ("t7", 107, True, ["comm", "exe"], "the delivered build tree"),
    ("t8", 108, False, [], "firefox is not an engine"),
    ("t9", 109, True, ["port"],
     "a stranger holding LISTEN 7805 is a stop whoever it is (R4)"),
    ("t10", 110, True, ["exe"],
     "a DELETED binary still resolves: ' (deleted)' is stripped"),
    ("t11", 111, False, [], "gnome-shell is not an engine"),
    ("t12", 112, True, ["exe"],
     "a build tree under a staging root, binary named anything (E3/E4)"),
    ("t13", 113, False, [],
     "R2 stated as a test: renamed outside every rule, no server -> MISSED, "
     "and the miss is the documented residual rather than a surprise"),
    ("t14", 114, False, [],
     "the probe's own python process does not match itself"),
    ("t15", None, True, ["port"],
     "a LISTEN 7805 of ANOTHER uid, pid not attributable, still reported"),
    ("t18", 115, True, ["exe"],
     "E1 ALONE: spacecrafter-pre installed outside every staging and build path "
     "-- the only case that can see a basename rule tightened to =="),
]


def self_test(mutant=None):
    global MUTANT
    MUTANT = mutant
    src = _fixture()
    found = {h["pid"]: h for h in scan(src)}
    npass = nfail = 0
    lines = []
    for cid, pid, must, chans, why in CASES:
        if pid is None:
            got = [h for h in found.values() if h["pid"] is None]
            ok = bool(got) == must
            gotch = got[0]["channels"] if got else []
            if ok and must:
                ok = sorted(gotch) == sorted(chans)
        else:
            h = found.get(pid)
            ok = (h is not None) == must
            gotch = h["channels"] if h else []
            if ok and must:
                ok = sorted(gotch) == sorted(chans)
        if ok:
            npass += 1
            lines.append("  PASS %-4s %s" % (cid, why))
        else:
            nfail += 1
            lines.append("  FAIL %-4s expected %s %s, got %s -- %s"
                         % (cid, "HIT" if must else "MISS", chans, gotch, why))
    # two structural cases that are not about one pid
    total_expected = len([c for c in CASES if c[2]])
    ok = len(found) == total_expected
    if ok:
        npass += 1
        lines.append("  PASS t16  the hit COUNT is exactly the expected one "
                     "(%d) -- no silent extra" % total_expected)
    else:
        nfail += 1
        lines.append("  FAIL t16  hit count %d, expected %d"
                     % (len(found), total_expected))
    line = fmt(found[102]) if 102 in found else ""
    ok = ("pid" in line and "uid" in line and "exe=" in line
          and "port=" in line) if 102 in found else False
    if ok:
        npass += 1
        lines.append("  PASS t17  a hit line carries pid . uid . comm . exe . "
                     "port . channels")
    else:
        nfail += 1
        lines.append("  FAIL t17  the hit line is not the documented format: %r"
                     % line)
    MUTANT = None
    print("sc_instances.py --self-test%s"
          % ("  --mutant " + mutant if mutant else ""))
    print("\n".join(lines))
    print("  %d PASS %d FAIL" % (npass, nfail))
    return 0 if nfail == 0 else 1


MUTANTS = {
    "comm_only": "the pre-F112 probe: channels exe and port dropped. "
                 "MUST fail t2,t3,t9,t10,t12,t15,t16",
    "cmdline":   "match the COMMAND LINE instead of exe (the F109 hazard). "
                 "MUST fail t6,t16",
    "no_port":   "channel port dropped.  MUST fail t9,t15,t16",
    "basename_eq": "E1 tightened to an exact ==.  MUST fail t3,t16",
}


# --------------------------------------------------------------------- main
def main(argv):
    args = list(argv[1:])
    if "--self-test" in args:
        args.remove("--self-test")
        mutant = None
        if "--mutant" in args:
            i = args.index("--mutant")
            try:
                mutant = args[i + 1]
            except IndexError:
                sys.stderr.write("--mutant needs a name; one of: %s\n"
                                 % ", ".join(sorted(MUTANTS)))
                return 4
            if mutant not in MUTANTS:
                sys.stderr.write("unknown mutant %r; one of: %s\n"
                                 % (mutant, ", ".join(sorted(MUTANTS))))
                return 4
        return self_test(mutant)

    if "--list-mutants" in args:
        for k in sorted(MUTANTS):
            print("%-14s %s" % (k, MUTANTS[k]))
        return 0

    quiet = "--quiet" in args
    if quiet:
        args.remove("--quiet")
    jsonfile = None
    if "--json" in args:
        i = args.index("--json")
        jsonfile = args[i + 1]
        del args[i:i + 2]
    do_kill = "--kill" in args
    if do_kill:
        args.remove("--kill")
    timeout = 10.0
    if "--timeout" in args:
        i = args.index("--timeout")
        timeout = float(args[i + 1])
        del args[i:i + 2]
    label = ""
    if "--assert" in args:
        args.remove("--assert")
        label = args[0] if args else ""
        args = args[1:] if args else []
    elif args and not args[0].startswith("-"):
        label = args[0]
        args = args[1:]
    if args:
        sys.stderr.write("sc_instances.py: unknown argument(s): %s\n"
                         % " ".join(args))
        return 4

    if do_kill:
        signalled, still = kill_hits(timeout)
        if not quiet:
            print("[%s] sc_instances --kill: signalled %s%s"
                  % (label or "sc_instances",
                     signalled or "nothing",
                     ("; STILL ALIVE %s" % still) if still else ""))
        return 2 if still else 0

    found = scan()
    if jsonfile:
        with open(jsonfile, "w") as fh:
            json.dump(found, fh, indent=1, sort_keys=True)
    if not quiet:
        tag = label or "sc_instances"
        if found:
            print("[%s] SPACECRAFTER ENGINE LIVE: %d hit(s) "
                  "(criterion: comm | exe | port %d, Sec.11.238)"
                  % (tag, len(found), PORT))
            for h in found:
                print(fmt(h))
                if h.get("note"):
                    print("      %s" % h["note"])
        else:
            print("[%s] no spacecrafter engine: 0 hits by comm, exe and "
                  "port %d (Sec.11.238)" % (tag, PORT))
    return 2 if found else 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except Exception as exc:                       # a probe that cannot run
        sys.stderr.write("sc_instances.py: HARNESS ERROR: %s\n" % exc)
        sys.exit(4)                                # must STOP, never pass
