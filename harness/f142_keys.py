#!/usr/bin/env python3
# F149 key leg - the 14 cases A-N of INTENT 11.312 (h) on the b21 pattern, in one fresh launch;
# cases P-AD (F154, INTENT 11.326): the joystick's buttons and hat on a virtual device attached at start;
# case AE (F156, INTENT 11.328), before O: an explicit `script action end` line ends the run once;
# case O (F151, INTENT 11.322) last: what a script's end leaves behind, read after HOME and after
# `configuration action load` inside an open `struct if` and a loop's first pass.
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
import argparse, json, os, re, select, signal, socket, sys, time, wave

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
    files.update({n: "define f154_%s 1" % n for n in ("p", "q", "r", "s", "u", "v", "vr", "w", "yr", "z")})
    files["x"] = "script action bind key BUTTON2 press %s/c.sts scope script" % RUN
    files["y"] = "script action bind release %s/yr.sts scope once" % RUN
    files["ae"] = "define f156_ae 1\nscript action end"
    for n, text in files.items():
        with open("%s/%s.sts" % (RUN, n), "w") as f:
            f.write(text + "\n")
    write_o()
    with wave.open(RUN + "/o.wav", "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(22050); w.writeframes(b"\x00\x00" * 22050)
    import struct, zlib
    chunk = lambda kind, data: struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    with open(O_IMAGE, "wb") as f:   # 4 x 4 RGBA, opaque white
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 4, 4, 8, 6, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress((b"\x00" + b"\xff" * 16) * 4)) + chunk(b"IEND", b""))


# Case O's show: line 5 opens an accepting `struct if`, line 6 a loop whose first pass outlasts the key.
O_IF, O_LOOP = 5, 6
# Show a generated image by an absolute path outside ~/.spacecrafter/: every path is accepted (INTENT D15).
O_IMAGE = RUN + "/o.png"


def write_o():
    with open(RUN + "/o.sts", "w") as f:
        f.write("\n".join(["define f142_o 1", "text action load name f142o string F151O azimuth 0 altitude 45",
                           "image action load name f142o filename %s" % O_IMAGE,
                           "audio action play filename %s/o.wav loop on" % RUN,
                           "struct if 1", "struct loop 20", "wait duration 10", "struct loop end", "struct if end",
                           "define f142_o_end 1"]) + "\n")


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


def vzm_print(tag):
    """Return the gdb line that prints Core::vzm's five orders after the event named tag."""
    return 'printf "F142VZM %s %s\\n", %s' % (tag, " ".join("%s=%%g" % v for v in VZM),
                                            ", ".join("$ui->core._M_ptr->vzm.%s" % v for v in VZM))


def vzm_read(since):
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        return re.findall(r"F142VZM .*$", f.read().decode("latin-1"), re.M)   # gdb prefixes "(gdb) "


def keys(seq, vzm=False, settle=1.0):
    """seq: (scancode suffix, 'P'|'R', plane name or None = keep key_Modifier as it is)."""
    cmds = []
    for k, ev, plane in seq:
        if plane is not None:
            cmds.append("set var $ui->key_Modifier = %d" % PLANE[plane])
        cmds.append("call $ui->handleKeys(SDL_SCANCODE_%s, 0, 0, s_gui::S_GUI_%s)"
                    % (k, "PRESSED" if ev == "P" else "RELEASED"))
        if vzm:
            cmds.append(vzm_print("%s %s" % (k, ev)))
    since = inject(cmds)
    time.sleep(settle)
    return vzm_read(since) if vzm else []


def press(k, plane="NONE"):
    return [(k, "P", plane), (k, "R", None)]


def wait_end(since, timeout=40):
    return wait_log(since, r"ScriptMgr: script end", timeout)


END_LINES = (r"ScriptMgr: script end", r"Execute_command script action end", r"End of script")


def end_lines(text, start=None):
    """Count each of END_LINES in text, from its first line matching start when given."""
    msgs = [msg for _, _, msg in entries(text)]
    if start is not None:
        msgs = msgs[next((i for i, msg in enumerate(msgs) if re.match(start + r"\s*$", msg)), len(msgs)):]
    return [sum(1 for msg in msgs if re.match(p + r"\s*$", msg)) for p in END_LINES]


def end_text(n):
    return "script end %d, Execute_command script action end %d, End of script %d" % tuple(n)


def script_path(tag):
    """Return FilePath's script directory, read in one gdb stop: every command re-fixes it at its entry."""
    since = inject(['printf "F156FP %s %%s\\n", FilePath::scriptPath._M_dataplus._M_p' % tag])
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        m = re.search(r"F156FP %s (.*)$" % tag, f.read().decode("latin-1"), re.M)
    return m.group(1) if m else None


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
    control("N", t, v)


