#!/usr/bin/env python3
# F149 key leg - the 14 cases A-N of INTENT 11.312 (h) on the b21 pattern, in one fresh launch.
#
#   f142_keys_run.sh <label> --phase before|after [--baseline <label of a before run>]
#
# A key event is injected where the event loop sends it: the engine stops at the entry of
# UI::handleInputs, UI::key_Modifier is set, UI::handleKeys(<scancode>, 0, 0, PRESSED|RELEASED) is
# called, the engine resumes. Bindings, shows and markers go over TCP 7805 with ABSOLUTE fixture
# paths under artifacts/f142/run (written here, one line each): the field's script directory is
# never written. The reading is ~/.spacecrafter/log/script.log between markers.
# `before` records the engine without the bind action (each bind line refused, the native
# defaults) and case N's reading; `after` judges P17-P30 and diffs case N against `--baseline`.
# Output: one `PASS|FAIL|NOTE <case> <criterion> observed= expected=` line per check.
import argparse, json, os, re, select, signal, socket, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--phase", required=True, choices=["before", "after"])
ap.add_argument("--baseline", default="")
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
AFTER = ARGS.phase == "after"
GDBLOG = OUT + "/gdb.log"
LOG = os.path.expanduser("~/.spacecrafter/log/script.log")
RUN = os.path.join(HERE, "artifacts", "f142", "run")
UI_HPP = os.path.join(HERE, "..", "..", "src", "uiModule", "ui.hpp")
FAILS, READ = [], {}
REFUSED = "command_script : unknown parameter from 'action' argument"

# The planes are his MODIFIER enum, read from the header, never typed here.
PLANE = {n: int(v, 16) for n, v in re.findall(r"^\s*(NONE|CTRL|KWIN|SUPER|SHIFT|ALT)\s*=\s*(0x[0-9A-Fa-f]+)",
                                              open(UI_HPP).read(), re.M)}
if len(PLANE) != 6:
    sys.exit("FAIL - planes_read observed=%s expected=6 MODIFIER values from ui.hpp" % PLANE)


def out(kind, case, crit, observed, expected="-"):
    print("%s %s %s observed=%s expected=%s" % (kind, case, crit, observed, expected), flush=True)


def check(case, crit, ok, observed, expected):
    out("PASS" if ok else "FAIL", case, crit, observed, expected)
    if not ok:
        FAILS.append(case + ":" + crit)


def note(case, crit, observed):
    out("NOTE", case, crit, observed)


def fixtures():
    os.makedirs(RUN, exist_ok=True)
    for f in os.listdir(RUN):
        os.remove(os.path.join(RUN, f))
    files = {n: "define f142_%s 1" % n for n in ("a", "b", "c", "d", "h", "rs", "rp")}
    files["g"] = "script action bind key KP_9 press %s/c.sts scope script" % RUN
    files["i"] = "script action bind release %s/d.sts scope once" % RUN
    files["j"] = "script action bind key KP_8 press %s/missing.sts" % RUN
    files["show"] = "\n".join(["define f142_show_1 1", "wait duration 5", "define f142_show_2 1", "wait duration 5",
                               "define f142_show_3 1", "wait duration 5", "define f142_show_4 1"])
    for n, text in files.items():
        with open("%s/%s.sts" % (RUN, n), "w") as f:
            f.write(text + "\n")


# ---- the script log ------------------------------------------------------------------------
def logsize():
    return os.path.getsize(LOG)


def logtext(a, b=None):
    with open(LOG, "rb") as f:
        f.seek(a)
        data = f.read() if b is None else f.read(b - a)
    return data.decode("latin-1")


def wait_log(since, pattern, timeout=10.0):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if re.search(pattern, logtext(since), re.M):
            return True
        time.sleep(0.1)
    return False


def entries(text):
    """(ms, type, message) per line of the script log."""
    return [(int(m.group(1)), m.group(2).strip(), m.group(3).rstrip("\r"))
            for m in re.finditer(r"^(\d{12}): \(([^)]*)\): (.*)$", text, re.M)]


