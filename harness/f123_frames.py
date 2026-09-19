#!/usr/bin/env python3
"""F123 frame comparator -- the DEFAULT VIEW's regression floor.

  python3 f123_frames.py <a.png> <b.png> [<a2.png> <b2.png> ...]

Prints, per pair: the count of pixels whose per-channel difference exceeds 8
(this project's standing screen unit -- "px>8", INTENT S11.131(h)), the maximum
channel difference, and the two frames' lit counts.  A pair of grabs of the SAME
path with nothing between them is the A/A FLOOR; any A/B claim is read against
it and never against zero.

Deliberately NOT a presence instrument: a diff cancels a missing disc where both
frames lack it (S11.99(e)).  Presence is f123_drive.py's lit-disc count; this
tool answers the other question -- did the frame MOVE."""
import sys
import numpy as np
from PIL import Image

LIT = 32
TH = 8


def load(p):
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def pair(pa, pb):
    a, b = load(pa), load(pb)
    if a.shape != b.shape:
        print("SHAPE MISMATCH %s %s vs %s" % (pa, a.shape, b.shape))
        return
    d = np.abs(a - b).max(axis=2)
    print("%-44s vs %-44s  px>%d = %8d   maxdiff = %3d   lit(a) = %8d  lit(b) = %8d"
          % (pa.split("/")[-1], pb.split("/")[-1], TH, int((d > TH).sum()), int(d.max()),
             int((a.max(axis=2) > LIT).sum()), int((b.max(axis=2) > LIT).sum())))


args = sys.argv[1:]
for i in range(0, len(args) - 1, 2):
    pair(args[i], args[i + 1])
