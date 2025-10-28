#ifndef FIXED_POINT_HPP
#define FIXED_POINT_HPP

#include <cmath>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <limits>
#include <numeric>
#include <charconv>

#define THROW_ON_FIXED_POINT_ERROR 1 // Enable to throw exceptions on error (division by zero, invalid format, etc...)
#define THROW_ON_FIXED_POINT_OVERFLOW 1 // Enable to throw exceptions on overflow (slower, but safer)

#if THROW_ON_FIXED_POINT_OVERFLOW
    // Enable overflow checking, slower but safer
    #define CHECKED_ADD(a, b, from) checked_add(a, b, from)
    #define CHECKED_SUB(a, b, from) checked_sub(a, b, from)
    #define CHECKED_MUL(a, b, from) checked_mul(a, b, from)
    #define CHECKED_DIV(a, b, from) checked_div(a, b, from)
    #define CHECKED_CAST(t, a, from) checked_cast<t>(a, from)
#else
    // Disable overflow checking, faster but unsafe
    #define CHECKED_ADD(a, b, from) (a + b)
    #define CHECKED_SUB(a, b, from) (a - b)
    #define CHECKED_MUL(a, b, from) (a * b)
    #define CHECKED_DIV(a, b, from) (a / b)
    #define CHECKED_CAST(t, a, from) (StorageType)(a)
#endif

/**
 * @brief Template class for fixed-point arithmetic
 * @tparam DecimalPlaces Number of decimal places after the decimal point (e.g., 2 = 2 digits, 4 = 4 digits)
 * @tparam StorageType Integral type used for internal storage (default: int64_t)
 * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
 * @note DecimalPlaces must be between 0 and 15 to limit overflow risks (though this is not enforced for StorageType flexibility)
 * @note This class may be upgraded in the future to support more features and higher precision/number (support of infinite number, instead of int64_t)
 */
template<int DecimalPlaces, typename StorageType = int64_t>
class FixedPoint {
    static_assert(std::is_integral<StorageType>::value, "StorageType must be an integral type");
    static_assert(!std::is_same_v<StorageType, bool>, "StorageType cannot be bool");
    static_assert(DecimalPlaces >= 0, "DecimalPlaces must be non-negative");

    // Cannot be checked since StorageType may not support a big enough number for 15 decimal places
    // static_assert(DecimalPlaces <= 15, "DecimalPlaces must be less than or equal to 15 to limit overflow");
private:
    StorageType value;
    // Unsigned version of StorageType for internal calculations
    using UnsignedStorageType = typename std::make_unsigned<StorageType>::type;
    // Are we using a signed StorageType? (for internal checks)
    static constexpr bool isSigned = std::is_signed<StorageType>::value;

    // ============================================================================
    // Helper function
    // ============================================================================
    /*
     * @brief Compute 10^places at compile-time
    */
    static StorageType pow10(int places) {
        StorageType scale = 1;
        for (int i = 0; i < places; ++i) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                if (scale > std::numeric_limits<StorageType>::max() / 10) {
                    throw std::overflow_error("FixedPoint: pow10: overflow");
                }
            #endif
            scale *= 10;
        }
        return scale;
    }

    /*
     * @brief Compute 10^places at compile-time without exception
    */
    static constexpr StorageType pow10_noexcept(int places) {
        StorageType scale = 1;
        for (int i = 0; i < places; ++i) {
            scale *= 10;
        }
        return scale;
    }

    /*
     * @brief Rescale raw internal value from one DecimalPlaces to another
     * @tparam From Original DecimalPlaces
     * @tparam To Target DecimalPlaces
     * @param v Raw internal value to rescale
     * @return Rescaled raw internal value
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int From, int To>
    static inline StorageType rescaleRaw(StorageType v) {
        if constexpr (From == To) return v;
        else if (v == 0) return 0;
        else if constexpr (From < To) {
            StorageType scale = pow10(To - From);
            #if THROW_ON_FIXED_POINT_OVERFLOW
                if constexpr (isSigned) {
                    if (v > std::numeric_limits<StorageType>::max() / scale) throw std::overflow_error("FixedPoint: rescaleRaw: overflow");
                    if (v < std::numeric_limits<StorageType>::min() / scale) throw std::overflow_error("FixedPoint: rescaleRaw: overflow");
                } else {
                    if (v > std::numeric_limits<StorageType>::max() / scale) throw std::overflow_error("FixedPoint: rescaleRaw: overflow");
                }
            #endif
            return v * scale;
        } else {
            StorageType scale = pow10(From - To);
            return v / scale;
        }
    }

    #if THROW_ON_FIXED_POINT_OVERFLOW
        /*
        * @brief Check if an addition is safe (no overflow), and perform it
        * @param a First operand
        * @param b Second operand
        * @return Result of the addition
        * @throws std::overflow_error if overflow occurs
        */
        static StorageType checked_add(StorageType a, StorageType b, const std::string& from = std::string("")) {
            using L = std::numeric_limits<StorageType>;
            if constexpr (isSigned) {
                if ((b > 0 && a > L::max() - b))  throw std::overflow_error("FixedPoint: " + from + ": add overflow");
                if ((b < 0 && a < L::min() - b))  throw std::overflow_error("FixedPoint: " + from + ": add overflow");
            } else {
                if (a > L::max() - b)             throw std::overflow_error("FixedPoint: " + from + ": add overflow");
            }
            return StorageType(a + b);
        }

        /*
        * @brief Check if an substraction is safe (no overflow), and perform it
        * @param a First operand
        * @param b Second operand
        * @return Result of the substraction
        * @throws std::overflow_error if overflow occurs
        */
        static StorageType checked_sub(StorageType a, StorageType b, const std::string& from = std::string("")) {
            using L = std::numeric_limits<StorageType>;
            if constexpr (isSigned) {
                if ((b < 0 && a > L::max() + b))  throw std::overflow_error("FixedPoint: " + from + ": sub overflow");
                if ((b > 0 && a < L::min() + b))  throw std::overflow_error("FixedPoint: " + from + ": sub overflow");
            } else {
                if (a < b)                        throw std::overflow_error("FixedPoint: " + from + ": sub overflow");
            }
            return StorageType(a - b);
        }

        /*
        * @brief Check if an multiplication is safe (no overflow), and perform it
        * @param a First operand
        * @param b Second operand
        * @return Result of the multiplication
        * @throws std::overflow_error if overflow occurs
        */
        static StorageType checked_mul(StorageType a, StorageType b, const std::string& from = std::string("")) {
            using L = std::numeric_limits<StorageType>;
            if (a == 0 || b == 0) return 0;
            if constexpr (isSigned) {
                if (a == -1 && b == L::min()) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                if (b == -1 && a == L::min()) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                if (a > 0) {
                    if (b > 0 && a > L::max() / b) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                    if (b < 0 && b < L::min() / a) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                } else { // a < 0
                    if (b > 0 && a < L::min() / b) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                    if (b < 0 && a < L::max() / b) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
                }
            } else {
                if (a > L::max() / b) throw std::overflow_error("FixedPoint: " + from + ": mul overflow");
            }
            return StorageType(a * b);
        }

        /*
         * @brief Check if a division is safe (no overflow/division by zero), and perform it
         * @param a Numerator
         * @param b Denominator
         * @return Result of the division
         * @throws std::overflow_error if overflow occurs
         * @throws std::runtime_error if division by zero occurs
        */
        static StorageType checked_div(StorageType a, StorageType b, const std::string& from = std::string("")) {
            using L = std::numeric_limits<StorageType>;
            if (b == 0) throw std::runtime_error("FixedPoint: " + from + ": division by zero");
            if constexpr (isSigned) {
                if (a == L::min() && b == -1) throw std::overflow_error("FixedPoint: " + from + ": div overflow");
            }
            return StorageType(a / b);
        }

        /*
        * @brief Check if a cast is safe (no overflow), and perform it
        * @tparam From Original type
        * @param v Value to cast
        * @return Casted value
        * @throws std::overflow_error if overflow occurs
        */
        template<class From>
        static StorageType checked_cast(From v, const std::string& from = std::string("")) {
            using ToLim = std::numeric_limits<StorageType>;
            long double ld = static_cast<long double>(v);
            if (ld < static_cast<long double>(ToLim::min()) ||
                ld > static_cast<long double>(ToLim::max()))
                throw std::overflow_error("FixedPoint: " + from + ": cast overflow");
            return static_cast<StorageType>(v);
        }
    #endif

    /*
    * @brief Compute floor((n * m) / d) for unsigned integers without wider types.
    *        - Aggressive gcd reductions
    *        - Consume denominator factors 2 and 5 when possible
    *        - Split by d to avoid forming huge n*m when n>=d or m>=d
    *        - Detect overflow (throw/saturate) before each multiply/add
    *
    * Preconditions: d != 0, n/m/d are magnitudes (unsigned domain).
    */
    template<typename U>
    static U mulDivUnsigned(U n, U m, U d, const std::string& from = std::string("")) {
        if (n == 0 || m == 0) return U(0);

        #if THROW_ON_FIXED_POINT_ERROR
            if (d == 0) throw std::runtime_error("FixedPoint: " + from + ": mulDivUnsigned: division by zero");
        #endif

        using Wide = unsigned long long; // standard C++: at least 64-bit
        const U UMAX = std::numeric_limits<U>::max();

        auto reduce_once = [](U& a, U& b) -> bool {
            U g = std::gcd(a, b);
            if (g > 1) { a /= g; b /= g; return true; }
            return false;
        };
        auto reduce_all_pairs = [&](U& a, U& b) {
            while (reduce_once(a, b)) {}
        };
        auto consume = [](U& num, U& den, U p) {
            while (den % p == 0) {
                if (num % p == 0) { num /= p; den /= p; }
                else break;
            }
        };
        auto add_checked_U = [&](U x, U y) -> U {
            Wide sum = (Wide)x + (Wide)y;
            #if THROW_ON_FIXED_POINT_OVERFLOW
                if (sum > (Wide)UMAX)
                    throw std::overflow_error("FixedPoint: " + from + ": mulDivUnsigned: addition overflow");
                return (U)sum;
            #else
                return (sum > (Wide)UMAX) ? UMAX : (U)sum;
            #endif
        };

        // 1) Aggressive reductions
        reduce_all_pairs(n, d);
        reduce_all_pairs(m, d);

        // 2) Consume 2 and 5
        consume(n, d, U(2)); consume(m, d, U(2));
        consume(n, d, U(5)); consume(m, d, U(5));

        // 3) Repeat gcd until stabilization
        for (;;) {
            bool changed = false;
            U g = std::gcd(n, d);
            if (g > 1) { n /= g; d /= g; changed = true; }
            g = std::gcd(m, d);
            if (g > 1) { m /= g; d /= g; changed = true; }
            if (!changed) break;
        }

        // 4) Split by d if n>=d or m>=d (avoids large products)
        if (n >= d) {
            U q = n / d, r = n % d;
            Wide term1W = (Wide)q * (Wide)m;
            #if THROW_ON_FIXED_POINT_OVERFLOW
                if (term1W > (Wide)UMAX)
                    throw std::overflow_error("FixedPoint: " + from + ": mulDivUnsigned: (n/d)*m overflow");
                U term1 = (U)term1W;
            #else
                U term1 = (term1W > (Wide)UMAX) ? UMAX : (U)term1W;
            #endif
            if (r == 0) return term1;
            U term2 = mulDivUnsigned<U>(r, m, d, from);
            return add_checked_U(term1, term2);
        }
        if (m >= d) {
            U q = m / d, r = m % d;
            Wide term1W = (Wide)n * (Wide)q;
            #if THROW_ON_FIXED_POINT_OVERFLOW
                if (term1W > (Wide)UMAX)
                    throw std::overflow_error("FixedPoint: " + from + ": mulDivUnsigned: n*(m/d) overflow");
                U term1 = (U)term1W;
            #else
                U term1 = (term1W > (Wide)UMAX) ? UMAX : (U)term1W;
            #endif
            if (r == 0) return term1;
            U term2 = mulDivUnsigned<U>(n, r, d, from);
            return add_checked_U(term1, term2);
        }

        // 5) Case "small" (n<d && m<d) : product in Wide, division, then check towards U
        Wide prod = (Wide)n * (Wide)m;
        Wide q = prod / (Wide)d;
        #if THROW_ON_FIXED_POINT_OVERFLOW
            if (q > (Wide)UMAX)
                throw std::overflow_error("FixedPoint: " + from + ": mulDivUnsigned: quotient overflow");
            return (U)q;
        #else
            return (q > (Wide)UMAX) ? UMAX : (U)q;
        #endif
    }

    static constexpr StorageType SCALE = pow10_noexcept(DecimalPlaces);
    static_assert(DecimalPlaces == 0 || (std::numeric_limits<StorageType>::max() / pow10_noexcept(DecimalPlaces - 1)) >= 10, "SCALE overflows StorageType");
