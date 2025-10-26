#ifndef FIXED_POINT_HPP
#define FIXED_POINT_HPP

#include <cmath>
#include <type_traits>
#include <iostream>
#include <sstream>

/**
 * @brief Template class for fixed-point arithmetic
 * @tparam DecimalPlaces Number of decimal places after the decimal point (e.g., 2 = 2 digits, 4 = 4 digits)
 * @note DecimalPlaces must be between 0 and 15 to limit overflow risks
 * @note This class may be upgraded in the future to support more features and higher precision/number (support of infinite number, instead of int64_t)
 */
template<int DecimalPlaces>
class FixedPoint {
    static_assert(DecimalPlaces >= 0, "DecimalPlaces must be non-negative");
    static_assert(DecimalPlaces <= 15, "DecimalPlaces must be less than or equal to 15 to limit overflow");
private:
    int64_t value;

    // ============================================================================
    // Helper function
    // ============================================================================
    static constexpr int64_t pow10(int places) {
        int64_t scale = 1;
        for (int i = 0; i < places; ++i) {
            scale *= 10;
        }
        return scale;
    }

    template<int From, int To>
    static inline int64_t rescaleRaw(int64_t v) {
        if constexpr (From == To) return v;
        else if constexpr (From < To) {
            return v * pow10(To - From);
        } else {
            return v / pow10(From - To);
        }
    }

    static constexpr int64_t SCALE = pow10(DecimalPlaces);

public:
    // ============================================================================
    // Constructors
    // ============================================================================
    FixedPoint() noexcept : value(0) {}

    FixedPoint(int val) {
        value = int64_t(val) * SCALE;
    }

    FixedPoint(float val) {
        value = int64_t(val * SCALE);
    }

    FixedPoint(double val) {
        value = int64_t(val * SCALE);
    }

    FixedPoint(const FixedPoint&) noexcept = default;

    template<int Other>
    explicit FixedPoint(const FixedPoint<Other>& other) {
        value = rescaleRaw<Other, DecimalPlaces>(other.getRawValue());
    }

    // ============================================================================
    // Constructor from raw value
    // ============================================================================
    static FixedPoint fromRaw(int64_t rawValue) {
        FixedPoint fp;
        fp.value = rawValue;
        return fp;
    }

    // ============================================================================
    // fromString
    // ============================================================================
    static FixedPoint fromString(std::string s) {
        // , and . normalization
        for (auto& c: s) if (c == ',') c = '.';

        bool neg = false;
        size_t i = 0;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) { neg = (s[i] == '-'); ++i; }
        if (i >= s.size()) throw std::runtime_error("fromString: empty after sign");

