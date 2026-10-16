#!/usr/bin/env python3
"""F160 - the per-frame screen record's leg (INTENT 11.334): one fresh launch drives
`body action screen_record`, then judges the files against the format of INTENT 11.333 (d).

    cd claude/harness && SC_SEAM_RECORD=1 bash f124_run.sh ../f160/<label> f125_adapt.py f160_leg.py --pin on|off
    python3 f160_leg.py --self-test                  # a synthetic record, three planted defects: exit 1, each named
    python3 f160_leg.py <outdir> --pin on|off --judge   # re-judge a run's files, no engine

SC_SEAM_RECORD=1 arms the engine's seam ring: the cost window (record OFF) and the cross-check
(the record's frames are a contiguous run of the ring) read it from the sentinel dump; without
it both print NOTE ... not reached. Lines: PASS / FAIL / NOTE (f124_run.sh echoes them); exit 1
on any FAIL. What is reused: `reuse` is f124_legs.py's, `check` / `send` / `_clean` are
b21_keypath.py's (both files run their scene at import), the log is read through logread.py.
"""
import argparse, ast, json, math, os, socket, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import logread

_src = ast.parse(open(os.path.join(HERE, "f124_legs.py")).read())
exec(compile(ast.Module([n for n in _src.body if isinstance(n, ast.FunctionDef) and n.name == "reuse"], []),
             "f124_legs.py", "exec"))
reuse(os.path.join(HERE, "b21_keypath.py"), ["check", "send", "_clean"])

FAILS = []
FPS_CAP = 144.0             # config.ini maximum_fps: a CAP, not the cadence (this host's display stack runs ~60 fps)
REST_TOL = 1e-6             # the mandate's bound on a still body's frame-to-frame change
ZOOM_TOL = 1e-3             # size x fov constant to this relative spread
XY_SLACK = 1e-6             # |pos| <= pi / half-fov (the antipode) x (1 + this), float32's ulp: the home body sits at the nadir
HEADER = [["#", "screen_record", "1"],
          ["#", "B", "engine(o|n)", "name", "x", "y", "size"],
          ["#", "F", "frame", "dt_ms", "jd", "drawn(o|n)", "fov_old_deg", "half_fov_new_rad"]]
MSG = {"no_state": "command 'body screen_record' : missing state",
       "no_filename": "command 'body screen_record' : missing filename",
       "bad_value": "command 'body screen_record' : unknown state value",
       "no_open": "command 'body screen_record' : cannot open "}
FILES = ("rest.tsv", "zoom.tsv", "keep.tsv", "none.tsv", "end.json", "end.json.navstr", "snap.json", "script.log")


def near(a, b):
    return abs(a - b) <= 1e-9 * abs(b)


def parse(path):
    """Return (header, frames, bad): the '#' lines split on tabs; one dict per F line with the
    B lines it closes; every line out of the grammar as (line number, reason)."""
    header, frames, bad, pend, i = [], [], [], [], 0
    with open(path, encoding="ascii", errors="replace") as f:
        for i, raw in enumerate(f, 1):
            t = raw.rstrip("\n").split("\t")
            if t[0] == "#":
                if frames or pend:
                    bad.append((i, "header line after data"))
                header.append(t)
            elif t[0] == "B":
                if len(t) != 6:
                    bad.append((i, "B line with %d fields" % len(t)))
                    continue
                try:
                    pend.append((t[1], t[2], float(t[3]), float(t[4]), float(t[5])))
                except ValueError:
                    bad.append((i, "B line not numeric"))
            elif t[0] == "F":
                if len(t) != 7:
                    bad.append((i, "F line with %d fields" % len(t)))
                else:
                    try:
                        frames.append(dict(line=i, frame=int(t[1]), dt=int(t[2]), jd=float(t[3]), drawn=t[4],
                                           fov_old=float(t[5]), half_fov_new=float(t[6]), B=pend))
                    except ValueError:
                        bad.append((i, "F line not numeric"))
                pend = []
            else:
                bad.append((i, "unknown line type %r" % t[0][:12]))
    if pend:
        bad.append((i, "%d B line(s) after the last F line" % len(pend)))
    return header, frames, bad