public:
    // ============================================================================
    // Constructors
    // ============================================================================
    /*
     * @brief Constructors from no value (default to 0)
    */
    constexpr FixedPoint() noexcept : value(0) {}

    /*
     * @brief Constructors from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from int are allowed for ease of use
    */
    FixedPoint(int val) {
        StorageType v = CHECKED_CAST(int, val, "int Ctor");
        value = CHECKED_MUL(v, SCALE, "int Ctor");
    }

    /*
     * @brief Constructors from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from float are allowed for ease of use
    */
    FixedPoint(float val) {
        value = CHECKED_CAST(float, val * SCALE, "float Ctor");
    }

    /*
     * @brief Constructors from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from double are allowed for ease of use
    */
    FixedPoint(double val) {
        value = CHECKED_CAST(double, val * SCALE, "double Ctor");
    }

    /*
     * @brief Copy constructor
    */
    constexpr FixedPoint(const FixedPoint&) noexcept = default;

    /*
     * @brief Constructor from another FixedPoint with different DecimalPlaces
     * @param other Other FixedPoint to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int Other>
    explicit FixedPoint(const FixedPoint<Other, StorageType>& other) {
        value = rescaleRaw<Other, DecimalPlaces>(other.getRawValue());
    }

    // ============================================================================
    // Constructor from raw value
    // ============================================================================
    /*
     * @brief Construct FixedPoint from raw internal value
     * @param rawValue Raw internal value
     * @return FixedPoint instance
    */
    static constexpr FixedPoint fromRaw(StorageType rawValue) {
        FixedPoint fp;
        fp.value = rawValue;
        return fp;
    }

    // ============================================================================
    // fromString
    // ============================================================================
    /*
     * @brief Construct FixedPoint from string representation
     * @param s String representation (e.g., "123.45" for DecimalPlaces=2)
     * @return FixedPoint instance
     * @throws std::runtime_error on invalid format
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    static FixedPoint fromString(std::string s) { // TODO: check optimizations
        // , and . normalization
        for (auto& c: s) if (c == ',') c = '.';

        // Check if there is any invalid character
        for (size_t idx = 0; idx < s.size(); ++idx) {
            char c = s[idx];
            if (!std::isdigit((unsigned char)c) && c != '+' && c != '-' && c != '.') {
                #if THROW_ON_FIXED_POINT_ERROR
                    throw std::runtime_error("FixedPoint: fromString: invalid character '" + std::string(1, c) + "' at position " + std::to_string(idx));
                #else
                    return FixedPoint(0); // Return 0 on error if exceptions are disabled
                #endif
            }
        }
        // Check if there is at most one dot
        size_t dotCount = 0;
        for (char c: s) if (c == '.') ++dotCount;
        if (dotCount > 1) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: fromString: multiple decimal points");
            #else
                return FixedPoint(0); // Return 0 on error if exceptions are disabled
            #endif
        }

        bool neg = false;
        size_t i = 0;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) { neg = (s[i] == '-'); ++i; }
        if (i >= s.size()) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: fromString: empty after sign");
            #else
                return FixedPoint(0); // Return 0 on error if exceptions are disabled
            #endif
        }
        // Check if we are trying to pass a negative number into an unsigned FixedPoint
        if (neg && !isSigned) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: fromString: negative number into unsigned FixedPoint");
            #else
                return FixedPoint(0); // Return 0 on error if exceptions are disabled
            #endif
        }

        StorageType intPart = 0;
        bool hasInt = false;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) {
            hasInt = true;
            int digit = s[i]-'0';
            if (intPart > std::numeric_limits<StorageType>::max() / 10 || // if intPart*10 would overflow
                (intPart == std::numeric_limits<StorageType>::max() / 10 && digit > (std::numeric_limits<StorageType>::max() % 10))) { // if the current digit would overflow
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: fromString: int overflow");
                #endif
            }
            intPart = intPart * 10 + digit;
            ++i;
        }

        StorageType fracPart = 0;
        int fracLen = 0;
        if (i < s.size() && s[i] == '.') {
            ++i;
            while (i < s.size() && std::isdigit((unsigned char)s[i]) && fracLen < DecimalPlaces) {
                fracPart = fracPart * 10 + (s[i]-'0');
                ++fracLen;
                ++i;
            }
            // ignore remaining digits beyond DecimalPlaces → TRUNCATION (no rounding)
            while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
        }

        if (!hasInt && fracLen == 0) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: fromString: no digits found");
            #else
                return FixedPoint(0); // Return 0 if no digits were found
            #endif
        }
        if (i != s.size()) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: fromString: trailing chars");
            #endif // ignore trailing chars if exceptions are disabled
        }

        // Scale
        for (int k = fracLen; k < DecimalPlaces; ++k) fracPart *= 10;

        // compose
        StorageType scale = pow10(DecimalPlaces);
        if (intPart > 0 && intPart > std::numeric_limits<StorageType>::max() / scale) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: fromString: scaled overflow");
            #endif
        }
        StorageType raw = intPart * scale + fracPart;
        // Check that raw is not overflowing when negated
        if (neg && raw == std::numeric_limits<StorageType>::min()) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: fromString: negation overflow");
            #endif
        }
        if (neg) raw = -raw;
        return fromRaw(raw);
    }

    // ============================================================================
    // Assignment operators
    // ============================================================================
    /*
     * @brief Copy assignment operator
    */
    FixedPoint& operator=(const FixedPoint&) noexcept = default;

    /*
     * @brief Assignment from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from int are allowed for ease of use
    */
    FixedPoint& operator=(int val) {
        StorageType v = CHECKED_CAST(int, val, "assignment from int");
        value = CHECKED_MUL(v, SCALE, "assignment from int");
        return *this;
    }

    /*
     * @brief Assignment from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from float are allowed for ease of use
    */
    FixedPoint& operator=(float val) {
        value = CHECKED_CAST(float, val * SCALE, "assignment from float");
        return *this;
    }

    /*
     * @brief Assignment from built-in types
     * @param val Value to convert
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @note Implicit conversions from double are allowed for ease of use
    */
    FixedPoint& operator=(double val) {
        value = CHECKED_CAST(double, val * SCALE, "assignment from double");
        return *this;
    }

    // ============================================================================
    // Arithmetic operators
    // ============================================================================
    /*
     * @brief Addition operator
     * @param other Other FixedPoint to add
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator+(const FixedPoint& other) const {
        return FixedPoint::fromRaw(CHECKED_ADD(value, other.value, "addition"));
    }

    /*
     * @brief Subtraction operator
     * @param other Other FixedPoint to subtract
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator-(const FixedPoint& other) const {
        return FixedPoint::fromRaw(CHECKED_SUB(value, other.value, "subtraction"));
    }

    /*
     * @brief Multiplication operator (safe, fast-path + fallback)
     * (value * other.value) / SCALE without overflowing the intermediate product.
     * @param other Other FixedPoint to multiply
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator*(const FixedPoint& other) const {
        using Lim = std::numeric_limits<StorageType>;
        using U   = std::make_unsigned_t<StorageType>;

        const StorageType a = value;
        const StorageType b = other.value;
        const StorageType S = SCALE;

        if (a == 0 || b == 0) return FixedPoint::fromRaw(0);

        // Fast path for 16/32-bit storage: widen to 64-bit then divide once.
        if constexpr (sizeof(StorageType) <= 4) {
            using Wide = std::conditional_t<isSigned, int64_t, uint64_t>;
            Wide wa = static_cast<Wide>(a);
            Wide wb = static_cast<Wide>(b);
            Wide wS = static_cast<Wide>(S);
            // Exact 64-bit product in Wide; still safe for 16/32-bit StorageType.
            Wide prod = wa * wb;
            Wide q = prod / wS; // integer division (trunc toward 0)
            return FixedPoint::fromRaw(CHECKED_CAST(Wide, q, "multiplication fast path"));
        }

        // General 64-bit path (or any larger integral type):
        // We compute (a*b)/S as: (q*b) + floor((r*b)/S) with a = q*S + r.
        // This avoids forming the full potentially overflowing product a*b.
        const bool neg = isSigned && ((a < 0) ^ (b < 0));

        auto uabs = [](StorageType x) -> U {
            if constexpr (isSigned) return x < 0 ? U(0) - U(x) : U(x);
            else                    return U(x);
        };

        U ua = uabs(a);
        U ub = uabs(b);
        U uS = uabs(S);

        // a = q*S + r with 0 <= r < S.
        U uq = ua / uS;
        U ur = ua % uS;

        // 1) term1 = q*b  (this cannot overflow if q <= max/|b|)
        //    We check in unsigned domain and then apply sign at the end.
        auto add_u_checked = [](U x, U y) -> U {
            if (x > std::numeric_limits<U>::max() - y) {
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: multiplication: accumulate overflow");
                #else
                    return std::numeric_limits<U>::max();
                #endif
            }
            return x + y;
        };

        U term1 = U(0);
        if (uq != 0) {
            if (ub > 0 && uq > std::numeric_limits<U>::max() / ub) {
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: multiplication: (q*b) overflow");
                #else
                    term1 = std::numeric_limits<U>::max();
                #endif
            } else {
                term1 = U(uq * ub);
            }
        }

        // 2) term2 = floor((r*b)/S) computed safely with gcd-based mulDiv.
        U term2 = mulDivUnsigned<U>(ur, ub, uS, "multiplication term2");

        // 3) magnitude = term1 + term2 (check overflow), then restore sign.
        U mag = add_u_checked(term1, term2);

        #if THROW_ON_FIXED_POINT_OVERFLOW
            if constexpr (isSigned) {
                if (!neg) {
                    if (mag > U(Lim::max())) throw std::overflow_error("FixedPoint: multiplication: result overflow");
                    return FixedPoint::fromRaw(static_cast<StorageType>(mag));
                } else {
                    U min_abs = U(Lim::max()) + 1; // |min|
                    if (mag > min_abs) throw std::overflow_error("FixedPoint: multiplication: result underflow");
                    auto s = -static_cast<std::make_signed_t<U>>(mag);
                    return FixedPoint::fromRaw(static_cast<StorageType>(s));
                }
            } else {
                if (mag > U(Lim::max())) throw std::overflow_error("FixedPoint: multiplication: result overflow");
                return FixedPoint::fromRaw(static_cast<StorageType>(mag));
            }
        #else
            if constexpr (isSigned) {
                auto s = neg ? -static_cast<std::make_signed_t<U>>(mag)
                            :  static_cast<std::make_signed_t<U>>(mag);
                return FixedPoint::fromRaw(static_cast<StorageType>(s));
            } else {
                return FixedPoint::fromRaw(static_cast<StorageType>(mag));
            }
        #endif
    }

    /*
     * @brief Division operator (safe)
     * (value * SCALE) / other.value without overflowing the intermediate product.
     * @param other Other FixedPoint to divide
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
     * @throws std::runtime_error on division by zero if THROW_ON_FIXED_POINT_ERROR is defined
    */
    FixedPoint operator/(const FixedPoint& other) const {
        using Lim = std::numeric_limits<StorageType>;
        using U   = std::make_unsigned_t<StorageType>;

        const StorageType a = value;
        const StorageType b = other.value;
        const StorageType S = SCALE;

        if (b == 0) {
            #if THROW_ON_FIXED_POINT_ERROR
                throw std::runtime_error("FixedPoint: division: division by zero");
            #else
                return FixedPoint::fromRaw((a >= 0) ? Lim::max() : Lim::min());
            #endif
        }

        const bool neg = isSigned && ((a < 0) ^ (b < 0));
        auto uabs = [](StorageType x) -> U {
            if constexpr (isSigned) return x < 0 ? U(0) - U(x) : U(x);
            else                          return U(x);
        };

        U ua = uabs(a);
        U ub = uabs(b);
        U uS = uabs(S);


        // ------------------------ PRECISE OVERFLOW PRE-CHECK ------------------------
        // We need to know whether floor((ua * uS) / ub) fits in U.
        //
        // Write ua = q0*ub + r0 with 0 <= r0 < ub.
        // Then floor((ua * uS) / ub) = q0*uS + floor((r0 * uS) / ub).
        //
        // If q0*uS already exceeds U::max → overflow.
        // If q0*uS == U::max, then ANY positive fractional part → overflow.
        // If q0*uS is close to the limit, we check whether the fractional part
        // floor((r0 * uS) / ub) would overflow the remaining headroom.

        U q0 = (ub != 0) ? (ua / ub) : U(0);
        U r0 = (ub != 0) ? (ua % ub) : U(0);

        const U UMAX = std::numeric_limits<U>::max();

        // 1) Check q0*uS overflow (without performing the multiplication)
        if (q0 != 0 && q0 > UMAX / uS) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: division: result overflow (integer part)");
            #else
                return FixedPoint::fromRaw(isSigned ? (neg ? Lim::min() : Lim::max()) : Lim::max());
            #endif
        }
        U q0uS = q0 * uS; // safe due to the check above

        // If integer part already saturated the full range, any fractional part overflows.
        if (q0uS == UMAX) {
            if (r0 != 0) {
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: division: result overflow (no room for fraction)");
                #else
                    return FixedPoint::fromRaw(isSigned ? (neg ? Lim::min() : Lim::max()) : Lim::max());
                #endif
            }
            // exact fit with zero fractional part
        } else {
            // 2) Bound the maximum possible fractional contribution:
            //    frac_max = floor(((ub-1) * uS) / ub) = uS - ceil(uS / ub)
            //    (because r0 < ub)
            U frac_max = U(0);
            if (ub != 0) {
                U ceil_uS_over_ub = (uS + (ub - 1)) / ub;    // ceil(uS/ub)
                frac_max = (uS >= ceil_uS_over_ub) ? (uS - ceil_uS_over_ub) : U(0);
            }

            // If q0uS + frac_max would exceed UMAX, overflow for sure.
            if (q0uS > UMAX - frac_max) {
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: division: result overflow (fractional bound)");
                #else
                    return FixedPoint::fromRaw(isSigned ? (neg ? Lim::min() : Lim::max()) : Lim::max());
                #endif
            }
        }
        // ---------------------- END PRECISE OVERFLOW PRE-CHECK ----------------------

        // Now compute the exact result safely.
        // We use the same decomposition: result = q0*uS + floor((r0 * uS) / ub)
        // The fractional term is computed with mulDivUnsigned AFTER gcd reductions,
        // which keeps intermediates small. In the problematic test, ub is small,
        // so this is very safe and fast. For large ub, reductions still help.

        // Safe unsigned multiply with overflow check for the integer part.
        auto add_u_checked = [](U x, U y) -> U {
            if (x > std::numeric_limits<U>::max() - y) {
                #if THROW_ON_FIXED_POINT_OVERFLOW
                    throw std::overflow_error("FixedPoint: division: accumulate overflow");
                #else
                    return std::numeric_limits<U>::max();
                #endif
            }
            return x + y;
        };

        U integerPart = q0uS;

        U fractionalPart = U(0);
        if (r0 != 0) {
            // Reduce before mul/div to shrink values
            U rn = r0, dm = ub, sc = uS;

            if (U g = std::gcd(rn, dm); g > 1) { rn /= g; dm /= g; }
            if (U g = std::gcd(sc, dm); g > 1) { sc /= g; dm /= g; }

            // Consume factors 2 and 5 from denominator into the scale when possible
            auto consume = [](U& num, U& den, U p) {
                while (den % p == 0) {
                    if (num % p == 0) { num /= p; den /= p; }
                    else break;
                }
            };
            consume(sc, dm, U(2));
            consume(sc, dm, U(5));

            // Compute floor((rn * sc) / dm) with the wide-safe helper.
            fractionalPart = mulDivUnsigned<U>(rn, sc, dm, "division fractional part");
        }

        U mag = add_u_checked(integerPart, fractionalPart);

        #if THROW_ON_FIXED_POINT_OVERFLOW
            if constexpr (isSigned) {
                if (!neg) {
                    if (mag > U(Lim::max())) throw std::overflow_error("FixedPoint: division: result overflow");
                    return FixedPoint::fromRaw(static_cast<StorageType>(mag));
                } else {
                    U min_abs = U(Lim::max()) + 1; // |min| for two's complement
                    if (mag > min_abs) throw std::overflow_error("FixedPoint: division: result underflow");
                    auto s = -static_cast<std::make_signed_t<U>>(mag);
                    return FixedPoint::fromRaw(static_cast<StorageType>(s));
                }
            } else {
                if (mag > U(Lim::max())) throw std::overflow_error("FixedPoint: division: result overflow");
                return FixedPoint::fromRaw(static_cast<StorageType>(mag));
            }
        #else
            if constexpr (isSigned) {
                auto s = neg ? -static_cast<std::make_signed_t<U>>(mag)
                            :  static_cast<std::make_signed_t<U>>(mag);
                return FixedPoint::fromRaw(static_cast<StorageType>(s));
            } else {
                return FixedPoint::fromRaw(static_cast<StorageType>(mag));
            }
        #endif
    }

    // ============================================================================
    // Arithmetic operators between different precisions
    // ============================================================================
    /*
     * @brief Addition operator between different DecimalPlaces
     * @param other Other FixedPoint to add
     * @return Resulting FixedPoint with max DecimalPlaces
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces), StorageType> operator+(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces, StorageType>;

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return ResultType::fromRaw(CHECKED_ADD(thisAdjusted, otherAdjusted, "addition different DP"));
    }

    /*
     * @brief Subtraction operator between different DecimalPlaces
     * @param other Other FixedPoint to subtract
     * @return Resulting FixedPoint with max DecimalPlaces
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces), StorageType> operator-(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces, StorageType>;

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return ResultType::fromRaw(CHECKED_SUB(thisAdjusted, otherAdjusted, "subtraction different DP"));
    }

    /*
     * @brief Multiplication operator between different DecimalPlaces
     * @param other Other FixedPoint to multiply
     * @return Resulting FixedPoint with max DecimalPlaces
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces), StorageType> operator*(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces, StorageType>;

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return (ResultType::fromRaw(thisAdjusted) * ResultType::fromRaw(otherAdjusted)); // Reuse existing operator* between FixedPoint with same DecimalPlaces
    }

    /*
     * @brief Division operator between different DecimalPlaces
     * @param other Other FixedPoint to divide
     * @return Resulting FixedPoint with max DecimalPlaces
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces), StorageType> operator/(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces, StorageType>;

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return (ResultType::fromRaw(thisAdjusted) / ResultType::fromRaw(otherAdjusted)); // Reuse existing operator/ between FixedPoint with same DecimalPlaces
    }

    // ============================================================================
    // Arithmetic operators with scalars
    // ============================================================================
    /*
     * @brief Addition operator with int
     * @param val int value to add
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator+(int val) const {
        return FixedPoint::fromRaw(CHECKED_ADD(value, CHECKED_MUL(CHECKED_CAST(int, val, "addition with int"), SCALE, "addition with int"), "addition with int"));
    }

    /*
     * @brief Substraction operator with int
     * @param val int value to subtract
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator-(int val) const {
        return FixedPoint::fromRaw(CHECKED_SUB(value, CHECKED_MUL(CHECKED_CAST(int, val, "subtraction with int"), SCALE, "subtraction with int"), "subtraction with int"));
    }

    /*
     * @brief Multiplication operator with int
     * @param val int value to multiply
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator*(int val) const {
        return FixedPoint::fromRaw(CHECKED_MUL(value, CHECKED_CAST(int, val, "multiplication with int"), "multiplication with int"));
    }

    /*
     * @brief Division operator with int
     * @param val int value to divide
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator/(int val) const {
        return FixedPoint::fromRaw(CHECKED_DIV(value, CHECKED_CAST(int, val, "division with int"), "division with int"));
    }

    /*
     * @brief Addition operator with float
     * @param val float value to add
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator+(float val) const {
        return FixedPoint::fromRaw(CHECKED_ADD(value, CHECKED_CAST(float, val * SCALE, "addition with float"), "addition with float"));
    }

    /*
     * @brief Substraction operator with float
     * @param val float value to subtract
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator-(float val) const {
        return FixedPoint::fromRaw(CHECKED_SUB(value, CHECKED_CAST(float, val * SCALE, "subtraction with float"), "subtraction with float"));
    }

    /*
     * @brief Multiplication operator with float
     * @param val float value to multiply
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator*(float val) const {
        return *this * FixedPoint(val); // Reuse existing operator* between FixedPoint
    }

    /*
     * @brief Division operator with float
     * @param val float value to divide
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator/(float val) const {
        return *this / FixedPoint(val); // Reuse existing operator/ between FixedPoint
    }

    /*
     * @brief Addition operator with double
     * @param val double value to add
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator+(double val) const {
        return FixedPoint::fromRaw(CHECKED_ADD(value, CHECKED_CAST(double, val * SCALE, "addition with double"), "addition with double"));
    }

    /*
     * @brief Substraction operator with double
     * @param val double value to subtract
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator-(double val) const {
        return FixedPoint::fromRaw(CHECKED_SUB(value, CHECKED_CAST(double, val * SCALE, "subtraction with double"), "subtraction with double"));
    }

    /*
     * @brief Multiplication operator with double
     * @param val double value to multiply
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator*(double val) const {
        return *this * FixedPoint(val); // Reuse existing operator* between FixedPoint
    }

    /*
     * @brief Division operator with double
     * @param val double value to divide
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator/(double val) const {
        return *this / FixedPoint(val); // Reuse existing operator/ between FixedPoint
    }

    // ============================================================================
    // External operators for commutativity
    // ============================================================================
    /*
     * @brief External addition operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator+(int lhs, FixedPoint rhs)  { rhs += lhs; return rhs; }
    /*
     * @brief External subtraction operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator-(int lhs, FixedPoint rhs)  { return FixedPoint(lhs) - rhs; }
    /*
     * @brief External multiplication operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator*(int lhs, FixedPoint rhs)  { rhs *= lhs; return rhs; }
    /*
     * @brief External division operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator/(int lhs, const FixedPoint& rhs) { return FixedPoint(lhs) / rhs; }

    /*
     * @brief External addition operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator+(float lhs, FixedPoint rhs)  { return FixedPoint(lhs) + rhs; }
    /*
     * @brief External subtraction operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator-(float lhs, FixedPoint rhs)  { return FixedPoint(lhs) - rhs; }
    /*
     * @brief External multiplication operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator*(float lhs, FixedPoint rhs)  { return FixedPoint(lhs) * rhs; }
    /*
     * @brief External division operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator/(float lhs, const FixedPoint& rhs) { return FixedPoint(lhs) / rhs; }

    /*
     * @brief External addition operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator+(double lhs, FixedPoint rhs) { return FixedPoint(lhs) + rhs; }
    /*
     * @brief External subtraction operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator-(double lhs, FixedPoint rhs) { return FixedPoint(lhs) - rhs; }
    /*
     * @brief External multiplication operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator*(double lhs, FixedPoint rhs) { return FixedPoint(lhs) * rhs; }
    /*
     * @brief External division operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return Resulting FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend FixedPoint operator/(double lhs, const FixedPoint& rhs) { return FixedPoint(lhs) / rhs; }

    // ============================================================================
    // Unary operators
    // ============================================================================
    /*
     * @brief Unary minus operator
     * @return Negated FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint operator-() const {
        #if THROW_ON_FIXED_POINT_OVERFLOW
            if (value == std::numeric_limits<StorageType>::min())
                throw std::overflow_error("FixedPoint: unary minus: overflow");
            if (!isSigned)
                throw std::overflow_error("FixedPoint: unary minus: overflow on unsigned type");
        #endif
        return FixedPoint::fromRaw(-value);
    }

    /*
     * @brief Unary plus operator
     * @return Same FixedPoint
    */
    FixedPoint operator+() const {
        return *this;
    }

    // ============================================================================
    // Composed assignment operators
    // ============================================================================
    /*
     * @brief Composed addition assignment operator
     * @param other Other FixedPoint to add
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator+=(const FixedPoint& other) {
        value = CHECKED_ADD(value, other.value, "addition assignment");
        return *this;
    }

    /*
     * @brief Composed subtraction assignment operator
     * @param other Other FixedPoint to subtract
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator-=(const FixedPoint& other) {
        value = CHECKED_SUB(value, other.value, "subtraction assignment");
        return *this;
    }

    /*
     * @brief Composed multiplication assignment operator
     * @param other Other FixedPoint to multiply
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator*=(const FixedPoint& other) {
        *this = *this * other; // Use existing operator*
        return *this;
    }

    /*
     * @brief Composed division assignment operator
     * @param other Other FixedPoint to divide
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator/=(const FixedPoint& other) {
        *this = *this / other; // Use existing operator/
        return *this;
    }

    // ============================================================================
    // Composed assignment operators between different precisions
    // ============================================================================
    /*
     * @brief Composed addition assignment operator between different DecimalPlaces
     * @param other Other FixedPoint to add
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint& operator+=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) {
        *this = *this + other;
        return *this;
    }

    /*
     * @brief Composed subtraction assignment operator between different DecimalPlaces
     * @param other Other FixedPoint to subtract
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint& operator-=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) {
        *this = *this - other;
        return *this;
    }

    /*
     * @brief Composed multiplication assignment operator between different DecimalPlaces
     * @param other Other FixedPoint to multiply
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint& operator*=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) {
        *this = *this * other;
        return *this;
    }

    /*
     * @brief Composed division assignment operator between different DecimalPlaces
     * @param other Other FixedPoint to divide
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    template<int OtherDecimalPlaces>
    FixedPoint& operator/=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) {
        *this = *this / other;
        return *this;
    }

    // ============================================================================
    // Composed assignment operators with scalars
    // ============================================================================
    /*
     * @brief Composed addition assignment operator with int
     * @param val int value to add
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator+=(int val) {
        value = CHECKED_ADD(value, CHECKED_MUL(CHECKED_CAST(int, val, "addition assignment with int"), SCALE, "addition assignment with int"), "addition assignment with int");
        return *this;
    }

    /*
     * @brief Composed subtraction assignment operator with int
     * @param val int value to subtract
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator-=(int val) {
        value = CHECKED_SUB(value, CHECKED_MUL(CHECKED_CAST(int, val, "subtraction assignment with int"), SCALE, "subtraction assignment with int"), "subtraction assignment with int");
        return *this;
    }

    /*
     * @brief Composed multiplication assignment operator with int
     * @param val int value to multiply
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator*=(int val) {
        value = CHECKED_MUL(value, CHECKED_CAST(int, val, "multiplication assignment with int"), "multiplication assignment with int");
        return *this;
    }

    /*
     * @brief Composed division assignment operator with int
     * @param val int value to divide
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator/=(int val) {
        value = CHECKED_DIV(value, CHECKED_CAST(int, val, "division assignment with int"), "division assignment with int");
        return *this;
    }

    /*
     * @brief Composed addition assignment operator with float
     * @param val float value to add
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator+=(float val) {
        value = CHECKED_ADD(value, CHECKED_CAST(float, val * SCALE, "addition assignment with float"), "addition assignment with float");
        return *this;
    }

    /*
     * @brief Composed subtraction assignment operator with float
     * @param val float value to subtract
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator-=(float val) {
        value = CHECKED_SUB(value, CHECKED_CAST(float, val * SCALE, "subtraction assignment with float"), "subtraction assignment with float");
        return *this;
    }

    /*
     * @brief Composed multiplication assignment operator with float
     * @param val float value to multiply
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator*=(float val) {
        *this = *this * FixedPoint(val); // Reuse existing operator* between FixedPoint
        return *this;
    }

    /*
     * @brief Composed division assignment operator with float
     * @param val float value to divide
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator/=(float val) {
        *this = *this / FixedPoint(val); // Reuse existing operator/ between FixedPoint
        return *this;
    }

    /*
     * @brief Composed addition assignment operator with double
     * @param val double value to add
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator+=(double val) {
        value = CHECKED_ADD(value, CHECKED_CAST(double, val * SCALE, "addition assignment with double"), "addition assignment with double");
        return *this;
    }

    /*
     * @brief Composed subtraction assignment operator with double
     * @param val double value to subtract
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator-=(double val) {
        value = CHECKED_SUB(value, CHECKED_CAST(double, val * SCALE, "subtraction assignment with double"), "subtraction assignment with double");
        return *this;
    }

    /*
     * @brief Composed multiplication assignment operator with double
     * @param val double value to multiply
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator*=(double val) {
        *this = *this * FixedPoint(val); // Reuse existing operator* between FixedPoint
        return *this;
    }

    /*
     * @brief Composed division assignment operator with double
     * @param val double value to divide
     * @return Reference to this FixedPoint
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    FixedPoint& operator/=(double val) {
        *this = *this / FixedPoint(val); // Reuse existing operator/ between FixedPoint
        return *this;
    }

    // ============================================================================
    // Comparison operators
    // ============================================================================
    /*
     * @brief Equality operator
     * @param other Other FixedPoint to compare
     * @return true if equal, false otherwise
    */
    bool operator==(const FixedPoint& other) const {
        return value == other.value;
    }

    /*
     * @brief Inequality operator
     * @param other Other FixedPoint to compare
     * @return true if not equal, false otherwise
    */
    bool operator!=(const FixedPoint& other) const {
        return value != other.value;
    }

    /*
     * @brief Less than operator
     * @param other Other FixedPoint to compare
     * @return true if this < other, false otherwise
    */
    bool operator<(const FixedPoint& other) const {
        return value < other.value;
    }

    /*
     * @brief Less than or equal operator
     * @param other Other FixedPoint to compare
     * @return true if this <= other, false otherwise
    */
    bool operator<=(const FixedPoint& other) const {
        return value <= other.value;
    }

    /*
     * @brief Greater than operator
     * @param other Other FixedPoint to compare
     * @return true if this > other, false otherwise
    */
    bool operator>(const FixedPoint& other) const {
        return value > other.value;
    }

    /*
     * @brief Greater than or equal operator
     * @param other Other FixedPoint to compare
     * @return true if this >= other, false otherwise
    */
    bool operator>=(const FixedPoint& other) const {
        return value >= other.value;
    }

    // ============================================================================
    // Comparison operators between different precisions
    // ============================================================================
    /*
     * @brief Equality operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if equal, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator==(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces);

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return thisAdjusted == otherAdjusted;
    }

    /*
     * @brief Inequality operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if not equal, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator!=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        return !(*this == other);
    }

    /*
     * @brief Less than operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if this < other, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator<(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces);

        StorageType thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        StorageType otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return thisAdjusted < otherAdjusted;
    }

    /*
     * @brief Less than or equal operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if this <= other, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator<=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        return *this < other || *this == other;
    }

    /*
     * @brief Greater than operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if this > other, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator>(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        return !(*this <= other);
    }

    /*
     * @brief Greater than or equal operator between different DecimalPlaces
     * @param other Other FixedPoint to compare
     * @return true if this >= other, false otherwise
    */
    template<int OtherDecimalPlaces>
    bool operator>=(const FixedPoint<OtherDecimalPlaces, StorageType>& other) const {
        return !(*this < other);
    }

    // ============================================================================
    // Comparison operators with scalars
    // ============================================================================
    /*
     * @brief Equality operator with int
     * @param val int value to compare
     * @return true if equal, false otherwise
    */
    bool operator==(int val) const {
        return *this == FixedPoint(val);
    }

    /*
     * @brief Inequality operator with int
     * @param val int value to compare
     * @return true if not equal, false otherwise
    */
    bool operator!=(int val) const {
        return *this != FixedPoint(val);
    }

    /*
     * @brief Less than operator with int
     * @param val int value to compare
     * @return true if this < val, false otherwise
    */
    bool operator<(int val) const {
        return *this < FixedPoint(val);
    }

    /*
     * @brief Less than or equal operator with int
     * @param val int value to compare
     * @return true if this <= val, false otherwise
    */
    bool operator<=(int val) const {
        return *this <= FixedPoint(val);
    }

    /*
     * @brief Greater than operator with int
     * @param val int value to compare
     * @return true if this > val, false otherwise
    */
    bool operator>(int val) const {
        return *this > FixedPoint(val);
    }

    /*
     * @brief Greater than or equal operator with int
     * @param val int value to compare
     * @return true if this >= val, false otherwise
    */
    bool operator>=(int val) const {
        return *this >= FixedPoint(val);
    }

    /*
     * @brief Equality operator with float
     * @param val float value to compare
     * @return true if equal, false otherwise
    */
    bool operator==(float val) const {
        return *this == FixedPoint(val);
    }

    /*
     * @brief Inequality operator with float
     * @param val float value to compare
     * @return true if not equal, false otherwise
    */
    bool operator!=(float val) const {
        return *this != FixedPoint(val);
    }

    /*
     * @brief Less than operator with float
     * @param val float value to compare
     * @return true if this < val, false otherwise
    */
    bool operator<(float val) const {
        return *this < FixedPoint(val);
    }

    /*
     * @brief Less than or equal operator with float
     * @param val float value to compare
     * @return true if this <= val, false otherwise
    */
    bool operator<=(float val) const {
        return *this <= FixedPoint(val);
    }

    /*
     * @brief Greater than operator with float
     * @param val float value to compare
     * @return true if this > val, false otherwise
    */
    bool operator>(float val) const {
        return *this > FixedPoint(val);
    }

    /*
     * @brief Greater than or equal operator with float
     * @param val float value to compare
     * @return true if this >= val, false otherwise
    */
    bool operator>=(float val) const {
        return *this >= FixedPoint(val);
    }

    /*
     * @brief Equality operator with double
     * @param val double value to compare
     * @return true if equal, false otherwise
    */
    bool operator==(double val) const {
        return *this == FixedPoint(val);
    }

    /*
     * @brief Inequality operator with double
     * @param val double value to compare
     * @return true if not equal, false otherwise
    */
    bool operator!=(double val) const {
        return *this != FixedPoint(val);
    }

    /*
     * @brief Less than operator with double
     * @param val double value to compare
     * @return true if this < val, false otherwise
    */
    bool operator<(double val) const {
        return *this < FixedPoint(val);
    }

    /*
     * @brief Less than or equal operator with double
     * @param val double value to compare
     * @return true if this <= val, false otherwise
    */
    bool operator<=(double val) const {
        return *this <= FixedPoint(val);
    }

    /*
     * @brief Greater than operator with double
     * @param val double value to compare
     * @return true if this > val, false otherwise
    */
    bool operator>(double val) const {
        return *this > FixedPoint(val);
    }

    /*
     * @brief Greater than or equal operator with double
     * @param val double value to compare
     * @return true if this >= val, false otherwise
    */
    bool operator>=(double val) const {
        return *this >= FixedPoint(val);
    }

    // ============================================================================
    // External comparison operators for commutativity
    // ============================================================================
    /*
     * @brief External equality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator==(int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) == rhs; }
    /*
     * @brief External inequality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if not equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator!=(int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) != rhs; }
    /*
     * @brief External less than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator< (int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) <  rhs; }
    /*
     * @brief External less than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator<=(int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) <= rhs; }
    /*
     * @brief External more than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator> (int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) >  rhs; }
    /*
     * @brief External more than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator>=(int lhs,  const FixedPoint& rhs){ return FixedPoint(lhs) >= rhs; }

    /*
     * @brief External equality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator==(float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) == rhs; }
    /*
     * @brief External inequality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if not equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator!=(float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) != rhs; }
    /*
     * @brief External less than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator< (float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) <  rhs; }
    /*
     * @brief External less than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator<=(float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) <= rhs; }
    /*
     * @brief External more than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator> (float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) >  rhs; }
    /*
     * @brief External more than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator>=(float lhs,const FixedPoint& rhs){ return FixedPoint(lhs) >= rhs; }

    /*
     * @brief External equality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator==(double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) == rhs; }
    /*
     * @brief External inequality operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if not equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator!=(double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) != rhs; }
    /*
     * @brief External less than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator< (double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) <  rhs; }
    /*
     * @brief External less than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if less than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator<=(double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) <= rhs; }
    /*
     * @brief External more than operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator> (double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) >  rhs; }
    /*
     * @brief External more than or equal operator to allow commutativity with scalars on the left-hand side
     * @param lhs Left-hand side scalar
     * @param rhs Right-hand side FixedPoint
     * @return true if more than or equal, false otherwise
     * @throws std::overflow_error on overflow if THROW_ON_FIXED_POINT_OVERFLOW is defined
    */
    friend bool operator>=(double lhs,const FixedPoint& rhs){ return FixedPoint(lhs) >= rhs; }

    // ============================================================================
    // Increment and decrement operators
    // ============================================================================
    /*
     * @brief Prefix increment operator
     * @return Reference to this FixedPoint after increment
    */
    FixedPoint& operator++() { // ++value
        if (value > std::numeric_limits<StorageType>::max() - SCALE) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: prefix increment: overflow");
            #else
                value = std::numeric_limits<StorageType>::max(); // saturate
                return *this;
            #endif
        }
        value += SCALE;
        return *this;
    }

    /*
     * @brief Postfix increment operator
     * @return Copy of FixedPoint before increment
    */
    FixedPoint operator++(int) { // value++
        FixedPoint temp = *this;
        if (value > std::numeric_limits<StorageType>::max() - SCALE) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: postfix increment: overflow");
            #else
                value = std::numeric_limits<StorageType>::max(); // saturate
                return temp;
            #endif
        }
        value += SCALE;
        return temp;
    }

    /*
     * @brief Prefix decrement operator
     * @return Reference to this FixedPoint after decrement
    */
    FixedPoint& operator--() { // --value
        if (value < std::numeric_limits<StorageType>::min() + SCALE) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: prefix decrement: overflow");
            #else
                value = std::numeric_limits<StorageType>::min(); // saturate
                return *this;
            #endif
        }
        value -= SCALE;
        return *this;
    }

    /*
     * @brief Postfix decrement operator
     * @return Copy of FixedPoint before decrement
    */
    FixedPoint operator--(int) { // value--
        FixedPoint temp = *this;
        if (value < std::numeric_limits<StorageType>::min() + SCALE) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: postfix decrement: overflow");
            #else
                value = std::numeric_limits<StorageType>::min(); // saturate
                return temp;
            #endif
        }
        value -= SCALE;
        return temp;
    }

    // ============================================================================
    // Conversion functions
    // ============================================================================
    /*
     * @brief Convert to float
     * @return float representation
    */
    float toFloat() const {
        return float(value) / SCALE;
    }

    /*
     * @brief Convert to double
     * @return double representation
    */
    double toDouble() const {
        return double(value) / SCALE;
    }

    /*
     * @brief Convert to int (truncating fractional part)
     * @return int representation
    */
    int toInt() const {
        return int(value / SCALE);
    }

    /*
     * @brief Convert to std::string
     * @return std::string representation
    */
    std::string toString() const {
        if constexpr (DecimalPlaces == 0) {
            return std::to_string(value / SCALE);
        }

        StorageType raw = value;

        bool isNegative = (isSigned && raw < 0);
        UnsignedStorageType absoluteValue = isNegative ? UnsignedStorageType(-(raw + 1)) + 1 : UnsignedStorageType(raw);

        UnsignedStorageType integerPart = absoluteValue / SCALE;
        UnsignedStorageType fractionalPart = absoluteValue % SCALE;

        // sign + integer part + '.' + fractional part + null terminator
        static constexpr std::size_t BUF_SIZE = 1 + std::numeric_limits<UnsignedStorageType>::digits10 + 1 + DecimalPlaces + 1; // max sizes for each part
        char buffer[BUF_SIZE];
        char *ptr = buffer;
        char *end = buffer + sizeof(buffer);

        if (isNegative) {
            *ptr++ = '-';
        }
        auto r1 = std::to_chars(ptr, end, integerPart, 10); // Convert integer part
        ptr = r1.ptr;
        *ptr++ = '.';

        UnsignedStorageType padding = SCALE / 10;
        while (padding > 0) {
            *ptr++ = char('0' + int((fractionalPart / padding) % 10));
            padding /= 10;
        }
        *ptr = '\0';
        return std::string(buffer);
    }

    /*
     * @brief Get the raw underlying storage value
     * @return Raw storage value
    */
    StorageType getRawValue() const {
        return value;
    }

    // ============================================================================
    // Explicit conversion operators
    // ============================================================================
    /*
     * @brief Explicit conversion operators to float
     * @note Implicit conversions are not allowed to prevent accidental data loss
    */
    explicit operator float() const {
        return toFloat();
    }

    /*
     * @brief Explicit conversion operators to double
     * @note Implicit conversions are not allowed to prevent accidental data loss
    */
    explicit operator double() const {
        return toDouble();
    }

    /*
     * @brief Explicit conversion operators to int
     * @note Implicit conversions are not allowed to prevent accidental data loss
    */
    explicit operator int() const {
        return toInt();
    }

    /*
     * @brief Explicit conversion operators to std::string
     * @note Implicit conversions are not allowed to prevent accidental data loss
    */
    explicit operator std::string() const {
        return toString();
    }

    // ============================================================================
    // Mathematical functions
    // ============================================================================
    /*
     * @brief Absolute value
     * @return A FixedPoint representing the absolute value of this FixedPoint
    */
    FixedPoint abs() const {
        if constexpr (!isSigned) {
            return fromRaw(value);
        }
        if (value >= 0) {
            return fromRaw(value);
        }
        if (value == std::numeric_limits<StorageType>::min()) {
            #if THROW_ON_FIXED_POINT_OVERFLOW
                throw std::overflow_error("FixedPoint: abs(): overflow on min value");
            #else
                return fromRaw(std::numeric_limits<StorageType>::max());
            #endif
        }
        return fromRaw(-value);
    }

    /*
     * @brief Floor function
     * @return A FixedPoint representing the largest integer less than or equal to this FixedPoint
    */
    FixedPoint floor() const {
        if (value >= 0) return fromRaw((value / SCALE) * SCALE);
        return fromRaw(value % SCALE == 0 ? value : ((value / SCALE) - 1) * SCALE);
    }

    /*
     * @brief Ceil function
     * @return A FixedPoint representing the smallest integer greater than or equal to this FixedPoint
    */
    FixedPoint ceil() const {
        if (value <= 0) return fromRaw((value / SCALE) * SCALE);
        return fromRaw(value % SCALE == 0 ? value : ((value / SCALE) + 1) * SCALE);
    }

    /*
     * @brief Round function
     * @return A FixedPoint representing this FixedPoint rounded to the nearest integer
    */
    FixedPoint round() const {
        const StorageType half = SCALE / 2;
        const StorageType adj  = (value >= 0 ? half : -half);
        return fromRaw(((value + adj) / SCALE) * SCALE);
    }

    /*
     * @brief Get the fractional part of the FixedPoint
     * @return A FixedPoint representing the fractional part of this FixedPoint
    */
    FixedPoint frac() const {
        if constexpr (DecimalPlaces == 0) return zero();
        StorageType fractionalPart = value % SCALE;
        if (fractionalPart < 0) fractionalPart += SCALE;
        return fromRaw(fractionalPart);
    }

    // ============================================================================
    // Display operator
    // ============================================================================
    /*
     * @brief Stream output operator
     * @param os Output stream
     * @param fp FixedPoint to output
     * @return Reference to output stream
    */
    friend std::ostream& operator<<(std::ostream& os, const FixedPoint& fp) {
        os << fp.toString();
        return os;
    }

    // ============================================================================
    // Useful constants
    // ============================================================================
    /*
     * @brief Get the FixedPoint representation of zero (0.0)
     * @return FixedPoint representing zero (0.0)
    */
    static constexpr FixedPoint zero() noexcept {
        return FixedPoint::fromRaw(0);
    }

    /*
     * @brief Get the FixedPoint representation of one (1.0)
     * @return FixedPoint representing one (1.0)
    */
    static constexpr FixedPoint one() noexcept {
        return FixedPoint::fromRaw(SCALE);
    }

    // ============================================================================
    // Type information
    // ============================================================================
    /*
     * @brief Get the number of decimal places for this FixedPoint type
     * @return Number of decimal places
    */
    static constexpr int getDecimalPlaces() noexcept {
        return DecimalPlaces;
    }

    /*
     * @brief Get the scale factor used for this FixedPoint type
     * @return Scale factor
    */
    static constexpr StorageType getScale() noexcept {
        return SCALE;
    }

    static void testingFunction() {
        using std::cout;
        using std::endl;

        auto hdr = [&](const char* title){
            cout << "\n========== " << title << " ==========\n";
        };
        auto RUN = [&](const char* name, auto&& fn){
            cout << "\n-- " << name << " --\n";
            try { fn(); cout << "  -> [OK]\n"; }
            catch(const std::exception& e){ cout << "  -> [EXC@" << name << "] " << e.what() << "\n"; }
        };
        auto dumpScalar = [&](const char* lbl, auto v){
            cout << lbl << " = " << v << "\n";
        };
        // Dump *générique* mais on ne l'appelle que sur FixedPoint<DP,ST>
        auto dumpFP = [&](const char* lbl, const auto& x){
            using X = std::decay_t<decltype(x)>;
            using RawT = decltype(x.getRawValue());
            constexpr bool Sgn = std::numeric_limits<RawT>::is_signed;
            cout << lbl
                << "  str="   << x.toString()
                << "  raw="   << x.getRawValue()
                << "  SCALE=" << X::getScale()
                << "  DP="    << X::getDecimalPlaces()
                << "  T="     << (Sgn ? "signed" : "unsigned")
                << " ("       << (sizeof(RawT)*8) << "b)"
                << "\n";
        };

        // --- Aliases absolus ---
        using I16_2 = FixedPoint<2, int16_t>;
        using I16_3 = FixedPoint<3, int16_t>;
        using I32_2 = FixedPoint<2, int32_t>;
        using I32_5 = FixedPoint<5, int32_t>;
        using I64_2 = FixedPoint<2, int64_t>;
        using I64_8 = FixedPoint<8, int64_t>;
        using U16_2 = FixedPoint<2, uint16_t>;
        using U32_2 = FixedPoint<2, uint32_t>;
        using U64_2 = FixedPoint<2, uint64_t>;

        cout << "=== FixedPoint self-test (absolute-type battery) ===\n";

        // ================== SIGNED int16_t ==================
        hdr("int16_t / DP mix");
        RUN("ctor / fromRaw / fromString", [&]{
            I16_2 a(12);         dumpFP("a(12)", a);
            I16_2 b(1.25f);      dumpFP("b(1.25f)", b);
            I16_2 c(1.25);       dumpFP("c(1.25)", c);
            I16_2 d = I16_2::fromRaw(I16_2::getScale()*2 + I16_2::getScale()/2); dumpFP("d(2.5 raw)", d);
            I16_2 e = I16_2::fromString("7,50"); dumpFP("e('7,50')", e);
        });
        RUN("add/sub (same DP)", [&]{
            auto a = I16_2::fromString("3.25");
            auto b = I16_2::fromString("1.50");
            dumpFP("a", a); dumpFP("b", b);
            dumpFP("a+b", a+b);
            dumpFP("a-b", a-b);
        });
        RUN("mul/div (same DP)", [&]{
            auto a = I16_2::fromString("3.00");
            auto b = I16_2::fromString("1.50");
            dumpFP("a", a); dumpFP("b", b);
            dumpFP("a*b", a*b);
            dumpFP("a/b", a/b);
        });
        RUN("scalars (right & left)", [&]{
            auto a = I16_2::fromString("2.50"); dumpFP("a", a);
            dumpFP("a+3", a+3);
            dumpFP("a-1.0", a-1.0);
            dumpFP("a*1.5f", a*1.5f);
            dumpFP("a/2", a/2);
            dumpFP("3+a", 3 + a);
            dumpFP("10.0-a", 10.0 - a);
            dumpFP("1.5f*a", 1.5f * a);
            dumpFP("7/a", 7 / a);
        });
        RUN("comparisons", [&]{
            auto a = I16_2::fromString("2.50");
            auto b = I16_2::fromString("2.50");
            auto c = I16_2::fromString("2.51");
            dumpFP("a", a); dumpFP("b", b); dumpFP("c", c);
            cout << "a==b? " << (a==b) << ", a!=c? " << (a!=c)
                << ", a<c? " << (a<c) << ", c>a? " << (c>a)
                << ", a<=b? " << (a<=b) << ", a>=b? " << (a>=b) << "\n";
            cout << "a==2? " << (a==2) << ", a>2.4? " << (a>2.4)
                << ", 2.5==a? " << (2.5==a) << ", 3<=a? " << (3<=a) << "\n";
        });
        RUN("helpers (abs/floor/ceil/round/frac)", [&]{
            auto a = I16_2::fromString("-3.75"); dumpFP("a", a);
            dumpFP("abs(a)", a.abs());
            dumpFP("floor(a)", a.floor());
            dumpFP("ceil(a)", a.ceil());
            dumpFP("round(a)", a.round());
            dumpFP("frac(a)", a.frac());
        });
        RUN("division by zero (should throw if enabled)", [&]{
            auto a = I16_2::fromString("1.0");
            auto z = I16_2::fromRaw(0);
            dumpFP("a", a); dumpFP("z", z);
            auto r = a / z; (void)r;
        });
        RUN("inter-precision add (I16_2 + I16_3)", [&]{
            I16_2 a = I16_2::fromString("3.40");
            I16_3 b = I16_3::fromString("1.234");
            dumpFP("a(I16_2)", a); dumpFP("b(I16_3)", b);
            auto r = a + b; dumpFP("a+b", r);
            auto r2 = b + a; dumpFP("b+a", r2);
            auto r3 = a * b; dumpFP("a*b", r3);
            auto r4 = a / b; dumpFP("a/b", r4);
        });

        // Cas d’overflow “probables” sur int16_t (selon tes macros ils throw/saturent)
        RUN("int16_t near max add overflow", [&]{
            auto nearMax = I16_2::fromRaw(std::numeric_limits<int16_t>::max() - (I16_2::getScale()/2));
            auto delta   = I16_2::fromRaw(I16_2::getScale());
            dumpFP("nearMax", nearMax); dumpFP("delta", delta);
            auto r = nearMax + delta; dumpFP("r", r);
        });
        RUN("int16_t mul big raw (fallback path)", [&]{
            auto a = I16_2::fromRaw(std::numeric_limits<int16_t>::max());
            auto b = I16_2::fromRaw(2);
            dumpFP("a", a); dumpFP("b", b);
            auto r = a * b; dumpFP("a*b", r);
        });

        // ================== SIGNED int32_t ==================
        hdr("int32_t / DP mix");
        RUN("I32_2 + I32_5 / * / /", [&]{
            I32_2 a = I32_2::fromString("12345.67");
            I32_5 b = I32_5::fromString("3.14159");
            dumpFP("a(I32_2)", a); dumpFP("b(I32_5)", b);
            dumpFP("a+b", a+b);
            dumpFP("b-a", b-a);
            dumpFP("a*b", a*b);
            dumpFP("a/b", a/b);
        });
        RUN("I32_5 fromString tricky", [&]{
            dumpFP("0012.3400", I32_5::fromString("0012.3400"));
            dumpFP("+3.14",     I32_5::fromString("+3.14"));
            // 2 points → selon TES règles: devrait throw/runtime 0
            auto x = I32_5::fromString("1.2.3"); dumpFP("1.2.3", x);
        });

        // ================== SIGNED int64_t ==================
        hdr("int64_t / haute précision");
        RUN("I64_2 * I64_8", [&]{
            I64_2 a = I64_2::fromString("123456789.25");
            I64_8 b = I64_8::fromString("0.00001234");
            dumpFP("a(I64_2)", a); dumpFP("b(I64_8)", b);
            dumpFP("a*b", a*b);
            dumpFP("a/b", a/b);
        });
        RUN("I64 near extremes ++/--", [&]{
            auto nearMax = I64_2::fromRaw(std::numeric_limits<int64_t>::max() - I64_2::getScale() + 1);
            dumpFP("nearMax", nearMax);
            ++nearMax; dumpFP("++nearMax", nearMax);
            auto nearMin = I64_2::fromRaw(std::numeric_limits<int64_t>::min() + I64_2::getScale() - 1);
            dumpFP("nearMin", nearMin);
            --nearMin; dumpFP("--nearMin", nearMin);
        });

        // ================== UNSIGNED ==================
        hdr("unsigned tests");
        RUN("U16_2 basics", [&]{
            U16_2 a = U16_2::fromString("12.50");
            U16_2 b = U16_2::fromString("1.25");
            dumpFP("a", a); dumpFP("b", b);
            dumpFP("a+b", a+b);
            dumpFP("a-b", a-b);
            dumpFP("a*b", a*b);
            dumpFP("a/0 (should throw)", a / U16_2::fromRaw(0));
        });
        RUN("U32_2 + U64_2 (même ST requis → on ne mixe pas)", [&]{
            U32_2 a = U32_2::fromString("1000.00");
            U32_2 b = U32_2::fromString("0.25");
            dumpFP("a(U32_2)", a); dumpFP("b(U32_2)", b);
            dumpFP("a+b", a+b);
            U64_2 c = U64_2::fromString("1.00");
            (void)c;
            cout << "NOTE: pas d'opérations U32 vs U64 (StorageType différent)\n";
        });

        // ================== Division Spam (stability check) ==================
        hdr("division stability spam");
        RUN("I32_5 division spam", [&]{
            // do a lot of random divisions to check stability
            I32_5 numerator = I32_5::fromString("12345.67890");
            I32_5 denominator = I32_5::fromString("1.00001");
            for (int i = 0; i < 100000; ++i) {
                auto result = numerator / denominator;
                // dumpFP("den", denominator);
                // dumpFP("div", result);
                // modify denominator slightly
                denominator = denominator + I32_5::fromRaw(1);
            }
        });

        cout << "\n=== End absolute-type self-test ===\n";
    }


};

