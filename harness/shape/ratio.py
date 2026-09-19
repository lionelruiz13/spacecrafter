import subprocess, sys
rev = sys.argv[1]; path='src/experimentalModule'
files = subprocess.run(['git','ls-tree','-r','--name-only',rev,'--',path],capture_output=True,text=True).stdout.split()
tot=[0,0,0]
rows=[]
for f in files:
    if not f.endswith(('.hpp','.cpp')): continue
    txt = subprocess.run(['git','show',f'{rev}:{f}'],capture_output=True).stdout.decode('utf-8','replace')
    c=k=b=0; inblk=False
    for l in txt.split('\n'):
        s=l.strip()
        if inblk:
            k+=1
            if '*/' in s: inblk=False
            continue
        if not s: b+=1
        elif s.startswith('//'): k+=1
        elif s.startswith('/*'):
            k+=1
            if '*/' not in s: inblk=True
        else: c+=1
    rows.append((f.split('/')[-1],c,k)); tot[0]+=c; tot[1]+=k
print(rev, 'files',len(rows),'code',tot[0],'comment',tot[1],'ratio %.2f'%(tot[1]/max(1,tot[0])))
if len(sys.argv)>2:
    for r in sorted(rows,key=lambda r:-(r[1]+r[2]))[:14]: print('   %-28s code %5d  comment %5d  ratio %.2f'%(r[0],r[1],r[2],r[2]/max(1,r[1])))