def control(case, t, v):
    """Write a control case's reading and judge it identical, line by line, to the --baseline run's."""
    msgs = [re.sub(r"f142_mark_%s_\d+" % case, "f142_mark_%s_#" % case, "(%s) %s" % (ty, msg)) for _, ty, msg in entries(t)]
    reading = msgs + v
    with open("%s/%s_reading.txt" % (OUT, case), "w") as f:
        f.write("\n".join(reading) + "\n")
    note(case, "reading", "%d log lines, %d state lines -> %s/%s_reading.txt" % (len(msgs), len(v), OUT, case))
    if AFTER:
        base = os.path.join(HERE, "artifacts", "f142", ARGS.baseline, "%s_reading.txt" % case)
        want = open(base).read().splitlines() if ARGS.baseline and os.path.exists(base) else None
        diff = None if want is None else next((i for i, (a, b) in enumerate(zip(reading, want)) if a != b),
                                              None if len(reading) == len(want) else min(len(reading), len(want)))
        check(case, "P30_control_identical_to_before", want is not None and diff is None,
              "first differing line %s: %s" % (diff, reading[diff] if diff is not None and diff < len(reading) else "")
              if diff is not None else "identical, %d lines" % len(reading), "identical to " + base)


# ---- the joystick (F154, INTENT 11.326): cases P-AD on a virtual device ------------------------
# The device is attached once at start from gdb (F137's recipe): SDL_JoystickAttachVirtual(1, 6, 16, 1), 16
# buttons so the field's [joy_default] BUTTON0-15 are read whole; JoypadController opens joystick 0, so the
# index must be 0. An event is HANDED to the door the b21 way: an SDL_Event malloc'ed once in the engine,
# filled and passed to JoypadController::handle in the stop, `mode` set in the same stop when a case needs
# it (UI::handleDeal rewrites it from the engine every frame). AC's second half and AD go through
# SDL_JoystickSetVirtualButton and real frames instead. The natives are read from the field's ini and the
# hat bits from SDL's header, never typed here.
APPLOG = os.path.expanduser("~/.spacecrafter/log/spacecrafter.log")
JOYINI = os.path.expanduser("~/.spacecrafter/joypad.ini")
SDL_JOY_H = "/usr/include/SDL2/SDL_joystick.h"


def joy_default():
    section, keys_ = None, {}
    for line in open(JOYINI, encoding="latin-1"):
        line = line.strip()
        if line.startswith("["):
            section = line.strip("[]")
        elif section == "joy_default" and "=" in line:
            k, v = line.split("=", 1)
            keys_[k.strip()] = v.strip()
    return keys_


INI = joy_default()
LIST = {k: re.findall(r'"([^"]*)"', v) for k, v in INI.items() if v.startswith("command(")}
STICK = {int(k[4:-9]): v.lower() == "true" for k, v in INI.items() if re.match(r"AXIS\d+_IS_STICK$", k)}
SDLH = open(SDL_JOY_H).read()
HAT = {}
for _n, _v in re.findall(r"^#define SDL_HAT_(\w+)\s+(.+?)\s*$", SDLH, re.M):
    _bits = re.findall(r"SDL_HAT_(\w+)", _v)
    HAT[_n] = HAT[_bits[0]] | HAT[_bits[1]] if _bits else int(_v, 16)
AXIS_MIN = int(re.search(r"^#define SDL_JOYSTICK_AXIS_MIN\s+(-?\d+)", SDLH, re.M).group(1))
# Every axis at rest from the first frame: a stick at 0, a trigger at AXIS_MIN - the device does not start there
# (axis 4, a stick in the field's ini, read as a full deflection at the attach: move_lat every frame).
REST = {a: 0 if stick else AXIS_MIN for a, stick in STICK.items()}
if len(HAT) != 9 or not LIST.get("BUTTON3") or sorted(REST) != list(range(6)):
    sys.exit("FAIL - joystick_inputs_read observed=hats %s lists %s axes %s expected=9 SDL_HAT_ values, "
             "[joy_default] BUTTON3 a command list, AXIS0-5_IS_STICK" % (HAT, sorted(LIST), REST))
JOY = {"ok": False}
# an Sint16 read through `(int)` loses its sign (-32768 prints 32768): give gdb the declared return type
AXES = " ".join("a%d=%%d" % a for a in sorted(REST)), ", ".join("(short)SDL_JoystickGetAxis($js, %d)" % a for a in sorted(REST))


def applog(a):
    with open(APPLOG, "rb") as f:
        f.seek(a)
        return f.read().decode("latin-1")


