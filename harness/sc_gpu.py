#!/usr/bin/env python3
"""sc_gpu -- THE LAUNCH-HEADROOM MEASUREMENT, and the holders by name.

F112, INTENT Sec.11.238.  The companion of sc_instances.py: that one answers "is
something else already running?", this one answers "is there room to run at all?".
Together they are the two preconditions every measuring launch in this corpus has.

WHY IT EXISTS.  2026-09-12, 11:26 (HOST-EVENTS 2026-09-12, Sec.11.232(d)): a 27.3B
model resident in the owner's ollama under OLLAMA_KEEP_ALIVE=-1 held 29 552 MiB of
a 32 607 MiB card.  The application started, read `available : 1591 MiB`, made
three dedicated allocations, and died at `Failed to allocate chunk of 256 MiB in
GPU memory` before any window existed.  The environment canary's --no-scene arm
PASSED, with a NOTE, while no launch on this host could start: its VRAM member was
a note against a guessed 8192 MiB, "NOT a gate" by its own text, and only the
photometric arm caught the fault -- by failing to launch.

THE NUMBER IS NOT HERE.  It is banked in f56_canary.sh's VALUES block as
BANK_GPU_NEED_MIB, with its derivation, because that is the instrument whose
re-banking discipline the corpus already trusts (one VALUES-block edit, with an
argument, never a tolerance widened to fit).  `--need bank` reads it from there, so
there is exactly ONE copy of the number in the corpus and every caller asks the
same file for it.

WHAT IS MEASURED, AND THE DEFECT IT AVOIDS.
  free    nvidia-smi --query-gpu=memory.free -- what a new process can still get.
          NOT memory.used: `used <= 4000` on a 32 GiB card says nothing about
          whether 6 GiB remain, and it refuses a host with 27 GiB free (measured
          2026-09-12 15:49, the owner's java at 2943 MiB).
  holders nvidia-smi -q -d PIDS, which lists GRAPHICS (G) processes as well as
          compute (C/C+G), untruncated.
          MEASURED 2026-09-12 15:35: `--query-compute-apps=pid,process_name,
          used_memory` -- the holder-naming call in f116_assert.sh and f114_run.sh
          -- returned nautilus 102 + gnome-text-editor 68 + gnome-system-monitor 70
          = 240 MiB of the 3293 then in use, and did NOT name java at 1385 MiB, nor
          gnome-shell, nor firefox.  A refusal that "names the holder" through that
          call names the wrong three.  The 2026-09-12 red is not a counter-example:
          llama-server happened to be a COMPUTE process.
          Each pid's uid and exe are resolved from /proc, so the report says WHOSE
          it is -- which is the difference between a fact the owner can act on and
          a number.  Nothing is ever unloaded here (Sec.11.174(h)).

usage:
  sc_gpu.py --need bank|<MiB> [--label L] [--json F] [--quiet]
  sc_gpu.py --holders                    name every holder, exit 0
  sc_gpu.py --self-test [--mutant NAME]
  exit 0 enough headroom | 3 not enough | 4 the measurement could not be made
"""

import json
import os
import pwd
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CANARY = HERE / "f56_canary.sh"
BANK_RE = re.compile(r"^BANK_GPU_NEED_MIB=(\d+)", re.M)

MUTANT = None


def bank_need():
    """The banked need, read from the canary's VALUES block -- its only home."""
    try:
        m = BANK_RE.search(CANARY.read_text())
    except OSError as exc:
        raise RuntimeError("cannot read the bank at %s: %s" % (CANARY, exc))
    if not m:
        raise RuntimeError(
            "BANK_GPU_NEED_MIB is not in %s's VALUES block.  The number lives "
            "there and nowhere else; this is a STOP, not a default." % CANARY)
    return int(m.group(1))