// ============================================================================
// Alias of common types for easier use
// ============================================================================
// 64-bit signed fixed-point types (9 223 372 036 854 775 807 maximum value)
using FixedPointI64_0 = FixedPoint<0, int64_t>;     // 0 digit after decimal point (integer)
using FixedPointI64_1 = FixedPoint<1, int64_t>;     // 1 digit after decimal point
using FixedPointI64_2 = FixedPoint<2, int64_t>;     // 2 digit after decimal point
using FixedPointI64_3 = FixedPoint<3, int64_t>;     // 3 digit after decimal point
using FixedPointI64_4 = FixedPoint<4, int64_t>;     // 4 digit after decimal point
using FixedPointI64_5 = FixedPoint<5, int64_t>;     // 5 digit after decimal point
using FixedPointI64_6 = FixedPoint<6, int64_t>;     // 6 digit after decimal point
using FixedPointI64_7 = FixedPoint<7, int64_t>;     // 7 digit after decimal point
using FixedPointI64_8 = FixedPoint<8, int64_t>;     // 8 digit after decimal point (float precision)
using FixedPointI64_9 = FixedPoint<9, int64_t>;     // 9 digit after decimal point
using FixedPointI64_10 = FixedPoint<10, int64_t>;   // 10 digit after decimal point
using FixedPointI64_11 = FixedPoint<11, int64_t>;   // 11 digit after decimal point
using FixedPointI64_12 = FixedPoint<12, int64_t>;   // 12 digit after decimal point
using FixedPointI64_13 = FixedPoint<13, int64_t>;   // 13 digit after decimal point
using FixedPointI64_14 = FixedPoint<14, int64_t>;   // 14 digit after decimal point
using FixedPointI64_15 = FixedPoint<15, int64_t>;   // 15 digit after decimal point (double precision)