def viewport(header):
    """Return {'projector': (cx, cy, r), 'renderer': (cx, cy, r), 'scissor': (ox, oy, w, h)} or None."""
    if len(header) < 4:
        return None
    t = header[3]
    if len(t) != 15 or t[:3] != ["#", "viewport", "projector"] or t[6] != "renderer" or t[10] != "scissor":
        return None
    try:
        return {"projector": tuple(float(v) for v in t[3:6]), "renderer": tuple(float(v) for v in t[7:10]),
                "scissor": tuple(float(v) for v in t[11:15])}
    except ValueError:
        return None


def series(frames):
    """(engine, name) -> [(frame index in the file, x, y, size)], and the (frame, engine, name) seen twice."""
    out, dup = {}, []
    for k, f in enumerate(frames):
        seen = set()
        for e, n, x, y, s in f["B"]:
            if (e, n) in seen:
                dup.append((f["frame"], e, n))
            seen.add((e, n))
            out.setdefault((e, n), []).append((k, x, y, s))
    return out, dup


def judge_file(tag, path, pin, names):
    """The per-file checks of the mandate; return the parsed frames (None when the file is absent)."""
    if not os.path.exists(path):
        check(tag + "_exists", False, path)
        return None, None
    header, frames, bad = parse(path)
    vp = viewport(header)
    check(tag + "_header", len(header) == 4 and header[:3] == HEADER and vp is not None,
          "%d '#' lines; first three %s 11.333 (d)'s tokens; viewport line %s" % (
              len(header), "=" if header[:3] == HEADER else "!=", vp))
    check(tag + "_grammar", not bad and bool(frames), "%d F lines, %d line(s) out of the grammar%s" % (
        len(frames), len(bad), (": " + "; ".join("line %d %s" % b for b in bad[:6])) if bad else ""))
    if not frames:
        return frames, vp
    idx = [f["frame"] for f in frames]
    breaks = [(a, b) for a, b in zip(idx, idx[1:]) if b != a + 1]
    check(tag + "_frames_consecutive", idx[0] == 0 and not breaks,
          "first %d last %d, breaks %s" % (idx[0], idx[-1], breaks[:6]))
    dts = [f["dt"] for f in frames]
    nonpos = [f["frame"] for f in frames if f["dt"] <= 0]
    fps = 1000.0 * len(dts) / sum(dts) if sum(dts) > 0 else 0.0
    check(tag + "_dt_positive", not nonpos, "dt min %d max %d; dt <= 0 at %s" % (min(dts), max(dts), nonpos[:6]))
    check(tag + "_fps_under_cap", 0.0 < fps <= 1.2 * FPS_CAP, "frames / sum(dt) = %d / %d ms = %.2f fps (cap %.0f; the cadence is the host's)" % (
        len(dts), sum(dts), fps, FPS_CAP))
    allb = [(f["frame"], b) for f in frames for b in f["B"]]
    unknown = sorted({b[1] for _, b in allb if b[1] not in names})
    check(tag + "_names", not unknown, "%d B lines, %d distinct names; not in ssystem.ini: %s" % (
        len(allb), len({b[1] for _, b in allb}), unknown))
    off = {}
    for f in frames:
        bound = {"o": math.pi / math.radians(f["fov_old"] / 2.0), "n": math.pi / f["half_fov_new"]}
        for e, n, x, y, s in f["B"]:
            if math.hypot(x, y) > bound[e] * (1.0 + XY_SLACK):
                m = off.setdefault((e, n), [0, 0.0])
                m[0] += 1
                m[1] = max(m[1], math.hypot(x, y) - bound[e])
    check(tag + "_pos_within_antipode", not off, "|pos| <= pi / half-fov of its engine; beyond: %s" % (
        ", ".join("%s %s %d lines, by %.6g" % (e, n, c, v) for (e, n), (c, v) in sorted(off.items())) or "none"))
    small = sorted({(e, n) for _, (e, n, x, y, s) in allb if not s > 0})
    check(tag + "_size_positive", not small, "size <= 0: %s" % small)
    ser, dup = series(frames)
    check(tag + "_unique_per_frame", not dup, "(frame, engine, name) twice: %s" % dup[:6])
    engines = {e for e, _ in ser}
    drawn = {f["drawn"] for f in frames}
    # Both engines compute on both pins (INTENT 11.334 (1)): the drawn field is the pin's, and at rest both engines
    # have bodies in the cone; a later phase may empty the old engine's cone (the zenith zoom), so only rest requires both.
    want = "n" if pin == "on" else "o"
    check(tag + "_drawn_is_the_pin", drawn == {want},
          "drawn field %s (expected %s); engines in B lines %s" % (sorted(drawn), want, sorted(engines)))
    if tag == "rest":
        missing = [f["frame"] for f in frames if not {"o", "n"} <= {b[0] for b in f["B"]}]
        check(tag + "_both_engines_every_frame", not missing,
              "frames lacking an o or an n line: %d %s" % (len(missing), missing[:6]))
    for e in sorted(engines):
        per = [sum(1 for b in f["B"] if b[0] == e) for f in frames]
        print("NOTE %s engine %s: %d bodies, %d-%d per frame: %s" % (
            tag, e, len([k for k in ser if k[0] == e]), min(per), max(per),
            " ".join(sorted(n for (ee, n) in ser if ee == e))), flush=True)
    return frames, vp


