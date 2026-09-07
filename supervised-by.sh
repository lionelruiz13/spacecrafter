#!/usr/bin/env bash
#
# supervised-by.sh
#
# Records the supervision chain in the commit trailers of Claude-authored
# commits, across the REPO PAIR (spacecrafter + spacecrafter/claude), WITHOUT
# touching a single byte of code.
#
# ---------------------------------------------------------------------------
# THE RULE (one pass, one fixed point)
# ---------------------------------------------------------------------------
# A commit is SELECTED when both hold:
#     * its AUTHOR is a Claude identity  -- `Claude <Model> <version>`
#     * it does not already carry a `Supervised-By:` trailer
# On a selected commit, two things happen:
#     1. every `Co-Authored-By:` line whose identity EQUALS the author's is
#        deleted -- it is redundant, the author already says it;
#     2. `Supervised-By: <string you type in>` is appended to the trailer block.
# Every OTHER `Co-Authored-By:` line is kept verbatim. That is the whole point:
# when Fable 5 supervises Opus 5, the commit is authored by Opus 5 (the one that
# wrote the code) and co-authored by Fable 5 (the one that supervised it), and
# that middle link must survive. The result reads top-down as the chain:
#
#     Author:            Claude Opus 5        <- wrote it
#     Co-Authored-By:    Claude Fable 5       <- supervised that
#     Supervised-By:     Vixy                 <- supervised that
#
# Both shapes fall out of the same rule, which is why there is only one:
#   * chain      -- author Opus 5, co-author Fable 5 -> nothing dropped, trailer
#                   appended (co-author identity != author identity)
#   * single run -- author Fable 5, co-author Fable 5 (Claude Code's own commit
#                   rule makes a model list itself) -> the co-author line is
#                   dropped as redundant, trailer appended
#   * bare       -- author Claude, no co-author at all -> trailer appended
# Running it twice is a no-op: after the first pass every Claude-authored commit
# carries exactly one `Supervised-By:`, which is precisely what deselects it.
#
# ---------------------------------------------------------------------------
# THE REPO PAIR, AND WHY ONE RUN MUST DO BOTH
# ---------------------------------------------------------------------------
# The harness repo is its own git repo cloned at `claude/` inside the code repo.
# The dependency between them is ONE-WAY BY DESIGN: every harness commit records
# the code state it was made against,
#
#     Code: master-beta @ 0453f75e
#
# and harness *.md cites code commits by sha, while the code repo cites nothing
# at all (measured: 0 hash-shaped tokens in tracked code *.md). So:
#
#     rewriting the code repo INVALIDATES references held in the harness repo
#     rewriting the harness repo invalidates nothing outside it
#
# An earlier version of this script rewrote one repo at a time and repointed
# only `*.md` inside THAT repo. Both halves of that are too narrow: the `Code:`
# references live in the harness repo's COMMIT MESSAGES, not its .md, and they
# live in the OTHER repo from the one being rewritten. Neither the file filter
# nor the repo scope could ever reach them. They would have gone dangling
# silently -- git never validates a sha written inside a message.
#
# Hence: one invocation, both repos, code first (the direction of the
# dependency, same reason the commit convention says "commit code first"), and
# the harness message filter applies BOTH transforms in a SINGLE pass -- the
# Supervised-By rule AND the old->new code-sha remap of `Code:` trailers. Two
# passes would re-sha the harness twice and invalidate whatever the first pass
# had just repointed.
#
# ---------------------------------------------------------------------------
# THE DEFAULT RANGE: `@{upstream}..HEAD`, NOT `HEAD`
# ---------------------------------------------------------------------------
# The rewrite changes shas. Doing that to a commit that was already pushed means
# force-pushing over history other clones may hold; doing it to a commit that
# was never pushed costs nothing. The upstream ref is exactly the line between
# those two, so it is the default, per repo. (It is not a conservatism dial: it
# is where the operation stops being free.)
#
# This is not hypothetical either. At the time this default was introduced, the
# old `HEAD` default selected 34 code commits, of which 4 -- ba578ede and the
# three 2026-07-22 INTENT commits -- were ALREADY PUSHED. Pass an explicit
# --code=<range> when you genuinely mean to rewrite published history.
#
# ---------------------------------------------------------------------------
# WHY `git filter-branch --msg-filter` (and not `git rebase`)
# ---------------------------------------------------------------------------
# `git rebase` REPLAYS commits: it re-applies each one onto a new base. If the
# range contains a MERGE commit, `--rebase-merges` emits a `merge -C <sha>` step
# that re-PERFORMS the merge -- so every conflict must be resolved again by hand,
# and a wrong resolution silently changes the code. Renaming a message needs no
# replay at all. Rebase is simply the wrong instrument for this job.
#
# filter-branch walks a range and rebuilds each commit, letting you filter parts
# of it. With --msg-filter it pipes ONLY the commit message through your command
# (message on stdin, new message on stdout) and reuses the original TREE and the
# original PARENTS verbatim. Therefore:
#   * merges keep both parents -- nothing is re-merged, no conflicts possible
#   * content cannot change    -- we assert this at the end rather than trust it
#   * a commit nothing changes about is not rebuilt AT ALL -- see the next
#     section; only rewritten commits and their descendants get new SHAs
#
# The filter is THIS SCRIPT re-invoked as `supervised-by.sh --msg-filter`, so
# the selection shown in the preview and the selection actually applied are the
# same code. Two copies of a rule are two rules waiting to drift apart.
#
# (Modern alternative: `git filter-repo --message-callback`, which is faster but
#  is a separate install. filter-branch is used here because it ships with git.)
#
# ---------------------------------------------------------------------------
# THE COMMIT FILTER: THE OTHER COMMITS ARE NOT REWRITTEN, THEY ARE LEFT ALONE
# ---------------------------------------------------------------------------
# [vixy 2026-09-07, INTENT 11.227(a), verbatim]:
#
#     "the rewrite should only rewrite the commit message (and author) on
#      commit Claude is the author, from the last pushed change to the HEAD.
#      I don't know much about how verified commit works, though, but the
#      commit themselves shouldn't be affected, so their signature shouldn't
#      move."
#
# filter-branch does NOT do that on its own. It rebuilds every commit in the
# range: its default commit filter is `git commit-tree "$@"`, which mints a new
# object out of tree + parents + the ident environment + the filtered message.
# For an unselected UNSIGNED commit that object comes out byte-identical, so the
# sha does not move and the rebuild is invisible -- which is why nobody noticed.
# For a SIGNED one it does not: `commit-tree` cannot write a `gpgsig` header, so
# the signature is dropped, and the sha moves with it. Worse, where the unsigned
# object that results ALREADY EXISTS in the history, the "rebuilt" commit IS
# that pre-existing commit, and a duplicated chain collapses onto its twin.
# Measured on a clone of this very pair (11.224(h), reproduced three times):
# master-beta 3829 -> 3816 commits, thirteen gone and one contributor's GitHub
# signature with them, while `git diff --quiet` kept passing because the tip
# TREE was untouched.
#
# So the rewrite is given a `--commit-filter`, and it answers one question per
# commit: is ANYTHING about this commit different? A GPG signature signs the
# WHOLE object -- tree, parents, author, committer, message -- so it survives
# exactly when all five are unchanged, and those five are what is compared:
#
#     all five equal  ->  print $GIT_COMMIT: the original object stays, sha and
#                         gpgsig and all. filter-branch accepts an existing id.
#     otherwise       ->  `git commit-tree "$@"`, exactly as before.
#
# Three of the five would not do. Comparing only message, parents and author
# would let a --tree-filter or a committer rewrite through, and emitting the
# original id for a commit that is NOT the original produces a silently WRONG
# history instead of a silently shortened one. Today the other two are inert by
# construction -- no tree/index/subdirectory filter is passed, so filter-branch
# hands the commit's own tree over verbatim, and the env-filter above exports
# GIT_AUTHOR_NAME and nothing else -- but a validation belongs on the value, not
# on the caller's promise to have validated it.
#
# THE ONE CASE THAT CANNOT BE SAVED, and it is named rather than hidden: a
# signed commit whose MAPPED PARENT changed, i.e. one sitting downstream of a
# rewritten Claude commit inside the range. Its object must change, so its
# signature cannot survive, by construction and not by any choice made here.
# Each one is recorded and reported by name -- a section-2(f) block at the
# rewrite and again in the closing summary. In the live pair that set is empty
# (the single signed commit hangs off an already-pushed parent), and a run that
# prints nothing about signatures is a run in which nothing was lost.
#
# ---------------------------------------------------------------------------
# WHY IDENTITIES ARE MATCHED BY NAME AND NOT BY EMAIL
# ---------------------------------------------------------------------------
# Every Claude identity shares <noreply@anthropic.com>. Matching co-author to
# author on the email would therefore call Fable 5 "redundant" on an Opus 5
# commit and delete the exact link this script exists to preserve. The model
# NAME is the only discriminating field, so both sides are reduced to the
# substring `Claude <Model> <version>` and those are compared for equality:
#   * `Claude Opus 5 (1M context) <noreply@anthropic.com>` (the trailer Claude
#     Code writes) reduces to `Claude Opus 5`, which equals what the git
#     author.name `Claude Opus 5` reduces to -- the parenthetical suffix cannot
#     make a self-reference look like a supervisor;
#   * `Claude Fable 5` != `Claude Opus 5`, so the supervisor line survives.
# Equality of the REDUCED forms, never substring containment: `Claude Fable 5`
# is a substring of `Claude Fable 50`, and would one day be a false match.
#
# ---------------------------------------------------------------------------
# THE .md RE-POINT (step E)
# ---------------------------------------------------------------------------
# The rewrite gives every touched commit a NEW sha, and the harness .md trackers
# (INTENT.md, the dispatch logs, ...) cite commits BY sha to trace work -- both
# harness commits AND code commits. After a rewrite those citations point at
# dangling objects and the traceability they exist for silently rots. Step E
# builds the old->new map for BOTH repos, merges them, and offers to repoint
# every abbreviated hash it finds. A token that prefixes rewritten commits in
# both maps at once is flagged ambiguous and never auto-replaced. It touches
# ONLY tracked *.md, and never applies without your y/N.
# (Requires bash 4+ for associative arrays.)
#
# ---------------------------------------------------------------------------
# THE PERSISTED SHA MAPS (written by step D, committed by step E)
# ---------------------------------------------------------------------------
# Step E reaches tracked *.md in the harness repo. That is not everything that
# cites these commits. It reaches neither the git HISTORY of those files nor any
# surface outside the pair -- the shared queue and host-event notes, the owner's
# own notes, scratch trees, the dispatch archive. Those spaces are exactly the
# ones that cannot be search-and-replaced, and a sha that resolves to nothing
# reachable is a dangling reference with no way back.
#
# So the old->new maps this run builds are no longer thrown away with the temp
# directory. They are copied to
#
#     <harness>/sha-maps/<UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>/
#         code.tsv      old-full-sha <TAB> new-full-sha, per code commit re-shaed
#         harness.tsv   the same for the harness repo
#         repair.tsv    trailers that were ALREADY dangling, mapped to their twin
#
# and committed by the closing commit. The two tips in the directory name are the
# PRE-rewrite tips: they name the state the map maps FROM, which is the state a
# reader holding a stale sha is trying to escape. A stale citation is then
# resolved by CONVENTION -- look the token up as a PREFIX in the maps, newest
# first -- the same shape the archive drawer uses for moved documents, and never
# a rewrite of the record. The maps are append-only files that are never edited:
# a map that is corrected is a map that has stopped being evidence.
#
# The consequence for the closing commit: it now happens whenever a map was
# written, even if there was nothing to repoint in *.md, because leaving the maps
# uncommitted would leave the run's own resolver in the working tree only. Its
# EXPECTED set is the touched *.md UNION the map files, so `commit -a` is still
# proved equal to the intended set rather than assumed to be.
#
# ---------------------------------------------------------------------------
# WHEN A TRAILER NAMES A BRANCH THAT NO LONGER RESOLVES (--branch-alias)
# ---------------------------------------------------------------------------
# A `Code:` trailer records the branch name as it was when the commit was made.
# Rename the branch and every historical trailer names a ref that is gone. This
# script used to answer that by judging the trailer against HEAD instead, with no
# message. That substitution is NOT verdict-preserving and it was measured: one
# and the same trailer reads reachable against `refs/heads/master-beta` and
# dangling against `refs/heads/2023-master`, so merely having a different branch
# checked out -- an ordinary act -- turned every trailer of the renamed branch
# into "dangling", which build_repair_map then calls UNREPAIRABLE, which is this
# script's own word for "repoint by hand".
#
# A verdict computed against a target nobody asked for is worse than no verdict,
# so there is no fallback any more: an unresolvable trailer branch is a STOP at
# the preview, before any prompt and under --dry-run as well (a preview that
# would lie must not exit 0). The repair channel is
#
#     --branch-alias=<old-name>=<new-name>        (repeatable)
#
# which says "judge trailers naming <old-name> against refs/heads/<new-name>".
# <new-name> must resolve at startup, so a typo fails immediately instead of
# quietly reintroducing the substitution this replaced.
#
# Usage:
#   ./supervised-by.sh                        # both repos, @{upstream}..HEAD each
#   ./supervised-by.sh --code=93a14377..HEAD  # override one side explicitly
#   ./supervised-by.sh --harness=HEAD~20..HEAD
#   ./supervised-by.sh --dry-run              # preview only, never prompts
#   ./supervised-by.sh --branch-alias=master-beta=main   # after a branch rename
# ---------------------------------------------------------------------------

