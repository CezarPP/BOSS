/*
 * algorithms.h
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#pragma once

namespace std {
    // Version (1): Using the default less-than comparison
    template<class T>
    constexpr const T &min(const T &a, const T &b) {
        return (b < a) ? b : a;
    }

    // Version (2): Allowing a custom comparison function
    template<class T, class Compare>
    constexpr const T &min(const T &a, const T &b, Compare comp) {
        return comp(b, a) ? b : a;
    }

    // Version (1): Using the default greater-than comparison
    template<class T>
    constexpr const T &max(const T &a, const T &b) {
        return (a > b) ? a : b;
    }

    // Version (2): Allowing a custom comparison function
    template<class T, class Compare>
    constexpr const T &max(const T &a, const T &b, Compare comp) {
        return comp(a, b) ? a : b;
    }

    // Version 1 / 5
    template<typename Iterator, typename UnaryPredicate>
    constexpr bool all_of(Iterator first, Iterator last, UnaryPredicate pred) {
        for (; first != last; ++first) {
            if (!pred(*first)) {
                return false;
            }
        }
        return true;
    }

    template<typename ForwardIterator, typename T>
    constexpr void fill(ForwardIterator first, ForwardIterator last, const T &value) {
        for (; first != last; ++first) {
            *first = value;
        }
    }
}