def judge_rest(tag, frames):
    """REST: every body of every engine still, frame to frame, at REST_TOL; present in every frame."""
    ser, _ = series(frames)
    n = len(frames)
    moving, partial = [], []
    for (e, name), pts in sorted(ser.items()):
        if len(pts) != n:
            partial.append("%s %s %d/%d" % (e, name, len(pts), n))
        d = [max(abs(b[i] - a[i]) for a, b in zip(pts, pts[1:])) if len(pts) > 1 else 0.0 for i in (1, 2, 3)]
        if max(d) > REST_TOL:
            moving.append("%s %s dx %.3g dy %.3g dsize %.3g" % (e, name, d[0], d[1], d[2]))
    check(tag + "_still", not moving, "max frame-to-frame |dx|,|dy|,|dsize| <= %g over %d bodies x engines; above: %s" % (
        REST_TOL, len(ser), "; ".join(moving)))
    check(tag + "_present_every_frame", not partial, "bodies not in every frame: %s" % "; ".join(partial))


def judge_zoom(tag, frames):
    """ZOOM: for a body present in every frame, size x its engine's fov constant, |pos| non-decreasing
    on every frame pair where that fov shrinks."""
    fov = {"o": [f["fov_old"] for f in frames], "n": [f["half_fov_new"] for f in frames]}
    span = {"o": (180.0, 20.0), "n": (math.pi / 2, math.pi / 18)}
    ok_span = all(abs(fov[e][0] / span[e][0] - 1) < 1e-3 and abs(fov[e][-1] / span[e][1] - 1) < 1e-3 for e in fov)
    check(tag + "_fov_span", ok_span, "first / last fov_old %.9g / %.9g deg, half_fov_new %.9g / %.9g rad (expected 180 / 20, pi/2 / pi/18)" % (
        fov["o"][0], fov["o"][-1], fov["n"][0], fov["n"][-1]))
    ser, _ = series(frames)
    n = len(frames)
    for e in sorted({k[0] for k in ser}):
        frozen = []
        for (ee, name), pts in sorted(ser.items()):
            pairs = [(a, b) for a, b in zip(pts, pts[1:]) if b[0] == a[0] + 1 and fov[e][b[0]] != fov[e][a[0]]]
            still = [1 for a, b in pairs if a[1:] == b[1:]]
            if ee == e and still:
                frozen.append("%s %d/%d" % (name, len(still), len(pairs)))
        check(tag + "_no_frozen_" + e, not frozen, "bodies whose x, y AND size stay bit-identical across a frame pair where "
              "their engine's fov changed (stale screen values), frozen pairs / fov-changing pairs: %s" % ", ".join(frozen))
        full = {name: pts for (ee, name), pts in ser.items() if ee == e and len(pts) == n}
        shrink = [k for k in range(n - 1) if fov[e][k + 1] < fov[e][k]]
        if not (full and shrink):
            # The whole-zoom law has no subject in this scene (no body of this engine stays in view throughout):
            # NOT REACHED, said as such; the law on each body's own present run is judged below regardless.
            print("NOTE %s_whole_zoom_%s not reached: %d bodies present in all %d frames, %d shrinking pairs" % (
                tag, e, len(full), n, len(shrink)), flush=True)
            full = {}
        rows, bad_c, bad_p = [], [], []
        for name, pts in sorted(full.items()):
            c = [p[3] * fov[e][p[0]] for p in pts]
            spread = max(c) / min(c) - 1.0 if min(c) > 0 else float("inf")
            r = [math.hypot(p[1], p[2]) for p in pts]
            dec = max([r[k] - r[k + 1] for k in shrink] or [0.0])
            rows.append("%s c %.6g spread %.3g |pos| %.6g->%.6g maxdec %.3g" % (name, c[0], spread, r[0], r[-1], dec))
            if spread > ZOOM_TOL:
                bad_c.append(name)
            if dec > 0:
                bad_p.append(name)
        if full:
            check(tag + "_size_x_fov_" + e, not bad_c, "spread <= %g; above: %s" % (ZOOM_TOL, bad_c))
            check(tag + "_pos_nondecreasing_" + e, not bad_p, "|pos| decreased while the fov shrank: %s" % bad_p)
        for row in rows:
            print("NOTE %s %s %s" % (tag, e, row), flush=True)
        span_rows, span_bad = [], []
        for (ee, name), pts in sorted(ser.items()):
            if ee != e:
                continue
            run = [pts[0]]
            for p in pts[1:] + [None]:
                if p is not None and p[0] == run[-1][0] + 1:
                    run.append(p)
                    continue
                pairs = [(a, b) for a, b in zip(run, run[1:]) if fov[e][b[0]] < fov[e][a[0]]]
                if len(pairs) >= 5 and not any(a[1:] == b[1:] for a, b in pairs):
                    c = [q[3] * fov[e][q[0]] for q in run]
                    spread = max(c) / min(c) - 1.0
                    dec = max(math.hypot(a[1], a[2]) - math.hypot(b[1], b[2]) for a, b in pairs)
                    span_rows.append("%s frames %d-%d (%d shrinking) c %.6g spread %.3g maxdec %.3g" % (
                        name, run[0][0], run[-1][0], len(pairs), c[0], spread, dec))
                    if spread > ZOOM_TOL or dec > 0:
                        span_bad.append(name)
                if p is not None:
                    run = [p]
        check(tag + "_law_on_spans_" + e, bool(span_rows) and not span_bad,
              "the same law on every body's own present run (>= 5 shrinking pairs, none frozen): %d runs, failing %s" % (
                  len(span_rows), span_bad))
        for row in span_rows:
            print("NOTE %s span %s %s" % (tag, e, row), flush=True)