def loads(text, name):
    return len(re.findall(r"ScriptMgr: load \S*/%s\.sts\s*$" % re.escape(name), text, re.M))


def runs(text, name):
    return len(re.findall(r"Execute_command define f142_%s 1\s*$" % re.escape(name), text, re.M))


def first_ms(text, pattern):
    for ms, _, msg in entries(text):
        if re.search(pattern, msg):
            return ms
    return None


# ---- the TCP channel -----------------------------------------------------------------------
S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
D = socket.create_connection(("127.0.0.1", 7805), timeout=15)   # the $DIAGON subscriber, as F137's probe
RX = []


def drain():
    for sock in (S, D):
        while select.select([sock], [], [], 0)[0]:
            data = sock.recv(65536)
            if not data:
                break
            RX.append(data.decode("latin-1"))


def tcp(cmd, timeout=10.0):
    since = logsize()
    S.sendall((cmd + "\n").encode())
    ok = wait_log(since, re.escape("Execute_command " + cmd), timeout)
    time.sleep(0.3)
    drain()
    if not ok:
        check("-", "tcp_line_executed", False, cmd, "Execute_command in the script log")
    return logtext(since)


MARKS = [0]


def mark(tag):
    """Start a segment: its offset, the marker line included."""
    MARKS[0] += 1
    since = logsize()
    tcp("define f142_mark_%s_%d 1" % (tag, MARKS[0]))
    return since


def bind(case, spec):
    """A bind line expected to be accepted (after) or refused as an unknown action (before)."""
    text = tcp("script action bind " + spec)
    if AFTER:
        bad = [m for _, t, m in entries(text) if t == "Error" or "Could not execute" in m]
        check(case, "bind_accepted", not bad, bad[:2] or "no error", "no error line")
    else:
        check(case, "bind_refused_unknown_action", REFUSED in text, REFUSED in text, REFUSED)
    return text


# ---- gdb: the key events -------------------------------------------------------------------
GDBN = [0]


def gdb_errors(since):
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        txt = f.read().decode("latin-1")
    return [l for l in txt.splitlines() if any(k in l for k in (
        "No symbol", "Cannot ", "Couldn't", "There is no member", "Invalid ", "not defined", "SIGSEGV",
        "SIGABRT", "could not be converted"))]


def inject(cmds, timeout=60):
    GDBN[0] += 1
    done = "F142GDB_%d_END" % GDBN[0]
    since = os.path.getsize(GDBLOG)
    os.kill(GDBPID, signal.SIGINT)
    time.sleep(0.3)
    with open(FIFO, "w") as f:
        for c in ["break UI::handleInputs", "continue", "delete breakpoints", "set $ui = this"] + cmds + \
                 ['printf "%s\\n"' % done, "continue"]:
            f.write(c + "\n")
    t0 = time.time()
    while time.time() - t0 < timeout:
        with open(GDBLOG, "rb") as f:
            if done.encode() in f.read():
                break
        time.sleep(0.2)
    else:
        check("-", "gdb_injection_%d_handshake" % GDBN[0], False, "no marker", done)
        return since
    bad = gdb_errors(since)
    if bad:
        check("-", "gdb_injection_%d_clean" % GDBN[0], False, bad[:3], "no gdb error")
    return since


VZM = "deltaAlt deltaAz deltaFov deltaLat deltaLon".split()


def keys(seq, vzm=False, settle=1.0):
    """seq: (scancode suffix, 'P'|'R', plane name or None = keep key_Modifier as it is)."""
    cmds = []
    for k, ev, plane in seq:
        if plane is not None:
            cmds.append("set var $ui->key_Modifier = %d" % PLANE[plane])
        cmds.append("call $ui->handleKeys(SDL_SCANCODE_%s, 0, 0, s_gui::S_GUI_%s)"
                    % (k, "PRESSED" if ev == "P" else "RELEASED"))
        if vzm:
            cmds.append('printf "F142VZM %s %s %s\\n", %s' % (k, ev, " ".join("%s=%%g" % v for v in VZM),
                        ", ".join("$ui->core._M_ptr->vzm.%s" % v for v in VZM)))
    since = inject(cmds)
    time.sleep(settle)
    if vzm:
        with open(GDBLOG, "rb") as f:
            f.seek(since)
            return re.findall(r"F142VZM .*$", f.read().decode("latin-1"), re.M)   # gdb prefixes "(gdb) "
    return []


