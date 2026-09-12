# F81 - the view offset's two couplings, measured

binary `c8e12950` - code master-beta @ d33bc14f (engine = 8d41fbe3), target `Sun`, offset 0.3, viewportRadius 1024 px (projector's own dump), render 2048x2048.

Scene: shipped default config, time FROZEN at jd 2461288.9841951793, target at MERIDIAN transit (off-meridian 0.00002 deg, altitude 53.400 deg), atmosphere/landscape/stars off, farm `init_view_pos` = the target's measured local-frame direction.

`r/R` = distance from the DRAWN CENTRE in dome radii; `theta` = r/R * halfFov.


## The table: 3 fovs x 2 channels x 2 paths

| fov | channel | path | predicted r/R | dump r/R | frame r/R | theta measured | predicted theta | side |
|----:|---------|------|--------------:|---------:|----------:|---------------:|----------------:|------|
| 180 | config | old | 0.30000 | 0.30000 | 0.29996 | 27.0000 | 27.0000 | up |
| 180 | config | new | 0.30000 | 0.30000 | 0.29996 | 27.0000 | 27.0000 | up |
| 90 | config | old | 0.30000 | 0.30000 | 0.29997 | 13.5000 | 13.5000 | up |
| 90 | config | new | 0.30000 | 0.30000 | 0.29997 | 13.5000 | 13.5000 | up |
| 40 | config | old | 0.30000 | 0.30000 | 0.29999 | 6.0000 | 6.0000 | up |
| 40 | config | new | 0.30000 | 0.30000 | 0.29998 | 6.0000 | 6.0000 | up |
| 180 | command | old | 0.00000 | 0.00000 | 0.00018 | 0.0000 | 0.0000 | down |
| 180 | command | new | 0.30000 | 0.30000 | 0.29996 | 27.0000 | 27.0000 | up |
| 90 | command | old | 0.30000 | 0.30000 | 0.30004 | 13.5000 | 13.5000 | down |
| 90 | command | new | 0.30000 | 0.30000 | 0.29997 | 13.5000 | 13.5000 | up |
| 40 | command | old | 1.05000 | 1.05000 | OFF FRAME | 21.0000 | 21.0000 | down |
| 40 | command | new | 0.30000 | 0.30000 | 0.29999 | 6.0000 | 6.0000 | up |

## P1 - the aim compensation, measured directly

- angle(localVision before, after) across `set zoom_offset 0.3` while armed: **27.000000 deg**  (predicted offset x 90 = 27.0; a fixed 45 deg would give 13.5)
- before `[0.596219045, 2.05e-07, 0.802821805]` -> after `[0.166761595, 2.06e-07, 0.985997247]`
- the LOCAL EAST component is unchanged across the rotation (2.046e-07 -> 2.057e-07): the axis IS local y (East), measured, not assumed
- altitude 53.4004 deg -> 80.4004 deg: the aim is RAISED by the compensation (sign, measured)

## P6 - the 90 deg coefficient read as a slope (fov 40, command channel, old path)

| offset | predicted r/R | dump r/R | frame r/R | mutated (45 deg) |
|-------:|--------------:|---------:|----------:|-----------------:|
| 0.1 | 0.35000 | 0.35000 | 0.35000 | 0.12500 |
| 0.2 | 0.70000 | 0.70000 | 0.70000 | 0.25000 |
| 0.3 | 1.05000 | 1.05000 | OFF FRAME | 0.37500 |

## Zero control (offset 0, prep launch) - the instrument's own zero

| fov | path | dump r/R | frame centroid px | r px |
|----:|------|---------:|-------------------|-----:|
| 180 | old | 0.00000 | (1024.180, 1024.044) | 0.185 |
| 180 | new | 0.00000 | (1024.143, 1024.031) | 0.146 |
| 90 | old | 0.00000 | (1024.135, 1024.032) | 0.139 |
| 90 | new | 0.00000 | (1024.115, 1024.031) | 0.119 |
| 40 | old | 0.00000 | (1024.027, 1024.007) | 0.028 |
| 40 | new | 0.00000 | (1024.046, 1024.026) | 0.053 |

## Supplementary leg - the same sink's OTHER observable (the re-aim is a teleport)

Aim at `Jupiter` (both paths centred on it), offset 0, ARMED on both paths, fov 40; then `set zoom_offset 0.3`:
- OLD aim jumps **29.4474 deg**, landing 27.0000 deg from `init_view_pos` (= the compensation, exactly); Jupiter's old screen [1024, 1024] -> [2347.12939453125, 603.903076171875] -- OFF the rendered image
- NEW keeps its aim: Jupiter's new screen [-3.3238976e-07, 1.86969231e-07] -> [-3.32995484e-07, 0.299997807] (r/R 0.30000, on screen)

## Discriminating checks

- (a) table filled with MEASURED values; fov-180 old/command reads r/R 0.00000 = the control; the mutated prediction (fixed 45 deg) misses P1 by 2x, the fov-180 cell by 13.5 deg, the fov-90 cell by 13.5 deg and the sweep slope by 2.8x
- (b) the two channels DISAGREE on the old path (see the table) and AGREE on the new path
- (c) frame centroid vs dump agree on the new path at every fov
- (d) real ~/.spacecrafter md5 in == out on all four launches (config.ini 03fbee59, ssystem.ini 545a51ef)
- (e) the sink's contract: control.viewOffset = {'reported': 0.3, 'old': 0.3, 'new': 0.3}