def ring(end_path):
    """The sentinel dump's header: (gates, seam) or (None, None)."""
    if not os.path.exists(end_path):
        return None, None
    with open(end_path, encoding="latin-1") as f:
        first = f.readline()
    try:
        hdr = json.loads(_clean(first))
    except Exception as e:
        print("NOTE end.json header does not parse: %s" % e, flush=True)
        return None, None
    return hdr.get("gates"), hdr.get("seam")


def judge_cost(rest, keep_last_frame_fov, seam):
    """The record's own frames against the seam ring: a contiguous run of it (the cross-check), and the
    rest window ON against the record-OFF window at the same fov (the cost ratio)."""
    if not seam or not seam.get("armed"):
        print("NOTE cost and ring cross-check NOT REACHED: the seam ring is not armed (SC_SEAM_RECORD unset)", flush=True)
        return
    steps = seam["steps"]
    key = [(f["dt"], f["jd"], f["fov_old"], f["half_fov_new"]) for f in rest]
    rk = [(s["dt"], s["jd"], s["fovOld"], s["halfFovNew"]) for s in steps]
    hit = [i for i in range(len(rk) - len(key) + 1) if rk[i] == key[0] and rk[i:i + len(key)] == key]
    check("ring_crosscheck", len(hit) == 1, "rest.tsv's %d F lines (dt, jd, fov_old, half_fov_new) found as a contiguous "
          "run of the ring's %d steps at %s (ring frames %s)" % (len(key), len(rk), hit[:3],
          [steps[i]["frame"] for i in hit[:3]]))
    tail = len(steps)
    while tail > 0 and near(steps[tail - 1]["fovOld"], 180.0):
        tail -= 1
    settle, start = 0, tail
    while start < len(steps) and settle < 1000:
        settle += steps[start]["dt"]
        start += 1
    off = steps[start:len(steps) - 3]
    on = rest[2:-2]
    if sum(s["dt"] for s in off) < 1500 or sum(f["dt"] for f in on) < 1500:
        check("cost_ratio", False, "windows under 1.5 s: on %d frames, off %d frames" % (len(on), len(off)))
        return
    fps_on = 1000.0 * len(on) / sum(f["dt"] for f in on)
    fps_off = 1000.0 * len(off) / sum(s["dt"] for s in off)
    check("cost_ratio", fps_on / fps_off >= 0.9, "fps record ON (rest.tsv, fov 180) %.3f over %d frames / record OFF (ring tail, "
          "fov 180, preceded by fov %s) %.3f over %d frames = %.4f (P10: >= 0.9)" % (
              fps_on, len(on), keep_last_frame_fov, fps_off, len(off), fps_on / fps_off))
    print("NOTE cost: per-frame period ON %.3f ms / OFF %.3f ms against D11's 1 ms; the cadence is capped at %g fps, so a "
          "ratio near 1 bounds the record's cost by the cap's slack, it does not measure it" % (
              1000.0 / fps_on, 1000.0 / fps_off, FPS_CAP), flush=True)


