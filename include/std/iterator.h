/*
 * iterator.h
 *
 *  Created on: 5/4/24.
 *      Author: Cezar PP
 */


#pragma once

#include "cstddef.h"
#include "type_traits.h"

namespace std {
    template<typename Iterator>
    size_t distance(Iterator it, Iterator end) {
        // Can be improved upon
        return end - it;
    }

    template<typename Iterator>
    struct reverse_iterator {
        using iterator_type = Iterator;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;

        reverse_iterator(Iterator it)
                : it(it) {
        }

        reference operator*() {
            return *it;
        }

        reverse_iterator &operator++() {
            --it;
            return *this;
        }

        reverse_iterator &operator--() {
            ++it;
            return *this;
        }

        bool operator==(const reverse_iterator &rhs) {
            return it == rhs.it;
        }

        bool operator!=(const reverse_iterator &rhs) {
            return it != rhs.it;
        }

    private:
        iterator_type it;
    };
}