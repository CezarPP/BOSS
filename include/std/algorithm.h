/*
 * algorithms.h
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#pragma once

#include "compare.h"
#include "utility.h"

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

    // equal(1)
    template<class InputIt1, class InputIt2>
    constexpr bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
        for (; first1 != last1; ++first1, ++first2)
            if (*first1 != *first2)
                return false;

        return true;
    }

    // equal(3)
    template<class InputIt1, class InputIt2, class BinaryPred>
    constexpr bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2, BinaryPred p) {
        for (; first1 != last1; ++first1, ++first2)
            if (!p(*first1, *first2))
                return false;

        return true;
    }

    // lexicographical_compare_three_way (1)
    template<class I1, class I2, class Cmp>
    constexpr auto lexicographical_compare_three_way(I1 f1, I1 l1, I2 f2, I2 l2, Cmp comp) -> decltype(comp(*f1, *f2)) {
        bool exhaust1 = (f1 == l1);
        bool exhaust2 = (f2 == l2);
        for (; !exhaust1 && !exhaust2; exhaust1 = (++f1 == l1), exhaust2 = (++f2 == l2))
            if (auto c = comp(*f1, *f2); c != 0)
                return c;

        return !exhaust1 ? std::strong_ordering::greater :
               !exhaust2 ? std::strong_ordering::less :
               std::strong_ordering::equal;
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

    // Version 3 / 5

    template<typename Iterator, typename UnaryPredicate>
    constexpr bool any_of(Iterator first, Iterator last, UnaryPredicate predicate) {
        for (; first != last; ++first) {
            if (predicate(*first)) {
                return true;
            }
        }
        return false;
    }

    template<typename ForwardIterator, typename T>
    constexpr void fill(ForwardIterator first, ForwardIterator last, const T &value) {
        for (; first != last; ++first) {
            *first = value;
        }
    }

    // find (1)
    template<class InputIt, class T>
    constexpr InputIt find(InputIt first, InputIt last, const T &value) {
        for (; first != last; ++first)
            if (*first == value)
                return first;

        return last;
    }

    // remove (1)
    template<class ForwardIt, class T>
    ForwardIt remove(ForwardIt first, ForwardIt last, const T &value) {
        first = std::find(first, last, value);
        if (first != last)
            for (ForwardIt i = first; ++i != last;)
                if (*i != value)
                    *first++ = std::move(*i);
        return first;
    }

    // copy (1)
    template<typename InputIt, typename OutputIt>
    constexpr OutputIt copy(InputIt first, InputIt last, OutputIt destination) {
        while (first != last) {
            *destination = *first;
            ++destination;
            ++first;
        }
        return destination;
    }


    // copy_n (1)
    template<typename InputIt, typename Size, typename OutputIt>
    constexpr OutputIt copy_n(InputIt first, Size count, OutputIt result) {
        while (count > 0) {
            *result = *first;
            ++result;
            ++first;
            --count;
        }
        return result;
    }
}