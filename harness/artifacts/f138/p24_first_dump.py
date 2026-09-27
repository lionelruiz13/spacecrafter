#!/usr/bin/env python3
# Driver for f124_run.sh: a fresh launch dumped BEFORE any command of ours (F134 block P24).
#   bash f124_run.sh ../f138/<label> artifacts/f138/p24_first_dump.py
import json, math, os, socket, sys, time
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
from dumpread import load_dump

OUT = sys.argv[1]
AU_KM = 149597870.691
s = socket.create_connection(('127.0.0.1', 7805), timeout=10)
p = os.path.join(OUT, 'p24_first.json')
s.sendall(('body action dual_dump filename ' + p + '\n').encode())
for _ in range(40):
    time.sleep(0.25)
    if os.path.exists(p) and os.path.getsize(p) > 0:
        break
time.sleep(2)
s.close()
h, pairs, mn, mo = load_dump(p, require_old=False)
c = h['camera']
alt = h['control']['altitude']
earth = next((r['new'] for r in pairs if r['name'] == 'Earth'), None)
print('reference=%s freeMode=%s boundToSurface=%s camera.distance_km=%.6f' % (
    c['reference'], c['freeMode'], c['boundToSurface'], c['distance'] * AU_KM))
print('control.altitude new=%r old=%r reported=%r drawnPath=%s' % (
    alt.get('new'), alt.get('old'), alt.get('reported'), h['control'].get('drawnPath')))
if earth:
    print('Earth.new scaledDatumRadius_km=%.6f scaling=%r visible=%r' % (
        earth['scaledDatumRadius'] * AU_KM, earth['scaling'], earth['visible']))
a = alt.get('new')
verdict = 'NOTE P24 control.altitude.new=%r m: ' % a
if a is not None and abs(a - 75) <= 1:
    verdict += '75 m +- 1 (the prediction)'
elif a is not None and abs(a + 6378000) < 10000:
    verdict += 'about -6378 km (the cold read survives at load: a row to mint)'
else:
    verdict += 'neither 75 m nor -6378 km'
print(verdict)