def press(k, plane="NONE"):
    return [(k, "P", plane), (k, "R", None)]


def wait_end(since, timeout=40):
    return wait_log(since, r"ScriptMgr: script end", timeout)


def reset():
    tcp("script action bind key * press - release -")


# ---- the cases (INTENT 11.312 (h)) ---------------------------------------------------------
def case_A():
    reset(); m = mark("A")
    bind("A", "key KP_8 press %s/a.sts" % RUN)
    keys(press("KP_8"))
    t = logtext(m); READ["A"] = t
    if AFTER:
        check("A", "P17_bound_script_runs", loads(t, "a") == 1 and runs(t, "a") == 1,
              "load a %d, run a %d" % (loads(t, "a"), runs(t, "a")), "1, 1")
        check("A", "P17_native_M08_not_run", loads(t, "M08") == 0, loads(t, "M08"), 0)
    else:
        note("A", "native", "load M08 %d, load a %d" % (loads(t, "M08"), loads(t, "a")))


def case_B():
    reset(); m = mark("B")
    bind("B", "key SHIFT+KP_8 release %s/rs.sts" % RUN)
    bind("B", "key KP_8 release %s/rp.sts" % RUN)
    v = keys([("KP_8", "P", "SHIFT"), ("KP_8", "R", "NONE")], vzm=True)
    tcp("camera action move_lat value 0")
    t = logtext(m); READ["B"] = t
    note("B", "deltaLat_after_release", v[-1] if v else "none")
    if AFTER:
        check("B", "P18_press_identity_release", loads(t, "rs") == 1 and loads(t, "rp") == 0,
              "load rs %d, load rp %d" % (loads(t, "rs"), loads(t, "rp")), "1, 0")
    else:
        note("B", "native", "load rs %d, load rp %d" % (loads(t, "rs"), loads(t, "rp")))


def case_C():
    reset(); m = mark("C")
    bind("C", "key KP_8 press %s/a.sts" % RUN)
    keys([("KP_8", "P", "NONE"), ("KP_8", "P", None), ("KP_8", "R", None),
          ("KP_1", "P", "NONE"), ("KP_1", "P", None), ("KP_1", "R", None)])
    t = logtext(m); READ["C"] = t
    obs = "load a %d, load M08 %d, load M01 %d" % (loads(t, "a"), loads(t, "M08"), loads(t, "M01"))
    if AFTER:
        check("C", "P19_repeat_bound_swallowed_unbound_twice",
              loads(t, "a") == 1 and loads(t, "M08") == 0 and loads(t, "M01") == 2, obs, "load a 1, load M08 0, load M01 2")
    else:
        note("C", "native", obs)


def case_D():
    reset(); m = mark("D")
    bind("D", "key KP_8 press %s/a.sts" % RUN)
    bind("D", "key KP_8 press -")
    keys(press("KP_8"))
    t = logtext(m); READ["D"] = t
    obs = "load M08 %d, load a %d" % (loads(t, "M08"), loads(t, "a"))
    if AFTER:
        check("D", "P20_dash_restores_native", loads(t, "M08") == 1 and loads(t, "a") == 0, obs, "load M08 1, load a 0")
    else:
        note("D", "native", obs)