def field_names():
    names = set()
    with open(os.path.expanduser("~/.spacecrafter/ssystem.ini"), encoding="latin-1") as f:
        for ln in f:
            k, _, v = ln.partition("=")
            if k.strip() == "name":
                names.add(v.strip())
    return names


def judge(out, pin):
    names = field_names()
    print("NOTE field: %d body names in ~/.spacecrafter/ssystem.ini" % len(names), flush=True)
    snap = json.load(open(os.path.join(out, "snap.json"))) if os.path.exists(os.path.join(out, "snap.json")) else {}
    rest, vp = judge_file("rest", os.path.join(out, "rest.tsv"), pin, names)
    zoom, _ = judge_file("zoom", os.path.join(out, "zoom.tsv"), pin, names)
    keep, _ = judge_file("keep", os.path.join(out, "keep.tsv"), pin, names)
    check("refused_file_absent", not os.path.exists(os.path.join(out, "none.tsv")), "none.tsv must not exist")
    if rest:
        judge_rest("rest", rest)
    if zoom:
        judge_zoom("zoom", zoom)
    gates, seam = ring(os.path.join(out, "end.json"))
    check("sentinel", gates is not None, "end.json header parsed, gates %s" % gates)
    if gates and vp:
        radii = (gates.get("viewportRadius"), vp["renderer"][2], vp["projector"][2])
        centers = (vp["projector"][:2], vp["renderer"][:2])
        r = radii[0]
        check("viewport_agree", r is not None and all(x == r for x in radii) and all(c == (r, r) for c in centers)
              and vp["scissor"] == (0.0, 0.0, 2.0 * r, 2.0 * r),
              "sentinel / renderer / projector radius %s, centers %s, scissor %s: one disc, centered, filling the render rect" % (
                  radii, centers, vp["scissor"]))
        print("NOTE viewport radius sentinel / renderer / projector = %s, centers projector / renderer = %s, scissor %s "
              "(the render rect is the host's, not the config's screen_w: 2048 here for screen_w 1024)" % (radii, centers, vp["scissor"]), flush=True)
    logtxt = open(os.path.join(out, "script.log"), encoding="latin-1").read() if os.path.exists(os.path.join(out, "script.log")) else ""
    counts = {k: logtxt.count(m) for k, m in MSG.items()}
    unopenable = "no_open" in snap.get("sent", [])
    check("refusals_logged", counts["no_state"] >= 1 and counts["bad_value"] >= 1 and counts["no_filename"] >= 2
          and (counts["no_open"] >= 2 or not unopenable),
          "script.log occurrences %s (expected no_state >= 1, no_filename >= 2 and no_open >= 2 with the keep phase's, "
          "bad_value >= 1 %s)" % (counts, "(sent)" if unopenable else "(NOT sent by this run's drive)"))
    if unopenable:
        check("unopenable_creates_nothing", not os.path.exists(os.path.join(out, "no_such_dir")),
              "the engine did not create the missing directory")
    for m in MSG.values():
        for ln in logtxt.splitlines():
            if m in ln:
                print("NOTE log line: %s" % ln.strip()[:240], flush=True)
                break
    if snap:
        check("record_untouched_by_refusals", snap.get("before") == snap.get("after"),
              "rest.tsv / zoom.tsv (size, mtime) before %s after %s the three refusals" % (snap.get("before"), snap.get("after")))
    if keep:
        fovs = [f["fov_old"] for f in keep]
        first90 = next((k for k, v in enumerate(fovs) if near(v, 90.0)), None)
        ok = (first90 is not None and first90 > 0 and len(set(fovs[:first90])) == 1
              and all(near(v, 90.0) for v in fovs[first90:]))
        check("record_runs_across_refusal", ok, "keep.tsv: %d frames at fov %s then %s at fov 90 - the fov 90 command came "
              "AFTER the two refused `state on` (missing filename, unopenable file)" % (first90 or 0, sorted(set(fovs[:first90 or 0])),
                                                (len(fovs) - first90) if first90 is not None else 0))
    if rest:
        judge_cost(rest, 90.0, seam)