        int64_t intPart = 0;
        bool hasInt = false;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) {
            hasInt = true;
            int digit = s[i]-'0';
            if (intPart > std::numeric_limits<int64_t>::max() / 10) throw std::overflow_error("fromString: int overflow");
            intPart = intPart * 10 + digit;
            ++i;
        }

        int64_t fracPart = 0;
        int fracLen = 0;
        if (i < s.size() && s[i] == '.') {
            ++i;
            while (i < s.size() && std::isdigit((unsigned char)s[i]) && fracLen < DecimalPlaces) {
                fracPart = fracPart * 10 + (s[i]-'0');
                ++fracLen;
                ++i;
            }
            // ignorer les chiffres au-delà → TRONCATURE (pas d'arrondi)
            while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
        }

        if (!hasInt && fracLen == 0) throw std::runtime_error("fromString: no digits");
        if (i != s.size()) throw std::runtime_error("fromString: trailing chars");

        // Scale
        for (int k = fracLen; k < DecimalPlaces; ++k) fracPart *= 10;

        // compose
        int64_t scale = pow10(DecimalPlaces);
        if (intPart > 0 && intPart > std::numeric_limits<int64_t>::max() / scale)
            throw std::overflow_error("fromString: scaled overflow");
        int64_t raw = intPart * scale + fracPart;
        if (neg) raw = -raw;
        return fromRaw(raw);
    }

    // ============================================================================
    // Assignment operators
    // ============================================================================
    FixedPoint& operator=(const FixedPoint&) noexcept = default;

    FixedPoint& operator=(int val) {
        value = int64_t(val) * SCALE;
        return *this;
    }

    FixedPoint& operator=(float val) {
        value = int64_t(val * SCALE);
        return *this;
    }

    FixedPoint& operator=(double val) {
        value = int64_t(val * SCALE);
        return *this;
    }

    // ============================================================================
    // Arithmetic operators
    // ============================================================================
    FixedPoint operator+(const FixedPoint& other) const {
        return FixedPoint::fromRaw(value + other.value);
    }

    FixedPoint operator-(const FixedPoint& other) const {
        return FixedPoint::fromRaw(value - other.value);
    }

    FixedPoint operator*(const FixedPoint& other) const {
        return FixedPoint::fromRaw((value * other.value) / SCALE);
    }

    FixedPoint operator/(const FixedPoint& other) const {
        return FixedPoint::fromRaw((value * SCALE) / other.value);
    }

    // ============================================================================
    // Arithmetic operators between different precisions
    // ============================================================================
    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces)> operator+(const FixedPoint<OtherDecimalPlaces>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces>;

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return ResultType::fromRaw(thisAdjusted + otherAdjusted);
    }

    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces)> operator-(const FixedPoint<OtherDecimalPlaces>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces>;

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return ResultType::fromRaw(thisAdjusted - otherAdjusted);
    }

    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces)> operator*(const FixedPoint<OtherDecimalPlaces>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces>;

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        int64_t result = (thisAdjusted * otherAdjusted) / ResultType::getScale();

        return ResultType::fromRaw(result);
    }

    template<int OtherDecimalPlaces>
    FixedPoint<(DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces)> operator/(const FixedPoint<OtherDecimalPlaces>& other) const {
        constexpr int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces) ? DecimalPlaces : OtherDecimalPlaces;
        using ResultType = FixedPoint<MaxDecimalPlaces>;

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        int64_t result = (thisAdjusted * ResultType::getScale()) / otherAdjusted;

        return ResultType::fromRaw(result);
    }

    // ============================================================================
    // Arithmetic operators with scalars
    // ============================================================================
    FixedPoint operator+(int val) const {
        return FixedPoint::fromRaw(value + int64_t(val) * SCALE);
    }

    FixedPoint operator-(int val) const {
        return FixedPoint::fromRaw(value - int64_t(val) * SCALE);
    }

    FixedPoint operator*(int val) const {
        return FixedPoint::fromRaw(value * val);
    }

    FixedPoint operator/(int val) const {
        return FixedPoint::fromRaw(value / val);
    }

    FixedPoint operator+(float val) const {
        return FixedPoint::fromRaw(value + int64_t(val * SCALE));
    }

    FixedPoint operator-(float val) const {
        return FixedPoint::fromRaw(value - int64_t(val * SCALE));
    }

    FixedPoint operator*(float val) const {
        return FixedPoint::fromRaw(int64_t(value * val));
    }

    FixedPoint operator/(float val) const {
        return FixedPoint::fromRaw(int64_t(value / val));
    }

    FixedPoint operator+(double val) const {
        return FixedPoint::fromRaw(value + int64_t(val * SCALE));
    }

    FixedPoint operator-(double val) const {
        return FixedPoint::fromRaw(value - int64_t(val * SCALE));
    }

    FixedPoint operator*(double val) const {
        return FixedPoint::fromRaw(int64_t(value * val));
    }

    FixedPoint operator/(double val) const {
        return FixedPoint::fromRaw(int64_t(value / val));
    }

    // ============================================================================
    // Unary operators
    // ============================================================================
    FixedPoint operator-() const {
        return FixedPoint::fromRaw(-value);
    }

    FixedPoint operator+() const {
        return *this;
    }

    // ============================================================================
    // Composed assignment operators
    // ============================================================================
    FixedPoint& operator+=(const FixedPoint& other) {
        value += other.value;
        return *this;
    }

    FixedPoint& operator-=(const FixedPoint& other) {
        value -= other.value;
        return *this;
    }

    FixedPoint& operator*=(const FixedPoint& other) {
        value = (value * other.value) / SCALE;
        return *this;
    }

    FixedPoint& operator/=(const FixedPoint& other) {
        value = (value * SCALE) / other.value;
        return *this;
    }

    // ============================================================================
    // Composed assignment operators between different precisions
    // ============================================================================
    template<int OtherDecimalPlaces>
    FixedPoint& operator+=(const FixedPoint<OtherDecimalPlaces>& other) {
        *this = *this + other;
        return *this;
    }

    template<int OtherDecimalPlaces>
    FixedPoint& operator-=(const FixedPoint<OtherDecimalPlaces>& other) {
        *this = *this - other;
        return *this;
    }

    template<int OtherDecimalPlaces>
    FixedPoint& operator*=(const FixedPoint<OtherDecimalPlaces>& other) {
        *this = *this * other;
        return *this;
    }

    template<int OtherDecimalPlaces>
    FixedPoint& operator/=(const FixedPoint<OtherDecimalPlaces>& other) {
        *this = *this / other;
        return *this;
    }

    // ============================================================================
    // Composed assignment operators with scalars
    // ============================================================================
    FixedPoint& operator+=(int val) {
        value += int64_t(val) * SCALE;
        return *this;
    }

    FixedPoint& operator-=(int val) {
        value -= int64_t(val) * SCALE;
        return *this;
    }

    FixedPoint& operator*=(int val) {
        value *= val;
        return *this;
    }

    FixedPoint& operator/=(int val) {
        value /= val;
        return *this;
    }

    FixedPoint& operator+=(float val) {
        value += int64_t(val * SCALE);
        return *this;
    }

    FixedPoint& operator-=(float val) {
        value -= int64_t(val * SCALE);
        return *this;
    }

    FixedPoint& operator*=(float val) {
        value = int64_t(value * val);
        return *this;
    }

    FixedPoint& operator/=(float val) {
        value = int64_t(value / val);
        return *this;
    }

    FixedPoint& operator+=(double val) {
        value += int64_t(val * SCALE);
        return *this;
    }

    FixedPoint& operator-=(double val) {
        value -= int64_t(val * SCALE);
        return *this;
    }

    FixedPoint& operator*=(double val) {
        value = int64_t(value * val);
        return *this;
    }

    FixedPoint& operator/=(double val) {
        value = int64_t(value / val);
        return *this;
    }

    // ============================================================================
    // Comparison operators
    // ============================================================================
    bool operator==(const FixedPoint& other) const {
        return value == other.value;
    }

    bool operator!=(const FixedPoint& other) const {
        return value != other.value;
    }

    bool operator<(const FixedPoint& other) const {
        return value < other.value;
    }

    bool operator<=(const FixedPoint& other) const {
        return value <= other.value;
    }

    bool operator>(const FixedPoint& other) const {
        return value > other.value;
    }

    bool operator>=(const FixedPoint& other) const {
        return value >= other.value;
    }

    // ============================================================================
    // Comparison operators between different precisions
    // ============================================================================
    template<int OtherDecimalPlaces>
    bool operator==(const FixedPoint<OtherDecimalPlaces>& other) const {
        int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces);

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return thisAdjusted == otherAdjusted;
    }

    template<int OtherDecimalPlaces>
    bool operator!=(const FixedPoint<OtherDecimalPlaces>& other) const {
        return !(*this == other);
    }

    template<int OtherDecimalPlaces>
    bool operator<(const FixedPoint<OtherDecimalPlaces>& other) const {
        int MaxDecimalPlaces = (DecimalPlaces > OtherDecimalPlaces ? DecimalPlaces : OtherDecimalPlaces);

        int64_t thisAdjusted = rescaleRaw<DecimalPlaces, MaxDecimalPlaces>(value);
        int64_t otherAdjusted = rescaleRaw<OtherDecimalPlaces, MaxDecimalPlaces>(other.getRawValue());

        return thisAdjusted < otherAdjusted;
    }

    template<int OtherDecimalPlaces>
    bool operator<=(const FixedPoint<OtherDecimalPlaces>& other) const {
        return *this < other || *this == other;
    }

    template<int OtherDecimalPlaces>
    bool operator>(const FixedPoint<OtherDecimalPlaces>& other) const {
        return !(*this <= other);
    }

    template<int OtherDecimalPlaces>
    bool operator>=(const FixedPoint<OtherDecimalPlaces>& other) const {
        return !(*this < other);
    }

    // ============================================================================
    // Comparison operators with scalars
    // ============================================================================
    bool operator==(int val) const {
        return *this == FixedPoint(val);
    }

    bool operator!=(int val) const {
        return *this != FixedPoint(val);
    }

    bool operator<(int val) const {
        return *this < FixedPoint(val);
    }

    bool operator<=(int val) const {
        return *this <= FixedPoint(val);
    }

    bool operator>(int val) const {
        return *this > FixedPoint(val);
    }

    bool operator>=(int val) const {
        return *this >= FixedPoint(val);
    }

    bool operator==(float val) const {
        return *this == FixedPoint(val);
    }

    bool operator!=(float val) const {
        return *this != FixedPoint(val);
    }

    bool operator<(float val) const {
        return *this < FixedPoint(val);
    }

    bool operator<=(float val) const {
        return *this <= FixedPoint(val);
    }

    bool operator>(float val) const {
        return *this > FixedPoint(val);
    }

    bool operator>=(float val) const {
        return *this >= FixedPoint(val);
    }

    bool operator==(double val) const {
        return *this == FixedPoint(val);
    }

    bool operator!=(double val) const {
        return *this != FixedPoint(val);
    }

    bool operator<(double val) const {
        return *this < FixedPoint(val);
    }

    bool operator<=(double val) const {
        return *this <= FixedPoint(val);
    }

    bool operator>(double val) const {
        return *this > FixedPoint(val);
    }

    bool operator>=(double val) const {
        return *this >= FixedPoint(val);
    }

    // ============================================================================
    // Increment and decrement operators
    // ============================================================================
    FixedPoint& operator++() {
        value += SCALE;
        return *this;
    }

    FixedPoint operator++(int) {
        FixedPoint temp = *this;
        value += SCALE;
        return temp;
    }

    FixedPoint& operator--() {
        value -= SCALE;
        return *this;
    }

    FixedPoint operator--(int) {
        FixedPoint temp = *this;
        value -= SCALE;
        return temp;
    }

    // ============================================================================
    // Conversion functions
    // ============================================================================
    float toFloat() const {
        return float(value) / SCALE;
    }

    double toDouble() const {
        return double(value) / SCALE;
    }

    int toInt() const {
        return int(value / SCALE);
    }

    std::string toString() const {
        std::ostringstream os;
        int64_t raw = value;

        if constexpr (DecimalPlaces == 0) { // if no decimal places, print as integer
            os << (raw / SCALE);
            return os.str();
        }

        bool isNegative = raw < 0;
        uint64_t absoluteValue = isNegative ? uint64_t(-(raw + 1)) + 1 : uint64_t(raw);

        uint64_t integerPart = absoluteValue / SCALE;
        uint64_t fractionalPart = absoluteValue % SCALE;

        if (isNegative) {
            os << '-';
        }
        os << integerPart << '.';

        uint64_t padding = SCALE / 10;
        while (padding > 0) {
            os << char('0' + int((fractionalPart / padding) % 10));
            padding /= 10;
        }
        return os.str();
    }

    int64_t getRawValue() const {
        return value;
    }

    // ============================================================================
    // Explicit conversion operators
    // ============================================================================
    explicit operator float() const {
        return toFloat();
    }

    explicit operator double() const {
        return toDouble();
    }

    explicit operator int() const {
        return toInt();
    }

    explicit operator std::string() const {
        return toString();
    }

    // ============================================================================
    // Mathematical functions
    // ============================================================================
    FixedPoint abs() const {
        return FixedPoint::fromRaw(value < 0 ? -value : value);
    }

    FixedPoint floor() const {
        if (value >= 0) return fromRaw((value / SCALE) * SCALE);
        return fromRaw(value % SCALE == 0 ? value : ((value / SCALE) - 1) * SCALE);
    }

    FixedPoint ceil() const {
        if (value <= 0) return fromRaw((value / SCALE) * SCALE);
        return fromRaw(value % SCALE == 0 ? value : ((value / SCALE) + 1) * SCALE);
    }

    FixedPoint round() const {
        const int64_t half = SCALE / 2;
        const int64_t adj  = (value >= 0 ? half : -half);
        return fromRaw(((value + adj) / SCALE) * SCALE);
    }

    FixedPoint frac() const {
        if constexpr (DecimalPlaces == 0) return zero();
        int64_t fractionalPart = value % SCALE;
        if (fractionalPart < 0) fractionalPart += SCALE;
        return FixedPoint::fromRaw(fractionalPart);
    }

    // ============================================================================
    // Display operator
    // ============================================================================
    friend std::ostream& operator<<(std::ostream& os, const FixedPoint& fp) {
        os << fp.toString();
        return os;
    }

    // ============================================================================
    // Useful constants
    // ============================================================================
    static constexpr FixedPoint zero() noexcept {
        return FixedPoint::fromRaw(0);
    }

    static constexpr FixedPoint one() noexcept {
        return FixedPoint::fromRaw(SCALE);
    }

    // ============================================================================
    // Type information
    // ============================================================================
    static constexpr int getDecimalPlaces() noexcept {
        return DecimalPlaces;
    }

    static constexpr int64_t getScale() noexcept {
        return SCALE;
    }
};

