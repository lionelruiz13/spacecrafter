#!/usr/bin/env python3
"""F102 - the uniform pool's arithmetic, and the body index N at the first refusal.

Reads:
  * the struct sizes MEASURED by f102_sizes (TSV: KEY<TAB>sizeof<TAB>carved),
  * the installed ~/.spacecrafter/ssystem.ini (the launch scene's bodies),
  * the two shows' authored body lines (both word orders - F98's lesson),
and produces the PREDICTION: how many bytes the shipped scene has already
carved out of the 1 MiB `uniform BufferMgr` before the first show runs, how
many bytes each further authored body carves, and the index N of the body at
which `acquireBuffer` first answers VK_NULL_HANDLE.

The allocator it models is BufferMgr::acquireBuffer (BufferMgr.cpp:37-68) with
uniformBuffer=true: round the request up to minUniformBufferOffsetAlignment,
take the SMALLEST free zone whose blocks are >= it, split, put the remainder
back. Nothing is released while a show authors bodies, so the free list holds
exactly ONE zone - the shrinking remainder - and the allocator is a bump
allocator that REFUSES rather than grows (createBuffer appears once in the
file, BufferMgr.cpp:8).

Usage:  f102_pool.py --sizes <sizes.tsv> [--ssystem PATH] [--fscripts DIR]
                     [--out <prediction.json>]
"""
import argparse, json, os, re, sys

# ---------------------------------------------------------------- size table
def load_sizes(path):
    sz = {}
    for line in open(path, encoding='ascii').read().splitlines():
        if not line.strip():
            continue
        k, raw, carved = line.split('\t')
        sz[k] = (int(raw), int(carved))
    return sz

# ------------------------------------------------------------ ssystem parser
def parse_ssystem(path):
    """[section] blocks of `key = value`; ISO-8859 (untracked, CLAUDE.md)."""
    bodies, cur = [], None
    for line in open(path, 'rb').read().decode('latin-1').splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if line.startswith('['):
            cur = {}
            bodies.append(cur)
            continue
        if cur is None or '=' not in line:
            continue
        k, v = line.split('=', 1)
        cur[k.strip()] = v.strip()
    return [b for b in bodies if b.get('name')]

def is_true(v):
    """Utility::isTrue - utility.hpp:160-171 (4/2/1-byte cases only)."""
    return v in ('true', 'TRUE', 'True', 'on', 'ON', 'On', '1')

def str_to_bool(v):
    """Utility::strToBool - utility.cpp:432-437 ('true' or '1', lowercased)."""
    return v.strip().lower() in ('true', '1')

def is_star(b):
    """strToBodyType: type Sun|Star -> the STAR bit (ModularSystem loadBody)."""
    return b.get('type', '') in ('Sun', 'Star')

# -------------------------------------------- per-body carved bytes, NEW path
def new_path_bytes(b, S):
    """Eager acquireBuffer bytes on context.uniformMgr for one loaded body.

    ModularSystem::loadBody:1456-1458 walks deduceBodyModuleList and calls
    ModuleLoaderMgr::loadModule for each slot; every module that owns a
    SharedBuffer acquires it in its CONSTRUCTOR, so the whole cost is paid at
    load. Slots with no uniformMgr site (HINT, ORBIT, TRAIL, TAIL, GRID, STAR,
    AXIS - AxisModule's uColor is a function-local static, AxisModule.cpp:68)
    contribute 0.
    """
    n, detail = 0, []
    tex_map = b.get('tex_map', '')
    if b.get('tex_ring') and str_to_bool(b.get('rings', '')):
        n += S['bodyRingVert'][1] + S['bodyRingFrag'][1]; detail.append('RING')
    if tex_map:
        if is_star(b):                                   # PhotosphereLoader 200
            n += S['globalVertProj'][1]; detail.append('MESH:Photosphere')
        elif b.get('tex_night') or b.get('tex_normal') or b.get('tex_heightmap'):
            night = bool(b.get('tex_night')); normal = bool(b.get('tex_normal'))
            height = bool(b.get('tex_heightmap')); spec = bool(b.get('tex_specular'))
            moonclass = b.get('type', '') == 'Moon'      # SurfaceModel::LUNAR
            if moonclass:
                tess = height
            else:
                tess = (night and height and spec) or (not night and not normal and height)
            ray = normal and height
            n += S['globalVertProj'][1] + S['meshFrag'][1]
            if tess:
                n += S['meshTescGeom'][1]
            if ray:
                n += S['rayMarchVert'][1] + S['rayMarchFrag'][1]
            detail.append('MESH:Layered%s%s' % ('+tes' if tess else '', '+ray' if ray else ''))
        else:
            n += S['globalVertProj'][1] + S['meshFrag'][1]; detail.append('MESH:Basic')
    if b.get('model_name') and b.get('type', '')[:4] == 'Arti':
        n += (S['ojmVert'][1] + S['ojmGeom'][1] + S['ojmLight'][1]
              + S['ojmShadowBlock'][1]); detail.append('OJM')
    if ('has_atmosphere' in b or 'atmosphere_lim_landscape' in b) and b.get('atmosphere_ext_model'):
        n += S['atmExtUBO'][1]; detail.append('ATMOSPHERE')
    if is_star(b) and b.get('tex_big_halo'):
        pass                                              # StarLoader: no uniformMgr site
    if is_true(b.get('oort', '')):
        n += S['oortMat'][1] + S['oortFrag'][1]; detail.append('OORT')
    return n, detail