def wait_applog(since, pattern, timeout=20.0):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if re.search(pattern, applog(since), re.M):
            return True
        time.sleep(0.1)
    return False


def joylines(since):
    return [msg for _, _, msg in entries(applog(since)) if "Joypad" in msg or "joy_default:" in msg]


def attach(tag, first=False):
    """Attach the virtual device; True when it is index 0 and the engine built its controller on it."""
    since, t0 = os.path.getsize(APPLOG), time.time()
    cmds = ["set $jidx = (int)SDL_JoystickAttachVirtual(1, 6, 16, 1)", 'printf "F154JIDX %s=%%d\\n", $jidx' % tag,
            "set $js = (void *)SDL_JoystickOpen($jidx)", 'printf "F154AXES %s open %s\\n", %s' % ((tag,) + AXES)]
    cmds += ["call (int)SDL_JoystickSetVirtualAxis($js, %d, %d)" % (a, v) for a, v in sorted(REST.items())]
    if first:
        cmds += ["set $ev = (SDL_Event *)(void *)malloc(sizeof(SDL_Event))", "call (void *)memset($ev, 0, sizeof(SDL_Event))"]
    g = inject(cmds)
    built = wait_applog(since, r"Joypad: no config detected switching to default", 20)
    secs = time.time() - t0
    time.sleep(0.5)
    inject(['printf "F154AXES %s rest %s\\n", %s' % ((tag,) + AXES)])
    with open(GDBLOG, "rb") as f:
        f.seek(g)
        txt = f.read().decode("latin-1")
    m = re.search(r"F154JIDX %s=(-?\d+)" % tag, txt)
    index = int(m.group(1)) if m else None
    axes = re.findall(r"F154AXES %s (open|rest) (.*)$" % tag, txt, re.M)
    note("-", "joystick_%s_axes" % tag, axes)
    rest = dict(axes).get("rest", "")
    JOY["ok"] = index == 0 and built and rest == " ".join("a%d=%d" % kv for kv in sorted(REST.items()))
    check("-", "joystick_%s_index_0_controller_built_axes_at_rest" % tag, JOY["ok"], "index %s, built %s, %.1f s, %s" % (
        index, built, secs, rest), "index 0, `Joypad: no config detected switching to default`, every axis at its ini rest")
    return joylines(since), secs


def detach(tag):
    since = os.path.getsize(APPLOG)
    inject(["call (int)SDL_JoystickDetachVirtual($jidx)", "call (void)SDL_JoystickClose($js)"])
    gone = wait_applog(since, r"delete JoypadController", 20)
    time.sleep(0.5)
    JOY["ok"] = False
    check("-", "joystick_%s_detached" % tag, gone, gone, "`delete JoypadController`")
    return joylines(since)


def jb(n, ev, mode=None):
    return ("B", n, ev, mode)


def jh(value, mode=None):
    return ("H", value, None, mode)


def jpress(n, mode=None):
    return [jb(n, "P", mode), jb(n, "R")]


def joy(seq, vzm=False, settle=1.0):
    """seq: jb(button, 'P'|'R', mode) / jh(SDL_HAT_ suffix, mode); mode None = keep the controller's."""
    cmds = []
    for kind, what, ev, mode in seq:
        if mode is not None:
            cmds.append("set var $ui->joypadController->mode = %d" % mode)
        if kind == "B":
            cmds += ["set var $ev->type = SDL_JOYBUTTON%s" % ("DOWN" if ev == "P" else "UP"),
                     "set var $ev->jbutton.button = %d" % what]
        else:
            cmds += ["set var $ev->type = SDL_JOYHATMOTION", "set var $ev->jhat.hat = 0",
                     "set var $ev->jhat.value = %d" % HAT[what]]
        cmds.append("call $ui->joypadController->handle(*$ev)")
        if vzm:
            cmds.append(vzm_print("BUTTON%d %s" % (what, ev) if kind == "B" else "HAT0 %s" % what))
    since = inject(cmds)
    time.sleep(settle)
    return vzm_read(since) if vzm else []


def sdl_button(n, value, settle=1.0):
    """Set the virtual button in SDL and let real frames pump it: no gdb call of the door."""
    inject(["call (int)SDL_JoystickSetVirtualButton($js, %d, %d)" % (n, value)])
    time.sleep(settle)


def orders(line):
    return {k: float(v) for k, v in re.findall(r"(\w+)=(-?[0-9.]+(?:e[-+]?\d+)?)", line)}


def ran(text, cmd):
    return len(re.findall(r"Execute_command %s\s*$" % re.escape(cmd), text, re.M))


def listed(text, key):
    """How many times each command of the ini's list for key ran."""
    return [ran(text, c) for c in LIST[key]]


