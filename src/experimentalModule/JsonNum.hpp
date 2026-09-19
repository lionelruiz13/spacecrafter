#ifndef JSON_NUM_HPP_
#define JSON_NUM_HPP_

#include <cmath>
#include <ostream>

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
