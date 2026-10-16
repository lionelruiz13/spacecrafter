#!/usr/bin/env python3
"""screen_record.py - read the per-frame screen record (format: INTENT 11.333 (d)) and exploit it (F162, INTENT 11.335).

    python3 -I screen_record.py summary <rec>
    python3 -I screen_record.py motion  <rec> [--engine o|n] [--k 8] [--floor 1e-6] [--top 10] [--all OUT.tsv|-]
    python3 -I screen_record.py within  <rec> [--all OUT.tsv|-]
    python3 -I screen_record.py compare <recA> <recB> [--engine-a n] [--engine-b o] [--align time|frame|jd] [--all OUT.tsv|-]
    python3 -I screen_record.py --self-test

The record stays raw; only these tables summarise it, and no row is dropped on its value: every body is a
row, ordered worst first, its frame / jd / fovs / drawn tag beside its delta. A cut by RANK (motion's step
lists, `--top`) prints how many rows it shows of how many; `--all` writes every row raw (`-` = stdout).
Units in the column names: [r] the viewport radius (the record's unit), [px] through the header's renderer
radius, [ms], [d] (jd), [deg] / [rad] (each engine's own fov unit). Two runs align on the engine's own time
base, the prefix sum of dt_ms (`--align time`: the nearest frame, within the larger dt), on jd, the sky's clock when
time runs (`--align jd`: the nearest frame, within the larger jd step), or on the index; dpos_w1 = the delta
minimized over B's frames j-1, j, j+1 (a command boundary puts two runs one frame apart).
load() refuses a malformed line LOUD with its line number (CLI exit 1); it never skips one.
"""
import argparse, bisect, collections, itertools, math, os, statistics, sys, tempfile

MAGIC = ["#", "screen_record", "1"]
B_COLS = ["engine", "name", "x", "y", "size"]
F_COLS = ["frame", "dt_ms", "jd", "drawn", "fov_old_deg", "half_fov_new_rad"]
ENGINES = ("o", "n")
P17 = ("jd", "elapsed", "fov", "dt")        # columns printed at the record's 17 digits; the rest at 9
KV = ["quantity", "value"]

Record = collections.namedtuple("Record", "path header frames")
Frame = collections.namedtuple("Frame", "frame dt jd drawn fov_old_deg half_fov_new_rad bodies line")


class Refused(Exception):
    """A line the format does not allow: the file, the line number, the reason."""
    def __init__(self, path, line, why):
        super().__init__("%s:%d: %s" % (path, line, why))
        self.path, self.line, self.why = path, line, why


def _declares(toks, want):
    """Tell whether a `#` line declares the columns `want`: first word of each field, `(o|n)` cut; more = annotation."""
    return [(t.split() or [""])[0].split("(")[0] for t in toks[:len(want)]] == want


def load(path):
    """Read a record -> Record(path, header, frames); bodies = {(engine, name): (x, y, size)} per frame."""
    header = {"version": 1, "B": None, "F": None, "viewport": None, "extra": []}
    frames, pending, first_pending, no = [], {}, 0, 0

    def num(tok, what):
        try:
            return float(tok)
        except ValueError:
            raise Refused(path, no, "%s %r is not a number" % (what, tok))
    with open(path, encoding="latin-1", newline="\n") as fh:
        for no, raw in enumerate(fh, 1):
            text = raw.rstrip("\n")
            t = text.split("\t")
            if no == 1:
                if t != MAGIC:
                    raise Refused(path, no, "first line %r is not '#<tab>screen_record<tab>1'" % text)
                continue
            if t[0] == "#":
                key = t[1] if len(t) > 1 else ""
                if frames or pending:
                    raise Refused(path, no, "header line after the first B / F line")
                if key in ("B", "F", "viewport") and header[key] is not None:
                    raise Refused(path, no, "a second '# %s' line" % key)
                if key in ("B", "F"):
                    want = B_COLS if key == "B" else F_COLS
                    if not _declares(t[2:], want):
                        raise Refused(path, no, "'# %s' declares %r, the format has %r" % (key, t[2:2 + len(want)], want))
                    header[key] = t[2:]
                elif key == "viewport":
                    if len(t) < 15 or (t[2], t[6], t[10]) != ("projector", "renderer", "scissor"):
                        raise Refused(path, no, "viewport line is not 'projector cx cy r renderer cx cy r scissor ox oy w h'")
                    v = [num(x, "viewport value") for x in t[3:6] + t[7:10] + t[11:15]]
                    header["viewport"] = {"projector": v[0:3], "renderer": v[3:6], "scissor": v[6:10], "annotation": t[15:]}
                else:
                    header["extra"].append(text)
                continue
            missing = [k for k in ("B", "F", "viewport") if header[k] is None]
            if missing:
                raise Refused(path, no, "data line before the header's %s line" % " / ".join(missing))
            if t[0] == "B":
                if len(t) != 6:
                    raise Refused(path, no, "B line has %d fields, 6 expected (B engine name x y size)" % len(t))
                if t[1] not in ENGINES:
                    raise Refused(path, no, "engine tag %r, 'o' or 'n' expected" % t[1])
                if not t[2] or (t[1], t[2]) in pending:
                    raise Refused(path, no, "body name %r empty or twice in one frame" % t[2])
                first_pending = first_pending or no
                pending[(t[1], t[2])] = tuple(num(x, c) for x, c in zip(t[3:], B_COLS[2:]))
            elif t[0] == "F":
                if len(t) != 7:
                    raise Refused(path, no, "F line has %d fields, 7 expected (F frame dt_ms jd drawn fov_old_deg half_fov_new_rad)" % len(t))
                if t[1] != str(len(frames)):
                    raise Refused(path, no, "frame index %r, %d expected (consecutive from 0)" % (t[1], len(frames)))
                if t[4] not in ENGINES:
                    raise Refused(path, no, "drawn tag %r, 'o' or 'n' expected" % t[4])
                dt, jd, fo, fn = (num(t[i], F_COLS[i - 1]) for i in (2, 3, 5, 6))
                frames.append(Frame(len(frames), dt, jd, t[4], fo, fn, pending, no))
                pending, first_pending = {}, 0
            else:
                raise Refused(path, no, "line type %r, '#', 'B' or 'F' expected" % t[0])
    if no == 0:
        raise Refused(path, 1, "empty file")
    if pending:
        raise Refused(path, first_pending, "B line after the file's last F line (its frame is never closed)")
    missing = [k for k in ("B", "F", "viewport") if header[k] is None]
    if missing:
        raise Refused(path, no, "header without its %s line" % " / ".join(missing))
    return Record(path, header, frames)