def nvidia(args):
    try:
        r = subprocess.run(["nvidia-smi"] + args, capture_output=True,
                           text=True, timeout=60)
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise RuntimeError("nvidia-smi: %s" % exc)
    if r.returncode != 0:
        raise RuntimeError("nvidia-smi %s exited %d: %s"
                           % (" ".join(args), r.returncode, r.stderr.strip()))
    return r.stdout


def memory():
    """(free, used, total) in MiB."""
    out = nvidia(["--query-gpu=memory.free,memory.used,memory.total",
                  "--format=csv,noheader,nounits"]).strip().splitlines()[0]
    f, u, t = [int(x.strip()) for x in out.split(",")]
    return f, u, t


def parse_pids(text):
    """Every process on the card, from `nvidia-smi -q -d PIDS`."""
    out = []
    for block in text.split("Process ID")[1:]:
        try:
            pid = int(block.strip().split()[1])
        except (IndexError, ValueError):
            continue
        ty = re.search(r"^\s*Type\s*:\s*(\S+)", block, re.M)
        nm = re.search(r"^\s*Name\s*:\s*(.*)$", block, re.M)
        mm = re.search(r"^\s*Used GPU Memory\s*:\s*(\d+)", block, re.M)
        out.append({"pid": pid,
                    "type": ty.group(1) if ty else "?",
                    "name": (nm.group(1).strip() if nm else "?"),
                    "mib": int(mm.group(1)) if mm else 0})
    return out


def parse_compute_apps(text):
    """The NARROW call, kept so the difference can be measured rather than
    asserted: --query-compute-apps=pid,process_name,used_memory --format=csv."""
    out = []
    for line in text.strip().splitlines():
        if not line.strip() or line.lower().startswith("pid"):
            continue
        f = [x.strip() for x in line.split(",")]
        if len(f) < 3:
            continue
        try:
            out.append({"pid": int(f[0]), "type": "C", "name": f[1],
                        "mib": int(f[2].split()[0])})
        except ValueError:
            continue
    return out


def resolve(rows):
    for h in rows:
        try:
            uid = os.stat("/proc/%d" % h["pid"]).st_uid
            h["uid"] = uid
            h["user"] = pwd.getpwuid(uid).pw_name
        except (OSError, KeyError):
            h["uid"], h["user"] = None, "?"
        try:
            h["exe"] = os.readlink("/proc/%d/exe" % h["pid"])
        except OSError:
            h["exe"] = None          # another account, under ptrace_scope=1
    return sorted(rows, key=lambda h: -h["mib"])


def select_holders(wide_text, narrow_text):
    """WHICH nvidia-smi call names the holders.  A seam, so --self-test can test
    the choice on fixtures instead of on the live card -- the first version of
    this file put the choice inside holders(), where its own mutant could not
    see it and scored 10 PASS 0 FAIL."""
    if MUTANT == "compute_only":
        return parse_compute_apps(narrow_text)
    return parse_pids(wide_text)


def holders():
    wide = nvidia(["-q", "-d", "PIDS"])
    narrow = ""
    if MUTANT == "compute_only":
        narrow = nvidia(["--query-compute-apps=pid,process_name,used_memory",
                         "--format=csv,noheader"])
    return resolve(select_holders(wide, narrow))


def fmt_holder(h):
    return ("    %-8d %-5s %7d MiB  uid %-5s %-10s %s"
            % (h["pid"], h["type"], h["mib"],
               "?" if h["uid"] is None else h["uid"], h["user"],
               (h["exe"] or h["name"])[:78]))


