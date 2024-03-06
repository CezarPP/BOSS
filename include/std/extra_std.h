#pragma once

template<class T, T v>
struct IntegralConstant {
    static constexpr T value = v;
    using ValueType = T;
    using Type = IntegralConstant;

    constexpr operator ValueType() const noexcept { return value; }

    [[nodiscard]] constexpr ValueType operator()() const noexcept { return value; }
};

using FalseType = IntegralConstant<bool, false>;
using TrueType = IntegralConstant<bool, true>;

template<typename T, unsigned ExpectedSize, unsigned ActualSize>
struct AssertSizeImpl : TrueType {
    static_assert(ActualSize == ExpectedSize,
                  "actual size does not match expected size");

    consteval explicit operator bool() const { return value; }
};

// Note: This type is useful, as the sizes will be visible in the
//       compiler error messages, as they will be part of the
//       template parameters. This is not possible with a
//       static_assert on the sizeof a type.
template<typename T, unsigned ExpectedSize>
using AssertSize = AssertSizeImpl<T, ExpectedSize, sizeof(T)>;