def elapsed(rec):
    """Return the engine's own time base per frame: the prefix sum of dt_ms, frame f's dt included."""
    return list(itertools.accumulate(f.dt for f in rec.frames))


def _rank(v):
    """Sort key, worst first: NaN, then the largest."""
    return (0, 0.0) if math.isnan(v) else (1, -v)


def _ratio(a, b):
    return a / b if b else (math.nan if a == 0 else math.copysign(math.inf, a))


def _rel(v, m):
    """Return a step over its median; a zero step is 0 whatever the median."""
    return 0.0 if v == 0 else _ratio(v, m)


def _median(vals):
    fin = [v for v in vals if math.isfinite(v)]
    return statistics.median(fin) if fin else math.nan


def _span(vals):
    """Return (min, max), NaN as soon as one value is NaN (min / max would order it at random)."""
    return (math.nan, math.nan) if not vals or any(map(math.isnan, vals)) else (min(vals), max(vals))


def _ranges(idx):
    """Compress [0, 1, 2, 5] -> '0-2,5': every frame, none dropped."""
    out = []
    for _, g in itertools.groupby(enumerate(idx), lambda p: p[1] - p[0]):
        g = [v for _, v in g]
        out.append(str(g[0]) if len(g) == 1 else "%d-%d" % (g[0], g[-1]))
    return ",".join(out) or "-"


def _ctx_cols(s=""):
    return ["frame" + s, "jd%s[d]" % s, "drawn" + s, "fov_old%s[deg]" % s, "half_fov_new%s[rad]" % s, "dt%s[ms]" % s]


def _ctx(f):
    return (f.frame, f.jd, f.drawn, f.fov_old_deg, f.half_fov_new_rad, f.dt)


def _side(f, engine):
    return {n: v for (e, n), v in f.bodies.items() if e == engine}


def _vp(v):
    return " | ".join([k + " " + " ".join("%.9g" % x for x in v[k]) for k in ("projector", "renderer", "scissor")]
                      + ([" ".join(v["annotation"])] if v["annotation"] else []))


def summary(rec):
    fr, el = rec.frames, elapsed(rec)
    dur = el[-1] if el else 0.0
    q = [("frames", len(fr)), ("duration[ms]", dur), ("fps[1/s]", _ratio(1000.0 * len(fr), dur))]
    for name, vals in (("dt[ms]", [f.dt for f in fr]), ("jd[d]", [f.jd for f in fr]),
                       ("fov_old[deg]", [f.fov_old_deg for f in fr]), ("half_fov_new[rad]", [f.half_fov_new_rad for f in fr])):
        lo, hi = _span(vals)
        q += [(name + " min", lo), (name + " max", hi)]
    q += [("frames drawn %s" % e, sum(f.drawn == e for f in fr)) for e in ENGINES]
    q += [("bodies %s" % e, len({n for f in fr for (g, n) in f.bodies if g == e})) for e in ENGINES]
    q += [("B lines %s" % e, sum(g == e for f in fr for (g, n) in f.bodies)) for e in ENGINES]
    q += [("viewport", _vp(rec.header["viewport"]))]
    q += [("header extra", x) for x in rec.header["extra"]]
    per = collections.defaultdict(list)
    for f in fr:
        for k, v in f.bodies.items():
            per[k].append((f.frame, v))
    rows = []
    for (e, n), s in sorted(per.items()):
        xs, ys, ss = zip(*(v for _, v in s))
        rows.append((e, n, len(s), s[0][0], s[-1][0]) + _span(xs) + _span(ys) + _span(ss))
    return [("record " + rec.path, KV, q),
            ("bodies, by engine and name", ["engine", "body", "frames_seen", "first", "last", "x_min[r]", "x_max[r]",
                                            "y_min[r]", "y_max[r]", "size_min[r]", "size_max[r]"], rows)]


