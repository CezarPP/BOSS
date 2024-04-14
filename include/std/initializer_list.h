/*
 * initializer_list.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */


#pragma once

namespace std {
    typedef uint64_t size_t;

    /*
     * std::initializer_list was introduced in C++11 as a standardized way to initialize objects with a list of values
     *
     * To make std::initializer_list work,
                C++ 11 introduced another concept named as “Uniform initialization” a.k.a “Braced initialization { }”
     * Braced initialization is the most widely usable initialization syntax,
                        it prevents narrowing conversions, and it’s immune to C++’s most vexing parse.
     */

    /*!
     * std::initializer_list<T> is a lightweight proxy object that provides access to an array of objects of type const T

     * @tparam T The type of the elements
     */
    template<typename T>
    class initializer_list {
    public:
        using value_type = T;
        using reference = const T &;
        using const_reference = const T &;
        using size_type = size_t;
        using iterator = const T *;
        using const_iterator = const T *;

    private:
        iterator begin_;
        size_type size_;

    public:
        constexpr initializer_list() : begin_(nullptr), size_(0) {}

        constexpr initializer_list(const T *begin, size_t size) : begin_(begin), size_(size) {}

        [[nodiscard]] size_t size() const noexcept {
            return size_;
        }

        constexpr const T *begin() const noexcept {
            return begin_;
        }

        constexpr const T *end() const noexcept {
            return begin_ + size_;
        }
    };


    template<typename T>
    inline constexpr const T *begin(initializer_list<T> list) noexcept {
        return list.begin();
    }

    template<typename T>
    inline constexpr const T *end(initializer_list<T> list) noexcept {
        return list.end();
    }
}