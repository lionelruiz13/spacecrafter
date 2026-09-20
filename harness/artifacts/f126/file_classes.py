#!/usr/bin/env python3
# F126: caller class PER FILE (not per directory). Evidence for each rule is in INTENT/11.262.md (c).
#   T = old path proper (the old engine's own code: Body family + decorations, SolarSystemDisplay, ProtoSystem, BodyTrace, AnchorManager)
#   G = shared engine whose trio-reading entry is reached only under the old-path gate today
#   W = seam / wiring's present home (Core, CoreLink, SSystemFactory, CoreBackup, App)
#   X = executors (per-frame advance + draw order, hold 11 of the mode gates)
#   S = shared sky / tools / media / in-galaxy code
import re
G = {'src/coreModule/landscape.cpp', 'src/coreModule/fog.cpp', 'src/atmosphereModule/atmosphere.cpp', 'src/coreModule/oort.cpp'}
W = {'src/coreModule/core.cpp', 'src/coreModule/core.hpp', 'src/coreModule/coreLink.cpp', 'src/coreModule/coreLink.hpp',
     'src/bodyModule/ssystem_factory.cpp', 'src/bodyModule/ssystem_factory.hpp', 'src/coreModule/backup_mgr.cpp', 'src/appModule/app.cpp'}
def klass(f):
    if f in W: return 'W'
    if f in G: return 'G'
    if f.startswith('src/bodyModule/') or f.startswith('src/navModule/'): return 'T'
    if f.startswith('src/executorModule/'): return 'X'
    return 'S'
