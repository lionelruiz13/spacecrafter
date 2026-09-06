#!/usr/bin/env python3
"""F102 - what the tester's whole fscripts/ corpus would carve out of the uniform pool.

Censuses every `body ... action load` in every installed .sts (BOTH word orders -
F98's lesson) and prices each push with the per-body arithmetic F102 measured:

  mode in_galaxy   -> OjmMgr::load, ONE uniform (ojm_mgr.cpp:77)          128 B
  solar push, tex_map, type Moon, no layered key
                   -> OLD Moon::selectShader ctor 192+128, NEW BasicMesh 192+832
  solar push, tex_map, layered key (tex_night|tex_normal|tex_heightmap)
                   -> OLD lazy (BigBody/SmallBody) or Moon ctor, NEW LayeredMesh
  solar push, no tex_map
                   -> no MESH module deduced; 0 eager uniform bytes on either path

Only the EAGER bytes (paid at load) are summed for old BigBody/SmallBody, which
allocate at first DRAW - those are reported separately as the lazy tail.

Usage: f102_corpus.py [--dir DIR] [--sizes sizes.tsv] [--json OUT]
"""
import argparse, json, os, re, sys

LOAD = re.compile(r'^\s*body\b(?=.*\baction\s+load\b)', re.I)
def kv(line):
    """the surface takes key/value pairs in any order (app_command_interface).

    Tokenise respecting double quotes, DROP the leading command word, then pair
    up. Pairing without dropping `body` shifts every key onto the previous
    value - which is exactly how a census silently reports a corpus it did not
    read (F98's word-order lesson, one turn further in).
    """
    toks = re.findall(r'"[^"]*"|\S+', line.strip())
    if toks and toks[0].lower() == 'body':
        toks = toks[1:]
    d = {}
    for i in range(0, len(toks) - 1, 2):
        d.setdefault(toks[i], toks[i + 1].strip('"'))
    return d

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dir', default=os.path.expanduser('~/.spacecrafter/scripts/fscripts'))
    ap.add_argument('--sizes', default=None)
    ap.add_argument('--json')
    a = ap.parse_args()

    S = {}
    if a.sizes:
        for l in open(a.sizes).read().splitlines():
            if l.strip():
                k, r, c = l.split('\t'); S[k] = int(c)
    OLD_MOON = S.get('globalVertProj', 192) + S.get('globalFrag', 128)     # 320
    NEW_BASIC = S.get('globalVertProj', 192) + S.get('meshFrag', 832)      # 1024
    IG = S.get('ojmContainerUniformData', 128)                             # 128

    per_show, tot = {}, dict(in_galaxy=0, moon_texmap=0, texmap_other=0, no_texmap=0,
                             bytes_eager=0, bytes_new_only=0, bytes_in_galaxy=0)
    for fn in sorted(os.listdir(a.dir)):
        if not fn.endswith('.sts'):
            continue
        path = os.path.join(a.dir, fn)
        txt = open(path, 'rb').read().decode('latin-1')
        c = dict(in_galaxy=0, moon_texmap=0, texmap_other=0, no_texmap=0,
                 bytes_eager=0, bytes_new_only=0)
        for line in txt.splitlines():
            if not LOAD.match(line):
                continue
            d = kv(line)
            if d.get('mode') in ('in_galaxy', 'in_universe', 'in_sandbox'):
                c['in_galaxy'] += 1; c['bytes_eager'] += IG; c['bytes_new_only'] += IG
                continue
            if not d.get('tex_map'):
                c['no_texmap'] += 1
                continue
            layered = any(d.get(k) for k in ('tex_night', 'tex_normal', 'tex_heightmap'))
            if d.get('type') == 'Moon' and not layered:
                c['moon_texmap'] += 1
                c['bytes_eager'] += OLD_MOON + NEW_BASIC
                c['bytes_new_only'] += NEW_BASIC
            else:
                c['texmap_other'] += 1
                c['bytes_eager'] += NEW_BASIC     # new path eager; old is lazy here
                c['bytes_new_only'] += NEW_BASIC
        if sum(c[k] for k in ('in_galaxy', 'moon_texmap', 'texmap_other', 'no_texmap')):
            per_show[fn] = c
            for k in c:
                tot[k] = tot.get(k, 0) + c[k]
    out = dict(dir=a.dir, shows=len(per_show), per_show=per_show, totals=tot,
               pool=1048576,
               pool_multiple_eager=round(tot['bytes_eager'] / 1048576.0, 3),
               pool_multiple_new_only=round(tot['bytes_new_only'] / 1048576.0, 3))
    print(json.dumps({k: v for k, v in out.items() if k != 'per_show'}, indent=1))
    print('per show (only shows that author):')
    for k, v in sorted(per_show.items(), key=lambda x: -x[1]['bytes_eager']):
        print('  %-22s ig %4d  moon+tex %5d  tex-other %4d  no-tex %3d  eager %9d B'
              % (k, v['in_galaxy'], v['moon_texmap'], v['texmap_other'], v['no_texmap'],
                 v['bytes_eager']))
    if a.json:
        json.dump(out, open(a.json, 'w'), indent=1)
    return 0

if __name__ == '__main__':
    sys.exit(main())
