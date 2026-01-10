#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <compare>
#include <type_traits>

template <typename size> class FixedPoint {
  private:
    constexpr FixedPoint(size raw, bool dont_scale)
        : value(raw) {}

  public:
    size value;

    static constexpr int FRACTION_BITS = 16;
    static constexpr int SCALE = 1 << FRACTION_BITS;

    // Constructors
    constexpr FixedPoint()
        : value(0) {}

    template <typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
    explicit constexpr FixedPoint(Int i)
        : value(static_cast<size>(i) * SCALE) {}

    explicit constexpr FixedPoint(double f)
        : value(static_cast<size>(f * SCALE)) {}

    static constexpr FixedPoint<size> from_raw(size raw) { return FixedPoint<size>(raw, true); }

    // Converters
    explicit constexpr operator double() const { return value / static_cast<double>(SCALE); }
    explicit constexpr operator int() const { return value >> FRACTION_BITS; }
    explicit constexpr operator unsigned int() const { return value >> FRACTION_BITS; }
    explicit constexpr operator short() const { return value >> FRACTION_BITS; }

    // Arithmatic
    constexpr FixedPoint<size> operator+(const FixedPoint<size> b) {
        return FixedPoint<size>(value + b.value, true);
    }

    constexpr FixedPoint<size> operator-(const FixedPoint<size> b) {
        return FixedPoint<size>(value - b.value, true);
    }

    constexpr FixedPoint<size> operator/(double b) {
        return FixedPoint<size>((size)(value / b), true);
    }

    constexpr FixedPoint<size> operator%(const FixedPoint<size> b) {
        return FixedPoint<size>(value % b.value, true);
    }

    // Other
    constexpr auto operator<=>(const FixedPoint& b) const = default;
};

template <typename size, typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
constexpr FixedPoint<size> operator*(const FixedPoint<size>& a, Int b) {
    return FixedPoint<size>::from_raw(a.value * b);
}

template <typename size, typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
constexpr FixedPoint<size> operator*(Int b, const FixedPoint<size>& a) {
    return FixedPoint<size>::from_raw(a.value * b);
}

#endif