STEP_COLS = ["engine", "body", "frame", "step[r]", "dx[r]", "dy[r]", "dsize[r]", "x[r]", "y[r]", "size[r]",
             "step/median", "|dsize|/median", "flag"] + _ctx_cols()[1:]


def motion(rec, engine=None, k=8.0, top=10, floor=1e-6):
    """Per body the frame-to-frame step and size step; JUMP = a SPIKE: step > floor and > k x the larger of its two
    neighbouring steps (a transition's own motion has comparable neighbours, a discontinuity has not); '-' jumps when
    no step is above the floor; SIZE-JUMP = the same on |dsize| with the floor relative to the size (orders, hides nothing)."""
    tables, every = [], []
    flag_order = {"NONFINITE": 0, "JUMP": 1, "": 2}
    for e in [engine] if engine else ENGINES:
        per = collections.defaultdict(list)
        for f in rec.frames:
            for n, v in _side(f, e).items():
                per[n].append((f, v))
        brows, plist, slist = [], [], []
        for name, s in sorted(per.items()):
            st = [(f1, math.hypot(v1[0] - v0[0], v1[1] - v0[1]), v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2], v1)
                  for (f0, v0), (f1, v1) in zip(s, s[1:]) if f1.frame == f0.frame + 1]
            med, meds = _median([x[1] for x in st if x[1]]), _median([abs(x[4]) for x in st])
            dl, sl = [x[1] for x in st], [abs(x[4]) for x in st]

            def spike(vals, i, fl):
                """NONFINITE / JUMP / '': a step above the floor and k x the larger finite neighbouring step."""
                if not math.isfinite(vals[i]):
                    return "NONFINITE"
                nb = max([vals[j] for j in (i - 1, i + 1) if 0 <= j < len(vals) and math.isfinite(vals[j])] or [0.0])
                return "JUMP" if vals[i] > fl and vals[i] > k * nb else ""
            rows = [(e, name, f.frame, d, dx, dy, ds, v[0], v[1], v[2], _rel(d, med), _rel(abs(ds), meds),
                     spike(dl, i, floor), spike(sl, i, floor * abs(v[2]))) + _ctx(f)[1:]
                    for i, (f, d, dx, dy, ds, v) in enumerate(st)]
            every += [r[:12] + ((r[12] + " SIZE-" + r[13]).strip() if r[13] else r[12],) + r[14:] for r in rows]
            seen = {f.frame for f, _ in s}
            absent = [i for i in range(s[0][0].frame, s[-1][0].frame + 1) if i not in seen]
            wp = min(rows, key=lambda r: _rank(r[3]), default=None)
            ws = min(rows, key=lambda r: _rank(abs(r[6])), default=None)
            jumps = sum(r[12] != "" for r in rows) if any(not r[3] <= floor for r in rows) else "-"
            brows.append((name, len(s), s[0][0].frame, s[-1][0].frame, len(absent), _ranges(absent), len(rows), med if med == med else "-",
                          wp[3] if wp else math.nan, wp[10] if wp else math.nan, jumps, meds,
                          abs(ws[6]) if ws else math.nan, ws[2] if ws else "-", ws[11] if ws else math.nan,
                          sum(r[13] != "" for r in rows)) + (_ctx(rec.frames[wp[2]]) if wp else ("-",) * 6))
            cut = top or None
            plist += sorted(rows, key=lambda r: _rank(r[3]))[:cut]
            slist += sorted(rows, key=lambda r: _rank(abs(r[6])))[:cut]
        brows.sort(key=lambda r: (_rank(r[9]), r[0]))
        plist.sort(key=lambda r: (flag_order[r[12]], _rank(r[10]), r[1], r[2]))
        slist.sort(key=lambda r: (flag_order[r[13]], _rank(r[11]), r[1], r[2]))
        n_steps = sum(r[6] for r in brows)
        tables.append(("motion per body, engine %s (k=%g, floor=%g r; ordered by max_step/median)" % (e, k, floor),
                       ["body", "frames_seen", "first", "last", "absent", "absent_frames", "steps", "median_nonzero_step[r]",
                        "max_step[r]", "max_step/median", "jumps(spike>%g r)" % floor, "median_|dsize|[r]", "max_|dsize|[r]",
                        "max_|dsize|_frame", "max_|dsize|/median", "size_jumps"] + _ctx_cols("@max_step"), brows))
        for title, lst, fl in (("position steps", plist, 12), ("size steps", slist, 13)):
            tables.append(("%s, engine %s: the %s largest per body, %d of %d steps shown (--top 0 = all; --all = every step)"
                           % (title, e, top or "all", len(lst), n_steps),
                           STEP_COLS, [r[:12] + (r[fl],) + r[14:] for r in lst]))
    return tables, STEP_COLS, every