def case_E():
    reset(); m = mark("E")
    bind("E", "key KP_8 press %s/a.sts" % RUN)
    bind("E", "key KP_1 press %s/b.sts" % RUN)
    bind("E", "key KP_8 release %s/rp.sts" % RUN)
    bind("E", "key * press %s/c.sts scope once" % RUN)
    bind("E", "key * press - release -")
    m1 = mark("E1")
    keys(press("KP_8") + press("KP_1"))
    m2 = mark("E2")
    bind("E", "key KP_8 press %s/a.sts" % RUN)
    keys(press("KP_8"))
    t1, t2 = logtext(m1, m2), logtext(m2); READ["E"] = logtext(m)
    o1 = "load M08 %d, load M01 %d, bound %d" % (loads(t1, "M08"), loads(t1, "M01"),
                                                sum(loads(t1, x) for x in ("a", "b", "c", "rp")))
    o2 = "load a %d, load M08 %d" % (loads(t2, "a"), loads(t2, "M08"))
    if AFTER:
        check("E", "P21_star_dash_empties_every_chain", loads(t1, "M08") == 1 and loads(t1, "M01") == 1 and
              sum(loads(t1, x) for x in ("a", "b", "c", "rp")) == 0, o1, "load M08 1, load M01 1, bound 0")
        check("E", "P21_specific_bind_after", loads(t2, "a") == 1 and loads(t2, "M08") == 0, o2, "load a 1, load M08 0")
    else:
        note("E", "native_1", o1); note("E", "native_2", o2)


def case_F():
    reset(); m = mark("F")
    bind("F", "key KP_8 press %s/b.sts" % RUN)
    bind("F", "key KP_8 press %s/a.sts scope once" % RUN)
    keys(press("KP_8"))
    keys(press("KP_8"))
    t = logtext(m); READ["F"] = t
    order = [x for _, _, msg in entries(t) for x in ("a", "b") if re.search(r"load \S*/%s\.sts$" % x, msg)]
    if AFTER:
        check("F", "P22_once_then_global", order == ["a", "b"], order, ["a", "b"])
    else:
        note("F", "native", "loads %s, load M08 %d" % (order, loads(t, "M08")))


def show(case):
    """Play the show and return (segment offset, ms of its first line)."""
    since = logsize()
    tcp("script action play filename %s/show.sts" % RUN)
    ok = wait_log(since, r"Execute_command define f142_show_1 1", 10)
    check(case, "show_started", ok, ok, True)
    return since


def case_G():
    reset(); m = mark("G")
    bind("G", "key KP_8 press %s/g.sts" % RUN)
    s = show("G")
    keys(press("KP_8"))
    keys(press("KP_9"))
    ended = wait_end(s)
    m2 = logsize()
    keys(press("KP_9"))
    t, tail = logtext(m), logtext(m2); READ["G"] = t
    t_end = first_ms(t, r"ScriptMgr: script end")
    t_c = first_ms(t, r"load \S*/c\.sts$")
    obs = "c %d at %s, show end at %s, M09 after end %d, M09 before %d" % (loads(t, "c"), t_c, t_end, loads(tail, "M09"),
                                                                          loads(logtext(m, m2), "M09"))
    if AFTER:
        check("G", "P23_script_scope_lives_to_the_show_end", ended and t_c is not None and t_end is not None and t_c < t_end
              and loads(t, "c") == 1 and loads(tail, "M09") == 1 and loads(logtext(m, m2), "M09") == 0,
              obs, "c 1 before the show end, M09 1 after, 0 before")
    else:
        note("G", "native", obs)


def case_H():
    reset(); m = mark("H")
    bind("H", "key KP_8 press %s/h.sts" % RUN)
    s = show("H")
    keys(press("KP_8"))
    ended = wait_end(s)
    t = logtext(m); READ["H"] = t
    ms = {x: first_ms(t, r"Execute_command define f142_%s 1$" % x) for x in ("show_1", "h", "show_2", "show_3")}
    note("H", "timestamps_ms", ms)
    if AFTER:
        ok = ended and None not in ms.values() and ms["show_1"] < ms["h"] <= ms["show_2"] < ms["show_3"] \
            and ms["show_2"] - ms["h"] <= 50 and ms["show_2"] - ms["show_1"] < 4500
        check("H", "P24_bound_lines_first_then_the_show_wait_cut", ok,
              "h-show_1 %s, show_2-h %s, show_2-show_1 %s" % tuple(
                  (ms[b] - ms[a]) if ms[a] is not None and ms[b] is not None else None
                  for a, b in (("show_1", "h"), ("h", "show_2"), ("show_1", "show_2"))),
              "show_1 < h <= show_2, show_2-h <= 50 ms, show_2-show_1 < 4500 ms")


