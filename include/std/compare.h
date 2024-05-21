/*
 * compare.h
 *
 *  Created on: 5/3/24.
 *      Author: Cezar PP
 */

#pragma once

/// A type introduced as part of the standard library to represent the result of a three-way comparison, also known as the "spaceship operator" <=>.

namespace std {
    struct strong_ordering {
        // Underlying value representation
        int value;

        // Constants for the comparison results
        static const strong_ordering less;
        static const strong_ordering equal;
        static const strong_ordering greater;

        // Constructor
        constexpr strong_ordering(int v) : value(v) {}

        // Comparison operators
        friend constexpr bool operator==(strong_ordering a, strong_ordering b) {
            return a.value == b.value;
        }

        friend constexpr bool operator!=(strong_ordering a, strong_ordering b) {
            return a.value != b.value;
        }

        friend constexpr bool operator<(strong_ordering a, strong_ordering b) {
            return a.value < b.value;
        }

        friend constexpr bool operator<=(strong_ordering a, strong_ordering b) {
            return a.value <= b.value;
        }

        friend constexpr bool operator>(strong_ordering a, strong_ordering b) {
            return a.value > b.value;
        }

        friend constexpr bool operator>=(strong_ordering a, strong_ordering b) {
            return a.value >= b.value;
        }
    };

    // Initialization of constants
    constexpr strong_ordering strong_ordering::less{-1};
    constexpr strong_ordering strong_ordering::equal{0};
    constexpr strong_ordering strong_ordering::greater{1};
}