def runs4(text, name):
    return len(re.findall(r"Execute_command define f154_%s 1\s*$" % re.escape(name), text, re.M))


def joycase(case):
    if not JOY["ok"]:
        check(case, "controller_present", False, "no controller on the virtual device", "attached at index 0")
    return JOY["ok"]


def joy_setup():
    lines, secs = attach("start", first=True)
    with open(OUT + "/ATT_reading.txt", "w") as f:
        f.write("\n".join(lines) + "\n")
    note("ATT", "attach_seconds", "%.1f" % secs)
    note("ATT", "ini_read", "lists %s, the axes set to their rest %s" % (sorted(LIST), REST))
    hats = [h for l in lines for h in re.findall(r'"joy_default:HAT0_(\w+)_ALT"', l)]
    check("ATT", "P7_init_reads_the_hat_in_enum_order", hats == ["UP", "DOWN", "LEFT", "RIGHT"], hats,
          ["UP", "DOWN", "LEFT", "RIGHT"])
    base = os.path.join(HERE, "artifacts", "f142", ARGS.baseline, "ATT_reading.txt")
    if ARGS.baseline and os.path.exists(base):
        want = open(base).read().splitlines()
        rest = lambda ls: [l for l in ls if "HAT0_" not in l]
        check("ATT", "P7_every_other_init_line_identical_in_order", rest(lines) == rest(want) and sorted(lines) == sorted(want),
              "%d / %d lines, the non-hat lines %s, the same set %s" % (len(lines), len(want), rest(lines) == rest(want),
                                                                       sorted(lines) == sorted(want)), "identical to " + base)
    else:
        note("ATT", "P7_baseline", "no ATT_reading.txt under --baseline '%s'" % ARGS.baseline)


def case_P():
    if not joycase("P"): return
    reset(); m = mark("P")
    bind("P", "key BUTTON3 press %s/p.sts" % RUN)
    joy(jpress(3))
    t = logtext(m); READ["P"] = t
    obs = "load p %d, run p %d, list %s" % (loads(t, "p"), runs4(t, "p"), listed(t, "BUTTON3"))
    check("P", "P18_press_runs_the_script_not_the_list", loads(t, "p") == 1 and runs4(t, "p") == 1
          and listed(t, "BUTTON3") == [0, 0], obs, "load p 1, run p 1, list [0, 0]")


def case_Q():
    if not joycase("Q"): return
    reset(); m = mark("Q")
    bind("Q", "key BUTTON3 release %s/q.sts" % RUN)
    joy([jb(3, "P")])
    m2 = mark("Q2")
    joy([jb(3, "R")])
    t1, t2 = logtext(m, m2), logtext(m2); READ["Q"] = logtext(m)
    obs = "press: list %s, load q %d | release: load q %d, list %s" % (listed(t1, "BUTTON3"), loads(t1, "q"),
                                                                       loads(t2, "q"), listed(t2, "BUTTON3"))
    check("Q", "P19_release_on_a_command_button", listed(t1, "BUTTON3") == [1, 1] and loads(t1, "q") == 0
          and loads(t2, "q") == 1 and listed(t2, "BUTTON3") == [0, 0], obs, "press: list [1, 1], load q 0 | release: load q 1")


def case_R():
    if not joycase("R"): return
    reset(); m = mark("R")
    bind("R", "key BUTTON3 release %s/r.sts" % RUN)
    joy([jb(3, "P", 0), jb(3, "R", 1)])
    t = logtext(m); READ["R"] = t
    check("R", "P20_one_release_chain_whatever_the_mode", loads(t, "r") == 1 and runs4(t, "r") == 1,
          "load r %d, run r %d" % (loads(t, "r"), runs4(t, "r")), "1, 1")


def case_S():
    if not joycase("S"): return
    reset(); m = mark("S")
    bind("S", "key BUTTON3 press %s/s.sts" % RUN)
    joy([jb(3, "P", 1), jb(3, "R")])
    joy([jb(3, "P", 0), jb(3, "R")])
    t = logtext(m); READ["S"] = t
    check("S", "P21_one_binding_on_both_sets", loads(t, "s") == 2 and runs4(t, "s") == 2 and listed(t, "BUTTON3") == [0, 0],
          "load s %d, run s %d, list %s" % (loads(t, "s"), runs4(t, "s"), listed(t, "BUTTON3")), "2, 2, list [0, 0]")


def case_T():
    if not joycase("T"): return
    reset(); m = mark("T")
    bind("T", "key BUTTON3 press %s/p.sts" % RUN)
    bind("T", "key BUTTON3 press -")
    joy(jpress(3))
    t = logtext(m); READ["T"] = t
    check("T", "P22_dash_restores_the_list", listed(t, "BUTTON3") == [1, 1] and loads(t, "p") == 0,
          "list %s, load p %d" % (listed(t, "BUTTON3"), loads(t, "p")), "list [1, 1], load p 0")


