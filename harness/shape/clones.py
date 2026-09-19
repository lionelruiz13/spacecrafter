import sys, re, collections, hashlib, os
W = int(os.environ.get("W","6"))
files = [l.strip() for l in sys.stdin if l.strip()]
def norm(path):
    out=[]; inblk=False
    for i,l in enumerate(open(path,'rb').read().decode('utf-8','replace').split('\n'),1):
        s=l.strip()
        if inblk:
            if '*/' in s: inblk=False
            continue
        if s.startswith('/*'):
            if '*/' not in s: inblk=True
            continue
        s = re.sub(r'//.*$','',s).strip()
        s = re.sub(r'\s+',' ',s)
        if not s or s in ('{','}','};','else','} else {','break;','return;','public:','private:','#endif'): continue
        out.append((i,s))
    return out
idx=collections.defaultdict(list); data={}
for f in files:
    n=norm(f); data[f]=n
    for k in range(len(n)-W+1):
        h=hashlib.md5('\n'.join(s for _,s in n[k:k+W]).encode()).digest()
        idx[h].append((f,k))
dup=collections.defaultdict(set)
pairs=collections.Counter()
for h,occ in idx.items():
    if len(occ)<2: continue
    for f,k in occ:
        for j in range(k,k+W): dup[f].add(j)
    fs=sorted(set((f,k) for f,k in occ))
    for a in range(len(fs)):
        for b in range(a+1,len(fs)):
            pairs[(fs[a][0],fs[b][0])]+=1
tot=sum(len(v) for v in data.values()); d=sum(len(v) for v in dup.values())
print(f"files {len(files)}  significant code lines {tot}  lines inside a >={W}-line literal clone: {d} ({100*d/tot:.1f}%)")
print("-- per file (top 20) --")
for f,v in sorted(dup.items(), key=lambda kv:-len(kv[1]))[:20]:
    print(f"  {len(v):5d} / {len(data[f]):5d}  {f}")
print("-- file pairs sharing clone windows (top 20) --")
for (a,b),c in pairs.most_common(20):
    print(f"  {c:5d}  {a.replace('src/','')}  <->  {b.replace('src/','')}")