def _diff(pairs, radius, la, lb, single):
    """Per body over frame pairs (fa, ta, A, fb, tb, B, near): the A - B delta, its worst, the bodies on one side only;
    near = B's frames j, j-1, j+1 as (frame, side) for compare (dpos_w1 = the delta minimized over them), None for within."""
    ctx = _ctx_cols() + ["elapsed[ms]"] if single else (
        _ctx_cols("_" + la) + ["elapsed_%s[ms]" % la] + _ctx_cols("_" + lb) + ["elapsed_%s[ms]" % lb])
    w1 = [] if single else ["dpos_w1[r]", "frame_%s_w1" % lb]
    cols = ["body", "dx[r]", "dy[r]", "dpos[r]", "dpos[px]"] + w1 + ["size_%s/size_%s" % (la, lb),
            "x_%s[r]" % la, "y_%s[r]" % la, "size_%s[r]" % la, "x_%s[r]" % lb, "y_%s[r]" % lb, "size_%s[r]" % lb] + ctx
    every, worst, wworst, both = [], {}, {}, collections.Counter()
    ratios, only = collections.defaultdict(list), collections.defaultdict(list)
    for fa, ta, A, fb, tb, B, near in pairs:
        c = _ctx(fa) + (ta,) + (() if single else _ctx(fb) + (tb,))
        for n in sorted(set(A) | set(B)):
            if n in A and n in B:
                (xa, ya, sa), (xb, yb, sb) = A[n], B[n]
                d = math.hypot(xa - xb, ya - yb)
                w = () if single else max(((math.hypot(xa - s[n][0], ya - s[n][1]), f) for f, s in near if n in s),
                                          key=lambda p: _rank(p[0]))
                q = _ratio(sa, sb)
                row = (n, xa - xb, ya - yb, d, d * radius) + w + (q, xa, ya, sa, xb, yb, sb) + c
                both[n] += 1
                ratios[n].append(q)
                if n not in worst or _rank(d) < _rank(worst[n][3]):
                    worst[n] = row
                if w and (n not in wworst or _rank(w[0]) < _rank(wworst[n][0])):
                    wworst[n] = (w[0], fa.frame, w[1])
            else:
                side = la if n in A else lb
                x, y, s = A.get(n) or B.get(n)
                row = (n,) + ("",) * (5 + len(w1)) + ((x, y, s, "", "", "") if side == la else ("", "", "", x, y, s)) + c
                only[(n, side)].append(fa.frame)
            every.append(row)
    head = ["body", "frames_both", "frames_%s_only" % la, "frames_%s_only" % lb, "size_ratio_min", "size_ratio_max"] + (
        [] if single else ["worst_dpos_w1[r]", "frame_%s@worst_w1" % la, "frame_%s@worst_w1" % lb])
    rows = []
    for n in sorted(set(both) | {n for n, _ in only}):
        w = worst.get(n)
        lo, hi = _span(ratios[n])
        rows.append((n, both[n], len(only.get((n, la), ())), len(only.get((n, lb), ())), lo, hi)
                    + (() if single else wworst.get(n, ("-",) * 3)) + (w[1:] if w else ("-",) * (len(cols) - 1)))
    ip = len(head) + 2
    rows.sort(key=lambda r: (_rank(r[ip]) if isinstance(r[ip], float) else (2, 0), r[0]))
    orows = sorted(((n, s, len(fr), _ranges(fr)) for (n, s), fr in only.items()), key=lambda r: (-r[2], r[0], r[1]))
    frame_key = "frame" if single else "frame_" + la
    return [("per body, %s - %s (ordered by the worst dpos; the row's values are the worst frame's)%s" % (la, lb, "" if single else
             "; dpos_w1 = the delta minimized over %s's frames j-1, j, j+1, worst_dpos_w1 = its worst over the frames" % lb),
             head + cols[1:], rows),
            ("bodies on one side only (frames = %s)" % frame_key, ["body", "only_in", "frames", "frames_list"], orows)], cols, every


def within(rec):
    """Return, per frame per body, the o - n delta of one record (both engines informed: an old-pinned run)."""
    el = elapsed(rec)
    pairs = [(f, t, _side(f, "o"), f, t, _side(f, "n"), None) for f, t in zip(rec.frames, el)]
    q = [("record", rec.path), ("frames", len(pairs))]
    q += [("frames with %s lines" % e, sum(bool(_side(f, e)) for f in rec.frames)) for e in ENGINES]
    if not any(p[2] for p in pairs) or not any(p[5] for p in pairs):
        q.append(("note", "one engine has no line: the within-run comparison is empty (a new-pinned run, or after B42)"))
    tables, cols, every = _diff(pairs, rec.header["viewport"]["renderer"][2], "o", "n", True)
    return [("within " + rec.path, KV, q)] + tables, cols, every


def _steps(v):
    """Return per frame the key's step into it (frame 0: its step out), the jd alignment's tolerance."""
    d = [abs(b - a) for a, b in zip(v, v[1:])]
    return d[:1] + d if d else [0.0] * len(v)