def check(need, label="", quiet=False, jsonfile=None):
    free, used, total = memory()
    if MUTANT == "used_not_free":
        ok = used <= 4000                       # the inherited F114/F116 line
        basis = "used %d MiB <= 4000" % used
    elif MUTANT == "no_gate":
        ok = True
        basis = "no gate"
    else:
        ok = free >= need
        basis = "free %d MiB against a need of %d MiB" % (free, need)
    rows = holders()
    res = {"label": label, "free_mib": free, "used_mib": used,
           "total_mib": total, "need_mib": need, "pass": bool(ok),
           "basis": basis, "holders": rows}
    if jsonfile:
        with open(jsonfile, "w") as fh:
            json.dump(res, fh, indent=1, sort_keys=True)
    if not quiet:
        tag = label or "sc_gpu"
        print("[%s] gpu: free %d MiB / used %d / total %d ; need %d MiB "
              "(Sec.11.238)" % (tag, free, used, total, need))
        if not ok:
            print("[%s] NOT ENOUGH GPU MEMORY TO LAUNCH -- %s" % (tag, basis))
            print("[%s] the holders, NOT unloaded (Sec.11.174(h) -- an "
                  "environment fault is reported, never mitigated here):" % tag)
            for h in rows:
                print(fmt_holder(h))
            print("[%s] the application allocates 512 + 160 + 1280 MiB dedicated "
                  "and then a 256 MiB chunk before any window exists; on "
                  "2026-09-12 it died at that chunk with 1591 MiB available "
                  "(HOST-EVENTS 2026-09-12)." % tag)
    return 0 if ok else 3, res


# ------------------------------------------------------------- the self-test
FIXTURE_PIDS = """
GPU 00000000:01:00.0
    Processes
        GPU instance ID                   : N/A
        Process ID                        : 5455
            Type                          : G
            Name                          : /usr/bin/gnome-shell
            Used GPU Memory               : 521 MiB
        Process ID                        : 121858
            Type                          : G
            Name                          : /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java
            Used GPU Memory               : 2943 MiB
        Process ID                        : 12520
            Type                          : C+G
            Name                          : /usr/bin/nautilus
            Used GPU Memory               : 102 MiB
        Process ID                        : 13091
            Type                          : C
            Name                          : /usr/local/lib/ollama/llama-server
            Used GPU Memory               : 29552 MiB
"""

FIXTURE_COMPUTE = """pid, process_name, used_gpu_memory [MiB]
12520, /usr/bin/nautilus, 102 MiB
13091, /usr/local/lib/ollama/llama-server, 29552 MiB
"""


def self_test(mutant=None):
    global MUTANT
    MUTANT = mutant
    P = F = 0
    lines = []

    def case(cid, ok, why):
        nonlocal P, F
        if ok:
            P += 1
            lines.append("  PASS %-4s %s" % (cid, why))
        else:
            F += 1
            lines.append("  FAIL %-4s %s" % (cid, why))

    wide = parse_pids(FIXTURE_PIDS)
    narrow = parse_compute_apps(FIXTURE_COMPUTE)
    wide_sum = sum(h["mib"] for h in wide)
    narrow_sum = sum(h["mib"] for h in narrow)

    case("g1", len(wide) == 4 and wide_sum == 33118,
         "the wide call parses 4 processes summing 33118 MiB (G and C+G and C)")
    case("g2", len(narrow) == 2 and narrow_sum == 29654,
         "the narrow call sees 2 of the same 4 -- java and gnome-shell are "
         "invisible to --query-compute-apps")
    case("g3", any(h["pid"] == 121858 for h in wide)
         and not any(h["pid"] == 121858 for h in narrow),
         "THE DEFECT, as a case: the 2943 MiB holder is named by the wide call "
         "and missed by the narrow one")
    case("g4", parse_pids("")[:] == [],
         "an empty nvidia-smi output parses to no holders rather than throwing")

    # the SELECTION, on the same two fixtures -- the case the first version of
    # this suite did not have, which is why --mutant compute_only was invisible
    chosen = select_holders(FIXTURE_PIDS, FIXTURE_COMPUTE)
    case("g11", any(h["pid"] == 121858 for h in chosen)
         and sum(h["mib"] for h in chosen) == 33118,
         "the gate SELECTS the wide call: the 2943 MiB holder is in the table "
         "the refusal prints, and the table sums to the whole card")

    # the decision function, on numbers rather than on the live card
    def verdict(free, used, need):
        if MUTANT == "used_not_free":
            return used <= 4000
        if MUTANT == "no_gate":
            return True
        return free >= need

    case("g5", verdict(27589, 4518, 6144) is True,
         "2026-09-12 15:49 (owner's java at 2943): 27589 free -> PASS at 6144")
    case("g6", verdict(2130, 30477, 6144) is False,
         "2026-09-12 11:26 (the red: the app died at init): 2130 free -> FAIL")
    case("g7", verdict(6144, 26463, 6144) is True,
         "exactly at the need is enough (>=, not >)")
    case("g8", verdict(6143, 26464, 6144) is False,
         "one MiB below the need is not")
    case("g9", verdict(29314, 3293, 6144) is True,
         "2026-09-12 15:35, the state this task opened on -> PASS")

    try:
        n = bank_need()
        case("g10", n == 6144,
             "the bank is read from f56_canary.sh's VALUES block and is %d" % n)
    except RuntimeError as exc:
        case("g10", False, "the bank could not be read: %s" % exc)

    MUTANT = None
    print("sc_gpu.py --self-test%s" % ("  --mutant " + mutant if mutant else ""))
    print("\n".join(lines))
    print("  %d PASS %d FAIL" % (P, F))
    return 0 if F == 0 else 1


