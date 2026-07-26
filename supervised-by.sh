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
#   * a commit whose message is untouched rebuilds byte-identical and keeps its
#     SHA; only rewritten commits and their descendants get new SHAs
#
# The filter is THIS SCRIPT re-invoked as `supervised-by.sh --msg-filter`, so
# the selection shown in the preview and the selection actually applied are the
# same code. Two copies of a rule are two rules waiting to drift apart.
#
# (Modern alternative: `git filter-repo --message-callback`, which is faster but
#  is a separate install. filter-branch is used here because it ships with git.)
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
# Usage:
#   ./supervised-by.sh                        # both repos, @{upstream}..HEAD each
#   ./supervised-by.sh --code=93a14377..HEAD  # override one side explicitly
#   ./supervised-by.sh --harness=HEAD~20..HEAD
#   ./supervised-by.sh --dry-run              # preview only, never prompts
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

# reachable_in_code <sha> [branch] -> 0 if that commit is on the branch's
# history. NOT `rev-parse --verify`, and this distinction is the whole point:
# filter-branch leaves the pre-rewrite objects in the database (that is exactly
# how build_map below reads the OLD side of a rewrite), so `rev-parse`,
# `cat-file -e` and friends SUCCEED on a sha that no branch can reach any more.
# A `Code:` trailer pointing there is broken in the only sense that matters --
# `git show` still works for whoever runs it today, and resolves to nothing at
# all once the objects are gc'd. Reachability is the property; existence is a
# proxy that fails silently in the exact case being tested for.
reachable_in_code() {
    local sha=$1 br=${2:-} target=HEAD
    git -C "${CODE_REPO}" rev-parse --verify -q "${sha}^{commit}" >/dev/null 2>&1 || return 1
    if [ -n "${br}" ] && git -C "${CODE_REPO}" rev-parse --verify -q "refs/heads/${br}" >/dev/null 2>&1; then
        target="refs/heads/${br}"
    fi
    git -C "${CODE_REPO}" merge-base --is-ancestor "${sha}" "${target}" 2>/dev/null
}

# list_code_trailers <harness-range> -> `<branch> <sha>` per distinct trailer.
list_code_trailers() {
    git -C "${HARNESS_REPO}" log --format='%B' "$1" \
      | sed -nE 's/^Code:[[:space:]]+([^[:space:]]+)[[:space:]]+@[[:space:]]+([0-9a-f]{7,40})[[:space:]]*$/\1 \2/p' \
      | sort -u
}