def align(ra, rb, mode="time"):
    """Pair each frame of A with one of B -> [(i, j or None)]: by elapsed engine time or by jd (nearest, ties to the
    earlier, within the larger dt / jd step at the two frames) or by index."""
    if mode == "frame":
        return [(i, i if i < len(rb.frames) else None) for i in range(len(ra.frames))]
    if mode == "jd":
        (ka, sa), (kb, sb) = (([f.jd for f in r.frames], _steps([f.jd for f in r.frames])) for r in (ra, rb))
    else:
        (ka, sa), (kb, sb) = ((elapsed(r), [f.dt for f in r.frames]) for r in (ra, rb))
    for r, e in ((ra, ka), (rb, kb)):
        bad = [i for i in range(len(e)) if not math.isfinite(e[i]) or (mode == "time" and i and not e[i] >= e[i - 1])]
        if bad:
            raise ValueError("%s: %s at frame %d: %s alignment undefined, use --align %s" % (r.path, "jd not finite" if mode == "jd"
                             else "elapsed time not finite or decreasing", bad[0], mode, "time" if mode == "jd" else "frame"))
        if mode == "jd" and len(set(e)) < 2:
            raise ValueError("%s: jd constant: jd alignment undefined, use --align time" % r.path)
    order = sorted(range(len(kb)), key=lambda c: (kb[c], c))
    keys, out = [kb[c] for c in order], []
    for i, t in enumerate(ka):
        p = bisect.bisect_left(keys, t)
        c = min((order[bisect.bisect_left(keys, keys[q])] for q in (p - 1, p) if 0 <= q < len(keys)),
                key=lambda c: (abs(kb[c] - t), c), default=None)
        if c is not None and abs(kb[c] - t) > max(sa[i], sb[c]):
            c = None
        out.append((i, c))
    return out


def compare(ra, rb, engine_a="n", engine_b="o", mode="time"):
    """Return run against run: A's engine_a against B's engine_b over the aligned frames; jd constant under
    `--align jd` falls back to time, said in the header."""
    span = [hi - lo for lo, hi in (_span([f.jd for f in r.frames]) for r in (ra, rb))]
    const = [p for p, s in zip("AB", span) if s == 0]
    used = "time" if mode == "jd" and const else mode
    m = align(ra, rb, used)
    ta, tb = elapsed(ra), elapsed(rb)
    sb = [_side(f, engine_b) for f in rb.frames]
    pairs = [(ra.frames[i], ta[i], _side(ra.frames[i], engine_a), rb.frames[j], tb[j], sb[j],
              [(c, sb[c]) for c in (j, j - 1, j + 1) if 0 <= c < len(sb)]) for i, j in m if j is not None]
    va, vb = ra.header["viewport"], rb.header["viewport"]
    q = [("A", ra.path), ("B", rb.path), ("engine A / B", "%s / %s" % (engine_a, engine_b)),
         ("align", used if used == mode else "time (FALLBACK: --align jd asked, jd constant)"),
         ("jd varies A / B (max - min [d])", " / ".join(("yes " if s > 0 else "no " if s == 0 else "") + "%.9g" % s for s in span)),
         ("frames A / B", "%d / %d" % (len(ra.frames), len(rb.frames))), ("aligned pairs", len(pairs)),
         ("A frames without a B frame", _ranges([i for i, j in m if j is None])),
         ("B frames no A frame aligned to", _ranges(sorted(set(range(len(rb.frames))) - {j for _, j in m}))),
         ("viewport A", _vp(va)), ("viewport B", _vp(vb)),
         ("viewport A vs B", "identical" if va == vb else "DIFFER - dpos[px] is through A's renderer radius")]
    if used != mode:
        q.append(("note", "--align jd asked, but jd is constant in %s: the jd key is undefined - aligned by time" % " and ".join(const)))
    elif mode == "time" and any(s > 0 for s in span):
        q.append(("note", "jd varies: under a running clock the sky is a function of jd - --align jd is its key (aligned by time, as asked)"))
    tables, cols, every = _diff(pairs, va["renderer"][2], "A", "B", False)
    return [("compare", KV, q)] + tables, cols, every


def _fmt(col, v):
    if isinstance(v, float):
        return ("%.17g" if any(p in col for p in P17) else "%.9g") % v
    return str(v)


def emit(tables, out):
    for title, cols, rows in tables:
        out.write("## %s - %d row(s)\n%s\n" % (title, len(rows), "\t".join(cols)))
        for r in rows:
            out.write("\t".join(_fmt(r[0] if cols == KV else c, v) for c, v in zip(cols, r)) + "\n")
        out.write("\n")