# -------------------------------------------- per-body carved bytes, OLD path
def old_path_bytes(b, S):
    """Returns (eager, lazy) - eager = paid in the body's CONSTRUCTOR, lazy =
    paid at the body's FIRST DRAW.

    Moon::selectShader is called from the Moon ctor (body_moon.cpp:78) - EAGER.
    BigBody::selectShader is called from BigBody::drawBody when `changed`
    (body_bigbody.cpp:301) and SmallBody::selectShader from SmallBody::drawBody
    when not `initialized` (body_smallbody.cpp:192) - LAZY, only for a body the
    frame actually draws. Artificial's three uniforms are in its ctor init list
    (body_artificial.cpp:84) - EAGER.
    """
    t = b.get('type', '')
    night = bool(b.get('tex_night')); normal = bool(b.get('tex_normal'))
    height = bool(b.get('tex_heightmap')); rings = bool(b.get('tex_ring')) and str_to_bool(b.get('rings',''))
    vp, gf, um = S['globalVertProj'][1], S['globalFrag'][1], S['Vec3f'][1]
    if t == 'Moon':
        if height:   return vp + S['moonFrag'][1] + S['meshTescGeom'][1], 0
        if night:    return vp + gf, 0
        if normal:   return vp + gf + um, 0
        return vp + gf, 0
    if t == 'Artificial':
        return S['artGeom'][1] + S['LightInfo'][1] + S['artVert'][1], 0
    if t in ('Sun', 'Star', 'Center'):
        # Sun's four big-halo uniforms live in createHaloShader (body_sun.cpp:146-152),
        # reached only for a body carrying a big halo; the sun set is defineSunSet.
        return 0, 4 * S['floatUniform'][1] + S['Mat4f'][1] + S['Vec3f'][1] + S['floatUniform'][1]
    # PLANET / DWARF -> BigBody ; ASTEROID / KBO / COMET / unknown -> SmallBody
    if t in ('Planet', 'Dwarf'):
        lazy = vp + gf
        if night and height:  lazy = vp + gf + S['meshTescGeom'][1]
        elif normal:          lazy = vp + gf + um
        elif rings:           lazy = vp + gf + S['Mat4f'][1] + S['bodyRingVert'][1]
        elif height:          lazy = vp + gf + S['meshTescGeom'][1]
        if rings:
            lazy += S['RingUniform'][1]        # ring.cpp:146, per ringed body
        return 0, lazy
    lazy = vp + gf + (um if normal else 0)
    return 0, lazy

# ------------------------------------------------------------- the simulation
def simulate(free0, seq_06, n06, seq_14, n14):
    """One free zone, shrinking; a request that does not fit is REFUSED and
    nothing is consumed (BufferMgr.cpp:46-55). Returns the trace."""
    free = free0
    first = None          # (show, body index 1-based, which request)
    errors_06 = errors_14 = 0
    per_body_hist = {}
    for i in range(1, n06 + 1):
        e = 0
        for j, req in enumerate(seq_06):
            if free >= req:
                free -= req
            else:
                e += 1
                if first is None:
                    first = ('06.sts', i, j)
        errors_06 += e
        per_body_hist[e] = per_body_hist.get(e, 0) + 1
    for i in range(1, n14 + 1):
        for j, req in enumerate(seq_14):
            if free >= req:
                free -= req
            else:
                errors_14 += 1
                if first is None:
                    first = ('14.sts', i, j)
    return dict(first=first, errors_06=errors_06, errors_14=errors_14,
                errors_total=errors_06 + errors_14, free_left=free,
                per_body_error_histogram=per_body_hist)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--sizes', required=True)
    ap.add_argument('--ssystem', default=os.path.expanduser('~/.spacecrafter/ssystem.ini'))
    ap.add_argument('--globals-bytes', type=int, default=None,
                    help='measured/enumerated non-body startup uniform bytes')
    ap.add_argument('--n14', type=int, default=202,
                    help='in-galaxy ojm loads executed before the death (arm C)')
    ap.add_argument('--out')
    a = ap.parse_args()

    S = load_sizes(a.sizes)
    POOL = S['POOL'][0]
    bodies = parse_ssystem(a.ssystem)
    new_tot = old_eager = old_lazy = 0
    rows = []
    for b in bodies:
        nb, det = new_path_bytes(b, S)
        oe, ol = old_path_bytes(b, S)
        new_tot += nb; old_eager += oe; old_lazy += ol
        rows.append((b.get('name'), b.get('type', ''), nb, oe, ol, '+'.join(det)))

    # the 06.sts body: type Moon, tex_map only  -> old ctor 192+128, new BasicMesh 192+832
    seq_06 = [S['globalVertProj'][1], S['globalFrag'][1],        # Moon::selectShader (ctor)
              S['globalVertProj'][1], S['meshFrag'][1]]          # BasicMesh ctor
    seq_14 = [S['ojmContainerUniformData'][1]]                   # ojm_mgr.cpp:77

    out = dict(pool=POOL, alignment=S['ALIGNMENT'][0],
               ssystem_bodies=len(bodies),
               baseline_new_path=new_tot, baseline_old_eager=old_eager,
               baseline_old_lazy_if_all_drawn=old_lazy,
               globals_bytes=a.globals_bytes,
               per_body_06=sum(seq_06), seq_06=seq_06,
               per_body_14=sum(seq_14), seq_14=seq_14, rows=rows)

    g = a.globals_bytes or 0
    for label, base in (('lazy_none', new_tot + old_eager + g),
                        ('lazy_all', new_tot + old_eager + old_lazy + g)):
        out[label] = dict(baseline=base,
                          **simulate(POOL - base, seq_06, 1013, seq_14, a.n14))
    if a.out:
        json.dump(out, open(a.out, 'w'), indent=1)
    print(json.dumps({k: v for k, v in out.items() if k != 'rows'}, indent=1))
    return 0

if __name__ == '__main__':
    sys.exit(main())
