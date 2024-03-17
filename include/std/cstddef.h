/*
 * cstddef.h
 *
 *  Created on: 3/13/24.
 *      Author: Cezar PP
 */

#pragma once

namespace std {
    // ptrdiff_t is used for differences between pointers
    using ptrdiff_t = long long; // On x86_64, it's typically a 64-bit signed integer

    // size_t is used for sizes of objects
    using size_t = unsigned long long; // On x86_64, it's typically a 64-bit unsigned integer

    // max_align_t is a type whose alignment requirement is at least as strict as that of every scalar type
    struct max_align_t {
        long long max_align_ll;
        long double max_align_ld;
    };

    // nullptr_t is the type of nullptr
    using nullptr_t = decltype(nullptr);

    // Define an enum class byte for byte manipulation
    enum class byte : unsigned char {
    };

    // Implementations for byte type operations
    template<class IntType>
    constexpr byte &operator<<=(byte &b, IntType shift) noexcept {
        return b = b << shift;
    }

    template<class IntType>
    constexpr byte operator<<(byte b, IntType shift) noexcept {
        return static_cast<byte>(static_cast<unsigned char>(b) << shift);
    }

    template<class IntType>
    constexpr byte &operator>>=(byte &b, IntType shift) noexcept {
        return b = b >> shift;
    }

    template<class IntType>
    constexpr byte operator>>(byte b, IntType shift) noexcept {
        return static_cast<byte>(static_cast<unsigned char>(b) >> shift);
    }

    constexpr byte &operator|=(byte &l, byte r) noexcept {
        return l = static_cast<byte>(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
    }

    constexpr byte operator|(byte l, byte r) noexcept {
        return static_cast<byte>(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
    }

    constexpr byte &operator&=(byte &l, byte r) noexcept {
        return l = static_cast<byte>(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
    }

    constexpr byte operator&(byte l, byte r) noexcept {
        return static_cast<byte>(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
    }

    constexpr byte &operator^=(byte &l, byte r) noexcept {
        return l = static_cast<byte>(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
    }

    constexpr byte operator^(byte l, byte r) noexcept {
        return static_cast<byte>(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
    }

    constexpr byte operator~(byte b) noexcept {
        return static_cast<byte>(~static_cast<unsigned char>(b));
    }

    template<class IntType>
    constexpr IntType to_integer(byte b) noexcept {
        return static_cast<IntType>(b);
    }
}

#ifndef NULL
#define NULL 0
#endif

// offsetof is a macro that gives the offset of a field in a struct.
#define offsetof(P, D) __builtin_offsetof(P, D)
