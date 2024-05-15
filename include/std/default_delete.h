/*
 * default_deleter.h
 *
 *  Created on: 5/15/24.
 *      Author: Cezar PP
 */

#pragma once

namespace std {
    // Normally defined in <memory>, but we can't include it there since our new and delete operators are custom
    template<typename T>
    struct default_delete {
        constexpr default_delete() = default;

        constexpr default_delete(const default_delete &) = default;

        void operator()(T *ptr) const {
            static_assert(sizeof(T) != 0, "Type must be complete");
            delete ptr;
        }
    };

    /// Partial specialization for arrays
    template<typename T>
    struct default_delete<T[]> {
        constexpr default_delete() = default;

        constexpr default_delete(const default_delete &) = default;

        void operator()(T *ptr) const {
            static_assert(sizeof(T) != 0, "Type must be complete");
            delete[] ptr;
        }
    };

}