MUTANTS = {
    "compute_only": "name holders with --query-compute-apps (the F114/F116 "
                    "call).  MUST fail g11 -- the refusal's table then omits "
                    "the 2943 MiB holder",
    "used_not_free": "gate on `used <= 4000`, the inherited line.  MUST fail "
                     "g5 (it refuses a host with 27 GiB free) and g7",
    "no_gate": "always pass (the pre-F112 NOTE).  MUST fail g6 and g8 -- it "
               "passes the state in which no launch can start",
}


def main(argv):
    args = list(argv[1:])
    global MUTANT
    if "--self-test" in args:
        args.remove("--self-test")
        mutant = None
        if "--mutant" in args:
            i = args.index("--mutant")
            mutant = args[i + 1] if i + 1 < len(args) else None
            if mutant not in MUTANTS:
                sys.stderr.write("unknown mutant %r; one of: %s\n"
                                 % (mutant, ", ".join(sorted(MUTANTS))))
                return 4
        return self_test(mutant)
    if "--list-mutants" in args:
        for k in sorted(MUTANTS):
            print("%-15s %s" % (k, MUTANTS[k]))
        return 0
    if "--mutant" in args:                       # only meaningful with a check
        i = args.index("--mutant")
        MUTANT = args[i + 1]
        del args[i:i + 2]
        if MUTANT not in MUTANTS:
            sys.stderr.write("unknown mutant %r\n" % MUTANT)
            return 4
    if "--holders" in args:
        for h in holders():
            print(fmt_holder(h))
        return 0
    quiet = "--quiet" in args
    if quiet:
        args.remove("--quiet")
    jsonfile = None
    if "--json" in args:
        i = args.index("--json")
        jsonfile = args[i + 1]
        del args[i:i + 2]
    label = ""
    if "--label" in args:
        i = args.index("--label")
        label = args[i + 1]
        del args[i:i + 2]
    if "--need" not in args:
        sys.stderr.write("sc_gpu.py: --need bank|<MiB> is required; there is no "
                         "default, because a guessed threshold is the thing this "
                         "replaces.\n")
        return 4
    i = args.index("--need")
    want = args[i + 1] if i + 1 < len(args) else ""
    del args[i:i + 2]
    if args:
        sys.stderr.write("sc_gpu.py: unknown argument(s): %s\n" % " ".join(args))
        return 4
    need = bank_need() if want == "bank" else int(want)
    rc, _ = check(need, label, quiet, jsonfile)
    return rc


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except Exception as exc:
        sys.stderr.write("sc_gpu.py: HARNESS ERROR: %s\n" % exc)
        sys.exit(4)