def case_U():
    if not joycase("U"): return
    reset(); m = mark("U")
    bind("U", "key BUTTON7 press %s/u.sts" % RUN)
    v = joy([jb(7, "P"), jb(7, "R")], vzm=True)
    t = logtext(m); READ["U"] = t
    fov = [orders(l).get("deltaFov") for l in v]
    note("U", "orders", v)
    check("U", "P23_native_press_suppressed_native_release_runs", loads(t, "u") == 1 and fov == [0, 0],
          "load u %d, deltaFov after press / release %s" % (loads(t, "u"), fov), "load u 1, [0, 0]")


def case_V():
    if not joycase("V"): return
    reset(); m = mark("V")
    bind("V", "key HAT0_UP press %s/v.sts release %s/vr.sts" % (RUN, RUN))
    v1 = joy([jh("UP")], vzm=True)
    m2 = mark("V2")
    v2 = joy([jh("CENTERED")], vzm=True)
    t1, t2 = logtext(m, m2), logtext(m2); READ["V"] = logtext(m)
    alt = [orders(l).get("deltaAlt") for l in v1 + v2]
    note("V", "orders", v1 + v2)
    check("V", "P24_hat_press_and_release_edges", loads(t1, "v") == 1 and loads(t1, "vr") == 0 and loads(t2, "vr") == 1
          and loads(t2, "v") == 0 and alt == [0, 0],
          "UP: load v %d, vr %d | CENTERED: load vr %d, v %d | deltaAlt %s" % (loads(t1, "v"), loads(t1, "vr"),
                                                                               loads(t2, "vr"), loads(t2, "v"), alt),
          "UP: v 1, vr 0 | CENTERED: vr 1, v 0 | [0, 0]")


def case_W():
    if not joycase("W"): return
    reset(); m = mark("W")
    bind("W", "key HAT0_LEFT press %s/w.sts" % RUN)
    v1 = joy([jh("LEFTUP")], vzm=True)
    v2 = joy([jh("CENTERED")], vzm=True)
    t = logtext(m); READ["W"] = t
    o1, o2 = (orders(v1[0]) if v1 else {}), (orders(v2[0]) if v2 else {})
    note("W", "orders", v1 + v2)
    got = [o1.get("deltaAlt"), o1.get("deltaAz"), o2.get("deltaAlt"), o2.get("deltaAz")]
    check("W", "P25_diagonal_two_edges", loads(t, "w") == 1 and got == [1, 0, 0, 0],
          "load w %d, LEFTUP deltaAlt / deltaAz %s, CENTERED %s" % (loads(t, "w"), got[:2], got[2:]),
          "load w 1, LEFTUP [1, 0], CENTERED [0, 0]")


def case_X():
    if not joycase("X"): return
    reset(); m = mark("X")
    bind("X", "key BUTTON3 press %s/x.sts" % RUN)
    s = show("X")
    joy(jpress(3))
    joy(jpress(2))
    ended = wait_end(s)
    m2 = logsize()
    joy(jpress(2))
    t, during, tail = logtext(m), logtext(m, m2), logtext(m2); READ["X"] = t
    t_end, t_c = first_ms(t, r"ScriptMgr: script end"), first_ms(t, r"load \S*/c\.sts$")
    obs = "x %d, c %d at %s, show end at %s, BUTTON2 list during %s, after %s" % (
        loads(t, "x"), loads(t, "c"), t_c, t_end, listed(during, "BUTTON2"), listed(tail, "BUTTON2"))
    check("X", "P26_script_scope_on_a_button_dies_with_the_show", ended and t_c is not None and t_end is not None
          and t_c < t_end and loads(t, "c") == 1 and listed(during, "BUTTON2") == [0] and listed(tail, "BUTTON2") == [1],
          obs, "c 1 before the show end, the BUTTON2 list 0 during, 1 after")


def case_Y():
    if not joycase("Y"): return
    reset(); m = mark("Y")
    rec = "%s/recy.sts" % RUN
    bind("Y", "key BUTTON3 press %s/y.sts" % RUN)
    tcp("script action record filename %s" % rec)
    joy([jb(3, "P")])
    m2 = mark("Y2")
    joy([jb(3, "R")])
    tcp("script action cancelrecord")
    t1, t2 = logtext(m, m2), logtext(m2); READ["Y"] = logtext(m)
    lines = [l.rstrip("\n") for l in open(rec)] if os.path.exists(rec) else []
    binds = [l for l in lines if "action bind" in l]
    want = 'script action bind key BUTTON3 release "%s/yr.sts" scope once' % RUN
    note("Y", "recorded_bind_lines", binds)
    check("Y", "P27_key_omitted_names_the_button_recorded_explicitly", loads(t1, "y") == 1 and loads(t1, "yr") == 0
          and loads(t2, "yr") == 1 and binds == [want], "press: y %d, yr %d | release: yr %d | %s" % (
              loads(t1, "y"), loads(t1, "yr"), loads(t2, "yr"), binds), "press: y 1, yr 0 | release: yr 1 | [%s]" % want)