def write_all(path, cols, rows):
    out = sys.stdout if path == "-" else open(path, "w")
    out.write("\t".join(cols) + "\n")
    for r in rows:
        out.write("\t".join(_fmt(c, v) for c, v in zip(cols, r)) + "\n")
    if out is not sys.stdout:
        out.close()


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--self-test", action="store_true")
    sub = ap.add_subparsers(dest="cmd")
    for name, recs in (("summary", ["rec"]), ("motion", ["rec"]), ("within", ["rec"]), ("compare", ["recA", "recB"])):
        p = sub.add_parser(name)
        for r in recs:
            p.add_argument(r)
        if name != "summary":
            p.add_argument("--all", metavar="OUT.tsv", help="write every row raw ('-' = stdout)")
    sub.choices["motion"].add_argument("--engine", choices=ENGINES)
    sub.choices["motion"].add_argument("--k", type=float, default=8.0)
    sub.choices["motion"].add_argument("--floor", type=float, default=1e-6, help="no step at or under it is a JUMP; a JUMP is a spike against its neighbouring steps [r]")
    sub.choices["motion"].add_argument("--top", type=int, default=10, help="steps listed per body (0 = all)")
    sub.choices["compare"].add_argument("--engine-a", choices=ENGINES, default="n")
    sub.choices["compare"].add_argument("--engine-b", choices=ENGINES, default="o")
    sub.choices["compare"].add_argument("--align", choices=["time", "frame", "jd"], default="time")
    a = ap.parse_args(argv)
    if a.self_test:
        return self_test()
    if not a.cmd:
        ap.print_help(sys.stderr)
        return 2
    try:
        if a.cmd == "summary":
            tables, cols, every = summary(load(a.rec)), None, None
        elif a.cmd == "motion":
            tables, cols, every = motion(load(a.rec), a.engine, a.k, a.top, a.floor)
        elif a.cmd == "within":
            tables, cols, every = within(load(a.rec))
        else:
            tables, cols, every = compare(load(a.recA), load(a.recB), a.engine_a, a.engine_b, a.align)
            for key, v in tables[0][2]:
                if key == "note":
                    print("screen_record: note: %s" % v, file=sys.stderr)
    except Refused as e:
        print("screen_record: REFUSED %s" % e, file=sys.stderr)
        return 1
    except ValueError as e:
        print("screen_record: %s" % e, file=sys.stderr)
        return 1
    emit(tables, sys.stdout)
    if getattr(a, "all", None):
        write_all(a.all, cols, every)
    return 0


def _lines(frames):
    """Write a self-test record from INTENT 11.333 (d): frames = [(dt, jd, [(engine, name, x, y, size)])], drawn o."""
    out = ["#\tscreen_record\t1", "#\tB\tengine(o|n)\tname\tx\ty\tsize\tx,y: Vulkan axes, 1 = the viewport radius",
           "#\tF\tframe\tdt_ms\tjd\tdrawn(o|n)\tfov_old_deg\thalf_fov_new_rad    the F line CLOSES its frame",
           "#\tviewport\tprojector\t512\t512\t512\trenderer\t512\t512\t512\tscissor\t0\t0\t1024\t1024"]
    for f, (dt, jd, bodies) in enumerate(frames):
        out += ["B\t%s\t%s\t%.9g\t%.9g\t%.9g" % b for b in bodies]
        out.append("F\t%d\t%.17g\t%.17g\to\t180\t%.17g" % (f, dt, jd, math.pi / 2))
    return out


def _synth(dt, n=6):
    """Write the two-engine record: Saturn jumps +0.2 at frame 3 (o - n = +0.001 in x), Mars is missing from n at
    frame 4, the Moon's size steps at frame 2 (n holds it at twice o's size)."""
    return _lines([(dt, 2461233.5 + f * dt / 86400000.0, [b for e in ENGINES for b in
                    [(e, "Saturn", 0.1 + 0.01 * f + 0.2 * (f >= 3) - 0.001 * (e == "n"), 0.2, 0.01)]
                    + ([] if e == "n" and f == 4 else [(e, "Mars", -0.3, 0.1, 0.002)])
                    + [(e, "Moon", 0, -0.5, (0.05 if f < 2 else 0.06) * (2 if e == "n" else 1))]]) for f in range(n)])


def _rows(tables, title):
    t = next(t for t in tables if t[0].startswith(title))
    return [dict(zip(t[1], r)) for r in t[2]]