set -euo pipefail

# ---------------------------------------------------------------------------
# The identity grammar. `Claude <Model> <version>`, model left open ([A-Za-z]+)
# rather than enumerated: an enumeration has to be edited every time a model is
# added, and the failure of a stale enumeration is SILENT -- the commit is not
# selected, no error is raised, and the missing trailer is noticed years later
# or never. (This is not hypothetical: the previous version of this script
# enumerated `Fable 5|Opus 4.8`, and by the time it was next run there were 15
# Opus 5 commits it walked straight past.) `[.]` not `\.` so the one string is
# valid both as a bash =~ regex and as an awk dynamic regex.
# ---------------------------------------------------------------------------
MODEL_RE='Claude [A-Za-z]+ [0-9]+([.][0-9]+)*'

# identity <string> -> the `Claude <Model> <version>` substring, empty if none.
# Used on the author name AND on each co-author value, so the two sides are
# always reduced by the same function.
identity() {
    local s=${1:-}
    [[ ${s} =~ ${MODEL_RE} ]] && printf '%s' "${BASH_REMATCH[0]}"
    return 0
}

# ---------------------------------------------------------------------------
# THE WILDCARD AUTHOR
# ---------------------------------------------------------------------------
# `git config user.name Claude` -- bare, no model, no version. It says "some
# Claude, model not recorded here", and it exists so the config NEVER has to be
# kept in sync with whichever model is acting. Keeping it in sync is discipline,
# and discipline fails silently: a stale `Claude Fable 5` config under an Opus 5
# session writes an INVERTED chain (author Fable 5, co-author Opus 5, read as
# "Fable wrote it, Opus supervised") that no later step can tell from a true one.
#
# The wildcard removes the sync requirement instead of demanding it, because the
# acting model's identity is ALREADY recorded, by Claude Code's own commit rule,
# in the `Co-Authored-By:` trailer it writes about itself. So the author is
# RESOLVED from the message rather than stated by config, and then written into
# the author field -- traceability lives in the field git tools read, not in a
# trailer a reader has to know to look for.
#
# Precondition, and it is the load-bearing one: the co-author trailer names the
# WRITER. That holds while Claude Code makes a model list itself. If a trailer
# ever names only a supervisor, this promotes the supervisor to author, and the
# result is indistinguishable from correct afterwards. Which is exactly why
# anything other than a single unambiguous candidate is refused below rather
# than guessed.
# ---------------------------------------------------------------------------
WILDCARD_AUTHOR='Claude'

# resolve_wildcard <message> -> the single Claude identity named by the
# Co-Authored-By lines. EMPTY when there are none (nothing to resolve from) or
# more than one distinct (no criterion here says which is the writer). Empty
# means "hand repair", never "pick one".
resolve_wildcard() {
    local line id; local -A seen=()
    while IFS= read -r line; do
        [[ ${line,,} == co-authored-by:* ]] || continue
        id=$(identity "${line#*:}"); [ -n "${id}" ] && seen["${id}"]=1
    done <<< "${1}"
    [ "${#seen[@]}" -eq 1 ] && printf '%s' "${!seen[@]}"
    return 0
}

# resolve_author <sha> <message> -> the identity to USE as author. A hand-given
# --author-fix wins over the trailer: it is the repair channel for the cases
# resolve_wildcard refuses.
resolve_author() {
    local sha=$1 msg=$2 s id
    if [ -n "${AUTHOR_FIX_MAP:-}" ] && [ -s "${AUTHOR_FIX_MAP}" ]; then
        while read -r s id; do
            [ -n "${s}" ] || continue
            [ "${sha:0:${#s}}" = "${s}" ] && { printf '%s' "${id}"; return 0; }
        done < "${AUTHOR_FIX_MAP}"
    fi
    resolve_wildcard "${msg}"
}

# has_supervisor <message> -> 0 if a `Supervised-By:` trailer line is present.
has_supervisor() {
    [[ $'\n'${1}$'\n' == *$'\n'Supervised-By:* ]]
}

# --branch-alias=<old>=<new>: `<old>` -> `<new>`. Empty until MAIN parses the
# arguments; every consultation happens after that, and `${A[k]:-}` on an empty
# associative array is well defined, so the declaration here is documentation as
# much as initialisation.
declare -A BRANCH_ALIAS=()

# resolve_branch_target <branch> -> the ref a trailer naming <branch> is judged
# against: its --branch-alias target if one was given, else `refs/heads/<branch>`.
# Returns 1, printing nothing, when the name is non-empty and NEITHER resolves.
#
# That third answer is the whole point of this function existing. The previous
# code answered it with `target=HEAD` and no message, which is not
# verdict-preserving: measured, one trailer reads reachable against
# `refs/heads/master-beta` and dangling against `refs/heads/2023-master`, so the
# answer depended on which branch happened to be checked out. Substituting a
# target nobody named produces a verdict that looks like every other verdict.
# The callers must be able to tell "I judged it" from "I could not judge it",
# which means this has to be able to say so.
#
# An EMPTY branch is not that case: it is "no branch was named", which is what
# a one-argument call means, and HEAD is then the only thing there is to ask.
resolve_branch_target() {
    local br=${1:-} tgt
    [ -n "${br}" ] || { printf 'HEAD'; return 0; }
    tgt=${BRANCH_ALIAS[${br}]:-}
    if [ -n "${tgt}" ]; then printf 'refs/heads/%s' "${tgt}"; return 0; fi
    if git -C "${CODE_REPO}" rev-parse --verify -q "refs/heads/${br}" >/dev/null 2>&1; then
        printf 'refs/heads/%s' "${br}"; return 0
    fi
    return 1
}

# reachable_in_code <sha> [branch] -> THREE outcomes, never a substitution:
#     0  reachable      -- <sha> is an ancestor of the branch's tip
#     1  unreachable    -- it is not, or its object is gone entirely
#     2  unjudgeable    -- <branch> resolves to no ref and to no --branch-alias
# NOT `rev-parse --verify`, and this distinction is the whole point:
# filter-branch leaves the pre-rewrite objects in the database (that is exactly
# how build_map below reads the OLD side of a rewrite), so `rev-parse`,
# `cat-file -e` and friends SUCCEED on a sha that no branch can reach any more.
# A `Code:` trailer pointing there is broken in the only sense that matters --
# `git show` still works for whoever runs it today, and resolves to nothing at
# all once the objects are gc'd. Reachability is the property; existence is a
# proxy that fails silently in the exact case being tested for.
#
# The target is resolved FIRST, before the object test: whether we can judge at
# all is prior to what the answer would be.
reachable_in_code() {
    local sha=$1 br=${2:-} target
    target=$(resolve_branch_target "${br}") || return 2
    git -C "${CODE_REPO}" rev-parse --verify -q "${sha}^{commit}" >/dev/null 2>&1 || return 1
    git -C "${CODE_REPO}" merge-base --is-ancestor "${sha}" "${target}" 2>/dev/null
}

# list_code_trailers <harness-range> -> `<branch> <sha>` per distinct trailer.
list_code_trailers() {
    git -C "${HARNESS_REPO}" log --format='%B' "$1" \
      | sed -nE 's/^Code:[[:space:]]+([^[:space:]]+)[[:space:]]+@[[:space:]]+([0-9a-f]{7,40})[[:space:]]*$/\1 \2/p' \
      | sort -u
}

# dangling_trailers <harness-range> -> the subset whose sha no branch reaches.
# Deliberately unchanged by the three-outcome split: anything that is not a
# proven 0 belongs on this list. An unjudgeable trailer is not silently promoted
# to "fine" -- it is refused outright, by the step-A guard below, long before
# this count is used for anything.
dangling_trailers() {
    local br sha
    while read -r br sha; do
        [ -n "${sha}" ] || continue
        reachable_in_code "${sha}" "${br}" || printf '%s %s\n' "${br}" "${sha}"
    done < <(list_code_trailers "$1")
}

# unresolvable_trailers <harness-range> -> `<branch> <count>` per DISTINCT branch
# name that resolve_branch_target refuses, with the number of distinct trailers
# naming it. Per branch and not per trailer: the branch is the thing that is
# broken and the thing the reader has to fix, and one ref test per name costs
# one git call instead of one per trailer.
unresolvable_trailers() {
    local n br
    while read -r n br; do
        [ -n "${br}" ] || continue
        if ! resolve_branch_target "${br}" >/dev/null; then
            printf '%s %s\n' "${br}" "${n}"
        fi
    done < <(list_code_trailers "$1" | awk '{print $1}' | sort | uniq -c)
}