def case_Z():
    if not joycase("Z"): return
    reset(); m = mark("Z")
    bind("Z", "key * press %s/z.sts" % RUN)
    joy(jpress(3))
    keys(press("KP_1"))
    m2 = mark("Z2")
    reset()
    joy(jpress(3))
    t1, t2 = logtext(m, m2), logtext(m2); READ["Z"] = logtext(m)
    check("Z", "P28_star_catches_a_button_and_a_key", loads(t1, "z") == 2 and listed(t1, "BUTTON3") == [0, 0]
          and loads(t1, "M01") == 0, "load z %d, list %s, load M01 %d" % (loads(t1, "z"), listed(t1, "BUTTON3"), loads(t1, "M01")),
          "load z 2, list [0, 0], load M01 0")
    check("Z", "P28_star_dash_restores_the_list", loads(t2, "z") == 0 and listed(t2, "BUTTON3") == [1, 1],
          "load z %d, list %s" % (loads(t2, "z"), listed(t2, "BUTTON3")), "load z 0, list [1, 1]")


def case_AA():
    if not joycase("AA"): return
    reset(); m = mark("AA")
    rows = []
    for tok in ("BUTTON", "HAT0_NORTH", "BUTTON3_ALT", "CTRL+BUTTON3", "ALT+BUTTON3"):
        errs = [msg for _, ty, msg in entries(tcp("script action bind key %s press %s/z.sts" % (tok, RUN))) if ty == "Error"]
        note("AA", "error_lines[%s]" % tok, errs)
        # the reason alone: neither the quoted token nor the line's echo can carry the forms
        reason = errs[0][:errs[0].rfind(" [")].replace("'%s'" % tok, "") if len(errs) == 1 else ""
        rows.append(len(errs) == 1 and "tcp#" in errs[0] and all(f in reason for f in ("BUTTON3", "HAT0_UP", "_ALT")))
    joy(jpress(3))
    t = logtext(m); READ["AA"] = t
    check("AA", "P29_one_tagged_error_each_naming_the_button_forms", all(rows), rows,
          "5 x True: one tcp#-tagged error line, its reason naming BUTTON3, HAT0_UP and _ALT")
    check("AA", "P29_nothing_stored", loads(t, "z") == 0 and listed(t, "BUTTON3") == [1, 1],
          "load z %d, list %s" % (loads(t, "z"), listed(t, "BUTTON3")), "load z 0, list [1, 1]")


def case_AB():
    if not joycase("AB"): return
    reset(); m = mark("AB")
    seq = [jb(1, "P", 0), jb(1, "R")] + jpress(3) + jpress(7) + [jh("UP"), jh("CENTERED"), jh("LEFTUP"), jh("CENTERED")]
    v = joy(seq, vzm=True, settle=2.0)
    t = logtext(m); READ["AB"] = t
    control("AB", t, v)


def case_AC():
    if not joycase("AC"): return
    reset(); m = mark("AC")
    bind("AC", "key BUTTON3 press %s/p.sts" % RUN)
    joy([jb(3, "P")])
    detach("AC1")
    attach("AC1")
    if not joycase("AC"): return
    joy(jpress(3))
    t = logtext(m)
    check("AC", "P31_press_after_a_replug_not_swallowed", loads(t, "p") == 2, "load p %d" % loads(t, "p"), "2")
    # a button SDL saw pressed, then the detach: does SDL hand the door a release?
    m2 = mark("AC2")
    bind("AC", "key BUTTON3 release %s/r.sts" % RUN)
    sdl_button(3, 1)
    lines = detach("AC2")
    t2 = logtext(m2); READ["AC"] = logtext(m)
    note("AC", "sdl_release_at_detach", "SDL press: load p %d | at the detach: load r %d, app log %s" % (
        loads(t2, "p"), loads(t2, "r"), [l for l in lines if "Joypad event" in l or "JoypadController" in l]))
    attach("AC2")


