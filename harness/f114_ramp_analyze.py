#!/usr/bin/env python3
"""F114 -- scores an f114_ramp.py run against the two ramp laws, both PREDICTED
in closed form before the run (artifacts/f114/prediction.txt section 4).

For each sample: t from the app's own clock (jd at timerate 1), each path's
angular PROGRESS from its own pre-command direction, and the divergence between
them. Progress is frame-independent (an angle between two directions of the
same path), so no cross-frame conversion enters the ramp curves; the END-state
parity uses the one frame both paths share -- old's helioToEye forward vs the
camera's absFwd (Camera.cpp:730-732).

Usage: f114_ramp_analyze.py <dir>
"""
import sys, os, json, math, gzip


def opendump(d, name):
    p = os.path.join(d, name + ".json")
    return open(p) if os.path.exists(p) else gzip.open(p + ".gz", "rt")


def has(d, name):
    p = os.path.join(d, name + ".json")
    return os.path.exists(p) or os.path.exists(p + ".gz")


def header(d, name):
    with opendump(d, name) as f:
        return json.loads(f.readline())


def norm(v):
    n = math.sqrt(sum(c * c for c in v))
    return [c / n for c in v] if n else v


def old_fwd(h):
    m = h["helioToEye"]
    return norm([-m[2], -m[6], -m[10]])


def new_fwd(h):
    return norm(h["camera"]["absFwd"])


def ang(a, b):
    return math.degrees(math.acos(max(-1.0, min(1.0, sum(x * y for x, y in zip(a, b))))))


def law_old(A, u):
    return A * u ** 4


def law_new(A, u):
    return 2 * A * u * u if u <= 0.5 else A - 2 * A * (1 - u) ** 2


def command_lead(d, cmd):
    """Seconds of rate-1 time between the `_before` dump and the ramp command.

    jd only advances while timerate != 0, and the driver sets rate 1 BEFORE
    issuing the ramp command, so the sample clock's origin is not the command.
    The lead is recovered from the driver's own log: each printed line is
    (send + pause + drain), and both constants are known per call site.
    Returns None when the log is absent (then the origin is the first sample).
    """
    p = os.path.join(d, "drive.log")
    if not os.path.exists(p):
        return None
    prev_rate1 = None
    for line in open(p):
        if ">> " not in line:
            continue
        t = float(line.split()[0]); text = line.split(">> ", 1)[1].strip()
        if text == "timerate rate 1":
            prev_rate1 = t - 0.5          # send(pause=0.3, drain=0.2)
        elif text == cmd and prev_rate1 is not None:
            return (t - 0.01) - prev_rate1   # send(pause=0.0, drain=0.01)
    return None


def leg(d, prefix, T, n):
    if not has(d, prefix + "_before"):
        return
    h0 = header(d, prefix + "_before")
    o0, n0 = old_fwd(h0), new_fwd(h0)
    hA = header(d, prefix + "_after")
    print("=== leg %s : duration %g s ===" % (prefix, T))
    print("  start : tracked=%r  angle(old,new)=%.6g deg  viewT=%.4g  autoMove=%s"
          % (h0["camera"]["tracked"], ang(o0, n0),
             h0["camera"]["plans"]["viewT"], h0["oldView"]["nav"]["plans"]["flagAutoMove"]))
    print("  end   : tracked=%r  angle(old,new)=%.6g deg  viewT=%.4g  autoMove=%s"
          % (hA["camera"]["tracked"], ang(old_fwd(hA), new_fwd(hA)),
             hA["camera"]["plans"]["viewT"], hA["oldView"]["nav"]["plans"]["flagAutoMove"]))
    A = ang(o0, old_fwd(hA))            # the total angle, from OLD's own move
    print("  total angle A (old, start->end) = %.4f deg ; new travelled %.4f deg"
          % (A, ang(n0, new_fwd(hA))))
    jd0 = h0["jd"]
    lead = command_lead(d, "zoom auto initial duration %g" % T)
    print("  clock : sample origin shifted by the rate-1 lead measured from the "
          "driver log = %s s" % ("%.3f" % lead if lead is not None else "n/a"))
    rows = []
    for i in range(n):
        name = "%s_%02d" % (prefix, i)
        if not has(d, name):
            continue
        h = header(d, name)
        t = (h["jd"] - jd0) * 86400.0 - (lead or 0.0)
        rows.append((t, ang(o0, old_fwd(h)), ang(n0, new_fwd(h)),
                     h["camera"]["plans"]["viewT"],
                     h["oldView"]["nav"]["plans"]["flagAutoMove"]))
    print("  %-8s %-8s %-9s %-9s %-9s %-9s %-9s %-8s" %
          ("t (s)", "u=t/T", "old meas", "old pred", "new meas", "new pred",
           "div meas", "u_old"))
    for (t, po, pn, vT, am) in rows:
        u = t / T
        # ORIGIN-FREE cross-check: old's law is exact and invertible, so old's
        # own progress gives the ramp fraction without any clock at all.
        uo = (po / A) ** 0.25 if 0 < po < A else (0.0 if po <= 0 else 1.0)
        print("  %-8.3f %-8.4f %-9.4f %-9.4f %-9.4f %-9.4f %-9.4f %-8.4f" %
              (t, u, po, law_old(A, min(max(u, 0.0), 1.0)), pn,
               law_new(A, min(max(u, 0.0), 1.0)), pn - po, uo))
    inband = [(pn - po) for (t, po, pn, _, _) in rows if 0 < t < T]
    if inband:
        print("  measured divergence over the sampled interior: max %.3f deg (%d samples)"
              % (max(inband), len(inband)))
    print("  predicted maximum: %.3f deg at u = 0.6823" % (0.5817 * A))
    # Origin-free scoring: new's measured progress against the predicted law
    # evaluated at OLD's own ramp fraction. No clock enters this comparison.
    err = []
    for (t, po, pn, _, _) in rows:
        if 0.02 < po < 0.98 * A:
            uo = (po / A) ** 0.25
            err.append(abs(pn - law_new(A, uo)))
    if err:
        print("  origin-free: |new measured - new predicted at u_old| max %.3f deg, "
              "mean %.3f deg over %d samples" % (max(err), sum(err) / len(err), len(err)))
    print()


if __name__ == "__main__":
    d = sys.argv[1]
    leg(d, "r1", 1.0, 12)
    leg(d, "r10", 10.0, 24)
    if has(d, "m_before"):
        h0 = header(d, "m_before"); h1 = header(d, "m_after")
        print("=== leg manual : the other autoZoomOut branch (fov doublings) ===")
        for tag, h in (("before", h0), ("after", h1)):
            print("  %-6s tracked=%-8r oldFlagTraking=%s halfFov=%.6g old fov(proj-side n/a) "
                  "angle(old,new)=%.6g deg"
                  % (tag, h["camera"]["tracked"], h["oldView"]["nav"]["flagTraking"],
                     h["camera"]["halfFov"], ang(old_fwd(h), new_fwd(h))))
