import re, sys, subprocess, collections, statistics
def measure(files, read):
    blk=collections.Counter(); lens=[]; trailing=full=0; fn=fn_c=0; fields=fields_c=0; code=0; openers=collections.Counter()
    FN=re.compile(r'^\s*(?!if\b|for\b|while\b|switch\b|return\b|else\b|do\b)[\w:<>~\*&,\s]+?[\w~]+\s*\([^;{}]*\)\s*(const)?\s*(noexcept)?\s*(override|final)?\s*(=\s*(0|default|delete))?\s*[;{]\s*(//.*)?$')
    FIELD=re.compile(r'^\s*(static\s+|const\s+|mutable\s+)*[\w:<>\*&,\s]+\s+\**\w+(\s*=\s*[^;]+|\{[^;]*\})?(\s*,\s*\w+(\s*=\s*[^;,]+)?)*;\s*(//.*)?$')
    for f in files:
        L=read(f).split('\n'); run=0; depth=0
        for i,l in enumerate(L):
            s=l.strip()
            if s.startswith('//') or s.startswith('/*') or s.startswith('*'):
                run+=1; full+=1; lens.append(len(s)); 
                if run==1: openers[re.sub(r'^[/!*\s]+','',s).split(' ')[0].lower()[:12]]+=1
                continue
            if run: blk[min(run,6)]+=1; prevc=True
            else: prevc=False
            run=0
            if not s: continue
            code+=1
            tr = '//' in re.sub(r'"(\\.|[^"\\])*"','""',s) 
            if tr: trailing+=1; lens.append(len(s.split('//',1)[1]))
            # was the previous non-blank line a comment?
            j=i-1
            while j>=0 and not L[j].strip(): j-=1
            pc = j>=0 and L[j].strip().startswith(('//','/*','*'))
            if FN.match(l) and '(' in l:
                fn+=1; fn_c += (pc or tr)
            elif FIELD.match(l) and '(' not in l.split('//')[0] and not s.startswith(('return','using','typedef','#','friend','case','break','continue')):
                fields+=1; fields_c += (pc or tr)
    return dict(code=code, full=full, trailing=trailing, fn=fn, fn_c=fn_c, fields=fields, fields_c=fields_c, blk=dict(sorted(blk.items())), med=int(statistics.median(lens)) if lens else 0, p90=int(sorted(lens)[int(len(lens)*0.9)]) if lens else 0, openers=openers.most_common(10))
def show(name, m):
    print(f"{name}\n  code lines {m['code']}  full-line comment lines {m['full']} (1 per {m['code']/max(1,m['full']):.1f})  trailing comments {m['trailing']}")
    print(f"  functions {m['fn']}, commented {m['fn_c']} ({100*m['fn_c']/max(1,m['fn']):.0f}%)   fields {m['fields']}, commented {m['fields_c']} ({100*m['fields_c']/max(1,m['fields']):.0f}%)")
    print(f"  comment blocks by length {m['blk']}   comment length median {m['med']} chars, p90 {m['p90']}")
    print(f"  first words: {m['openers']}")
sh=lambda *a: subprocess.run(a,capture_output=True).stdout.decode('utf-8','replace')
base=[f for f in sh('git','ls-tree','-r','--name-only','4dfe7bb3','--','src/experimentalModule').split() if f.endswith('.hpp')]
head=[f for f in sh('git','ls-files','src/experimentalModule').split() if f.endswith('.hpp')]
show(f"OWNER, 4dfe7bb3: {len(base)} headers", measure(base, lambda f: sh('git','show','4dfe7bb3:'+f)))
show(f"NOW, HEAD: {len(head)} headers", measure(head, lambda f: open(f,errors='replace').read()))
both=[f for f in head if f in base]
show(f"NOW, only the {len(both)} headers that existed in his state", measure(both, lambda f: open(f,errors='replace').read()))
svc=['src/experimentalModule/'+x for x in ['SessionFile.hpp','ModularSystemFormat.hpp','CameraAnchors.hpp','ShadowService.hpp']]
show("NOW, the four dense service headers (all agent-created)", measure(svc, lambda f: open(f,errors='replace').read()))