def case_AD():
    if not joycase("AD"): return
    reset(); m = mark("AD")
    bind("AD", "key BUTTON3 press %s/p.sts" % RUN)
    sdl_button(3, 1)
    sdl_button(3, 0)
    t = logtext(m); READ["AD"] = t
    note("AD", "pump", "list %s, load p %d" % (listed(t, "BUTTON3"), loads(t, "p")))
    check("AD", "P32_virtual_button_reaches_the_door_through_the_pump", loads(t, "p") == 1 and listed(t, "BUTTON3") == [0, 0],
          "load p %d, list %s" % (loads(t, "p"), listed(t, "BUTTON3")), "load p 1, list [0, 0]")


# ---- case AE (F156, INTENT 11.328): an explicit `script action end` line ends the run once ------------
def case_AE():
    reset(); m = mark("AE")
    rec = "%s/recae.sts" % RUN
    tcp("script action record filename %s" % rec)
    s = logsize()
    tcp("script action play filename %s/ae.sts" % RUN)
    ended = wait_end(s)
    time.sleep(1.0)
    fp = script_path("AE")
    tcp("script action cancelrecord")
    m2 = mark("AE2")
    show("AE")
    tcp("script action end")   # a TCP line runs after the drain, never inside it: the control
    time.sleep(1.0)
    fp2 = script_path("AE2")
    m3 = mark("AE3")
    t, t2 = logtext(m, m2), logtext(m2, m3); READ["AE"] = logtext(m, m3)
    n, n2 = end_lines(t), end_lines(t2)
    recorded = [l.strip() for l in open(rec)].count("script action end") if os.path.exists(rec) else None
    other = [msg for _, ty, msg in entries(t) if ty == "Error" or msg.startswith("script annotation")]
    ran = len(re.findall(r"Execute_command define f156_ae 1\s*$", t, re.M))
    note("AE", "end_lines_explicit", end_text(n)); note("AE", "end_lines_tcp", end_text(n2))
    note("AE", "recorded_end_lines", recorded); note("AE", "error_or_annotation_lines", other)
    note("AE", "filepath_script_path_after_explicit_end", fp); note("AE", "filepath_script_path_after_tcp_end", fp2)
    check("AE", "setup_the_line_before_the_end_ran", ended and ran == 1, "ended %s, define f156_ae ran %d" % (ended, ran),
          "ended, 1")
    check("AE", "tcp_end_runs_once_control", n2 == [1, 1, 0], end_text(n2), end_text([1, 1, 0]))
    if AFTER:
        check("AE", "P3_explicit_end_runs_once", n == [1, 1, 0] and not other, "%s, error / annotation lines %s" % (
            end_text(n), other), end_text([1, 1, 0]) + ", none")
        check("AE", "recorded_end_line_once", recorded == 1, recorded, 1)


# ---- case O (INTENT 11.322): what a script's end leaves behind, on two end paths through UI::init ----
IFS = "(*(IfSwap**)&$c->ifSwap)"   # read a unique_ptr through its one member: gdb finds _M_head_impl ambiguous
OSTATE = [("loopOpen", "%d", "$c->loopOpen"), ("loopOpenerLine", "%u", "$c->loopOpener.line"),
          ("swapCommand", "%d", "$c->swapCommand"),
          ("ifOpeners", "%d", "(int)(%s->m_openers._M_impl._M_finish - %s->m_openers._M_impl._M_start)" % (IFS, IFS)),
          ("texts", "%lu", "(*(TextMgr**)&$m->textMgr)->textUsr._M_t._M_impl._M_node_count"),
          ("images", "%lu", "(*(ImageMgr**)&$m->imageMgr)->active_images._M_impl._M_node._M_size"),
          ("musicPlaying", "%d", "(*(Audio**)&$m->audio)->music_isPlaying"),
          ("audioState", "%d", "(int)(*(Audio**)&$m->audio)->state")]


def ostate(tag):
    """Return the gdb line that prints the commander's if / loop state and Media's three contents."""
    return 'printf "F151O %s %s\\n", %s' % (tag, " ".join("%s=%s" % (n, f) for n, f, _ in OSTATE),
                                            ", ".join(e for _, _, e in OSTATE))


def unclosed(text):
    return [msg for _, ty, msg in entries(text) if ty == "Error" and "o.sts:" in msg and "never closed" in msg]


def play_o():
    write_o()
    since = logsize()
    tcp("script action play filename %s/o.sts" % RUN)
    ok = wait_log(since, r"Execute_command define f142_o 1", 10)
    check("O", "show_started", ok, ok, True)
    time.sleep(2.0)


