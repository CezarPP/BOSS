/*
 * memory.h
 *
 *  Created on: 4/13/24.
 *      Author: Cezar PP
 */


#pragma once

#include "new_custom.h"


namespace std {
    /*
    * The std::allocator class template is the default Allocator used by all standard library containers if no user-specified allocator is provided.
    * The default allocator is stateless, that is, all instances of the given allocator are interchangeable,
    * compare equal and can deallocate memory allocated by any other instance of the same allocator type.
    */
    template<typename T>
    class allocator {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        allocator() noexcept = default;  // Default constructor
        allocator(const allocator &) noexcept = default;  // Copy constructor

        ~allocator() = default;  // Destructor

        [[nodiscard]] T *allocate(size_type n) {
            // Allocate memory for n objects of type T
            return static_cast<T *>(::operator new(n * sizeof(T)));
        }

        void deallocate(T *p, size_type) noexcept {
            // Deallocate memory pointed by p, which was allocated for n objects of type T
            ::operator delete(p);
        }
    };

    template<class T1, class T2>
    constexpr bool operator==(const allocator<T1> &lhs, const allocator<T2> &rhs) noexcept {
        // All instances of allocator are interchangeable, hence always equal
        return true;
    }

    template<typename T>
    constexpr T *addressof(T &arg) noexcept {
        return reinterpret_cast<T *>(
                &const_cast<char &>(
                        reinterpret_cast<const volatile char &>(arg)
                )
        );
    }

    template<class T>
    const T *addressof(const T &&) = delete;
}