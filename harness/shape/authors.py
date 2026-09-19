import subprocess, re, sys, collections
paths = sys.argv[1:] or ['src']
out = subprocess.run(['git','log','--no-merges','-p','--format=@@@C %h|%an|%ad','--date=short','--']+paths,
                     capture_output=True).stdout.decode('utf-8','replace')
stats = collections.defaultdict(lambda: collections.Counter())
author=None; inblock=False
for line in out.split('\n'):
    if line.startswith('@@@C '):
        h,an,ad = line[5:].split('|'); author=an; stats[author]['commits']+=1; continue
    if line.startswith('+++') or line.startswith('---') or line.startswith('diff ') or line.startswith('index '): continue
    if not line or line[0] not in '+-': continue
    s = line[1:].strip()
    kind = 'blank' if not s else ('comment' if (s.startswith('//') or s.startswith('/*') or s.startswith('*')) else 'code')
    stats[author][('add_' if line[0]=='+' else 'del_')+kind]+=1
print(f"{'author':18} {'commits':>7} {'+code':>7} {'-code':>7} {'+cmt':>7} {'-cmt':>7} {'cmt/code added':>15} {'del/add code':>13}")
for a,c in sorted(stats.items(), key=lambda kv:-kv[1]['add_code']):
    print(f"{a:18} {c['commits']:7} {c['add_code']:7} {c['del_code']:7} {c['add_comment']:7} {c['del_comment']:7} {c['add_comment']/max(1,c['add_code']):15.2f} {c['del_code']/max(1,c['add_code']):13.2f}")
