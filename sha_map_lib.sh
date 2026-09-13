#!/usr/bin/env bash
#
# sha_map_lib.sh -- the sha-map drawer's ONE writer, and the prefix resolver
#                   both rewrite tools need.  Sourced, never executed.
#
# ---------------------------------------------------------------------------
# WHY THIS FILE EXISTS
# ---------------------------------------------------------------------------
# Two scripts in this directory rewrite history and therefore change shas:
# `supervised-by.sh` (message rewrite, both repos) and `purge-path.sh` (path
# removal, one repo).  Each leaves behind the same hazard -- citations of the
# old shas in surfaces no rewrite can reach: the git history of the cited files,
# the shared notes, the archive drawer, the owner's own trees -- and the same
# answer: a persisted `<old> TAB <new>` map, looked up by PREFIX, newest
# directory first.  `sha-maps/README.md` states that contract for readers.
#
# Until F119 only supervised-by.sh wrote the drawer; purge-path.sh re-shaed
# commits and persisted nothing at all, so the 2026-09-07 purge (harness
# 9116a6a) left a rewrite with no resolver.  The fix could have been a second
# copy of the writer in the second script.  It is not, and I2 is the reason: two
# copies of one convention do not stay one convention, they become two that
# LOOK like one -- and the failure is silent, because each script's own output
# keeps looking right.  The directory layout, the file names, the "empty rather
# than missing" rule and the README are one decision, so they live in one place
# and both callers ask for them.
#
# The cost of sourcing is stated rather than hidden: a caller whose copy of this
# file is missing dies at the `source` line with a shell error naming the path.
# That is the loud failure; the alternative -- each script carrying its own
# copy and drifting -- is the quiet one.
#
# ---------------------------------------------------------------------------
# THE DRAWER'S SHAPE (one directory per rewrite run)
# ---------------------------------------------------------------------------
#     sha-maps/<UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>/
#         code.tsv      one line per code-repo commit whose sha changed
#         harness.tsv   the same for the harness repo
#         repair.tsv    trailers ALREADY dangling when the run started, mapped
#                       to the fingerprint twin the run repaired them to
#
# The two tips in the name are the PRE-rewrite ones: they name the state the map
# maps FROM, which is the state a reader holding a stale sha is trying to escape.
#
# ALL THREE FILES ARE ALWAYS WRITTEN, empty where the run had nothing to say --
# a reader never has to tell "this run touched no harness commit" from "this
# writer forgot the file", and `wc -l` over a directory is a complete census
# without a per-file existence test.  A run that changed NO sha at all writes no
# directory: an entirely empty directory is not evidence of anything.
# ---------------------------------------------------------------------------

# sha_map_dir_name <code-tip8> <harness-tip8> -> the directory name, no path.
# A tip the caller does not have is spelled `--------`: eight characters that
# cannot be confused with a sha and sort before every one of them, rather than
# `00000000`, which is a legal abbreviated sha and would one day be read as one.
SHA_MAP_NO_TIP='--------'
sha_map_dir_name() {
    # Written with plain tests, not `${1:---------}`: that spelling is an
    # operator followed by eight dashes and reads as nine, which is the kind of
    # off-by-one nobody sees in review.
    local c=${1:-} h=${2:-}
    [ -n "${c}" ] || c="${SHA_MAP_NO_TIP}"
    [ -n "${h}" ] || h="${SHA_MAP_NO_TIP}"
    printf 'sha-maps/%s-%s-%s' "$(date -u +%Y%m%dT%H%M%SZ)" "${c}" "${h}"
}

# sha_map_write <repo> <dirname> <code-map|''> <harness-map|''> <repair-map|''>
#
# Copies the three maps VERBATIM into <repo>/<dirname>/ (no reformatting: the
# file the filter consulted is the file that is kept), writes sha-maps/README.md
# if it is not there yet, and STAGES everything.  Staging is part of the job:
# both callers close with `commit -a`, which stages tracked modifications and
# silently ignores untracked files, so a map left unstaged would be written and
# then quietly left behind.
#
# Sets SHA_MAP_FILES (array, repo-relative paths) and SHA_MAP_DIR (the dirname,
# empty when nothing was written).  Returns 0 always; "nothing to write" is data.
sha_map_write() {
    local repo=$1 dir=$2 codemap=${3:-} harnessmap=${4:-} repairmap=${5:-}
    local m src any=0
    SHA_MAP_FILES=(); SHA_MAP_DIR=""

    # "Did this run change any sha at all" is asked BEFORE the directory is
    # created, so the no-op run leaves no trace to explain.
    for src in "${codemap}" "${harnessmap}" "${repairmap}"; do
        [ -n "${src}" ] && [ -s "${src}" ] && any=1
    done
    [ "${any}" = 1 ] || return 0

    mkdir -p "${repo}/${dir}"
    local i=0
    for m in code harness repair; do
        i=$((i + 1))
        case ${i} in 1) src=${codemap} ;; 2) src=${harnessmap} ;; 3) src=${repairmap} ;; esac
        if [ -n "${src}" ] && [ -s "${src}" ]; then
            cp "${src}" "${repo}/${dir}/${m}.tsv"
        else
            : > "${repo}/${dir}/${m}.tsv"
        fi
        SHA_MAP_FILES+=("${dir}/${m}.tsv")
    done
    SHA_MAP_DIR="${dir}"

    # The contract, written ONCE. It is not regenerated per run: a file that is
    # rewritten every time is a file whose content is nobody's decision.
    if [ ! -e "${repo}/sha-maps/README.md" ]; then
        sha_map_readme > "${repo}/sha-maps/README.md"
        SHA_MAP_FILES+=("sha-maps/README.md")
    fi
    git -C "${repo}" add -- "${SHA_MAP_FILES[@]}"
    return 0
}