# map_lookup <mapfile> <short-or-full-sha> -> new sha truncated to the same
# length, empty if the token prefixes no rewritten commit, `AMBIG` if it
# prefixes more than one. The map file is `<old-full-sha><TAB><new-full-sha>`
# per line. Prefix resolution rather than exact match, because references in
# the wild are abbreviated and their length is not ours to choose.
map_lookup() {
    local mapfile=$1 tok=$2 n=${#2} hit="" old new
    [ -s "${mapfile}" ] || return 0
    while IFS=$'\t' read -r old new; do
        if [ "${old:0:${n}}" = "${tok}" ]; then
            [ -n "${hit}" ] && { printf 'AMBIG'; return 0; }
            hit="${new}"
        fi
    done < "${mapfile}"
    [ -n "${hit}" ] && printf '%s' "${hit:0:${n}}"
    return 0
}

# ---------------------------------------------------------------------------
# THE MSG-FILTER MODE
# ---------------------------------------------------------------------------
# filter-branch runs this once per commit in the range, message on stdin, new
# message on stdout, with $GIT_COMMIT set to the ORIGINAL sha and the author
# ident exported. Two INDEPENDENT transforms are applied:
#
#   (1) `Code:` trailer remap, when $CODE_MAP names a non-empty map file.
#   (2) the Supervised-By rule, when the commit is selected.
#
# Independent, not nested: a harness commit that already carries Supervised-By
# is deselected from (2) but may still need (1). Folding (1) under (2)'s guard
# would work today only because no harness commit has the trailer yet -- and
# would break on the very next run, once this run has given all of them one.
#
# Anything left unchanged is emitted byte-for-byte, because an unchanged message
# rebuilds to the same SHA: the cheapest way to keep history stable is to not
# touch it.
# ---------------------------------------------------------------------------
# --- the env-filter: rewrite the AUTHOR of wildcard commits -----------------
# filter-branch `eval`s the env-filter string, so this mode PRINTS shell code
# for it to eval. It runs before the msg-filter within the same commit
# iteration and its export survives into it (verified), so the identity is
# resolved exactly once and both filters act on the same answer.
# Printing nothing leaves the author untouched -- which is what an unresolvable
# wildcard must get: the commit stays visibly broken instead of being half
# repaired into something that looks finished.
if [ "${1:-}" = "--emit-env" ]; then
    [ "${GIT_AUTHOR_NAME:-}" = "${WILDCARD_AUTHOR}" ] || exit 0
    RESOLVED=$(resolve_author "${GIT_COMMIT}" "$(git log -1 --format='%B' "${GIT_COMMIT}")")
    # Emit only a WELL-FORMED identity. An empty or malformed name would be
    # exported verbatim and git rejects it ("empty ident name not allowed"),
    # aborting the whole rewrite -- a validation belongs on the value, not on
    # the caller's promise to have validated it.
    [ -n "$(identity "${RESOLVED}")" ] && printf 'export GIT_AUTHOR_NAME=%q\n' "${RESOLVED}"
    exit 0
fi

if [ "${1:-}" = "--msg-filter" ]; then
    # Read the message preserving its exact trailing newlines: $(...) strips
    # them, so a sentinel is appended and then removed.
    MSG=$(cat; printf X); MSG=${MSG%X}
    NEW="${MSG}"

    # --- (1) Code: <branch> @ <sha>  ->  the sha's post-rewrite form ---------
    if [ -n "${CODE_MAP:-}" ] && [ -s "${CODE_MAP}" ] \
       && [[ $'\n'${NEW} == *$'\n'Code:\ * ]]; then
        OUT=""; TOUCHED=0
        while IFS= read -r LINE; do
            if [[ ${LINE} =~ ^(Code:[[:space:]]+[^[:space:]]+[[:space:]]+@[[:space:]]+)([0-9a-f]{7,40})[[:space:]]*$ ]]; then
                REPL=$(map_lookup "${CODE_MAP}" "${BASH_REMATCH[2]}")
                if [ -n "${REPL}" ] && [ "${REPL}" != "AMBIG" ]; then
                    LINE="${BASH_REMATCH[1]}${REPL}"; TOUCHED=1
                elif [ "${REPL}" = "AMBIG" ]; then
                    # Never guess. Recorded so step D can refuse to proceed.
                    printf '%s %s\n' "${GIT_COMMIT}" "${BASH_REMATCH[2]}" \
                        >> "${CODE_MAP}.ambig" 2>/dev/null || true
                fi
            fi
            OUT+="${LINE}"$'\n'
        done <<< "${NEW}"
        [ "${TOUCHED}" = 1 ] && NEW="${OUT%$'\n'}"
    fi

    # --- (2) the Supervised-By rule ------------------------------------------
    # GIT_AUTHOR_NAME is exported by filter-branch, and by the time we get here
    # it is the RESOLVED name when --emit-env resolved a wildcard. The `git log`
    # fallback keeps this correct if that export ever stops happening, rather
    # than silently reducing an empty string to "not a Claude author" and
    # skipping every commit.
    #
    # An UNRESOLVED wildcard is still the bare `Claude`, which MODEL_RE does not
    # match (it demands a model AND a version) -- so AUTHOR_ID comes out empty
    # and the whole transform is skipped. That is deliberate, not incidental:
    # the commit keeps its wrong author AND gains no Supervised-By, so it stays
    # selected and stays listed as needing hand repair. Appending the trailer
    # would deselect it and quietly retire the only signal that it is broken.
    AUTHOR_NAME=${GIT_AUTHOR_NAME:-$(git log -1 --format='%an' "${GIT_COMMIT}")}
    AUTHOR_ID=$(identity "${AUTHOR_NAME}")
    if [ -n "${AUTHOR_ID}" ] && ! has_supervisor "${NEW}"; then
        # Drop the redundant self co-author. The $(...) is load-bearing, not
        # laziness: it strips ALL trailing newlines, which is what removes the
        # blank line the deleted trailer leaves behind. Without it, `body /
        # blank / <deleted>` reaches interpret-trailers as a paragraph break and
        # the message ends on a stray empty line that the commits keeping their
        # co-author do not have -- two shapes in the corpus for one rule.
        BODY=$(printf '%s' "${NEW}" | awk -v aid="${AUTHOR_ID}" -v re="${MODEL_RE}" '
                tolower($0) ~ /^co-authored-by:/ {
                    v = $0; sub(/^[^:]*:[ \t]*/, "", v)
                    if (match(v, re) && substr(v, RSTART, RLENGTH) == aid) next
                }
                { print }
            ')
        # interpret-trailers is what knows where a trailer block starts, whether
        # a blank line is needed to open one, and (--if-exists doNothing) that an
        # existing Supervised-By must not be duplicated -- a second, independent
        # guard on top of the has_supervisor test above.
        NEW=$(printf '%s\n' "${BODY}" \
              | git interpret-trailers --if-exists doNothing \
                                       --trailer "Supervised-By: ${SUPERVISOR}")
        NEW="${NEW}"$'\n'
    fi

    printf '%s' "${NEW}"
    exit 0
fi

# ---------------------------------------------------------------------------
# THE COMMIT-FILTER MODE
# ---------------------------------------------------------------------------
# filter-branch runs this once per commit, INSTEAD of `git commit-tree "$@"`,
# with: the filtered message on stdin, `<tree> -p <mapped parent>...` in "$@",
# $GIT_COMMIT set to the ORIGINAL sha, the author/committer environment as the
# env-filter left it -- and whatever this prints on stdout is taken as the new
# commit id, INCLUDING an id that already exists. The rule is in the header
# section "THE COMMIT FILTER"; this is its implementation.
#
# The five fields a signature signs are compared against the ORIGINAL OBJECT,
# read here rather than asked of git a field at a time: `git cat-file commit`
# once per commit is the whole cost. The header ends at the first TRULY EMPTY
# line -- a `gpgsig` block's own blank lines are indented by one space, so they
# are not it, which is what makes this the same split filter-branch itself makes
# when it feeds the message to the msg-filter.
# ---------------------------------------------------------------------------
if [ "${1:-}" = "--commit-filter" ]; then
    shift
    CF_ARGS=("$@")                     # kept verbatim for `git commit-tree "$@"`

    # The message as filter-branch will store it. Same sentinel as the
    # msg-filter, for the same reason: $(...) strips trailing newlines and a
    # commit message's are part of it.
    CF_MSG=$(cat; printf X); CF_MSG=${CF_MSG%X}

    CF_OBJ=$(git cat-file commit "${GIT_COMMIT}"; printf X); CF_OBJ=${CF_OBJ%X}
    CF_HEAD=${CF_OBJ%%$'\n\n'*}
    CF_ORIG_MSG=${CF_OBJ#"${CF_HEAD}"$'\n\n'}

    # One pass over the header. `case` and not `[ ] &&`: an AND-list whose test
    # fails returns non-zero, and as a loop body's last statement that becomes
    # the loop's status under `set -e` (the trap measured at 11.224(f)).
    # Continuation lines of a multi-line header start with a space, so none of
    # these patterns can match one.
    CF_TREE=""; CF_PARENTS=""; CF_AUTHOR=""; CF_COMMITTER=""; CF_SIGNED=0
    while IFS= read -r CF_L; do
        case "${CF_L}" in
            "tree "*)      CF_TREE=${CF_L#tree } ;;
            "parent "*)    CF_PARENTS="${CF_PARENTS}${CF_L#parent }"$'\n' ;;
            "author "*)    CF_AUTHOR=${CF_L#author } ;;
            "committer "*) CF_COMMITTER=${CF_L#committer } ;;
            gpgsig*)       CF_SIGNED=1 ;;
        esac
    done <<< "${CF_HEAD}"

    # What "$@" is asking for, in the same two shapes.
    CF_NEW_TREE=${CF_ARGS[0]:-}; CF_NEW_PARENTS=""; CF_I=1
    while [ "${CF_I}" -lt "${#CF_ARGS[@]}" ]; do
        if [ "${CF_ARGS[${CF_I}]}" = "-p" ] && [ $((CF_I + 1)) -lt "${#CF_ARGS[@]}" ]; then
            CF_NEW_PARENTS="${CF_NEW_PARENTS}${CF_ARGS[$((CF_I + 1))]}"$'\n'
            CF_I=$((CF_I + 2))
        else
            # A shape this comparison does not understand. Do not claim equality
            # for something unread: fall through to the rebuild.
            CF_NEW_PARENTS="?unparsed"; break
        fi
    done

    # The ident lines `git commit-tree` would write, rebuilt from the
    # environment filter-branch exported. set_ident writes GIT_*_DATE as
    # `@<seconds> <tz>`; the object stores it without the `@`.
    CF_AD=${GIT_AUTHOR_DATE:-};    CF_AD=${CF_AD#@}
    CF_CD=${GIT_COMMITTER_DATE:-}; CF_CD=${CF_CD#@}
    CF_A_ENV="${GIT_AUTHOR_NAME:-} <${GIT_AUTHOR_EMAIL:-}> ${CF_AD}"
    CF_C_ENV="${GIT_COMMITTER_NAME:-} <${GIT_COMMITTER_EMAIL:-}> ${CF_CD}"

    CF_SAME=1
    [ "${CF_NEW_TREE}"    = "${CF_TREE}"      ] || CF_SAME=0
    [ "${CF_NEW_PARENTS}" = "${CF_PARENTS}"   ] || CF_SAME=0
    [ "${CF_MSG}"         = "${CF_ORIG_MSG}"  ] || CF_SAME=0
    [ "${CF_A_ENV}"       = "${CF_AUTHOR}"    ] || CF_SAME=0
    [ "${CF_C_ENV}"       = "${CF_COMMITTER}" ] || CF_SAME=0

    if [ "${CF_SAME}" = 1 ]; then
        printf '%s' "${GIT_COMMIT}"
        exit 0
    fi

    # Rebuilt. If the original carried a signature, this is where it is lost --
    # `commit-tree` has no way to write a `gpgsig` header. Record it so rewrite()
    # can name it; a signature that disappears without being named is the whole
    # failure this mode exists to end.
    if [ "${CF_SIGNED}" = 1 ] && [ -n "${SIG_DROPPED:-}" ]; then
        printf '%s\t%s\n' "${GIT_COMMIT}" \
            "$(git log -1 --format='%an  %s' "${GIT_COMMIT}")" >> "${SIG_DROPPED}"
    fi
    printf '%s' "${CF_MSG}" | git commit-tree "${CF_ARGS[@]}"
    exit 0
fi

# ===========================================================================
# MAIN
# ===========================================================================

# --- argument parsing -------------------------------------------------------
# A bare positional used to mean "the range", back when there was one repo in
# play. It now cannot mean one thing, so it is REFUSED rather than silently
# reinterpreted -- a changed meaning that still runs is the worst outcome here.
CODE_RANGE=""; HARNESS_RANGE=""; DRY_RUN=0; NO_COMMIT=0; SUPERVISOR="${SUPERVISOR:-}"
AUTHOR_FIX_MAP=$(mktemp); export AUTHOR_FIX_MAP
trap 'rm -f "${AUTHOR_FIX_MAP}"' EXIT
for arg in "$@"; do
    case "${arg}" in
        --code=*)       CODE_RANGE="${arg#*=}" ;;
        --harness=*)    HARNESS_RANGE="${arg#*=}" ;;
        --supervisor=*) SUPERVISOR="${arg#*=}" ;;
        --dry-run)      DRY_RUN=1 ;;
        --no-commit)    NO_COMMIT=1 ;;
        # --author-fix=<sha>=<Claude Model Version> : the hand-repair channel for
        # a wildcard author the trailers cannot resolve. Repeatable.
        --author-fix=*) AF="${arg#*=}"
                        AFS="${AF%%=*}"; AFI="${AF#*=}"
                        if [ -z "${AFS}" ] || [ "${AFS}" = "${AFI}" ] || [ -z "$(identity "${AFI}")" ]; then
                            echo "error: --author-fix wants <sha>=<Claude Model Version>, got '${AF}'." >&2
                            exit 1
                        fi
                        printf '%s %s\n' "${AFS}" "${AFI}" >> "${AUTHOR_FIX_MAP}" ;;
        # --branch-alias=<old>=<new> : the repair channel for a `Code:` trailer
        # whose branch was renamed away. Repeatable. The SHAPE is checked here;
        # that <new> actually resolves is checked once the code repo is known,
        # a few lines below -- it cannot be checked before that, because the
        # repo pair is discovered after this loop and the branch lives in the
        # CODE repo, not in whatever repo the caller happens to be standing in.
        --branch-alias=*) BA="${arg#*=}"
                        BAO="${BA%%=*}"; BAN="${BA#*=}"
                        if [ -z "${BAO}" ] || [ "${BAO}" = "${BA}" ] || [ -z "${BAN}" ]; then
                            echo "error: --branch-alias wants <old-branch>=<new-branch>, got '${BA}'." >&2
                            exit 1
                        fi
                        BRANCH_ALIAS["${BAO}"]="${BAN}" ;;
        # Printed from the file itself so the help cannot drift from the header.
        # The end of the header is FOUND, not counted: a line number here is a
        # magic constant that truncates the help silently the next time the
        # header grows, which is the failure mode this file argues against
        # everywhere else.
        -h|--help)      awk 'NR>=2 { if ($0 == "set -euo pipefail") exit; print }' "$0" \
                          | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "error: unrecognised argument '${arg}'." >&2
           echo "       A bare range is no longer accepted: this script now acts on BOTH" >&2
           echo "       repos, so a range must say which one. Use --code=<range> and/or" >&2
           echo "       --harness=<range>. Default for each is '@{upstream}..HEAD'." >&2
           exit 1 ;;
    esac
