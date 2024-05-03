/*
 * compare.h
 *
 *  Created on: 5/3/24.
 *      Author: Cezar PP
 */

#pragma once

/// A type introduced as part of the standard library to represent the result of a three-way comparison, also known as the "spaceship operator" <=>.

namespace std {
    class strong_ordering {
    public:
        // Constants representing the order
        static const strong_ordering less;
        static const strong_ordering equal;
        static const strong_ordering greater;

        // Compare function to simulate the spaceship operator
        template<typename T>
        static strong_ordering compare(const T &left, const T &right) {
            if (left < right) return less;
            if (left > right) return greater;
            return equal;
        }

        // Operators to allow natural use of this class in conditions
        bool operator==(strong_ordering other) const { return value == other.value; }

        bool operator!=(strong_ordering other) const { return value != other.value; }

        bool operator<(strong_ordering other) const { return value < other.value; }

        bool operator<=(strong_ordering other) const { return value <= other.value; }

        bool operator>(strong_ordering other) const { return value > other.value; }

        bool operator>=(strong_ordering other) const { return value >= other.value; }

    private:
        int value; // Internal state where -1 is less, 0 is equal, 1 is greater

        // Private constructor to control the creation of instances
        constexpr strong_ordering(int val) : value(val) {}
    };

    // Definitions of constants
    // const strong_ordering strong_ordering::less(-1);
    // const strong_ordering strong_ordering::equal(0);
    // const strong_ordering strong_ordering::greater(1);

}