#!/usr/bin/env python3
"""screen_drive.py - play the screen record campaign's scene on one pinned path (INTENT 11.333 (f); F162, INTENT 11.335).

    cd claude/harness && bash f124_run.sh ../screen/<label> f125_adapt.py screen_drive.py --pin on|off [--scene FILE]
    python3 -I screen_drive.py <outdir> --pin on|off [--scene FILE] --dry-run    # no socket: the scene file + the sequence

Writes the scene (default screen_scene.sts) with @OUT@ = the absolute outdir into <outdir>/scene.sts, pins the path
(`flag experimental_path on|off`), plays the scene (`script action play filename <abs>`), polls the sentinel - the
scene's last `body action dual_dump` file - for the scene's summed waits + 60 s, then sends `shutdown action now`.
The phase files, the sentinel and the bound are read from the scene, never listed here.
Checks: each phase file exists, was written by this run, loads (screen_record.load: frame indices consecutive from 0,
malformed lines refused), holds a frame; every F line's drawn tag is the pin's (on = n, off = o); a new-pinned record
holds no o line and an old-pinned one both engines (INTENT 11.333 (d)); the sentinel parses (dumpread.load_dump) and
its header's drawn path is the pin's.
Exit: 0 every check PASS, 1 a FAIL, 2 the scene or the outdir refused (nothing sent), 3 the sentinel never came.
"""
import argparse, ast, os, re, socket, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dumpread
import screen_record

_src = HERE + "/f124_legs.py"
_node = next(n for n in ast.parse(open(_src).read()).body if isinstance(n, ast.FunctionDef) and n.name == "reuse")
exec(compile(ast.Module([_node], []), _src, "exec"), globals())
reuse(HERE + "/b21_keypath.py", ["check", "send"])
FAILS = []
TAG = {"on": "n", "off": "o"}
DRAWN = {"on": "new", "off": "old"}


def refuse(why):
    print("=== screen_drive REFUSED: %s ===" % why, flush=True)
    sys.exit(2)


def plan(scene):
    """Read the scene -> (phase files, summed waits in s, sentinel file); refuse a scene the bound cannot be read from."""
    phases, waits, last_record, last_off, sentinel, sentinel_at = [], 0.0, None, 0, None, 0
    for no, line in enumerate(scene.splitlines(), 1):
        w = line.split()
        if not w or w[0].startswith("#"):
            continue
        kv = dict(zip(w[1::2], w[2::2]))
        if w[0] == "wait":
            try:
                waits += float(kv["duration"]) if set(kv) == {"duration"} else float("nan")
            except ValueError:
                waits = float("nan")
            if waits != waits:
                refuse("scene line %d %r: only 'wait duration <seconds>' gives the poll its bound" % (no, line))
        elif w[0] == "body" and kv.get("action") == "screen_record":
            last_record = kv.get("state")
            if last_record == "on" and kv.get("filename"):
                phases.append(kv["filename"])
            elif last_record == "off":
                last_off = no
            else:
                refuse("scene line %d %r: 'state on filename <f>' or 'state off' expected" % (no, line))
        elif w[0] == "body" and kv.get("action") == "dual_dump" and kv.get("filename"):
            sentinel, sentinel_at = kv["filename"], no
    if not phases or last_record != "off" or sentinel is None or sentinel_at < last_off:
        refuse("the scene must open a phase, close the last one by 'state off', then write a dual_dump (the sentinel)")
    if len(set(phases)) != len(phases):
        refuse("two phases name one file: %s" % phases)
    return phases, waits, sentinel


def sentinel_header(path):
    """Return the sentinel's header once its file holds still over 0.5 s, else None."""
    if not os.path.exists(path):
        return None
    size = os.path.getsize(path)
    time.sleep(0.5)
    return dumpread.load_dump(path, require_old=False)[0] if os.path.getsize(path) == size else None