// 64-bit unsigned fixed-point types (18 446 744 073 709 551 615 maximum value)
using FixedPointU64_0 = FixedPoint<0, uint64_t>;     // 0 digit after decimal point (integer)
using FixedPointU64_1 = FixedPoint<1, uint64_t>;     // 1 digit after decimal point
using FixedPointU64_2 = FixedPoint<2, uint64_t>;     // 2 digit after decimal point
using FixedPointU64_3 = FixedPoint<3, uint64_t>;     // 3 digit after decimal point
using FixedPointU64_4 = FixedPoint<4, uint64_t>;     // 4 digit after decimal point
using FixedPointU64_5 = FixedPoint<5, uint64_t>;     // 5 digit after decimal point
using FixedPointU64_6 = FixedPoint<6, uint64_t>;     // 6 digit after decimal point
using FixedPointU64_7 = FixedPoint<7, uint64_t>;     // 7 digit after decimal point
using FixedPointU64_8 = FixedPoint<8, uint64_t>;     // 8 digit after decimal point (float precision)
using FixedPointU64_9 = FixedPoint<9, uint64_t>;     // 9 digit after decimal point
using FixedPointU64_10 = FixedPoint<10, uint64_t>;   // 10 digit after decimal point
using FixedPointU64_11 = FixedPoint<11, uint64_t>;   // 11 digit after decimal point
using FixedPointU64_12 = FixedPoint<12, uint64_t>;   // 12 digit after decimal point
using FixedPointU64_13 = FixedPoint<13, uint64_t>;   // 13 digit after decimal point
using FixedPointU64_14 = FixedPoint<14, uint64_t>;   // 14 digit after decimal point
using FixedPointU64_15 = FixedPoint<15, uint64_t>;   // 15 digit after decimal point (double precision)