done

SELF=$(readlink -f "$0")          # absolute: filter-branch runs from a temp cwd

# --- repo pair discovery ----------------------------------------------------
# The harness is "always cloned as claude/ at the root of spacecrafter", so the
# pair is derivable from wherever this was invoked. Both directions are handled
# because both are natural places to be standing.
HERE=$(git rev-parse --show-toplevel)
if [ -d "${HERE}/claude/.git" ] || [ -f "${HERE}/claude/.git" ]; then
    CODE_REPO="${HERE}"; HARNESS_REPO="${HERE}/claude"
elif [ "$(basename "${HERE}")" = "claude" ] && git -C "$(dirname "${HERE}")" rev-parse --show-toplevel >/dev/null 2>&1; then
    CODE_REPO=$(git -C "$(dirname "${HERE}")" rev-parse --show-toplevel); HARNESS_REPO="${HERE}"
else
    CODE_REPO="${HERE}"; HARNESS_REPO=""
fi

# --- validate every --branch-alias target, now that the code repo is known ---
# Before anything is read, previewed or rewritten. An alias pointing at a branch
# that does not exist would put the substitution this option replaced straight
# back in, one indirection further away from the reader.
for BAO in "${!BRANCH_ALIAS[@]}"; do
    BAN="${BRANCH_ALIAS[${BAO}]}"
    if ! git -C "${CODE_REPO}" rev-parse --verify -q "refs/heads/${BAN}" >/dev/null 2>&1; then
        echo "error: --branch-alias=${BAO}=${BAN} names no local branch in ${CODE_REPO}." >&2
        echo "       Local branches there: $(git -C "${CODE_REPO}" for-each-ref --format='%(refname:short)' refs/heads | tr '\n' ' ')" >&2
        exit 1
    fi
done

# default_range <repo> -> `@{upstream}..HEAD`, or an error naming the fix.
default_range() {
    local r=$1 up
    up=$(git -C "${r}" rev-parse --abbrev-ref --symbolic-full-name '@{upstream}' 2>/dev/null || true)
    if [ -z "${up}" ]; then
        echo "error: ${r} has no upstream, so the pushed/unpushed line is undefined." >&2
        echo "       Set one, or pass an explicit range." >&2
        exit 1
    fi
    printf '%s..HEAD' "${up}"
}
[ -n "${CODE_RANGE}" ]    || CODE_RANGE=$(default_range "${CODE_REPO}")
if [ -n "${HARNESS_REPO}" ]; then
    [ -n "${HARNESS_RANGE}" ] || HARNESS_RANGE=$(default_range "${HARNESS_REPO}")
fi

# ---------------------------------------------------------------------------
# THE CLEAN-TREE PRECONDITION -- and why it is what makes `commit -a` sound
# ---------------------------------------------------------------------------
# Three things converge on the same requirement, which is why it is checked once
# and hard:
#
#  1. filter-branch REFUSES to run against a dirty tree. Discovering that at
#     step C, after the prompts, is discovering it late.
#  2. The closing commit uses `-a`, which stages tracked modifications and
#     SILENTLY IGNORES untracked files. Run against a tree that was already
#     dirty, it would sweep up unrelated edits under this script's message and
#     still leave new files behind -- committing the wrong set, twice over.
#  3. This script is run at a CONSOLIDATION point. "Is everything committed?" is
#     the question being asked at that moment anyway, so the check is not
#     overhead here; it is the first thing worth knowing.
#
# Requiring a fully clean tree up front turns the closing commit from a hope into
# a proof: if nothing was dirty before, then everything dirty afterwards was
# written BY THIS SCRIPT, so `-a` is exactly the intended set -- and that is
# asserted again before committing rather than assumed from this paragraph.
#
# --dry-run skips it: previewing is read-only, so a dirty tree is no reason to
# refuse to LOOK. You can always see what a run would do.
# ---------------------------------------------------------------------------
if [ "${DRY_RUN}" = 0 ]; then
    DIRTY_LIST=""
    for r in "${CODE_REPO}" ${HARNESS_REPO:+"${HARNESS_REPO}"}; do
        # -uall so untracked files inside untracked DIRECTORIES are named one by
        # one; the default collapses them to `dir/`, which hides how much is there.
        o=$(git -C "${r}" status --porcelain --untracked-files=all)
        [ -n "${o}" ] && DIRTY_LIST+="  ${r}"$'\n'"$(printf '%s' "${o}" | sed 's/^/      /')"$'\n'
    done
    if [ -n "${DIRTY_LIST}" ]; then
        echo "STOP: uncommitted work is present. Nothing has been touched." >&2
        echo >&2
        printf '%s' "${DIRTY_LIST}" >&2
        echo >&2
        echo "  Commit or stash it first. This runs at a consolidation point, so this" >&2
        echo "  listing IS the consolidation check: '??' entries are files no commit" >&2
        echo "  would have captured -- the closing 'commit -a' cannot see them either." >&2
        echo "  (--dry-run still previews; it changes nothing.)" >&2
        exit 1
    fi
fi

