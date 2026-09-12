# sha-maps/ -- old sha -> new sha, one directory per history rewrite

Written by `claude/supervised-by.sh`. Every directory here is the record of ONE
run that re-hashed commits, named

    <UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>

after the PRE-rewrite tips of the two repositories: the state the map maps FROM.

    code.tsv      one line per code-repo commit whose sha changed
    harness.tsv   one line per harness-repo commit whose sha changed
    repair.tsv    trailers that were ALREADY dangling when the run started,
                  mapped to the fingerprint twin the run repaired them to

Each line is `<old-full-sha> TAB <new-full-sha>`. An unselected commit whose
parents did not change keeps its object -- sha and signature -- and appears in
no file.

`repair.tsv` is a verbatim SUBSET of `code.tsv` for the same run: the script
appends the repair pairs to the code map so the message filter consults one
file, and the copy kept here is that file, unedited. The two are not two
sources of one fact -- `repair.tsv` says which of those pairs were pre-existing
damage rather than this run's own work.

## Why the files exist

A rewrite gives every touched commit a new sha. The script repoints citations in
tracked `*.md`, and that is all it can reach. It does not reach the git history
of those same files, the shared notes, this repository's archive drawer, or
anything in the owner's own trees. Those citations stay valid only if the old
sha remains RESOLVABLE, and after a garbage collection nothing but a map can
resolve it.

## How to resolve a sha that no longer exists

Look the token up as a PREFIX of the old side, newest directory first, and take
the new side truncated to the same length. That is the same rule the script uses
internally, and the same shape as the archive drawer's resolution-by-convention
for moved documents: a reference is never rewritten in the record, it is
resolved through the map. A token that prefixes more than one old sha is
ambiguous and must be lengthened, not guessed.

## The rule these files live by

They are append-only and are NEVER edited, not to tidy them, not to merge them,
not to drop entries that look obsolete. A map that has been corrected has
stopped being evidence of what a particular run did, which is the only thing it
is for. If a run was wrong, the next run's map records the correction as its own
line, and both stay.