def case_I():
    reset(); m = mark("I")
    bind("I", "key KP_8 press %s/i.sts" % RUN)
    keys([("KP_8", "P", "NONE")])
    keys([("KP_8", "R", "NONE")])
    m2 = mark("I2")
    err = tcp("script action bind release %s/d.sts scope once" % RUN)
    keys(press("KP_9"))
    t, t2 = logtext(m, m2), logtext(m2); READ["I"] = logtext(m)
    errs = [msg for _, ty, msg in entries(err) if ty in ("Error", "Debug")]
    note("I", "tcp_key_omitted_lines", errs)
    if AFTER:
        check("I", "P25_key_omitted_is_the_trigger_key", loads(t, "i") == 1 and loads(t, "d") == 1,
              "load i %d, load d %d" % (loads(t, "i"), loads(t, "d")), "1, 1")
        check("I", "P25_key_omitted_over_tcp_is_an_error", any("key" in e and "tcp#" in e for e in errs)
              and loads(t2, "d") == 0 and loads(t2, "M09") == 1,
              "error lines %d, load d %d, load M09 %d" % (len(errs), loads(t2, "d"), loads(t2, "M09")),
              "a tcp#-tagged error naming the key, load d 0, load M09 1")
    else:
        note("I", "native", "load i %d, load d %d, load M08 %d" % (loads(t, "i"), loads(t, "d"), loads(t, "M08")))


def case_J():
    reset(); m = mark("J")
    bad = ["key KP_8", "key KP_8 press %s/a.sts scope never" % RUN, "key NO_SUCH press %s/a.sts" % RUN,
           "key KP_8 press %s/missing.sts" % RUN]
    lines, refused = [], 0
    for spec in bad:
        text = tcp("script action bind " + spec)
        refused += REFUSED in text
        lines.append([msg for _, ty, msg in entries(text) if ty == "Error"])
        note("J", "error_lines[%s]" % spec, [msg for _, ty, msg in entries(text) if ty in ("Error", "Debug")])
    keys(press("KP_8"))
    s = logsize()
    tcp("script action play filename %s/j.sts" % RUN)
    wait_end(s)
    t, tf = logtext(m), logtext(s); READ["J"] = t
    annotated = open("%s/j.sts" % RUN).read()
    if AFTER:
        check("J", "P26_one_tagged_error_line_each", all(len(l) == 1 and "tcp#" in l[0] for l in lines),
              [len(l) for l in lines], "[1, 1, 1, 1], each tcp#-tagged")
        check("J", "P26_nothing_stored", loads(t, "M08") == 1 and loads(t, "a") == 0,
              "load M08 %d, load a %d" % (loads(t, "M08"), loads(t, "a")), "1, 0")
        ferr = [msg for _, ty, msg in entries(tf) if ty == "Error"]
        check("J", "P26_file_line_tag_and_annotation", len(ferr) == 1 and "j.sts:1" in ferr[0] and "#!" in annotated,
              "%s | %s" % (ferr, annotated.strip()), "one error tagged j.sts:1, a #! tail in j.sts")
    else:
        note("J", "native", "refused %d of 4, load M08 %d, j.sts tail %s" % (refused, loads(t, "M08"), "#!" in annotated))


def case_K():
    reset(); m = mark("K")
    bind("K", "key KP_8 press %s/a.sts scope once" % RUN)
    bind("K", "key KP_8 press %s/b.sts" % RUN)
    keys(press("KP_8"))
    keys(press("KP_8"))
    t = logtext(m); READ["K"] = t
    obs = "load b %d, load a %d" % (loads(t, "b"), loads(t, "a"))
    if AFTER:
        check("K", "P27_global_discards_the_pending_once", loads(t, "b") == 2 and loads(t, "a") == 0, obs, "load b 2, load a 0")
    else:
        note("K", "native", obs + ", load M08 %d" % loads(t, "M08"))