def judge(path, pin, t0):
    """Check one phase file against the pin; print its frames, duration and line counts."""
    name = os.path.basename(path)
    if not os.path.exists(path):
        return check(name + "_exists", False, path)
    check(name + "_written_by_this_run", os.path.getmtime(path) >= t0, "mtime %.1f, run start %.1f" % (os.path.getmtime(path), t0))
    try:
        rec = screen_record.load(path)
    except screen_record.Refused as e:
        return check(name + "_loads", False, str(e))
    lines = {e: sum(g == e for f in rec.frames for (g, _) in f.bodies) for e in screen_record.ENGINES}
    other = [f.frame for f in rec.frames if f.drawn != TAG[pin]]
    print("-- %s: %d frames, %.1f ms, B lines o %d n %d" % (name, len(rec.frames), (screen_record.elapsed(rec) or [0])[-1],
                                                            lines["o"], lines["n"]), flush=True)
    check(name + "_loaded_and_holds_a_frame", len(rec.frames) > 0, "%d frames, indices consecutive from 0 (load)" % len(rec.frames))
    check(name + "_drawn_tag_is_the_pin", not other, "tag %s expected; other on frames %s" % (TAG[pin], screen_record._ranges(other)))
    if pin == "on":
        check(name + "_new_pin_has_no_o_line", lines["o"] == 0, "%d o lines" % lines["o"])
    else:
        check(name + "_old_pin_has_both_engines", lines["o"] > 0 and lines["n"] > 0, "o %d n %d" % (lines["o"], lines["n"]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("out")
    ap.add_argument("--pin", required=True, choices=["on", "off"])
    ap.add_argument("--scene", default=os.path.join(HERE, "screen_scene.sts"))
    ap.add_argument("--dry-run", action="store_true", help="write the scene, print the sequence, open no socket")
    a = ap.parse_args()
    out = os.path.abspath(a.out)
    if re.search(r"\s", out):
        refuse("outdir %r holds whitespace: the engine splits a line's words on it" % out)
    text = open(a.scene).read()
    if "@OUT@" not in text:
        refuse("%s holds no @OUT@: its files would not land in the outdir" % a.scene)
    scene = text.replace("@OUT@", out)
    phases, waits, sentinel = plan(scene)
    os.makedirs(out, exist_ok=True)
    scene_path = os.path.join(out, "scene.sts")
    with open(scene_path, "w") as fh:
        fh.write(scene)
    seq = ["flag experimental_path " + a.pin, "script action play filename " + scene_path]
    stale = [p for p in phases + [sentinel] if os.path.exists(p)]
    print("-- scene %s -> %s (@OUT@ left: %d)" % (a.scene, scene_path, open(scene_path).read().count("@OUT@")), flush=True)
    print("-- phases %d: %s" % (len(phases), " ".join(os.path.basename(p) for p in phases)), flush=True)
    print("-- sentinel %s; bound %.1f s = the scene's waits %.1f s + 60 s" % (sentinel, waits + 60, waits), flush=True)
    if a.dry_run:
        print("-- DRY RUN: no socket, nothing sent; the live sequence:", flush=True)
        for c in seq:
            print(">> " + c, flush=True)
        print("-- poll %s every 0.5 s until dumpread.load_dump gives its header, at most %.1f s" % (sentinel, waits + 60), flush=True)
        print(">> shutdown action now", flush=True)
        print("-- then judge each phase file against drawn tag %s; files already there (a live run refuses them): %s"
              % (TAG[a.pin], stale or "none"), flush=True)
        return 0
    if stale:
        refuse("files of a previous run are in the outdir, choose another label: %s" % stale)
    sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
    t0, hdr = time.time(), None
    try:
        send(sock, seq[0], 2)
        send(sock, seq[1], 0.5)
        deadline = time.time() + waits + 60
        while hdr is None and time.time() < deadline:
            time.sleep(0.5)
            hdr = sentinel_header(sentinel)
    finally:
        try:
            send(sock, "shutdown action now", 1)
        except OSError as e:
            print("-- shutdown not sent: %s" % e, flush=True)
        sock.close()
    check("sentinel_reached", hdr is not None, "%s after %.1f s (bound %.1f s)" % (sentinel, time.time() - t0, waits + 60))
    if hdr is None:
        print("=== screen_drive pin %s STOPPED: the sentinel never came ===" % a.pin, flush=True)
        return 3
    drawn = (hdr.get("control") or {}).get("drawnPath")
    check("sentinel_drawn_path_is_the_pin", drawn == DRAWN[a.pin], "control.drawnPath = %s" % drawn)
    for p in phases:
        judge(p, a.pin, t0)
    print("=== screen_drive pin %s: %s ===" % (a.pin, "ALL PASS" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