# dangling_trailers <harness-range> -> the subset whose sha no branch reaches.
dangling_trailers() {
    local br sha
    while read -r br sha; do
        [ -n "${sha}" ] || continue
        reachable_in_code "${sha}" "${br}" || printf '%s %s\n' "${br}" "${sha}"
    done < <(list_code_trailers "$1")
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

# ===========================================================================
# MAIN
# ===========================================================================

# --- argument parsing -------------------------------------------------------
# A bare positional used to mean "the range", back when there was one repo in
# play. It now cannot mean one thing, so it is REFUSED rather than silently
# reinterpreted -- a changed meaning that still runs is the worst outcome here.
CODE_RANGE=""; HARNESS_RANGE=""; DRY_RUN=0; SUPERVISOR="${SUPERVISOR:-}"
AUTHOR_FIX_MAP=$(mktemp); export AUTHOR_FIX_MAP
trap 'rm -f "${AUTHOR_FIX_MAP}"' EXIT
for arg in "$@"; do
    case "${arg}" in
        --code=*)       CODE_RANGE="${arg#*=}" ;;
        --harness=*)    HARNESS_RANGE="${arg#*=}" ;;
        --supervisor=*) SUPERVISOR="${arg#*=}" ;;
        --dry-run)      DRY_RUN=1 ;;
        # --author-fix=<sha>=<Claude Model Version> : the hand-repair channel for
        # a wildcard author the trailers cannot resolve. Repeatable.
        --author-fix=*) AF="${arg#*=}"
                        AFS="${AF%%=*}"; AFI="${AF#*=}"
                        if [ -z "${AFS}" ] || [ "${AFS}" = "${AFI}" ] || [ -z "$(identity "${AFI}")" ]; then
                            echo "error: --author-fix wants <sha>=<Claude Model Version>, got '${AF}'." >&2
                            exit 1
                        fi
                        printf '%s %s\n' "${AFS}" "${AFI}" >> "${AUTHOR_FIX_MAP}" ;;
        -h|--help)      sed -n '2,140p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
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
N_CODE=${#SEL_SHAS[@]}; M_CODE=${#MANUAL_SHAS[@]}
N_HARNESS=0
if [ -n "${HARNESS_REPO}" ]; then
    preview "HARNESS" "${HARNESS_REPO}" "${HARNESS_RANGE}"
    N_HARNESS=${#SEL_SHAS[@]}; M_HARNESS=${#MANUAL_SHAS[@]}
fi

# The cross-repo consequence, stated BEFORE the confirmation rather than
# discovered afterwards: this is the number that the old one-repo-at-a-time
# script left dangling without ever mentioning it.
N_TRAILER=0; BASELINE_DANGLING=0
if [ -n "${HARNESS_REPO}" ]; then
    N_TRAILER=$(list_code_trailers "${HARNESS_RANGE}" | wc -l)
    BASELINE_DANGLING=$(dangling_trailers "${HARNESS_RANGE}" | wc -l)
    echo "Cross-repo: ${N_TRAILER} distinct 'Code: ... @ <sha>' trailer(s) in the harness range;"
    echo "            those pointing into the rewritten code range are remapped in the SAME"
    echo "            harness pass. Harness *.md citing code shas is repointed in E."
    if [ "${BASELINE_DANGLING}" -gt 0 ]; then
        echo
        echo "            ${BASELINE_DANGLING} of them ALREADY point at commits no branch reaches --"
        echo "            damage from an earlier rewrite that had no cross-repo step. This run"
        echo "            repairs each one whose twin is uniquely identifiable by fingerprint:"
        dangling_trailers "${HARNESS_RANGE}" | while read -r br sha; do
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
    if ! FILTER_BRANCH_SQUELCH_WARNING=1 \
         git -C "${repo}" filter-branch -f \
            --env-filter "eval \"\$($(printf '%q' "${SELF}") --emit-env)\"" \
            --msg-filter "$(printf '%q --msg-filter' "${SELF}")" -- "${range}"; then
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
}

# build_map <repo> <original-tip> <outfile> -- old full sha -> new full sha.
# The two SIDES of the rewrite: the original tip still resolves (its objects are
# dangling but present), so its history is the OLD side and HEAD is the NEW one.
# `--not` on each gives the symmetric difference: exactly the commits whose sha
# changed. A commit that rebuilt byte-identical appears in NEITHER list, so it
# is correctly absent from the map. Pairing is by a fingerprint the rewrite
# preserves -- tree + author-date + author + SUBJECT -- since only trailer LINES
# changed and tree/author never do. That is what makes it 1:1 even across merges.
build_map() {
    local repo=$1 tip=$2 out=$3 ns os nf
    : > "${out}"
    local -A BY_FP=()
    while read -r ns; do
        [ -n "${ns}" ] || continue
        BY_FP["$(git -C "${repo}" log -1 --format='%T|%at|%ae|%s' "${ns}")"]="${ns}"
    done < <(git -C "${repo}" rev-list HEAD --not "${tip}" 2>/dev/null || true)
    while read -r os; do
        [ -n "${os}" ] || continue
        nf="${BY_FP[$(git -C "${repo}" log -1 --format='%T|%at|%ae|%s' "${os}")]:-}"
        [ -n "${nf}" ] && printf '%s\t%s\n' "${os}" "${nf}" >> "${out}"
    done < <(git -C "${repo}" rev-list "${tip}" --not HEAD 2>/dev/null || true)
}

# build_repair_map <harness-range> <outfile> -- heal trailers ALREADY dangling
# when this run started, i.e. damage left by an EARLIER rewrite that had no
# cross-repo step. Same fingerprint (tree + author-date + author + subject) as
# build_map, for the same reason: a message-only rewrite preserves all four, so
# the old sha's twin on the branch is identifiable without trusting anything the
# rewrite itself wrote. A unique match is repaired; zero or several is REPORTED,
# never guessed -- a wrong repair is indistinguishable from a right one
# afterwards, which is precisely the property that forbids guessing here.
# Sets REPAIRED_N / UNREPAIRABLE_N.
build_repair_map() {
    local range=$1 out=$2 br sha fp tree c hits
    : > "${out}"; REPAIRED_N=0; UNREPAIRABLE_N=0
    while read -r br sha; do
        [ -n "${sha}" ] || continue
        if ! git -C "${CODE_REPO}" rev-parse --verify -q "${sha}^{commit}" >/dev/null 2>&1; then
            echo "    UNREPAIRABLE ${sha} (object gone -- gc'd; nothing left to match)" >&2
            UNREPAIRABLE_N=$((UNREPAIRABLE_N + 1)); continue
        fi
        fp=$(git -C "${CODE_REPO}" log -1 --format='%T|%at|%ae|%s' "${sha}")
        tree=${fp%%|*}
        local target=HEAD
        git -C "${CODE_REPO}" rev-parse --verify -q "refs/heads/${br}" >/dev/null 2>&1 && target="refs/heads/${br}"
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
    done < <(dangling_trailers "${range}")
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
        build_repair_map "${HARNESS_RANGE}" "${WORK}/repair.map"
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
                done < <(cd "${MD_REPO}" && grep -lwF "${tok}" "${MD_FILES[@]}" 2>/dev/null || true)
            done
            echo
            echo "Repointed (UNCOMMITTED -- review, then commit):"
            echo "    git -C ${MD_REPO} diff -- '*.md'"
            ;;
        *)  echo "Not applied. Re-run and answer y, or repoint by hand from the list above." ;;
    esac
fi
echo "----------------------------------------------------------------------"

echo
echo "Undo:    git -C ${CODE_REPO} reset --hard ${CODE_TIP}"
[ -n "${HARNESS_TIP}" ] && \
echo "         git -C ${HARNESS_REPO} reset --hard ${HARNESS_TIP}"
echo "Publish: force-push BOTH (shas changed), code first:"
echo "         git -C ${CODE_REPO} push --force-with-lease"
[ -n "${HARNESS_TIP}" ] && \
echo "         git -C ${HARNESS_REPO} push --force-with-lease"