# --- refuse to run on a repo that is mid-operation ---------------------------
for r in "${CODE_REPO}" ${HARNESS_REPO:+"${HARNESS_REPO}"}; do
    d=$(git -C "${r}" rev-parse --git-dir)
    case "${d}" in /*) ;; *) d="${r}/${d}" ;; esac
    if [ -d "${d}/rebase-merge" ] || [ -d "${d}/rebase-apply" ]; then
        echo "error: a rebase is in progress in ${r}. Finish it, or 'git rebase --abort'." >&2
        exit 1
    fi
done

# ---------------------------------------------------------------------------
# scan_range <repo> <range>
# One `git log` pass, records NUL-separated, fields separated by \x02 (a byte
# that cannot occur in a name or a message). It applies the same identity() the
# filter applies, so this preview cannot disagree with the rewrite about WHICH
# commits are selected -- only about nothing at all. It is a function because
# the post-rewrite check calls it AGAIN on the rewritten history: "selection is
# now empty" is the fixed point this script converges to, and asserting it with
# the same code that produced the preview is the only way the assertion cannot
# be fooled by a bug the preview and the check happen to share.
# ---------------------------------------------------------------------------
scan_range() {
    local repo=$1 range=$2
    SEL_SHAS=(); SEL_LINE=(); MANUAL_SHAS=(); MANUAL_LINE=()
    N_DROP=0; N_KEEP=0; N_BOTH=0; N_BARE=0; N_DONE=0; N_OTHER=0; N_AUTHORFIX=0
    local REC SHA REST AN BODY AID LINE CID SELF_CO OTHER_CO LBL WILD
    while IFS= read -r -d '' REC || [ -n "${REC}" ]; do
        SHA=${REC%%$'\x02'*};  REST=${REC#*$'\x02'}
        AN=${REST%%$'\x02'*};  BODY=${REST#*$'\x02'}

        # A bare `Claude` author is the wildcard: resolve the real identity from
        # the co-author trailer. Unresolvable -> not selected, listed for hand
        # repair, and left completely alone by the rewrite.
        WILD=0
        if [ "${AN}" = "${WILDCARD_AUTHOR}" ]; then
            AN=$(resolve_author "${SHA}" "${BODY}")
            if [ -z "${AN}" ]; then
                MANUAL_SHAS+=("${SHA}")
                MANUAL_LINE+=("  [ MANUAL   ] $(git -C "${repo}" log -1 --format='%h  %s' "${SHA}")")
                continue
            fi
            WILD=1
        fi

        AID=$(identity "${AN}")
        if [ -z "${AID}" ]; then N_OTHER=$((N_OTHER + 1)); continue; fi
        if has_supervisor "${BODY}" && [ "${WILD}" = 0 ]; then N_DONE=$((N_DONE + 1)); continue; fi

        SELF_CO=0; OTHER_CO=0
        while IFS= read -r LINE; do
            [[ ${LINE,,} == co-authored-by:* ]] || continue
            CID=$(identity "${LINE#*:}")
            if [ "${CID}" = "${AID}" ]; then SELF_CO=1; else OTHER_CO=1; fi
        done <<< "${BODY}"

        if   [ "${SELF_CO}" = 1 ] && [ "${OTHER_CO}" = 1 ]; then LBL="drop+keep"; N_BOTH=$((N_BOTH + 1))
        elif [ "${SELF_CO}" = 1 ];                          then LBL="drop     "; N_DROP=$((N_DROP + 1))
        elif [ "${OTHER_CO}" = 1 ];                         then LBL="keep     "; N_KEEP=$((N_KEEP + 1))
        else                                                     LBL="bare     "; N_BARE=$((N_BARE + 1))
        fi
        # `*` marks "the author is rewritten too", as a prefix rather than a
        # longer label: squeezing it into the label truncated the label instead.
        # Built as plain variables, never as a conditional inside the array
        # append: a `$(... && echo ...)` that takes the false branch exits
        # non-zero, and under `set -e` that kills the scan mid-loop, silently.
        local MARK=' ' SUF=''
        if [ "${WILD}" = 1 ]; then
            MARK='*'; SUF="  ->  ${AN}"; N_AUTHORFIX=$((N_AUTHORFIX + 1))
        fi
        SEL_SHAS+=("${SHA}")
        SEL_LINE+=("  [${MARK}${LBL}] $(git -C "${repo}" log -1 --format='%h  %<(15,trunc)%an %s' "${SHA}")${SUF}")
    done < <(git -C "${repo}" log -z --format="%H%x02%an%x02%B" "${range}")
}

preview() {                        # preview <label> <repo> <range>
    scan_range "$2" "$3"
    echo "=== $1: $2"
    echo "    range ${3}  --  $(git -C "$2" rev-list --count "$3") commit(s) in range"
    # The identity NEW commits here would carry, and which file states it. This
    # script only ever reads the author of commits already made -- but the value
    # below is what the NEXT one gets, and a wrong one lands the inverted chain
    # silently. It is printed here because this is the one screen where the
    # reader's attention is already on authorship; knowing it elsewhere is
    # knowing it outside the window where it is actionable.
    # --show-origin, never the plain get: the plain get returns the EFFECTIVE
    # value, so it reads as success whether the value came from the file you
    # meant to write or from one shadowing it. It cannot discriminate the
    # failure, so it cannot detect it.
    local WHO WHENCE
    WHO=$(git -C "$2" config user.name || true)
    WHENCE=$(git -C "$2" config --show-origin user.name 2>/dev/null | cut -f1 || true)
    if [ "${WHO}" = "${WILDCARD_AUTHOR}" ]; then
        echo "    new commits authored: '${WHO}' -- wildcard, resolved from the co-author trailer  [${WHENCE}]"
    elif [ -n "$(identity "${WHO}")" ]; then
        echo "    new commits authored: '${WHO}' -- explicit identity, used as-is  [${WHENCE}]"
    else
        echo "    new commits authored: '${WHO}' -- NOT a Claude identity; such commits are not selected  [${WHENCE}]"
    fi
    if [ "${#SEL_SHAS[@]}" -eq 0 ]; then
        echo "    nothing selected (${N_DONE} already carry Supervised-By, ${N_OTHER} not Claude-authored)"
    else
        printf '%s\n' "${SEL_LINE[@]}"
        echo "    drop=${N_DROP}  drop+keep=${N_BOTH}  keep=${N_KEEP}  bare=${N_BARE}"
        echo "    '*' = author ALSO rewritten, resolved from the wildcard (${N_AUTHORFIX})"
        echo "    untouched: ${N_DONE} already supervised, ${N_OTHER} non-Claude"
    fi
    # The wildcard cases nothing can resolve. Listed with the command that fixes
    # them, because "needs manual repair" without the repair instruction is just
    # a nag: it names the problem and leaves the reader to rediscover the tool.
    if [ "${#MANUAL_SHAS[@]}" -gt 0 ]; then
        echo
        echo "    ${#MANUAL_SHAS[@]} commit(s) have the bare '${WILDCARD_AUTHOR}' author and NO single"
        echo "    Co-Authored-By identity to resolve it from. They are left entirely"
        echo "    untouched -- no author, no trailer -- so they stay visible. Name the"
        echo "    model yourself and re-run:"
        printf '%s\n' "${MANUAL_LINE[@]}"
        echo
        for s in "${MANUAL_SHAS[@]}"; do
            echo "        --author-fix=${s:0:8}='Claude <Model> <version>' \\"
        done
    fi
    echo
}

# --- A. preview both --------------------------------------------------------
preview "CODE   " "${CODE_REPO}" "${CODE_RANGE}"
N_CODE=${#SEL_SHAS[@]}; M_CODE=${#MANUAL_SHAS[@]}; A_CODE=${N_AUTHORFIX}
N_HARNESS=0
if [ -n "${HARNESS_REPO}" ]; then
    preview "HARNESS" "${HARNESS_REPO}" "${HARNESS_RANGE}"
    N_HARNESS=${#SEL_SHAS[@]}; M_HARNESS=${#MANUAL_SHAS[@]}; A_HARNESS=${N_AUTHORFIX}
fi

# --- A'. refuse to guess: a trailer branch that resolves to nothing -----------
# Placed here, after the previews and before EVERYTHING that consumes a
# reachability verdict -- the cross-repo count, the repair map, the "Nothing to
# do" exit, the confirmation prompt -- because each of those would otherwise be
# computed against a substituted target and would look exactly like a good one.
# It fires under --dry-run too, and exits 1 there: a preview that would lie must
# not exit 0, or the lie is what gets believed.
if [ -n "${HARNESS_REPO}" ]; then
    UNRES=$(unresolvable_trailers "${HARNESS_RANGE}")
    if [ -n "${UNRES}" ]; then
        {
        echo "STOP: a 'Code:' trailer names a branch this repository cannot resolve."
        echo
        echo "  WHAT"
        printf '%s\n' "${UNRES}" | while read -r br n; do
            echo "    ${n} distinct 'Code: ${br} @ <sha>' trailer(s) in the harness range"
            echo "    '${HARNESS_RANGE}' name branch '${br}', and neither refs/heads/${br}"
            echo "    nor any --branch-alias resolves it in ${CODE_REPO}."
        done
        echo
        echo "  VALID STATE"
        echo "    Every branch named in a trailer resolves to refs/heads/<name> in the code"
        echo "    repo, or is given a replacement with --branch-alias. Names that resolve"
        echo "    there right now:"
        echo "      $(git -C "${CODE_REPO}" for-each-ref --format='%(refname:short)' refs/heads | tr '\n' ' ')"
        echo
        echo "  CONSEQUENCE"
        echo "    Their reachability cannot be judged, so nothing downstream of it can be"
        echo "    trusted: the dangling count in the preview, the repair map, and the"
        echo "    post-rewrite assertion that every trailer still resolves. Until this"
        echo "    message existed the script answered by judging them against HEAD, which"
        echo "    is not verdict-preserving -- the same trailer reads reachable against"
        echo "    refs/heads/master-beta and dangling against refs/heads/2023-master -- so"
        echo "    the verdict depended on which branch was checked out, and a run from the"
        echo "    wrong checkout marked every trailer of the renamed branch UNREPAIRABLE."
        echo
        echo "  FIX -- either one, before re-running"
        # One fix per line, its explanation on the NEXT line: the paths in these
        # commands are as long as the repository path, so anything aligned to
        # the right of them stops being aligned on somebody else's checkout.
        printf '%s\n' "${UNRES}" | while read -r br n; do
            echo "    (a) judge them against the renamed branch -- it must already exist:"
            echo "        ./supervised-by.sh --branch-alias=${br}=<the-new-name> ..."
            echo "    (b) or bring the old name back, if it is meant to keep existing:"
            echo "        git -C ${CODE_REPO} branch ${br} <the-new-name>"
        done
        echo
        echo "  Nothing has been touched."
        } >&2
        exit 1
    fi
fi

# The cross-repo consequence, stated BEFORE the confirmation rather than
# discovered afterwards: this is the number that the old one-repo-at-a-time
# script left dangling without ever mentioning it.
N_TRAILER=0; BASELINE_DANGLING=0; BASELINE_DANGLING_LIST=""
if [ -n "${HARNESS_REPO}" ]; then
    N_TRAILER=$(list_code_trailers "${HARNESS_RANGE}" | wc -l)
    # CAPTURED, not just counted, and captured HERE -- before anything is
    # rewritten. "Already dangling when this run started" is a property of this
    # moment, and re-deriving it after the code rewrite answers a different
    # question: by then every trailer the rewrite just invalidated is dangling
    # too. (Measured 2026-09-07: re-derived, build_repair_map mapped 73 trailers
    # instead of 2, those 71 duplicated what build_map had already mapped, and
    # map_lookup called every one of them AMBIG -- so the run STOPped and rolled
    # back on damage it had itself created, in every case where a trailer was
    # dangling beforehand. On this pair that is every run.)
    BASELINE_DANGLING_LIST=$(dangling_trailers "${HARNESS_RANGE}")
    BASELINE_DANGLING=$(printf '%s' "${BASELINE_DANGLING_LIST}" | grep -c '^' || true)
    echo "Cross-repo: ${N_TRAILER} distinct 'Code: ... @ <sha>' trailer(s) in the harness range;"
    echo "            those pointing into the rewritten code range are remapped in the SAME"
    echo "            harness pass. Harness *.md citing code shas is repointed in E."
    if [ "${BASELINE_DANGLING}" -gt 0 ]; then
        echo
        echo "            ${BASELINE_DANGLING} of them ALREADY point at commits no branch reaches --"
        echo "            damage from an earlier rewrite that had no cross-repo step. This run"
        echo "            repairs each one whose twin is uniquely identifiable by fingerprint:"
        printf '%s\n' "${BASELINE_DANGLING_LIST}" | while read -r br sha; do
            echo "              ${sha}  ($(git -C "${CODE_REPO}" log -1 --format='%s' "${sha}" 2>/dev/null | cut -c1-52))"
        done
    fi
    echo
fi

if [ "${N_CODE}" -eq 0 ] && [ "${N_HARNESS}" -eq 0 ]; then
    echo "Nothing to do in either repo."
    exit 0
fi
if [ "${DRY_RUN}" = 1 ]; then echo "(--dry-run: stopping here.)"; exit 0; fi

# --- B. acquire the string --------------------------------------------------
if [ -z "${SUPERVISOR}" ]; then
    # `read` pulls one line from stdin into a variable. -r keeps a backslash a
    # backslash; -p prints a prompt first (a bash extension, not POSIX sh).
    read -r -p "Supervised-By string (e.g. 'Vixy <vixy@example.com>'): " SUPERVISOR
fi
[ -n "${SUPERVISOR}" ] || { echo "error: empty string; aborting." >&2; exit 1; }
# The key is added by interpret-trailers; pasting it in too yields the silent
# nonsense `Supervised-By: Supervised-By: Vixy <...>`, so refuse it outright.
if [[ ${SUPERVISOR} == Supervised-By:* ]]; then
    echo "error: give the VALUE only -- the 'Supervised-By: ' key is added for you." >&2
    exit 1
fi
export SUPERVISOR

read -r -p "Rewrite ${N_CODE} code + ${N_HARNESS} harness commit(s)? [y/N] " CONFIRM
case "${CONFIRM}" in [yY]*) ;; *) echo "aborted."; exit 0 ;; esac

WORK=$(mktemp -d); trap 'rm -rf "${WORK}" "${AUTHOR_FIX_MAP}"' EXIT   # one trap: a
                    # second `trap ... EXIT` REPLACES the first, it does not add.
# Where --commit-filter records a signature it could not carry over. Exported
# for the same reason CODE_MAP is: the filter is a separate process, and this is
# the only channel back from it. One line per commit, `<sha> TAB <who>  <subject>`.
SIG_DROPPED="${WORK}/signatures-dropped"; export SIG_DROPPED
CODE_TIP=$(git -C "${CODE_REPO}" rev-parse HEAD)
HARNESS_TIP=""
[ -n "${HARNESS_REPO}" ] && HARNESS_TIP=$(git -C "${HARNESS_REPO}" rev-parse HEAD)

# rollback_all: every exit path from here on must land in the state we started
# from. A half-rewritten pair -- code re-shaed, harness still citing the old
# shas -- is the one state neither repo can be repaired from automatically.
rollback_all() {
    git -C "${CODE_REPO}" reset --hard "${CODE_TIP}" >/dev/null
    if [ -n "${HARNESS_TIP}" ]; then
        git -C "${HARNESS_REPO}" reset --hard "${HARNESS_TIP}" >/dev/null
        echo "Rolled BOTH repos back (code ${CODE_TIP:0:8}, harness ${HARNESS_TIP:0:8})." >&2
    else
        echo "Rolled the code repo back (${CODE_TIP:0:8}); no harness repo in play." >&2
    fi
}

# rewrite <repo> <range> <original-tip> <selected-count> -- rewrite, then prove
# it. Step 1 (content identity) proves nothing was BROKEN. It does not prove
# anything HAPPENED: a filter that silently does nothing (bad quoting, an
# unexported variable, a $0 that does not resolve from filter-branch's temp cwd)
# passes it perfectly, because doing nothing preserves content exactly. Step 2
# re-runs the selection: every selected commit has by construction gained the
# trailer that deselects it, so the count must be 0.
rewrite() {
    local repo=$1 range=$2 tip=$3 expected=$4 expected_manual=${5:-0}
    # Explicitly tested, NOT left to `set -e`: set -e exits the script on the
    # spot, which SKIPS rollback_all and leaves whatever was already rewritten
    # in place. Harmless when the code repo fails first (filter-branch does not
    # move the ref on abort), fatal when the harness fails AFTER the code repo
    # succeeded -- the inconsistent pair this script exists to prevent.
    # Where the signature losses of THIS repo's pass start in the file: the two
    # passes share one file, and each must report only its own.
    local sig_before=0 sig_after=0
    if [ -s "${SIG_DROPPED:-/dev/null}" ]; then sig_before=$(wc -l < "${SIG_DROPPED}"); fi
    if ! FILTER_BRANCH_SQUELCH_WARNING=1 \
         git -C "${repo}" filter-branch -f \
            --env-filter "eval \"\$($(printf '%q' "${SELF}") --emit-env)\"" \
            --msg-filter "$(printf '%q --msg-filter' "${SELF}")" \
            --commit-filter "$(printf '%q --commit-filter' "${SELF}") \"\$@\"" \
            -- "${range}"; then
        echo "STOP: filter-branch failed in ${repo}." >&2; rollback_all; exit 1
    fi
    if ! git -C "${repo}" diff --quiet "${tip}" HEAD; then
        echo "STOP: content of ${repo} differs from ${tip}." >&2; rollback_all; exit 1
    fi
    scan_range "${repo}" "${range}"
    if [ "${#SEL_SHAS[@]}" -ne 0 ]; then
        echo "STOP: ${#SEL_SHAS[@]}/${expected} commit(s) still selected in ${repo}" >&2
        echo "      after the rewrite -- the filter did not apply." >&2
        rollback_all; exit 1
    fi
    # The author rewrite needs its own assertion: it happens in the env-filter,
    # a completely separate channel from the message, so a message rewrite that
    # lands perfectly proves nothing at all about it. Every bare wildcard left
    # must be one we KNEW we could not resolve.
    local left
    left=$(git -C "${repo}" log --format='%an' "${range}" | grep -cxF "${WILDCARD_AUTHOR}" || true)
    if [ "${left}" -ne "${expected_manual}" ]; then
        echo "STOP: ${left} commit(s) in ${repo} still have the bare '${WILDCARD_AUTHOR}' author," >&2
        echo "      but only ${expected_manual} were unresolvable -- the env-filter did not apply." >&2
        rollback_all; exit 1
    fi
    echo "VERIFIED ${repo}: content identical, selection now empty (${expected} rewritten," \
         "${expected_manual} left as unresolvable wildcards)."

    # A signature that could not be carried over is REPORTED, here and again in
    # the closing summary. It is the one loss this rewrite can still cause, it is
    # somebody else's signature, and it is irreversible once published.
    if [ -s "${SIG_DROPPED:-/dev/null}" ]; then sig_after=$(wc -l < "${SIG_DROPPED}"); fi
    if [ "${sig_after}" -gt "${sig_before}" ]; then
        echo
        echo "  WARNING: $((sig_after - sig_before)) commit(s) in ${repo} carried a GPG signature"
        echo "  that could NOT be carried over, and were rebuilt WITHOUT it:"
        tail -n +$((sig_before + 1)) "${SIG_DROPPED}" | while IFS=$'\t' read -r s d; do
            echo "      ${s:0:8}  ${d}"
        done
        echo "  WHAT: each of these is a SIGNED commit that had to be rebuilt because one of"
        echo "    its MAPPED PARENTS changed -- it sits downstream of a rewritten commit"
        echo "    inside the range. Every other commit this run did not select keeps its"
        echo "    object untouched, sha and signature included; these cannot."
        echo "  CONSEQUENCE: a signature signs the WHOLE object -- tree, parents, author,"
        echo "    committer, message -- so a parent's new sha invalidates it by construction,"
        echo "    and 'git commit-tree' cannot write a 'gpgsig' header in any case. The"
        echo "    rebuilt commit is content-identical and UNSIGNED; its old sha is in the map."
        echo "    Check:  git -C ${repo} log --format='%G? %h %an %s' ${range} | grep -v '^N '"
        echo "  THIS IS NOT DECIDED HERE. It is the signer's commit, not this script's, and"
        echo "    it is cheaper to decide before the force-push than after. Undo is printed"
        echo "    at the end of this run."
        echo
    fi
}

# build_map <repo> <original-tip> <outfile> -- old full sha -> new full sha.
# The two SIDES of the rewrite: the original tip still resolves (its objects are
# dangling but present), so its history is the OLD side and HEAD is the NEW one.
# `--not` on each gives the symmetric difference: exactly the commits whose sha
# changed. A commit that rebuilt byte-identical appears in NEITHER list, so it
# is correctly absent from the map. Pairing is by a fingerprint the rewrite
# preserves -- tree + author-date + author + SUBJECT -- since only trailer LINES
# changed and tree/author never do. That is what makes it 1:1 even across merges.
UNPAIRED_TOTAL=0
build_map() {
    local repo=$1 tip=$2 out=$3 ns os nf u
    local -a UNPAIRED=()
    : > "${out}"
    local -A BY_FP=()
    while read -r ns; do
        [ -n "${ns}" ] || continue
        BY_FP["$(git -C "${repo}" log -1 --format='%T|%at|%ae|%s' "${ns}")"]="${ns}"
    done < <(git -C "${repo}" rev-list HEAD --not "${tip}" 2>/dev/null || true)
    while read -r os; do
        [ -n "${os}" ] || continue
        nf="${BY_FP[$(git -C "${repo}" log -1 --format='%T|%at|%ae|%s' "${os}")]:-}"
        # An `if`, not `[ ] && printf`: an AND-list whose test fails returns
        # non-zero, and when that is the last statement of the last iteration
        # the whole function returns non-zero -- which under `set -e` killed the
        # run on the spot, after the code repo had been rewritten and before
        # rollback_all could be reached. Measured 2026-09-07 on a clone of this
        # pair: the run died silently between "VERIFIED <code>" and "Code sha
        # map:", leaving exactly the half-rewritten pair this script exists to
        # prevent. An empty result is DATA here, not an error.
        if [ -n "${nf}" ]; then
            printf '%s\t%s\n' "${os}" "${nf}" >> "${out}"
        else
            UNPAIRED+=("${os}")
        fi
    done < <(git -C "${repo}" rev-list "${tip}" --not HEAD 2>/dev/null || true)

    # An old commit with no new counterpart is a hole in the map, so it is said
    # here rather than left to be inferred from a line count.
    if [ "${#UNPAIRED[@]}" -gt 0 ]; then
        UNPAIRED_TOTAL=$((UNPAIRED_TOTAL + ${#UNPAIRED[@]}))
        echo
        echo "  WARNING: ${#UNPAIRED[@]} rewritten commit(s) in ${repo} have NO counterpart in"
        echo "  the new history, so they are ABSENT from the map:"
        for u in "${UNPAIRED[@]}"; do
            echo "      ${u:0:8}  $(git -C "${repo}" log -1 --format='%an  %s' "${u}" | cut -c1-64)"
        done
        echo "  WHAT IT MEANS: the branch is now that many commits SHORTER."
        echo "    old history $(git -C "${repo}" rev-list --count "${tip}") commit(s)," \
             "new history $(git -C "${repo}" rev-list --count HEAD)."
        echo "  CONSEQUENCE: a citation of one of those shas resolves to nothing, and step E"
        echo "    cannot repoint it either -- the map has no answer for it. The content"
        echo "    assertion above still passes: the TIP TREE is unchanged, only the shape of"
        echo "    the history is not."
        echo "  KNOWN CAUSE, measured on this repository pair 2026-09-07: 'git commit-tree'"
        echo "    DROPS the 'gpgsig' header, so a GPG-signed commit rebuilds as its unsigned"
        echo "    form. Where an identical unsigned commit already exists elsewhere in the"
        echo "    history, the rebuilt commit IS that commit: a duplicated chain collapses"
        echo "    onto its twin, one commit per twin, and stops where the twins run out."
        echo "    Since the --commit-filter was added an unselected commit is no longer"
        echo "    rebuilt at all, so reaching this warning means something else changed the"
        echo "    commit -- read the list above before believing the count."
        echo "    Check:  git -C ${repo} cat-file commit <old-sha> | grep -c gpgsig"
        echo "    Find where one went:  git -C ${repo} log HEAD --format='%H %T' \\"
        echo "        | awk -v t=\$(git -C ${repo} log -1 --format=%T <old-sha>) '\$2==t'"
        echo "  THIS IS NOT DECIDED HERE. Whether to accept losing those signatures and that"
        echo "    history, or to stop and resolve the duplication first, is the operator's"
        echo "    call -- and it is much cheaper to make BEFORE the force-push than after."
        echo "    Undo is printed at the end of this run."
        echo
    fi
    return 0
}

# build_repair_map <outfile> -- heal trailers ALREADY dangling when this run
# started, i.e. damage left by an EARLIER rewrite that had no cross-repo step.
# It reads ${BASELINE_DANGLING_LIST}, captured at step A BEFORE anything was
# rewritten, and not a fresh dangling_trailers call: by the time this runs the
# code repo has been re-shaed, so a fresh call also returns every trailer THIS
# run just invalidated -- which build_map has already mapped, and mapping them
# twice makes map_lookup report each one AMBIG and aborts the run.
# Same fingerprint (tree + author-date + author + subject) as
# build_map, for the same reason: a message-only rewrite preserves all four, so
# the old sha's twin on the branch is identifiable without trusting anything the
# rewrite itself wrote. A unique match is repaired; zero or several is REPORTED,
# never guessed -- a wrong repair is indistinguishable from a right one
# afterwards, which is precisely the property that forbids guessing here.
# Sets REPAIRED_N / UNREPAIRABLE_N.
build_repair_map() {
    local out=$1 br sha fp tree c hits
    : > "${out}"; REPAIRED_N=0; UNREPAIRABLE_N=0
    while read -r br sha; do
        [ -n "${sha}" ] || continue
        if ! git -C "${CODE_REPO}" rev-parse --verify -q "${sha}^{commit}" >/dev/null 2>&1; then
            echo "    UNREPAIRABLE ${sha} (object gone -- gc'd; nothing left to match)" >&2
            UNREPAIRABLE_N=$((UNREPAIRABLE_N + 1)); continue
        fi
        fp=$(git -C "${CODE_REPO}" log -1 --format='%T|%at|%ae|%s' "${sha}")
        tree=${fp%%|*}
        # An ASSERTION, not a fallback: step A' refuses an unresolvable trailer
        # branch before any prompt, so reaching this is a broken guard, not a
        # case to cope with. Coping here is what produced the defect -- the twin
        # search would run over whatever HEAD points at and its single hit would
        # be indistinguishable from a correct repair afterwards.
        local target
        if ! target=$(resolve_branch_target "${br}"); then
            echo "STOP: trailer branch '${br}' resolves to no ref and no --branch-alias," >&2
            echo "      so ${sha} cannot be searched for a twin. Step A' should have" >&2
            echo "      refused this run before it started; that guard is broken." >&2
            rollback_all; exit 1
        fi
        hits=()
        while read -r c; do
            [ -n "${c}" ] || continue
            [ "$(git -C "${CODE_REPO}" log -1 --format='%T|%at|%ae|%s' "${c}")" = "${fp}" ] && hits+=("${c}")
        done < <(git -C "${CODE_REPO}" log "${target}" --format='%H %T' | awk -v t="${tree}" '$2==t {print $1}')
        if [ "${#hits[@]}" -eq 1 ]; then
            printf '%s\t%s\n' "$(git -C "${CODE_REPO}" rev-parse "${sha}")" "${hits[0]}" >> "${out}"
            echo "    repair ${sha} -> ${hits[0]:0:8}  ($(git -C "${CODE_REPO}" log -1 --format='%s' "${hits[0]}" | cut -c1-50))"
            REPAIRED_N=$((REPAIRED_N + 1))
        else
            echo "    UNREPAIRABLE ${sha} (${#hits[@]} fingerprint matches on ${target}; repoint by hand)" >&2
            UNREPAIRABLE_N=$((UNREPAIRABLE_N + 1))
        fi
    done < <(printf '%s\n' "${BASELINE_DANGLING_LIST}")
}

# --- C. code repo first (the direction of the dependency) -------------------
if [ "${N_CODE}" -gt 0 ]; then
    rewrite "${CODE_REPO}" "${CODE_RANGE}" "${CODE_TIP}" "${N_CODE}" "${M_CODE}"
fi
build_map "${CODE_REPO}" "${CODE_TIP}" "${WORK}/code.map"
export CODE_MAP="${WORK}/code.map"
echo "Code sha map: $(wc -l < "${CODE_MAP}") commit(s) changed sha."

# --- D. harness: Supervised-By AND Code: remap, in ONE pass -----------------
if [ -n "${HARNESS_REPO}" ]; then
    # Damage from EARLIER runs is healed by the same pass, at no extra cost:
    # the map the filter consults simply gains the old->twin pairs. Built now,
    # after the code rewrite, so the twin recorded is the CURRENT sha of that
    # commit and not one this run is about to invalidate again.
    if [ "${BASELINE_DANGLING}" -gt 0 ]; then
        echo "Pre-existing dangling Code: trailers (from an earlier rewrite):"
        build_repair_map "${WORK}/repair.map"
        [ -s "${WORK}/repair.map" ] && cat "${WORK}/repair.map" >> "${CODE_MAP}"
    else
        REPAIRED_N=0; UNREPAIRABLE_N=0
    fi

    if [ "${N_HARNESS}" -gt 0 ] || [ -s "${CODE_MAP}" ]; then
        rewrite "${HARNESS_REPO}" "${HARNESS_RANGE}" "${HARNESS_TIP}" "${N_HARNESS}" "${M_HARNESS}"
    fi

    # The targeted assertion for the failure this whole restructure exists for.
    # A remap that silently did nothing passes both generic checks above, so it
    # needs its own. The bar is REACHABILITY, not existence -- see
    # reachable_in_code: the objects survive a rewrite, so an existence test
    # reports success in exactly the case it is here to catch. Tolerance is the
    # count that was already unrepairable before this run: a good run must not
    # be rolled back for damage it did not cause, and must not hide it either.
    if [ -s "${CODE_MAP}.ambig" ]; then
        echo "STOP: a Code: trailer sha prefixes more than one rewritten commit." >&2
        cat "${CODE_MAP}.ambig" >&2; rollback_all; exit 1
    fi
    DANGLING=$(dangling_trailers "${HARNESS_RANGE}" | wc -l)
    if [ "${DANGLING}" -gt "${UNREPAIRABLE_N}" ]; then
        echo "STOP: ${DANGLING} Code: trailer(s) unreachable in the code repo," >&2
        echo "      only ${UNREPAIRABLE_N} were unrepairable beforehand -- the remap did not apply." >&2
        dangling_trailers "${HARNESS_RANGE}" | sed 's/^/      /' >&2
        rollback_all; exit 1
    fi
    echo "VERIFIED harness: every 'Code: ... @ <sha>' trailer is reachable in the code repo" \
         "(${REPAIRED_N} pre-existing repaired, ${UNREPAIRABLE_N} left for hand repair)."
    build_map "${HARNESS_REPO}" "${HARNESS_TIP}" "${WORK}/harness.map"
fi

# --- D'. persist the maps beside the ledger ---------------------------------
# The maps built above are this run's only record of old sha -> new sha, and
# until now they died with ${WORK}. Step E repoints tracked *.md; everything
# else that cites these commits -- the git history of those same files, the
# shared notes, the archive, the owner's own trees -- cannot be reached by any
# rewrite and can only be resolved by LOOKING THE TOKEN UP. So the maps are
# copied out verbatim (no reformatting: the file the filter consulted is the
# file that is kept) and committed by step E.
MAP_FILES=(); MAP_DIR=""
if [ -n "${HARNESS_REPO}" ]; then
    # The two tips are the PRE-rewrite ones -- the state the map maps FROM,
    # which is the state a reader holding a stale sha is standing in.
    MAP_DIR="sha-maps/$(date -u +%Y%m%dT%H%M%SZ)-${CODE_TIP:0:8}-${HARNESS_TIP:0:8}"
    mkdir -p "${HARNESS_REPO}/${MAP_DIR}"
    for m in code harness repair; do
        if [ -s "${WORK}/${m}.map" ]; then
            cp "${WORK}/${m}.map" "${HARNESS_REPO}/${MAP_DIR}/${m}.tsv"
            MAP_FILES+=("${MAP_DIR}/${m}.tsv")
        fi
    done
    if [ "${#MAP_FILES[@]}" -eq 0 ]; then
        rmdir "${HARNESS_REPO}/${MAP_DIR}" 2>/dev/null || true
        MAP_DIR=""
    else
        # The contract, written ONCE. It is not regenerated per run: a file that
        # is rewritten every time is a file whose content is nobody's decision.
        if [ ! -e "${HARNESS_REPO}/sha-maps/README.md" ]; then
            cat > "${HARNESS_REPO}/sha-maps/README.md" <<'SHAMAPREADME'
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
SHAMAPREADME
            MAP_FILES+=("sha-maps/README.md")
        fi
        # Staged now: `commit -a` in step E stages tracked modifications and
        # ignores untracked files, so a map left unstaged would be written and
        # then silently left behind.
        git -C "${HARNESS_REPO}" add -- "${MAP_FILES[@]}"
        echo "Sha maps written to ${HARNESS_REPO}/${MAP_DIR}/ :" \
             "$(for f in "${MAP_FILES[@]}"; do printf '%s(%s) ' "${f##*/}" "$(wc -l < "${HARNESS_REPO}/${f}")"; done)"
    fi