def self_test():
    fails = []

    def check(name, ok, detail=""):
        print("%s %s  %s" % ("PASS" if ok else "FAIL", name, detail), flush=True)
        if not ok:
            fails.append(name)
    with tempfile.TemporaryDirectory() as d:
        def put(name, lines):
            p = os.path.join(d, name)
            with open(p, "w") as fh:
                fh.write("\n".join(lines) + "\n")
            return p
        r7, r8 = load(put("r7.tsv", _synth(7.0))), load(put("r8.tsv", _synth(8.0)))
        q = {r["quantity"]: r["value"] for r in _rows(summary(r7), "record")}
        seen = {(r["engine"], r["body"]): r["frames_seen"] for r in _rows(summary(r7), "bodies")}
        check("summary_counts", (q["frames"], q["duration[ms]"], q["bodies o"], q["bodies n"], seen[("n", "Mars")],
                                 seen[("o", "Mars")]) == (6, 42.0, 3, 3, 5, 6), "frames %s, %s ms, bodies o %s n %s, Mars n %s o %s"
              % (q["frames"], q["duration[ms]"], q["bodies o"], q["bodies n"], seen[("n", "Mars")], seen[("o", "Mars")]))
        mt, JC = motion(r7, "n")[0], "jumps(spike>1e-06 r)"
        per = {r["body"]: r for r in _rows(mt, "motion per body")}
        steps = _rows(mt, "position steps")
        check("motion_flags_the_jump_at_its_frame", per["Saturn"][JC] == 1 and per["Saturn"]["frame@max_step"] == 3
              and (steps[0]["body"], steps[0]["frame"], steps[0]["flag"]) == ("Saturn", 3, "JUMP")
              and not [s for s in steps if s["flag"] and (s["body"], s["frame"]) != ("Saturn", 3)],
              "first step row %s frame %s %s step %.9g" % (steps[0]["body"], steps[0]["frame"], steps[0]["flag"], steps[0]["step[r]"]))
        sz = _rows(mt, "size steps")
        check("motion_flags_the_size_step", per["Moon"]["size_jumps"] == 1 and per["Moon"]["max_|dsize|_frame"] == 2
              and (sz[0]["body"], sz[0]["frame"], sz[0]["flag"]) == ("Moon", 2, "JUMP"), "Moon size step at frame %s" % sz[0]["frame"])
        check("motion_names_the_absent_frame", per["Mars"]["absent_frames"] == "4" and per["Saturn"]["absent_frames"] == "-",
              "Mars absent %s" % per["Mars"]["absent_frames"])
        wt = within(r7)[0]
        w = {r["body"]: r for r in _rows(wt, "per body")}
        only = _rows(wt, "bodies on one side only")
        check("within_reports_the_o_n_delta", _rows(wt, "per body")[0]["body"] == "Saturn"
              and abs(w["Saturn"]["dpos[r]"] - 0.001) < 1e-12 and abs(w["Saturn"]["dpos[px]"] - 0.512) < 1e-9
              and w["Saturn"]["frames_both"] == 6 and abs(w["Moon"]["size_o/size_n"] - 0.5) < 1e-12 and w["Mars"]["dpos[r]"] == 0,
              "Saturn %.9g r = %.9g px, Moon size o/n %.9g" % (w["Saturn"]["dpos[r]"], w["Saturn"]["dpos[px]"], w["Moon"]["size_o/size_n"]))
        check("within_reports_the_missing_body", [(o["body"], o["only_in"], o["frames_list"]) for o in only] == [("Mars", "o", "4")],
              repr(only))
        mt_ = [j for _, j in align(r7, r8, "time")]
        check("compare_aligns_7ms_on_8ms_by_time", mt_ == [0, 1, 2, 2, 3, 4] and [j for _, j in align(r7, r8, "frame")]
              == [0, 1, 2, 3, 4, 5], "time %s (A3 at 28 ms: B2 24 / B3 32 tie -> the earlier)" % mt_)
        ct = {r["body"]: r for r in _rows(compare(r7, r8, "o", "o", "time")[0], "per body")}
        cf = {r["body"]: r for r in _rows(compare(r7, r8, "o", "o", "frame")[0], "per body")}
        check("compare_deltas_follow_the_alignment", abs(ct["Saturn"]["dpos[r]"] - 0.21) < 1e-9 and ct["Saturn"]["frame_A"] == 3
              and ct["Saturn"]["frame_B"] == 2 and cf["Saturn"]["dpos[r]"] == 0,
              "time: Saturn %.9g at A3/B2; frame: %.9g" % (ct["Saturn"]["dpos[r]"], cf["Saturn"]["dpos[r]"]))
        co = _rows(compare(r7, r8)[0], "bodies on one side only")
        check("compare_reports_the_set_difference", [(o["body"], o["only_in"], o["frames_list"]) for o in co] == [("Mars", "B", "4")],
              repr(co))
        l7, l8 = load(put("l7.tsv", _synth(7.0, 10))), load(put("l8.tsv", _synth(8.0, 10)))
        check("compare_frame8_of_7ms_meets_frame7_of_8ms", dict(align(l7, l8, "time"))[8] == 7
              and dict(align(l7, l8, "frame"))[8] == 8, "time %s, frame %s" % (dict(align(l7, l8, "time"))[8], dict(align(l7, l8, "frame"))[8]))
        J, s = 2461233.5, 0.016
        one = lambda tag, dts, jds, xs: load(put(tag + ".tsv", _lines([(t, j, [("o", "Mercury", x, 0, 1e-3)]) for t, j, x in zip(dts, jds, xs)])))
        ja = one("ja", [16.0] * 8, [J + s * f for f in range(8)], [0.1 + s * f for f in range(8)])
        jb = one("jb", [18.0] + [16.0] * 8, [J + s * (g - 1) for g in range(9)], [0.1 + s * (g - 1) for g in range(9)])
        jc = one("jc", [16.0] * 8, [J] * 8, [0.1] * 8)
        hd = lambda t: {r["quantity"]: r["value"] for r in _rows(t, "compare")}
        cj, ct_, cc = compare(ja, jb, "o", "o", "jd")[0], compare(ja, jb, "o", "o", "time")[0], compare(ja, jc, "o", "o", "jd")[0]
        dj, dt_ = _rows(cj, "per body")[0]["dpos[r]"], _rows(ct_, "per body")[0]["dpos[r]"]
        check("compare_aligns_by_jd_not_by_time", align(ja, jb, "jd") == [(f, f + 1) for f in range(8)] and dj == 0
              and align(ja, jb, "time") == [(f, f) for f in range(8)] and abs(dt_ - s) < 1e-9,
              "jd: A f -> B f+1, dpos %.9g; time (elapsed 2 ms apart): A f -> B f, dpos %.9g" % (dj, dt_))
        check("compare_says_jd_varies_and_names_align_jd", hd(ct_)["jd varies A / B (max - min [d])"].startswith("yes")
              and "--align jd is its key" in hd(ct_).get("note", "") and hd(cj)["align"] == "jd" and "note" not in hd(cj), repr(hd(ct_).get("note")))
        try:
            refused = "no refusal: %s" % align(ja, jc, "jd")
        except ValueError as e:
            refused = str(e)
        check("compare_jd_constant_falls_back_to_time_loud", hd(cc)["align"].startswith("time (FALLBACK") and hd(cc)["aligned pairs"] == 8
              and "constant in B" in hd(cc).get("note", "") and "jd constant" in refused, "%s | align() %s" % (hd(cc).get("note"), refused))
        still = lambda f, step: 0.5 + 1e-9 * (f % 2) + step * (f >= 5)
        fr = load(put("floor.tsv", _lines([(16.0, J, [("o", "Ariel", still(f, 5e-7), 0, 1e-3), ("o", "Oberon", still(f, 5e-6), 0, 1e-3),
                                                      ("o", "Umbriel", 0.3 + 2e-6 * sum(f >= g for g in (3, 6, 8)), 0, 1e-3),
                                                      ("o", "Puck", 0.3 + 2e-6 * f, 0, 1e-3)]) for f in range(10)])))
        (t6, _, e6), (t7, _, e7) = motion(fr, "o"), motion(fr, "o", floor=1e-7)
        p6, p7 = ({r["body"]: r for r in _rows(t, "motion per body")} for t in (t6, t7))
        f6, f7 = ([(r[1], r[2]) for r in e if r[12].startswith("JUMP")] for e in (e6, e7))
        check("motion_floor_flags_nothing_under_it_and_drops_no_row", p6["Ariel"][JC] == "-" and ("Ariel", 5) not in f6
              and sum(r[1] == "Ariel" for r in e6) == 9, "Ariel jumps %s, flagged %s" % (p6["Ariel"][JC], f6))
        check("motion_flags_a_jump_above_the_floor", ("Oberon", 5) in f6 and p6["Oberon"][JC] == 1 and ("Ariel", 5) in f7
              and p7["Ariel"]["jumps(spike>1e-07 r)"] == 1, "floor 1e-6: %s; floor 1e-7: %s" % (f6, f7))
        check("motion_spike_flags_isolated_steps_not_a_steady_drift", p6["Umbriel"][JC] == 3 and p6["Puck"][JC] == 0
              and abs(p6["Umbriel"]["median_nonzero_step[r]"] - 2e-6) < 1e-12,
              "Umbriel (three isolated 2e-6 steps) jumps %s, median %.9g; Puck (2e-6 every frame) jumps %s"
              % (p6["Umbriel"][JC], p6["Umbriel"]["median_nonzero_step[r]"], p6["Puck"][JC]))
        wl = lambda n, sh: _lines([(16.0, J, [("o", "Titan", 0.1 + 0.01 * (f - sh), 0, 1e-3), ("o", "Rhea", 0.5 + 0.005 * sh, 0, 1e-3)])
                                   for f in range(n)])
        cw = _rows(compare(load(put("wa.tsv", wl(6, 0))), load(put("wb.tsv", wl(7, 1))), "o", "o", "time")[0], "per body")
        check("compare_dpos_w1_meets_the_shifted_frame", [r["body"] for r in cw] == ["Titan", "Rhea"] and abs(cw[0]["dpos[r]"] - 0.01) < 1e-9
              and cw[0]["dpos_w1[r]"] == 0 and cw[0]["frame_B_w1"] == cw[0]["frame_B"] + 1 and cw[0]["worst_dpos_w1[r]"] == 0
              and abs(cw[1]["worst_dpos_w1[r]"] - 0.005) < 1e-9, "Titan dpos %.9g at A%s/B%s, dpos_w1 %.9g at B%s, worst_w1 %s"
              % (cw[0]["dpos[r]"], cw[0]["frame_A"], cw[0]["frame_B"], cw[0]["dpos_w1[r]"], cw[0]["frame_B_w1"], cw[0]["worst_dpos_w1[r]"]))
        base = _synth(7.0)
        f2 = next(i for i, x in enumerate(base) if x.startswith("F\t2\t"))
        for k, i, text, insert in (("a 5-field B line", 4, "B\to\tSaturn\t0.1\t0.2", False),
                                   ("a frame index that skips", f2, "F\t3" + base[f2][3:], False),
                                   ("an engine tag x", 5, "B\tx" + base[5][3:], False),
                                   ("a B line after the last F line", len(base), "B\to\tSaturn\t0\t0\t0", True),
                                   ("a first line not the magic", 0, "#\tscreen_record\t2", False),
                                   ("a header line after data", 7, "#\tnote", True),
                                   ("a body twice in one frame", 5, base[4], False)):
            lines = list(base)
            if insert:
                lines.insert(i, text)
            else:
                lines[i] = text
            p = put("broken.tsv", lines)
            try:
                load(p)
                got = None
            except Refused as e:
                got = e
            check("refused: " + k, got is not None and got.line == i + 1, "line %d expected, got %s" % (i + 1, got))
        check("cli_exit_1_on_a_refused_record", main(["summary", put("broken.tsv", ["#\tscreen_record\t2"])]) == 1)
    print("=== self-test: %d FAIL ===" % len(fails))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
