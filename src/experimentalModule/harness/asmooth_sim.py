# ASmooth retarget-path verification - replays ASmooth.hpp formulas exactly.
# Goal (Vixy): smallest equal-magnitude accel/decel reaching dst with null
# velocity within minDuration. Physics: p(t)=c+b*t+a*t^2 per phase (update()'s
# compose commits exactly this, and b += 2*a*t matches its derivative).
import math, random
random.seed(7)

def plan_general(c, b, dst, T, fix_a):
    """set()'s mid-transit branch after freeze: returns (a, t1, t2)."""
    d = dst - c
    tmp = math.sqrt(d*d + b*T*(b*T/2 - d))
    t1 = (d - tmp)/b
    if t1 < 0 or t1 > T:
        t1 = (d + tmp)/b
    if fix_a:
        a = -b / (t1*4 - T*2)     # derived: a = -b/(4*t1 - 2*T)
    else:
        a = -b / (t1*2 - T*4)     # code as written
    return a, t1, T - t1

def run_phases(c, b, a, t1, t2):
    """Commit both phases with update()'s own (correct) composition."""
    c += t1*(b + t1*a); b += t1*a*2; a = -a      # phase 1
    c += t2*(b + t2*a); b += t2*a*2              # phase 2
    return c, b

print("== retarget with b!=0: landing error (final_pos-dst, final_vel)")
worst = {False: 0.0, True: 0.0}
for trial in range(20000):
    b   = random.uniform(-5, 5) or 1.0
    c   = random.uniform(-10, 10)
    dst = random.uniform(-10, 10)
    T   = random.uniform(0.2, 4)
    d = dst - c
    if d*d + b*T*(b*T/2 - d) < 0: continue       # no real root: clamp path, skip
    for fix in (False, True):
        a, t1, t2 = plan_general(c, b, dst, T, fix)
        if not (0 <= t1 <= T): continue
        fc, fv = run_phases(c, b, a, t1, t2)
        err = max(abs(fc - dst), abs(fv))
        worst[fix] = max(worst[fix], err)
print(f"  code formula  a=-b/(2*t1-4*T): worst |error| = {worst[False]:.3e}")
print(f"  fixed formula a=-b/(4*t1-2*T): worst |error| = {worst[True]:.3e}")

print("\n== concrete case b=2 c=0 dst=2 T=2 (t1=0: pure deceleration)")
for fix in (False, True):
    a, t1, t2 = plan_general(0, 2, 2, 2, fix)
    fc, fv = run_phases(0, 2, a, t1, t2)
    print(f"  fix_a={fix}: a={a:+.4f} t1={t1:.3f} -> lands at {fc:+.4f} (dst=2), vel={fv:+.4f}")

print("\n== b==0 limit vs general law (degenerate quadratic, leading coeff 2b->0)")
# special case in the patch: t1=T/2, a=(d/2)/(T/2)^2 = 2d/T^2
d, T = 3.0, 2.0
a_sp = (d/2)/((T/2)**2)
fc, fv = run_phases(0, 0, a_sp, T/2, T/2)
print(f"  patch: a={a_sp:.4f} -> lands at {fc:.4f} (dst={d}), vel={fv:+.4f}")
# citardauq root (2C/(-B -+ sqrt)) stays finite at b=0: t1 = (2dT-bT^2)/(2(d+tmp))
for b in (0.0, 1e-9, 0.5):
    tmp = math.sqrt(d*d + b*T*(b*T/2 - d))
    t1 = (2*d*T - b*T*T)/(2*(d + tmp))
    print(f"  citardauq b={b}: t1={t1:.6f}" + ("  (== T/2, analytic limit)" if b == 0 else ""))

print("\n== freeze-position slip: c += timer*(b + DURATION*a) vs timer*(b + TIMER*a)")
a, b, dur = 1.5, 0.8, 2.0
for timer in (0.0, 0.5, 1.0, 2.0):
    code = timer*(b + dur*a); true = timer*(b + timer*a)
    print(f"  timer={timer}: code={code:+.3f} true={true:+.3f} err={code-true:+.3f}")

print("\n== operator T() readback: ((a*t)+b*t)+c [linear] vs ((a*t)+b)*t+c [quadratic]")
for t in (0.25, 0.5, 1.0):
    lin = (a*t + b*t); quad = (a*t + b)*t
    print(f"  t={t}: linear={lin:+.4f} quadratic={quad:+.4f} err={lin-quad:+.4f}")

print("\n== update() compose: deltaTime = timer - duration AFTER duration=nextDuration")
print("  carry-over into phase 2 = timer - d2 instead of timer - d1;")
print("  error = d1 - d2 -> zero for fresh movements (d1==d2==T/2),")
print("  nonzero exactly after a retarget (t1 != T/2) - wall-time shift only,")
print("  committed positions unaffected (commit uses full durations).")
