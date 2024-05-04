/*
 * iterator.h
 *
 *  Created on: 5/4/24.
 *      Author: Cezar PP
 */


#pragma once

#include "cstddef.h"

namespace std {
    template<typename Iterator>
    size_t distance(Iterator it, Iterator end) {
        // Can be improved upon
        return end - it;
    }
}