// 32-bit signed fixed-point types (2 147 483 647 maximum value)
using FixedPointI32_0 = FixedPoint<0, int32_t>;     // 0 digit after decimal point (integer)
using FixedPointI32_1 = FixedPoint<1, int32_t>;     // 1 digit after decimal point
using FixedPointI32_2 = FixedPoint<2, int32_t>;     // 2 digit after decimal point
using FixedPointI32_3 = FixedPoint<3, int32_t>;     // 3 digit after decimal point
using FixedPointI32_4 = FixedPoint<4, int32_t>;     // 4 digit after decimal point
using FixedPointI32_5 = FixedPoint<5, int32_t>;     // 5 digit after decimal point
using FixedPointI32_6 = FixedPoint<6, int32_t>;     // 6 digit after decimal point
using FixedPointI32_7 = FixedPoint<7, int32_t>;     // 7 digit after decimal point
using FixedPointI32_8 = FixedPoint<8, int32_t>;     // 8 digit after decimal point (float precision)

// 32-bit unsigned fixed-point types (4 294 967 295 maximum value)
using FixedPointU32_0 = FixedPoint<0, uint32_t>;     // 0 digit after decimal point (integer)
using FixedPointU32_1 = FixedPoint<1, uint32_t>;     // 1 digit after decimal point
using FixedPointU32_2 = FixedPoint<2, uint32_t>;     // 2 digit after decimal point
using FixedPointU32_3 = FixedPoint<3, uint32_t>;     // 3 digit after decimal point
using FixedPointU32_4 = FixedPoint<4, uint32_t>;     // 4 digit after decimal point
using FixedPointU32_5 = FixedPoint<5, uint32_t>;     // 5 digit after decimal point
using FixedPointU32_6 = FixedPoint<6, uint32_t>;     // 6 digit after decimal point
using FixedPointU32_7 = FixedPoint<7, uint32_t>;     // 7 digit after decimal point
using FixedPointU32_8 = FixedPoint<8, uint32_t>;     // 8 digit after decimal point (float precision)

// 16-bit signed fixed-point types (32 767 maximum value)
using FixedPointI16_0 = FixedPoint<0, int16_t>;     // 0 digit after decimal point (integer)
using FixedPointI16_1 = FixedPoint<1, int16_t>;     // 1 digit after decimal point
using FixedPointI16_2 = FixedPoint<2, int16_t>;     // 2 digit after decimal point
using FixedPointI16_3 = FixedPoint<3, int16_t>;     // 3 digit after decimal point

// 16-bit unsigned fixed-point types (65 535 maximum value)
using FixedPointU16_0 = FixedPoint<0, uint16_t>;     // 0 digit after decimal point (integer)
using FixedPointU16_1 = FixedPoint<1, uint16_t>;     // 1 digit after decimal point
using FixedPointU16_2 = FixedPoint<2, uint16_t>;     // 2 digit after decimal point
using FixedPointU16_3 = FixedPoint<3, uint16_t>;     // 3 digit after decimal point

#endif // FIXED_POINT_HPP
