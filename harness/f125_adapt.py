#!/usr/bin/env python3
"""F125 - let f124_run.sh run ANY driver, and record the host's load beside the run.

    bash f124_run.sh ../f125/<label> f125_adapt.py [--poke '<gdb command>']... [--gdb] <driver.py> [driver args...]

f124_run.sh calls its driver as `<outdir> <gdb_pid> <gdb_fifo> [args]`. A plain socket
driver wants `<outdir> [args]`: this adapter drops the two gdb arguments unless --gdb is
given. The label `../f125/<label>` lands the records under artifacts/f125/ with the
runner untouched. Before and after the driver it appends GPU memory and RAM to the
run's meta.txt (another account's process shares this host). Exit: the driver's.
--poke runs one gdb command in the live engine BEFORE the driver (a gdb-side perturbation
for an able-to-fail record, e.g. moving a gate's static); the run's meta.txt says so.
"""
import ast, os, signal, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
out, gdbpid, fifo, rest = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]
pokes = []
while len(rest) >= 2 and rest[0] == "--poke":
    pokes, rest = pokes + [rest[1]], rest[2:]
keep_gdb = bool(rest) and rest[0] == "--gdb"
if keep_gdb:
    rest = rest[1:]
if not rest:
    raise SystemExit(__doc__)
driver, dargs = rest[0], rest[1:]


def host_load(tag):
    lines = ["--- host load, %s ---" % tag]
    for cmd in (["nvidia-smi", "--query-gpu=memory.used,memory.total", "--format=csv,noheader"],
                ["free", "-g"]):
        try:
            r = subprocess.run(cmd, capture_output=True, text=True, timeout=20,
                               env=dict(os.environ, LC_ALL="C"))
            lines += ["$ " + " ".join(cmd)] + r.stdout.rstrip().splitlines()
        except Exception as e:
            lines.append("$ %s -> %s" % (" ".join(cmd), e))
    with open(os.path.join(out, "meta.txt"), "a") as f:
        f.write("\n".join(lines) + "\n")


host_load("before the driver")
with open(os.path.join(out, "meta.txt"), "a") as f:
    f.write("adapted driver   : %s %s (md5 %s)\n" % (
        driver, " ".join(dargs),
        subprocess.run(["md5sum", os.path.join(HERE, driver)], capture_output=True,
                       text=True).stdout.split(" ")[0]))
if pokes:
    # F124's reuse() from its file, then b21's gdb handshake through it: change them there.
    GDBPID, FIFO, GDBLOG = int(gdbpid), fifo, os.path.join(out, "gdb.log")
    _src = ast.parse(open(os.path.join(HERE, "f124_legs.py")).read())
    exec(compile(ast.Module([n for n in _src.body
                             if isinstance(n, ast.FunctionDef) and n.name == "reuse"], []),
                 "f124_legs.py", "exec"))
    reuse(os.path.join(HERE, "b21_keypath.py"), ["gdb_marker_count", "gdb_inject"])
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(pokes, "F125POKE")
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        said = [l for l in f.read().decode("latin-1").splitlines() if l.strip()]
    with open(os.path.join(out, "meta.txt"), "a") as f:
        f.write("PERTURBED RUN    : gdb poke %s (handshake %s); gdb said %s\n" % (pokes, ok, said[-4:]))
    if not ok:
        sys.exit("the gdb poke did not complete")
argv = [sys.executable, os.path.join(HERE, driver), out] + ([gdbpid, fifo] if keep_gdb else []) + dargs
rc = subprocess.call(argv)
host_load("after the driver")
sys.exit(rc)