# sha_map_summary <repo> -> the one-line report both callers print.
sha_map_summary() {
    local repo=$1 f out=""
    [ "${#SHA_MAP_FILES[@]}" -gt 0 ] || { printf '(none)'; return 0; }
    for f in "${SHA_MAP_FILES[@]}"; do
        out+="$(printf '%s(%s) ' "${f##*/}" "$(wc -l < "${repo}/${f}")")"
    done
    printf '%s' "${out}"
}

sha_map_readme() {
    cat <<'SHAMAPREADME'
# sha-maps/ -- old sha -> new sha, one directory per history rewrite

Written by `claude/sha_map_lib.sh`, called by `claude/supervised-by.sh` and
`claude/purge-path.sh`. Every directory here is the record of ONE run that
re-hashed commits, named

    <UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>

after the PRE-rewrite tips of the two repositories: the state the map maps FROM.
A tip the run did not have is spelled `--------`.

    code.tsv      one line per code-repo commit whose sha changed
    harness.tsv   one line per harness-repo commit whose sha changed
    repair.tsv    trailers that were ALREADY dangling when the run started,
                  mapped to the fingerprint twin the run repaired them to

Each line is `<old-full-sha> TAB <new-full-sha>`. An unselected commit whose
parents did not change keeps its object -- sha and signature -- and appears in
no file. All three files are always present; an empty one means the run had
nothing of that kind, never that the writer omitted it.

`repair.tsv` is a verbatim SUBSET of `code.tsv` for the same run: the script
appends the repair pairs to the code map so the message filter consults one
file, and the copy kept here is that file, unedited. The two are not two
sources of one fact -- `repair.tsv` says which of those pairs were pre-existing
damage rather than this run's own work.

## Why the files exist

A rewrite gives every touched commit a new sha. The scripts repoint citations in
tracked files, and that is all they can reach. They do not reach the git history
of those same files, the shared notes, this repository's archive drawer, or
anything in the owner's own trees. Those citations stay valid only if the old
sha remains RESOLVABLE, and after a garbage collection nothing but a map can
resolve it.

## How to resolve a sha that no longer exists

Look the token up as a PREFIX of the old side, newest directory first, and take
the new side truncated to the same length. That is the same rule the scripts use
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
SHAMAPREADME
}

# ---------------------------------------------------------------------------
# THE PREFIX RESOLVER
#
# Moved here VERBATIM from purge-path.sh (its lines 296-309 before F119), which
# is where it was measured and where the reason for its shape is recorded:
# resolving an abbreviated citation is a prefix lookup, and there are a LOT of
# lookups -- the harness carries 269 111 distinct hex tokens in tracked files
# (md5 sums, JSON digests, engine hashes), the code tree 102 772, of which a
# handful are commit shas.  One subshell per token is minutes of work and was
# measured as such: the first version of that scan timed out at two minutes, and
# so did F119's first attempt at the same scan over the code tree.  So the table
# is built ONCE -- every prefix length 7..40 of every candidate sha, collisions
# marked AMBIG -- and every token is then a single hash lookup in one awk pass.
#
# It is here rather than in one of the two scripts because both now need it:
# supervised-by.sh's step E has to resolve tokens over the whole code tree, at
# which size its per-token bash loop is not usable.
# ---------------------------------------------------------------------------

# sha_prefix_table <file of full shas, one per line> <out-table>
sha_prefix_table() {
    cut -c1-40 "$1" | awk '{ for (n = 7; n <= length($0); n++) { pfx = substr($0, 1, n)
             if (pfx in seen && seen[pfx] != $0) seen[pfx] = "AMBIG"; else seen[pfx] = $0 } }
         END { for (pfx in seen) print pfx, seen[pfx] }' > "$2"
}

# sha_resolve_tokens <out-table> <file of tokens, one per line> -> "<token> <sha|AMBIG>"
sha_resolve_tokens() {
    awk 'NR==FNR { m[$1] = $2; next } ($1 in m) { print $1, m[$1] }' "$1" "$2"
}
