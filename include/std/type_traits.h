/*
 * type_traits.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */


#include "utility.h"

namespace std {
    /* enable_if & disable_if */
    /**
     * If B is true, std::enable_if has a public member typedef type, equal to T; otherwise, there is no member typedef.
     * @tparam B
     * @tparam T
     */
    template<bool B, class T = void>
    struct enable_if {
    };

    template<class T>
    struct enable_if<true, T> {
        typedef T type;
    };

    template<bool B, class T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    template<bool B, class T = void>
    struct disable_if {
    };

    template<class T>
    struct disable_if<false, T> {
        typedef T type;
    };

    template<bool B, class T = void>
    using disable_if_t = typename disable_if<B, T>::type;

    /* remove_reference */
    /**
     * If the type T is a reference type, provides the member typedef type which is the type referred to by T.
     * Otherwise type is T
     * @tparam T
     */
    template<class T>
    struct remove_reference {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T &> {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T &&> {
        typedef T type;
    };

    template<class T>
    using remove_reference_t = typename remove_reference<T>::type;


    /**
     * Wraps a static constant of specified type. It is the base class for the C++ type traits.
     * @tparam T Type of constant
     * @tparam V Value of constant
     */
    template<typename T, T V>
    struct integral_constant {
        static constexpr T value = V;

        using value_type = T;
        using type = integral_constant;

        constexpr operator value_type() const noexcept {
            return value;
        }

        constexpr value_type operator()() const noexcept {
            return value;
        }
    };

    template<bool B>
    using bool_constant = integral_constant<bool, B>;

    /// Instantiation of integral_constant to represent the bool value true
    using true_type = integral_constant<bool, true>;
    /// Instantiation of integral_constant to represent the bool value false
    using false_type = integral_constant<bool, false>;

    /* declval and add_rvalue_reference */
    namespace detail {
        template<class T>
        struct type_identity {
            using type = T;
        }; // or use std::type_identity (since C++20)

        template<class T>
        // Note that “cv void&” is a substitution failure
        auto try_add_lvalue_reference(int) -> type_identity<T &>;

        template<class T>
        // Handle T = cv void case
        auto try_add_lvalue_reference(...) -> type_identity<T>;

        template<class T>
        auto try_add_rvalue_reference(int) -> type_identity<T &&>;

        template<class T>
        auto try_add_rvalue_reference(...) -> type_identity<T>;
    } // namespace detail

    template<class T>
    struct add_lvalue_reference
            : decltype(detail::try_add_lvalue_reference<T>(0)) {
    };

    template<class T>
    struct add_rvalue_reference
            : decltype(detail::try_add_rvalue_reference<T>(0)) {
    };


    template<typename T>
    typename std::add_rvalue_reference<T>::type declval() noexcept {
        static_assert(false, "declval not allowed in an evaluated context");
    }

    /* void_t */
    template<class...>
    using void_t = void;

    /* is_move_constructible */
    template<typename T, typename = void>
    struct is_move_constructible : std::false_type {
    };

    template<typename T>
    struct is_move_constructible<T, std::void_t<decltype(T(std::declval<T &&>()))>> : std::true_type {
    };

    template<typename T>
    inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

    /* is_move_assignable */
    template<typename T, typename = void>
    struct is_move_assignable : std::false_type {
    };

    template<typename T>
    struct is_move_assignable<T, std::void_t<decltype(std::declval<T &>() = std::declval<T &&>())>> : std::true_type {
    };

    template<typename T>
    inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

    /* conditional */
    template<bool B, class T, class F>
    struct conditional {
        using type = T;
    };

    template<class T, class F>
    struct conditional<false, T, F> {
        using type = F;
    };

    template<bool B, class T, class F>
    using conditional_t = typename conditional<B, T, F>::type;


    /* is_destructible */
    template<typename T, typename = void>
    struct is_destructible : std::false_type {
    };

    template<typename T>
    struct is_destructible<T, std::void_t<decltype(std::declval<T &>().~T())>> : std::true_type {
    };

    template<typename T>
    struct is_trivially_destructible : std::bool_constant<__has_trivial_destructor(T)> {
    };

    template<typename T>
    struct is_nothrow_destructible : std::bool_constant<noexcept(std::declval<T &>().~T())> {
    };

    template<class T>
    inline constexpr bool is_destructible_v = is_destructible<T>::value;

    template<class T>
    inline constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;

    template<class T>
    inline constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;
}