fi


# --- E. repoint harness .md (both maps) -------------------------------------
# Only tracked .md is scanned: those are the tracing docs. Code is never edited
# here, and nothing is written without an explicit y/N.
echo
echo "--- .md tracker commit references -------------------------------------"
MERGED="${WORK}/merged.map"; : > "${MERGED}"
[ -s "${WORK}/code.map" ]    && cat "${WORK}/code.map"    >> "${MERGED}"
[ -s "${WORK}/harness.map" ] && cat "${WORK}/harness.map" >> "${MERGED}"

MD_REPO="${HARNESS_REPO:-${CODE_REPO}}"
declare -A TOK_NEW=(); declare -A TOK_AMBIG=()
# Declared here, not inside the `y` branch: the closing commit below is reached
# whether or not anything was repointed (the sha maps alone can be the reason),
# so its expected-set test must be able to read an EMPTY touched set.
declare -A TOUCHED_FILES=()
MD_FILES=(); mapfile -t MD_FILES < <(git -C "${MD_REPO}" ls-files -- '*.md' 2>/dev/null || true)
if [ -s "${MERGED}" ] && [ "${#MD_FILES[@]}" -gt 0 ]; then
    while IFS= read -r tok; do
        [ -n "${tok}" ] || continue
        r=$(map_lookup "${MERGED}" "${tok}")
        if   [ "${r}" = "AMBIG" ]; then TOK_AMBIG["${tok}"]=1
        elif [ -n "${r}" ];        then TOK_NEW["${tok}"]="${r}"
        fi
    done < <(cd "${MD_REPO}" && grep -hoE '\b[0-9a-f]{7,40}\b' "${MD_FILES[@]}" 2>/dev/null | sort -u || true)