// ============================================================================
// External operators for commutativity
// ============================================================================
template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator+(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) + rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator-(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) - rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator*(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) * rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator/(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) / rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator+(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) + rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator-(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) - rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator*(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) * rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator/(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) / rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator+(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) + rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator-(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) - rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator*(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) * rhs;
}

template<int DecimalPlaces>
FixedPoint<DecimalPlaces> operator/(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) / rhs;
}

// ============================================================================
// External comparison operators for commutativity
// ============================================================================
template<int DecimalPlaces>
bool operator==(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) == rhs;
}

template<int DecimalPlaces>
bool operator!=(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) != rhs;
}

template<int DecimalPlaces>
bool operator<(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) < rhs;
}

template<int DecimalPlaces>
bool operator<=(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) <= rhs;
}

template<int DecimalPlaces>
bool operator>(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) > rhs;
}

template<int DecimalPlaces>
bool operator>=(int lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) >= rhs;
}

template<int DecimalPlaces>
bool operator==(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) == rhs;
}

template<int DecimalPlaces>
bool operator!=(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) != rhs;
}

template<int DecimalPlaces>
bool operator<(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) < rhs;
}

template<int DecimalPlaces>
bool operator<=(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) <= rhs;
}

template<int DecimalPlaces>
bool operator>(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) > rhs;
}