def drive(out, pin):
    for f in FILES:
        if os.path.exists(os.path.join(out, f)):
            os.remove(os.path.join(out, f))
    stat = lambda: {f: [os.path.getsize(os.path.join(out, f)), os.path.getmtime(os.path.join(out, f))]
                    for f in ("rest.tsv", "zoom.tsv") if os.path.exists(os.path.join(out, f))}
    S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
    send(S, "date jday 2461233.5", 1)
    send(S, "timerate rate 0", 1)
    send(S, "flag experimental_path " + pin, 1)
    send(S, "zoom fov 180 duration 0", 2)
    send(S, "body action screen_record state on filename %s/rest.tsv" % out, 3)
    send(S, "body action screen_record state on filename %s/zoom.tsv" % out, 0.6)
    send(S, "zoom fov 20 duration 3", 4)
    send(S, "body action screen_record state off", 1)
    snap = {"before": stat()}
    send(S, "body action screen_record", 1)
    send(S, "body action screen_record state on", 1)
    send(S, "body action screen_record state maybe filename %s/none.tsv" % out, 1)
    send(S, "body action screen_record state on filename %s/no_such_dir/none.tsv" % out, 1)
    snap["sent"] = ["no_state", "no_filename", "bad_value", "no_open"]
    snap["after"] = stat()
    send(S, "body action screen_record state on filename %s/keep.tsv" % out, 1)
    send(S, "body action screen_record state on", 1)
    send(S, "body action screen_record state on filename %s/no_such_dir/none2.tsv" % out, 1)
    send(S, "zoom fov 90 duration 0", 1)
    send(S, "body action screen_record state off", 1)
    send(S, "zoom fov 180 duration 0", 4)
    send(S, "body action dual_dump filename %s/end.json" % out, 0.6)
    end, size, t0 = os.path.join(out, "end.json"), -1, time.time()
    while time.time() - t0 < 60:
        cur = os.path.getsize(end) if os.path.exists(end) else -1
        if cur > 0 and cur == size:
            break
        size = cur
        time.sleep(1.0)
    print("NOTE sentinel end.json %d bytes after %.1f s" % (size, time.time() - t0), flush=True)
    with open(os.path.join(out, "script.log"), "w", encoding="latin-1") as f:
        f.write(logread.text(os.path.expanduser("~/.spacecrafter/log"), "script"))
    json.dump(snap, open(os.path.join(out, "snap.json"), "w"))
    send(S, "shutdown action now", 1)


