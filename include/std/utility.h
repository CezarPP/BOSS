/*
 * utility.h
 *
 *  Created on: 4/9/24.
 *      Author: Cezar PP
 */

#pragma once


#include "type_traits.h"

namespace std {
    template<typename T1, typename T2>
    class pair {
    public:
        T1 first;
        T2 second;

        /// Default constructor
        constexpr pair() : first(T1()), second(T2()) {}

        /// Initialized constructor
        constexpr pair(const T1 &a, const T2 &b) : first(a), second(b) {}

        /// Copy constructor (optional, since the default copy constructor works fine for most cases)
        constexpr pair(const pair<T1, T2> &other) : first(other.first), second(other.second) {}

        /// Assignment operator (optional, since the default assignment operator works fine for most cases)
        constexpr pair &operator=(const pair<T1, T2> &other) {
            if (this != &other) { // self-assignment check
                this->first = other.first;
                this->second = other.second;
            }
            return *this;
        }
    };

    /// std::move is basically a cast to Rvalue Reference
    template<typename T>
    constexpr typename remove_reference<T>::type &&move(T &&t) {
        return static_cast<typename remove_reference<T>::type &&>(t);
    }

    template<typename T>
    constexpr T &&forward(typename remove_reference<T>::type &t) {
        return static_cast<T &&>(t);
    }

    template<typename T>
    constexpr T &&forward(typename remove_reference<T>::type &&t) {
        return static_cast<T &&>(t);
    }

    /**
     * Swaps 2 values using std::move
     * noexcept specification not necessary, since the kernel doesn't use exceptions
     * @tparam T
     * @param a
     * @param b
     */
    template<typename T>
    requires (std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
    void swap(T &a, T &b) /*noexcept(std::is_nothrow_move_constructible<T>::value &&
                                   std::is_nothrow_move_assignable<T>::value) */ {
        T temp = std::move(a);
        a = std::move(b);
        b = std::move(temp);
    }

    template<typename T, T... Values>
    struct integer_sequence {
        static constexpr size_t size() noexcept {
            return sizeof...(Values);
        }
    };

    template <size_t... Values>
    using index_sequence = integer_sequence<size_t, Values...>;

    template <typename, size_t, bool>
    struct sequence_concat_impl;

    template <typename T, T... I, size_t N>
    struct sequence_concat_impl<integer_sequence<T, I...>, N, false> {
        using type = integer_sequence<T, I..., (N + I)...>;
    };

    template <typename T, T... I, size_t N>
    struct sequence_concat_impl<integer_sequence<T, I...>, N, true> {
        using type = integer_sequence<T, I..., (N + I)..., 2 * N>;
    };

    // The 0 and 1 cannot be deduced directly, must use SFINAE
    // Base type for generating a sequence
    template <typename T, T N, typename Enable = void>
    struct make_integer_sequence_impl;

    // The general case construct a list by concatenating
    template <typename T, T N, typename Enable>
    struct make_integer_sequence_impl {
        using type = typename sequence_concat_impl<typename make_integer_sequence_impl<T, N / 2>::type, N / 2, N % 2 == 1>::type;
    };

    // Specialization for empty sequence
    template <typename T, T N>
    struct make_integer_sequence_impl<T, N, std::enable_if_t<(N == 0)>> {
        using type = integer_sequence<T>;
    };

    // Specialization for sequence of length one
    template <typename T, T N>
    struct make_integer_sequence_impl<T, N, std::enable_if_t<(N == 1)>> {
        using type = integer_sequence<T, 0>;
    };

    template <typename T, T N>
    using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;

    template <size_t N>
    using make_index_sequence = make_integer_sequence<size_t, N>;
}