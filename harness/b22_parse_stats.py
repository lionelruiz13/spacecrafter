#!/usr/bin/env python3
# Parse query_statistics log/statistics.dat: sequence of {int32 type; float32
# datetime}, datetime = seconds since the frame's FRAME_START (type 0 resets it).
# Report per-frame stage durations over the settled tail, in microseconds.
import struct, sys, numpy as np, json, os
# capture type indices (capture.hpp enum order)
FRAME_START=0; FADER_UPDATE=18; EXECUTOR_UPDATE=19; FRAME_ACQUIRE=20
DRAW_RESOURCE_READY=21; EXECUTOR_DRAW=22; UI_DRAW=23; ASYNC_FRAME_SUBMIT=26
NAMES=["FRAME_START","INIT_VULKAN","INIT_GENERAL","INIT_APPDRAW","INIT_SPLASH","INIT_FONT",
 "INIT_CORE","INIT_UI","APP_INIT_BEGIN","APP_INIT_END","APP_MAINLOOP_START","APP_MAINLOOP_END",
 "SCRIPT_UPDATE","NETWORK_UPDATE","UI_UPDATE","MEDIA_AUDIO_UPDATE","MEDIA_IMAGE_UPDATE",
 "MEDIA_PLAYER_UPDATE","FADER_UPDATE","EXECUTOR_UPDATE","FRAME_ACQUIRE","DRAW_RESOURCE_READY",
 "EXECUTOR_DRAW","UI_DRAW","MEDIA_DRAW","FOREGROUND_DRAW","ASYNC_FRAME_SUBMIT"]

def load(path):
    raw=open(path,"rb").read()
    npts=len(raw)//8
    arr=struct.unpack("<"+("if")*npts, raw)
    pts=[(arr[2*i],arr[2*i+1]) for i in range(npts)]
    frames=[]; cur=None
    for typ,dt in pts:
        if typ==FRAME_START:
            if cur is not None: frames.append(cur)
            cur={FRAME_START:0.0}
        elif cur is not None:
            cur[typ]=dt
    if cur is not None: frames.append(cur)
    return frames

def stage(frames, a, b):
    out=[]
    for f in frames:
        if a in f and b in f:
            v=(f[b]-f[a])*1e6  # us
            if 0.0 <= v < 1e5:  # drop ring-buffer/wrap artifacts
                out.append(v)
    return np.array(out)

def report(path, tail=600):
    frames=load(path)
    # settled tail: last `tail` complete frames (must contain the draw stages)
    good=[f for f in frames if EXECUTOR_DRAW in f and DRAW_RESOURCE_READY in f and ASYNC_FRAME_SUBMIT in f]
    seg=good[-tail:] if len(good)>tail else good
    print(f"{path}: {len(frames)} frames, {len(good)} complete, analyzing last {len(seg)}")
    res={}
    for nm,a,b in [("EXECUTOR_DRAW",DRAW_RESOURCE_READY,EXECUTOR_DRAW),
                   ("EXECUTOR_UPDATE",FADER_UPDATE,EXECUTOR_UPDATE),
                   ("FRAME_ACQUIRE",FRAME_ACQUIRE-1,FRAME_ACQUIRE),  # EXECUTOR_UPDATE->FRAME_ACQUIRE
                   ("TOTAL_to_submit",FRAME_START,ASYNC_FRAME_SUBMIT)]:
        v=stage(seg,a,b)
        if len(v):
            res[nm]={"n":len(v),"median_us":float(np.median(v)),"mean_us":float(np.mean(v)),
                     "p05_us":float(np.percentile(v,5)),"p95_us":float(np.percentile(v,95)),
                     "min_us":float(v.min()),"max_us":float(v.max())}
            print(f"  {nm:18} median={np.median(v):8.2f}us mean={np.mean(v):8.2f}us "
                  f"p05={np.percentile(v,5):8.2f} p95={np.percentile(v,95):8.2f} "
                  f"min={v.min():7.2f} max={v.max():8.2f} (n={len(v)})")
    return res

if __name__=="__main__":
    out={}
    for p in sys.argv[1:]:
        out[p]=report(p)
    if len(sys.argv)>1:
        json.dump(out, open("stats_report.json","w"), indent=1)