def case_O():
    reset(); m = mark("O")
    play_o()
    l1 = logsize()
    since = inject(["set $c = $ui->app->commander._M_ptr", "set $m = $ui->media._M_ptr", ostate("pre"),
                    "set var $ui->key_Modifier = %d" % PLANE["NONE"],
                    "call $ui->handleKeys(SDL_SCANCODE_HOME, 0, 0, s_gui::S_GUI_PRESSED)",
                    "call $ui->handleKeys(SDL_SCANCODE_HOME, 0, 0, s_gui::S_GUI_RELEASED)", ostate("post")], timeout=120)
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        st = {tag: {k: int(v) for k, v in (kv.split("=") for kv in rest.split())}
              for tag, rest in re.findall(r"F151O (pre|post) (.*)$", f.read().decode("latin-1"), re.M)}
    home_ended = wait_log(l1, r"Execute_command script action end", 30)
    time.sleep(1.0)
    t1, o1 = logtext(l1), open(RUN + "/o.sts").read()
    home = end_lines(t1, r"Execute_command script action end")   # from initial.sts:17's line
    mark("O2")
    play_o()
    l2 = logsize()
    tcp("configuration action load", 30)
    load_ended = wait_log(l2, r"End of script\s*$", 30)
    time.sleep(1.0)
    t2, o2 = logtext(l2), open(RUN + "/o.sts").read()
    READ["O"] = logtext(m)
    pre, post = st.get("pre", {}), st.get("post", {})
    u1, u2 = unclosed(t1), unclosed(t2)
    tails1, tails2 = [l for l in o1.splitlines() if "#!" in l], [l for l in o2.splitlines() if "#!" in l]
    note("O", "state_pre", pre); note("O", "state_post_HOME", post)
    note("O", "unclosed_after_HOME", u1); note("O", "o_sts_tails_after_HOME", tails1)
    note("O", "unclosed_after_configuration_load", u2); note("O", "o_sts_tails_after_configuration_load", tails2)
    note("O", "end_lines_from_initial_sts_end", end_text(home))
    g = lambda d, k: d.get(k, -1)
    check("O", "setup_if_open_loop_first_pass_media_up", g(pre, "loopOpen") == 1 and g(pre, "loopOpenerLine") == O_LOOP
          and g(pre, "ifOpeners") == 1 and g(pre, "texts") >= 1 and g(pre, "images") >= 1 and g(pre, "musicPlaying") == 1,
          pre, "loopOpen 1, loopOpenerLine %d, ifOpeners 1, texts >= 1, images >= 1, musicPlaying 1" % O_LOOP)
    if AFTER:
        check("O", "P7_media_cleared_on_HOME", g(post, "texts") == 0 and g(post, "images") == 0
              and g(post, "musicPlaying") == 0, "texts %s, images %s, musicPlaying %s" % (
                  g(post, "texts"), g(post, "images"), g(post, "musicPlaying")), "0, 0, 0")
        check("O", "P7_if_loop_state_reset_on_HOME", g(post, "loopOpen") == 0 and g(post, "loopOpenerLine") == 0
              and g(post, "ifOpeners") == 0 and g(post, "swapCommand") == 0,
              "loopOpen %s at line %s, ifOpeners %s, swapCommand %s" % (
                  g(post, "loopOpen"), g(post, "loopOpenerLine"), g(post, "ifOpeners"), g(post, "swapCommand")),
              "loopOpen 0 at line 0, ifOpeners 0, swapCommand 0")
        check("O", "P7_no_false_unclosed_after_HOME", home_ended and not u1 and not tails1,
              "initial.sts ended %s, errors %s, tails %s" % (home_ended, u1, tails1), "ended, no error at o.sts, no #! tail")
        check("O", "P7_no_false_unclosed_after_configuration_load", load_ended and not u2 and not tails2,
              "startup.sts ended %s, errors %s, tails %s" % (load_ended, u2, tails2), "ended, no error at o.sts, no #! tail")
        check("O", "P4_initial_sts_explicit_end_runs_once", home == [1, 1, 0], end_text(home), end_text([1, 1, 0]))


fixtures()
D.sendall(b"$DIAGON\n"); time.sleep(1.0); drain()
tcp("timerate rate 0")
joy_setup()
for c in (case_A, case_B, case_C, case_D, case_E, case_F, case_G, case_H, case_I, case_J, case_K, case_L, case_M, case_N,
          case_P, case_Q, case_R, case_S, case_T, case_U, case_V, case_W, case_X, case_Y, case_Z, case_AA, case_AB,
          case_AC, case_AD, case_AE, case_O):
    c()
reset()
mark("END")
drain()
# the virtual device at rest moves nothing: UI::moveLat / moveLon write `add y|z` each frame an axis is deflected
AXIS_CMDS = len(re.findall(r"Execute_command add [yz] ", logtext(0)))
check("-", "no_axis_command_over_the_run", AXIS_CMDS == 0, AXIS_CMDS, 0)
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
