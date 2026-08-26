#ifndef JSON_NUM_HPP_
#define JSON_NUM_HPP_

#include <cmath>
#include <ostream>

// A JSON-LEGAL NUMBER FOR THE NEW PATH'S DUMP CHANNEL (INTENT §5.103).
//
// The reason, verbatim from the old path's own copy of this rule
// (navigator.cpp:500-508, whose comment states it for the plan coefficients):
// a value is legitimately non-finite in states the dump exists to record, and
// `nan` / `-nan` / `inf` are not JSON, so ONE such value makes a consumer fail
// on the WHOLE line - which is exactly the line you wanted to read. Measured
// (F38, §11.150(m)): a scene-C dump carrying NaN in 73 of 153 records could not
// be opened by `analyze.py` at all, and every OTHER scene lost two body lines.
// The value is PRESERVED as a quoted token rather than nulled: "this radius is
// NaN" is the finding, and a `null` erases which of NaN / +inf / -inf it was.
//
// EXACT FOR FINITE VALUES BY CONSTRUCTION - the property that lets this be
// applied to a landed evidence format: the finite branch is `out << n.v` with
// `v` the caller's own type, i.e. the same overload, the same stream state, the
// same characters as before. There is no reformatting to verify.
//
// The old path keeps its three local copies (navigator/observer/projector) -
// unchanged by construction, §11.52(b) - so this is the new path's single
// authority, not a fourth copy of theirs.
template<typename T>
struct JNum { T v; };

template<typename T>
inline std::ostream &operator<<(std::ostream &out, JNum<T> n)
{
    if (std::isfinite(n.v))
        return out << n.v;
    if (n.v != n.v)
        return out << "\"nan\"";
    return out << ((n.v > 0) ? "\"inf\"" : "\"-inf\"");
}

//! Deduction helper: `out << jn(x)` reads as `out << x` plus the guarantee.
template<typename T>
inline JNum<T> jn(T v) { return JNum<T>{v}; }

#endif /* end of include guard: JSON_NUM_HPP_ */