def self_test():
    """A synthetic record per 11.333 (d) with one skipped frame index, one 5-field B line and one jump
    at rest; its clean twin must pass the same checks. Exit 1 when each defect is named."""
    import tempfile
    d = tempfile.mkdtemp(prefix="f160_selftest_")
    head = ("#\tscreen_record\t1\n#\tB\tengine(o|n)\tname\tx\ty\tsize\n"
            "#\tF\tframe\tdt_ms\tjd\tdrawn(o|n)\tfov_old_deg\thalf_fov_new_rad\n"
            "#\tviewport\tprojector\t512\t512\t512\trenderer\t512\t512\t512\tscissor\t0\t0\t1024\t1024\n")

    def write(name, skip, five, jump):
        lines = [head]
        for k in range(200):
            if skip and k == 50:
                continue
            for e in "on":
                x = 0.25 + (1e-3 if (jump and e == "n" and k >= 120) else 0.0)
                lines.append("B\t%s\tSaturn\t%.9g\t-0.125\t0.00123456789\n" % (e, x))
                lines.append("B\t%s\tJupiter\t-0.5\t0.375\t0.0123456789\n" % e)
            if five and k == 80:
                lines.append("B\tn\tMars\t0.1\t0.2\n")
            lines.append("F\t%d\t7\t2461233.5\to\t180\t1.5707963267948966\n" % k)
        p = os.path.join(d, name)
        open(p, "w").write("".join(lines))
        return p

    def write_zoom(name, frozen):
        lines = [head]
        for k in range(100):
            u = min(max((k - 10) / 60.0, 0.0), 1.0)
            fo, hf = 180.0 - 160.0 * u, math.pi / 2 - (math.pi / 2 - math.pi / 18) * u
            for e, fe, f0 in (("o", fo, 180.0), ("n", hf, math.pi / 2)):
                lines.append("B\t%s\tSaturn\t%.9g\t0\t%.9g\n" % (e, 0.1 * f0 / fe, 0.001 * f0 / fe))
                g = 1.0 if (frozen and e == "n") else f0 / fe
                lines.append("B\t%s\tJupiter\t%.9g\t%.9g\t%.9g\n" % (e, 0.1 * g, 0.0625 * g, 1e-4 * g))
            lines.append("F\t%d\t7\t2461233.5\to\t%.17g\t%.17g\n" % (k, fo, hf))
        p = os.path.join(d, name)
        open(p, "w").write("".join(lines))
        return p

    names = {"Saturn", "Jupiter", "Mars"}
    print("--- the clean twin ---", flush=True)
    n0 = len(FAILS)
    fr, _ = judge_file("clean", write("clean.tsv", False, False, False), "off", names)
    judge_rest("clean", fr)
    fr, _ = judge_file("czoom", write_zoom("czoom.tsv", False), "off", names)
    judge_zoom("czoom", fr)
    clean_ok = len(FAILS) == n0
    print("--- the planted record ---", flush=True)
    fr, _ = judge_file("planted", write("planted.tsv", True, True, True), "off", names)
    judge_rest("planted", fr)
    fr, _ = judge_file("pzoom", write_zoom("pzoom.tsv", True), "off", names)
    judge_zoom("pzoom", fr)
    named = {"skip": "planted_frames_consecutive" in FAILS, "five_fields": "planted_grammar" in FAILS,
             "jump": "planted_still" in FAILS, "frozen": "pzoom_no_frozen_n" in FAILS}
    for f in os.listdir(d):
        os.remove(os.path.join(d, f))
    os.rmdir(d)
    ok = clean_ok and all(named.values())
    print("SELF-TEST: clean twin all PASS: %s; planted defects named: %s -> %s" % (
        clean_ok, named, "exit 1 (the leg fails on each)" if ok else "exit 2 (BLIND or BROKEN)"), flush=True)
    sys.exit(1 if ok else 2)


if __name__ == "__main__":
    if sys.argv[1:] == ["--self-test"]:
        self_test()
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--pin", required=True, choices=["on", "off"])
    ap.add_argument("--judge", action="store_true")
    A = ap.parse_args()
    OUT = os.path.realpath(A.out)
    print("=== F160 leg, pin %s, out %s, %s ===" % (A.pin, OUT, "judge only" if A.judge else "drive + judge"), flush=True)
    if not A.judge:
        drive(OUT, A.pin)
    judge(OUT, A.pin)
    print("=== %d FAIL: %s ===" % (len(FAILS), FAILS), flush=True)
    sys.exit(1 if FAILS else 0)