template<int DecimalPlaces>
bool operator>=(float lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) >= rhs;
}

template<int DecimalPlaces>
bool operator==(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) == rhs;
}

template<int DecimalPlaces>
bool operator!=(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) != rhs;
}

template<int DecimalPlaces>
bool operator<(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) < rhs;
}

template<int DecimalPlaces>
bool operator<=(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) <= rhs;
}

template<int DecimalPlaces>
bool operator>(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) > rhs;
}

template<int DecimalPlaces>
bool operator>=(double lhs, const FixedPoint<DecimalPlaces>& rhs) {
    return FixedPoint<DecimalPlaces>(lhs) >= rhs;
}

// ============================================================================
// Alias of common types for easier use
// ============================================================================
using FixedPoint0 = FixedPoint<0>;     // 0 digit after decimal point (integer)
using FixedPoint1 = FixedPoint<1>;     // 1 digit after decimal point
using FixedPoint2 = FixedPoint<2>;     // 2 digit after decimal point
using FixedPoint3 = FixedPoint<3>;     // 3 digit after decimal point
using FixedPoint4 = FixedPoint<4>;     // 4 digit after decimal point
using FixedPoint5 = FixedPoint<5>;     // 5 digit after decimal point
using FixedPoint6 = FixedPoint<6>;     // 6 digit after decimal point
using FixedPoint7 = FixedPoint<7>;     // 7 digit after decimal point
using FixedPoint8 = FixedPoint<8>;     // 8 digit after decimal point (float precision)

#endif // FIXED_POINT_HPP