fi

if [ "${#TOK_NEW[@]}" -eq 0 ]; then
    echo "No .md references to rewritten commits -- trackers are already consistent."
else
    echo "These ${MD_REPO} .md references point at rewritten (now-dangling) commits:"
    echo
    for tok in "${!TOK_NEW[@]}"; do
        while IFS= read -r loc; do
            echo "    ${loc}  (${tok} -> ${TOK_NEW[${tok}]})"
        done < <(cd "${MD_REPO}" && grep -nwF "${tok}" "${MD_FILES[@]}" 2>/dev/null | cut -d: -f1,2 || true)
    done
    echo
    if [ "${#TOK_AMBIG[@]}" -gt 0 ]; then
        echo "  (${#TOK_AMBIG[@]} token(s) prefix more than one rewritten commit -- skipped,"
        echo "   repoint those by hand.)"
        echo
    fi
    APPLY=n
    if [ -t 0 ]; then
        read -r -p "Repoint these in the working tree (uncommitted, review before commit)? [y/N] " APPLY
    else
        echo "  (non-interactive shell: proposing only, nothing applied.)"
    fi
    case "${APPLY}" in
        [yY]*)
            for tok in "${!TOK_NEW[@]}"; do
                new="${TOK_NEW[${tok}]}"
                while IFS= read -r f; do
                    # \b so a token is never matched inside a longer hex run
                    sed -i -E "s/\\b${tok}\\b/${new}/g" "${MD_REPO}/${f}"
                    TOUCHED_FILES["${f}"]=1
                done < <(cd "${MD_REPO}" && grep -lwF "${tok}" "${MD_FILES[@]}" 2>/dev/null || true)
            done
            echo
            echo "Repointed ${#TOK_NEW[@]} citation(s) across ${#TOUCHED_FILES[@]} file(s)."
            ;;
        *)  echo "Not applied. Re-run and answer y, or repoint by hand from the list above." ;;
    esac