def case_L():
    reset(); m = mark("L")
    bind("L", "key SUPER+KP_1 press %s/a.sts" % RUN)
    keys([("GRAVE", "P", "NONE"), ("GRAVE", "R", None)] + press("KP_1", None) + press("KP_1", None))
    t = logtext(m); READ["L"] = t
    obs = "load a %d, load M11 %d, load M01 %d" % (loads(t, "a"), loads(t, "M11"), loads(t, "M01"))
    if AFTER:
        check("L", "P28_super_plane_binding_consumes_super", loads(t, "a") == 1 and loads(t, "M11") == 0
              and loads(t, "M01") == 1, obs, "load a 1, load M11 0, load M01 1")
    else:
        note("L", "native", obs)


def case_M():
    reset(); m = mark("M")
    rec = "%s/rec.sts" % RUN
    bind("M", "key SHIFT+KP_8 press %s/i.sts" % RUN)
    tcp("script action record filename %s" % rec)
    keys([("KP_8", "P", "SHIFT")])
    keys([("KP_8", "R", "NONE")])
    tcp("script action cancelrecord")
    t = logtext(m); READ["M"] = t
    lines = [l.rstrip("\n") for l in open(rec)] if os.path.exists(rec) else []
    binds = [l for l in lines if "action bind" in l]
    note("M", "recorded_bind_lines", binds)
    if AFTER:
        check("M", "P29_recorded_with_the_explicit_key", len(binds) == 1 and "key SHIFT+KP_8" in binds[0]
              and loads(t, "d") == 1, "%s, load d %d" % (binds, loads(t, "d")), "one bind line carrying key SHIFT+KP_8, load d 1")
    else:
        note("M", "native", "bind lines recorded %d, load d %d" % (len(binds), loads(t, "d")))


def case_N():
    reset(); m = mark("N")
    seq = []
    for k in ("KP_8", "KP_1", "UP", "PAGEUP", "LEFT", "RIGHT", "DOWN", "PAGEDOWN"):
        seq += press(k, "NONE")
    for k in ("KP_8", "KP_2", "KP_4", "KP_6"):
        seq += [(k, "P", "SHIFT"), (k, "R", "NONE")]
    v = keys(seq, vzm=True, settle=2.0)
    t = logtext(m); READ["N"] = t
    msgs = [re.sub(r"f142_mark_N_\d+", "f142_mark_N_#", "(%s) %s" % (ty, msg)) for _, ty, msg in entries(t)]
    reading = msgs + v
    with open(OUT + "/N_reading.txt", "w") as f:
        f.write("\n".join(reading) + "\n")
    note("N", "reading", "%d log lines, %d state lines -> %s/N_reading.txt" % (len(msgs), len(v), OUT))
    if AFTER:
        base = os.path.join(HERE, "artifacts", "f142", ARGS.baseline, "N_reading.txt")
        want = open(base).read().splitlines() if ARGS.baseline and os.path.exists(base) else None
        diff = None if want is None else next((i for i, (a, b) in enumerate(zip(reading, want)) if a != b),
                                              None if len(reading) == len(want) else min(len(reading), len(want)))
        check("N", "P30_control_identical_to_before", want is not None and diff is None,
              "first differing line %s: %s" % (diff, reading[diff] if diff is not None and diff < len(reading) else "")
              if diff is not None else "identical, %d lines" % len(reading), "identical to " + base)


fixtures()
D.sendall(b"$DIAGON\n"); time.sleep(1.0); drain()
tcp("timerate rate 0")
for c in (case_A, case_B, case_C, case_D, case_E, case_F, case_G, case_H, case_I, case_J, case_K, case_L, case_M, case_N):
    c()
reset()
mark("END")
drain()
diag = [l for l in "".join(RX).replace("\x00", "").splitlines() if l.startswith("$DIAG")]   # each record ends "\n\0"
with open(OUT + "/tcp_rx.txt", "w") as f:
    f.write("".join(RX))
note("-", "diag_records_received", len(diag))
with open(OUT + "/diag.txt", "w") as f:
    f.write("\n".join(diag) + "\n")
with open(OUT + "/readings.json", "w") as f:
    json.dump({k: [m for _, _, m in entries(v)] for k, v in READ.items()}, f, indent=1)
print("\n=== %s ===" % ("ALL PASS" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
S.close(); D.close()
sys.exit(1 if FAILS else 0)
