#!/usr/bin/env python3
"""F102 - read the arm-C applog and measure where the uniform pool runs out.

Segments the application's own stdout by its `Execute_command` echoes and
attributes every "Can't allocate buffer in 'uniform BufferMgr' !" to the body
being loaded AND to the path that asked for it:

  Execute_command body name "X" ... action load      <- 06.sts spelling
  Loading new Stellar System object... X             <- OLD path start (protosystem.cpp:517)
      [refusals here belong to the OLD path]
  Loading body X                                     <- NEW path start (ModularSystem.cpp:1061)
      [refusals here belong to the NEW path]

  Execute_command body action load mode in_galaxy ...  <- 14.sts spelling, neither path
      [refusals here belong to OjmMgr::load (ojm_mgr.cpp:77)]
      Succesfull loading ojm X                         <- ojm_mgr.cpp:86

Usage: f102_applog.py <applog> [--json OUT] [--excerpt OUT --excerpt-around N]
"""
import argparse, json, re, sys

ERR = "Can't allocate buffer in 'uniform BufferMgr' !"
OLD_MARK = "Loading new Stellar System object... "
NEW_MARK = "Loading body "
OJM_OK = "Succesfull loading ojm "
ANSI = re.compile(r'\x1b\[[0-9;]*m')

def strip(s):
    return ANSI.sub('', s)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('applog')
    ap.add_argument('--json')
    ap.add_argument('--excerpt')
    ap.add_argument('--excerpt-lines', type=int, default=40)
    a = ap.parse_args()

    raw = open(a.applog, 'rb').read()
    lines = [strip(l) for l in raw.decode('utf-8', errors='replace').splitlines()]

    bodies = []          # one dict per Execute_command body ... action load
    cur = None
    stray = []           # refusals outside any body segment
    for i, l in enumerate(lines):
        if 'Execute_command body ' in l and ' action load' in l:
            m = re.search(r'Execute_command (body .*)$', l)
            cmd = m.group(1) if m else l
            in_galaxy = ' mode in_galaxy' in cmd
            nm = re.search(r'\bname\s+("[^"]*"|\S+)', cmd)
            cur = dict(line=i + 1, name=(nm.group(1).strip('"') if nm else '?'),
                       in_galaxy=in_galaxy, phase='cmd',
                       err_old=0, err_new=0, err_cmd=0, ojm_ok=0)
            bodies.append(cur)
            continue
        if cur is None:
            if ERR in l:
                stray.append(i + 1)
            continue
        if l.startswith(OLD_MARK) or OLD_MARK in l:
            cur['phase'] = 'old'
        elif l.startswith(NEW_MARK) or (NEW_MARK in l and 'Loading body' in l):
            cur['phase'] = 'new'
        elif OJM_OK in l:
            cur['ojm_ok'] += 1
        if ERR in l:
            cur['err_' + {'cmd': 'cmd', 'old': 'old', 'new': 'new'}[cur['phase']]] += 1
            cur.setdefault('err_lines', []).append(i + 1)

    ss = [b for b in bodies if not b['in_galaxy']]     # 06.sts spelling (both paths)
    ig = [b for b in bodies if b['in_galaxy']]         # 14.sts spelling (in-galaxy ojm)
    def tot(b):
        return b['err_old'] + b['err_new'] + b['err_cmd']

    first = None
    for k, b in enumerate(ss):
        if tot(b):
            first = (k + 1, b)
            break
    first_ig = None
    for k, b in enumerate(ig):
        if tot(b):
            first_ig = (k + 1, b)
            break

    all_err = [i + 1 for i, l in enumerate(lines) if ERR in l]
    out = dict(
        applog=a.applog, lines=len(lines), refusals_total=len(all_err),
        first_refusal_line=all_err[0] if all_err else None,
        last_refusal_line=all_err[-1] if all_err else None,
        refusals_outside_any_body=stray,
        solar_pushes=len(ss), in_galaxy_pushes=len(ig),
        refusals_in_solar_pushes=sum(tot(b) for b in ss),
        refusals_in_in_galaxy_pushes=sum(tot(b) for b in ig),
        refusals_old_path=sum(b['err_old'] for b in ss),
        refusals_new_path=sum(b['err_new'] for b in ss),
        ojm_success_lines=sum(b['ojm_ok'] for b in bodies),
        N_first_refusing_solar_push=(first[0] if first else None),
        N_first_refusing_solar_push_name=(first[1]['name'] if first else None),
        N_first_refusing_solar_push_split=(
            dict(old=first[1]['err_old'], new=first[1]['err_new'], cmd=first[1]['err_cmd'])
            if first else None),
        first_refusing_in_galaxy_index=(first_ig[0] if first_ig else None),
    )
    # refusal-count histogram over the solar pushes at and after the first
    hist = {}
    for b in ss:
        hist[tot(b)] = hist.get(tot(b), 0) + 1
    out['solar_push_refusal_histogram'] = dict(sorted(hist.items()))
    histg = {}
    for b in ig:
        histg[tot(b)] = histg.get(tot(b), 0) + 1
    out['in_galaxy_refusal_histogram'] = dict(sorted(histg.items()))
    # the split pattern of the first 12 refusing solar pushes
    out['first_12_refusing_solar'] = [
        dict(idx=k + 1, name=b['name'], old=b['err_old'], new=b['err_new'], cmd=b['err_cmd'])
        for k, b in enumerate(ss) if tot(b)][:12]
    # the last 6 refusing solar pushes
    out['last_6_refusing_solar'] = [
        dict(idx=k + 1, name=b['name'], old=b['err_old'], new=b['err_new'], cmd=b['err_cmd'])
        for k, b in enumerate(ss) if tot(b)][-6:]

    print(json.dumps(out, indent=1))
    if a.json:
        json.dump(out, open(a.json, 'w'), indent=1)
    if a.excerpt and all_err:
        c = all_err[0] - 1
        lo = max(0, c - a.excerpt_lines // 2)
        hi = min(len(lines), c + a.excerpt_lines // 2)
        with open(a.excerpt, 'w') as f:
            f.write("# %s lines %d-%d (ANSI stripped) - the FIRST refusal is line %d\n"
                    % (a.applog, lo + 1, hi, all_err[0]))
            for i in range(lo, hi):
                f.write("%6d  %s\n" % (i + 1, lines[i]))
    return 0

if __name__ == '__main__':
    sys.exit(main())