fi

# --- the closing commit -----------------------------------------------------
# Reached whenever this run WROTE something into the harness working tree: the
# repointed *.md, the persisted sha maps, or both. The maps alone are reason
# enough -- a run that rewrote history and left its only old->new record
# uncommitted in a working tree has produced the dangling references it exists
# to prevent, and has hidden that behind a clean-looking summary.
#
# `-a` is used as asked, but it is only CORRECT because the tree was verified
# clean at step 0: everything dirty now was written by this script. That is
# asserted here rather than inherited from the earlier check -- the gap between
# the two is the whole run, and an assumption that held at the start is not
# evidence about the end. The map files are part of the expected set, and they
# are already staged, so `-a` captures them with the rest.
if [ "${#TOUCHED_FILES[@]}" -gt 0 ] || [ "${#MAP_FILES[@]}" -gt 0 ]; then
    EXPECTED=$(printf '%s\n' "${!TOUCHED_FILES[@]}" "${MAP_FILES[@]}" | sort -u)
    ACTUAL=$(git -C "${MD_REPO}" status --porcelain --untracked-files=all | cut -c4- | sort)
    # The subject says what the commit actually is. A run that repointed nothing
    # and only recorded its maps must not claim a repoint it did not do.
    SUBJECT="Repoint tracker citations after the supervision-trailer rewrite"
    [ "${#TOUCHED_FILES[@]}" -gt 0 ] || \
        SUBJECT="Record the old->new sha maps of the supervision-trailer rewrite"
    MAPLINE="no sha maps were written (nothing changed sha)."
    if [ "${#MAP_FILES[@]}" -gt 0 ]; then
        MAPLINE="the sha maps of this run: ${MAP_FILES[*]}"
    fi
    if [ "${NO_COMMIT}" = 1 ]; then
        echo "(--no-commit: left uncommitted.)  git -C ${MD_REPO} diff -- '*.md'"
        if [ "${#MAP_FILES[@]}" -gt 0 ]; then
            echo "  The sha maps are written and STAGED but NOT committed. They are this"
            echo "  run's only record of old sha -> new sha; commit them, or the rewrite"
            echo "  has no resolver: ${MAP_FILES[*]}"
        fi
    elif [ "${EXPECTED}" != "${ACTUAL}" ]; then
        echo "NOT COMMITTED: the dirty set is not the set this script wrote." >&2
        echo "  expected:" >&2; printf '%s\n' "${EXPECTED}" | sed 's/^/      /' >&2
        echo "  actual:"   >&2; printf '%s\n' "${ACTUAL}"   | sed 's/^/      /' >&2
        echo "  'commit -a' would capture the wrong set. Review and commit by hand." >&2
        if [ "${#MAP_FILES[@]}" -gt 0 ]; then
            echo "  The sha maps are part of the expected set and must be committed with the" >&2
            echo "  rest: ${MAP_FILES[*]}" >&2
        fi
    elif ! [[ ${SUPERVISOR} =~ ^.+\ \<[^\>]+\>$ ]]; then
        # The commit is authored by the SUPERVISOR: this consolidation is
        # their act, not a model's. It also sidesteps a trap -- committing
        # under the bare `Claude` wildcard with no co-author to resolve it
        # would produce a commit THIS SCRIPT flags as unrepairable on its
        # next run. Refuse rather than create that.
        echo "NOT COMMITTED: the Supervised-By string is not a git identity" >&2
        echo "  ('Name <email>'), so it cannot author the commit, and the configured" >&2
        echo "  identity may be the bare wildcard -- which this script would flag as" >&2
        echo "  unrepairable next run. Commit by hand." >&2
        if [ "${#MAP_FILES[@]}" -gt 0 ]; then
            echo "  The sha maps are written and STAGED and are part of that commit:" >&2
            echo "  ${MAP_FILES[*]}" >&2
        fi
    else
        CODE_TRAILER=""
        if [ "${MD_REPO}" = "${HARNESS_REPO:-}" ]; then
            CODE_TRAILER="Code: $(git -C "${CODE_REPO}" branch --show-current) @ $(git -C "${CODE_REPO}" rev-parse --short=8 HEAD)"
        fi
        git -C "${MD_REPO}" commit -q -a --author="${SUPERVISOR}" -F - <<COMMITMSG
${SUBJECT}

${N_CODE} code and ${N_HARNESS} harness commit(s) were rewritten to record the
supervision chain: Supervised-By recorded, redundant self co-authors dropped,
$((A_CODE + A_HARNESS)) author field(s) resolved from the bare-Claude wildcard.
Rewriting a message changes the commit's sha, so every tracker line citing one
of them by sha was left pointing at an object no branch reaches.

This commit carries the repoint -- ${#TOK_NEW[@]} citation(s) across
${#TOUCHED_FILES[@]} file(s), old sha -> new sha, no prose changed -- and
${MAPLINE}
The maps are what resolves a stale sha in every surface a rewrite cannot reach:
the git history of these same files, notes outside this pair, the archive. Look
a token up as a PREFIX of the old side, newest directory first; the record is
never rewritten. sha-maps/README.md carries the rule.

The 'Code: <branch> @ <sha>' trailers in harness commit messages are NOT part of
this commit -- they are remapped inside the rewrite pass itself, since they live
in the messages being rewritten. ${REPAIRED_N:-0} of them were dangling before this run
(damage from an earlier rewrite that had no cross-repo step) and were repaired
in the same pass; ${UNREPAIRABLE_N:-0} could not be resolved automatically.

Generated by claude/supervised-by.sh; the working tree was verified clean before
the run and verified to contain only these files before this commit, which is
what makes 'commit -a' equal to the intended set rather than merely close to it.

${CODE_TRAILER}
COMMITMSG
        echo "Committed in ${MD_REPO}:"
        git -C "${MD_REPO}" log -1 --format='    %h  %s  (author %an)'
    fi
fi
echo "----------------------------------------------------------------------"

echo
if [ "${UNPAIRED_TOTAL}" -gt 0 ]; then
    # Repeated at the end because the warning above scrolls past in a run this
    # long, and the decision it asks for has to be made before the push.
    echo "READ BEFORE PUBLISHING: ${UNPAIRED_TOTAL} rewritten commit(s) had no counterpart in"
    echo "         the new history -- the branch is shorter and the map cannot resolve them."
    echo "         Search this output for 'WARNING:' for the list and the cause."
    echo
fi
if [ -s "${SIG_DROPPED:-/dev/null}" ]; then
    # Same reason as above: the block at the rewrite scrolls past, and this is
    # the last screen before the operator decides whether to push.
    echo "READ BEFORE PUBLISHING: $(wc -l < "${SIG_DROPPED}") commit(s) carried a GPG signature that"
    echo "         could NOT be carried over -- each had a parent rewritten by this run, and a"
    echo "         signature signs the parents too. They are content-identical and now UNSIGNED:"
    while IFS=$'\t' read -r s d; do
        echo "             ${s:0:8}  ${d}"
    done < "${SIG_DROPPED}"
    echo "         Every other unselected commit kept its object, its sha and its signature."
    echo
fi
echo "Undo:    git -C ${CODE_REPO} reset --hard ${CODE_TIP}"
[ -n "${HARNESS_TIP}" ] && \
echo "         git -C ${HARNESS_REPO} reset --hard ${HARNESS_TIP}"
echo "Publish: force-push BOTH (shas changed), code first:"
echo "         git -C ${CODE_REPO} push --force-with-lease"
[ -n "${HARNESS_TIP}" ] && \
echo "         git -C ${HARNESS_REPO} push --force-with-lease"
