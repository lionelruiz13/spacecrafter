import subprocess, sys
PATH = 'src/experimentalModule'

def classify(txt):  # Return 'code' / 'comment' / 'blank' per line of txt.split('\n'); a trailing comment counts as code
    out=[]; inblk=False
    for l in txt.split('\n'):
        s=l.strip()
        if inblk:
            out.append('comment')
            if '*/' in s: inblk=False
            continue
        if not s: out.append('blank')
        elif s.startswith('//'): out.append('comment')
        elif s.startswith('/*'):
            out.append('comment')
            if '*/' not in s: inblk=True
        else: out.append('code')
    return out

def main():
    rev = sys.argv[1]
    files = subprocess.run(['git','ls-tree','-r','--name-only',rev,'--',PATH],capture_output=True,text=True).stdout.split()
    tot=[0,0]
    rows=[]
    for f in files:
        if not f.endswith(('.hpp','.cpp')): continue
        txt = subprocess.run(['git','show',f'{rev}:{f}'],capture_output=True).stdout.decode('utf-8','replace')
        cls = classify(txt)
        c, k = cls.count('code'), cls.count('comment')
        rows.append((f.split('/')[-1],c,k)); tot[0]+=c; tot[1]+=k
    print(rev, 'files',len(rows),'code',tot[0],'comment',tot[1],'ratio %.2f'%(tot[1]/max(1,tot[0])))
    if len(sys.argv)>2:
        for r in sorted(rows,key=lambda r:-(r[1]+r[2]))[:14]: print('   %-28s code %5d  comment %5d  ratio %.2f'%(r[0],r[1],r[2],r[2]/max(1,r[1])))

if __name__ == '__main__':
    main()
