# Run harness/shape/form.py's own measure()/show() on named files: HEAD (git show) against the working tree.
import ast, subprocess, sys
src = open('claude/harness/shape/form.py').read()
g = {}
for n in ast.parse(src).body:
    if isinstance(n, (ast.Import, ast.ImportFrom)) or (isinstance(n, ast.FunctionDef) and n.name in ('measure', 'show')):
        exec(compile(ast.Module([n], []), 'form.py', 'exec'), g)
for f in sys.argv[1:]:
    g['show']('%s @ HEAD' % f, g['measure']([f], lambda p: subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True).stdout.decode('utf-8', 'replace')))
    g['show']('%s @ working tree' % f, g['measure']([f], lambda p: open(p, errors='replace').read()))
