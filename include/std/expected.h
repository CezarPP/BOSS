/*
 * expected.h
 *
 *  Created on: 5/4/24.
 *      Author: Cezar PP
 */

#pragma once

#include "new.h"
#include "utility.h"

namespace std {
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

    constexpr struct only_set_valid_t {
    } only_set_valid;

    template<typename E>
    struct exceptional {
        using error_type = E;

        error_type error;

        exceptional()
                : error() {
        }

        explicit exceptional(error_type e)
                : error(e) {
        }
    };

    template<typename T, typename E>
    union trivial_expected_storage {
        using value_type = T;
        using error_type = E;

        error_type error;
        value_type value;

        constexpr trivial_expected_storage()
                : value() {
        }

        constexpr trivial_expected_storage(const exceptional<error_type> &e)
                : error(e.error) {
        }

        template<typename... Args>
        constexpr trivial_expected_storage(Args &&... args)
                : value(std::forward<Args>(args)...) {
        }

        ~trivial_expected_storage() = default;
    };

    template<typename E>
    union trivial_expected_storage<void, E> {
        using value_type = void;
        using error_type = E;

        error_type error;

        constexpr trivial_expected_storage() {
        }

        constexpr trivial_expected_storage(const exceptional<error_type> &e)
                : error(e.error) {
        }

        ~trivial_expected_storage() = default;
    };

    template<typename T, typename E>
    struct non_trivial_expected_storage {
        using value_type = T;
        using error_type = E;

        error_type error;
        value_type value;

        constexpr non_trivial_expected_storage()
                : value() {
        }

        constexpr non_trivial_expected_storage(const exceptional<error_type> &e)
                : error(e.error) {
        }

        template<typename... Args>
        constexpr non_trivial_expected_storage(Args &&... args)
                : value(std::forward<Args>(args)...) {
        }

        ~non_trivial_expected_storage() = default;
    };

    template<typename E>
    struct non_trivial_expected_storage<void, E> {
        using value_type = void;
        using error_type = E;

        error_type error;

        constexpr non_trivial_expected_storage() {
        }

        constexpr non_trivial_expected_storage(const exceptional<error_type> &e)
                : error(e.error) {
        }

        ~non_trivial_expected_storage() = default;
    };

    template<typename T, typename E>
    struct trivial_expected_base {
        using value_type = T;
        using error_type = E;

        bool has_value;
        trivial_expected_storage<T, E> storage;

        trivial_expected_base()
                : has_value(true), storage() {
        }

        trivial_expected_base(only_set_valid_t, bool hv)
                : has_value(hv) {
        }

        trivial_expected_base(const value_type &v)
                : has_value(true), storage(v) {
        }

        trivial_expected_base(value_type &&v)
                : has_value(true), storage(std::forward<value_type>(v)) {
        }

        trivial_expected_base(const exceptional<error_type> &e)
                : has_value(false), storage(e) {
        }

        ~trivial_expected_base() = default;
    };

    template<typename E>
    struct trivial_expected_base<void, E> {
        using error_type = E;

        bool has_value;
        trivial_expected_storage<void, E> storage;

        trivial_expected_base()
                : has_value(true), storage() {
        }

        trivial_expected_base(only_set_valid_t, bool hv)
                : has_value(hv) {
        }

        trivial_expected_base(const exceptional<error_type> &e)
                : has_value(false), storage(e) {
        }

        ~trivial_expected_base() = default;
    };

    template<typename T, typename E>
    struct non_trivial_expected_base {
        using value_type = T;
        using error_type = E;

        bool has_value;
        non_trivial_expected_storage<T, E> storage;

        non_trivial_expected_base()
                : has_value(true), storage() {
        }

        non_trivial_expected_base(only_set_valid_t, bool hv)
                : has_value(hv) {
        }

        non_trivial_expected_base(const value_type &v)
                : has_value(true), storage(v) {
        }

        non_trivial_expected_base(value_type &&v)
                : has_value(true), storage(std::forward<value_type>(v)) {
        }

        non_trivial_expected_base(const exceptional<error_type> &e)
                : has_value(false), storage(e) {
        }
    };

    template<typename E>
    struct non_trivial_expected_base<void, E> {
        using error_type = E;

        bool has_value;
        non_trivial_expected_storage<void, E> storage;

        non_trivial_expected_base()
                : has_value(true), storage() {
        }

        non_trivial_expected_base(only_set_valid_t, bool hv)
                : has_value(hv) {
        }

        non_trivial_expected_base(const exceptional<error_type> &e)
                : has_value(false), storage(e) {
        }
    };

    template<typename T, typename E>
    using expected_base = std::conditional_t<
            std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value,
            trivial_expected_base<T, E>,
            non_trivial_expected_base<T, E>>;

    template<typename T, typename E = size_t>
    struct expected : expected_base<T, E> {
        using value_type = T;
        using error_type = E;

        typedef expected<T, E> this_type;
        typedef expected_base<T, E> base_type;

    private:
        value_type *value_ptr() {
            return std::addressof(base_type::storage.value);
        }

        constexpr const value_type *value_ptr() const {
            return std::addressof(base_type::storage.value);
        }

        error_type *error_ptr() {
            return std::addressof(base_type::storage.error);
        }

        constexpr const error_type *error_ptr() const {
            return std::addressof(base_type::storage.error);
        }

        [[nodiscard]] constexpr const bool &contained_has_value() const &{
            return base_type::has_value;
        }

        bool &contained_has_value() &{
            return base_type::has_value;
        }

        bool &&contained_has_value() &&{
            return std::move(base_type::has_value);
        }

        constexpr const value_type &contained_value() const &{
            return base_type::storage.value;
        }

        value_type &contained_value() &{
            return base_type::storage.value;
        }

        value_type &&contained_value() &&{
            return std::move(base_type::storage.value);
        }

        constexpr const error_type &contained_error() const &{
            return base_type::storage.error;
        }

        error_type &contained_error() &{
            return base_type::storage.error;
        }

        error_type &&contained_error() &&{
            return std::move(base_type::storage.error);
        }

    public:
        constexpr expected(const value_type &v)
                : base_type(v) {
        }

        constexpr expected(value_type &&v)
                : base_type(std::forward<value_type>(v)) {
        }

        expected(const expected &rhs)
                : base_type(only_set_valid, rhs.valid()) {
            if (rhs.valid()) {
                ::new(value_ptr()) value_type(rhs.contained_value());
            } else {
                ::new(error_ptr()) error_type(rhs.contained_error());
            }
        }

        expected(expected &&rhs)
                : base_type(only_set_valid, rhs.valid()) {
            if (rhs.valid()) {
                new(value_ptr()) value_type(std::move(rhs.contained_value()));
            } else {
                new(error_ptr()) error_type(std::move(rhs.contained_error()));
            }
        }

        expected(const exceptional<error_type> &e)
                : base_type(e) {
        }

        expected()
                : base_type() {}

        ~expected() = default;

        expected &operator=(const expected &rhs) {
            this_type(rhs).swap(*this);
            return *this;
        }

        expected &operator=(expected &&rhs) {
            this_type(std::move(rhs)).swap(*this);
            return *this;
        }

        expected &operator=(const value_type &v) {
            this_type(v).swap(*this);
            return *this;
        }

        expected &operator=(value_type &&v) {
            this_type(std::move(v)).swap(*this);
            return *this;
        }

        void swap(expected &rhs) {
            if (valid()) {
                if (rhs.valid()) {
                    std::swap(contained_value(), rhs.contained_value());
                } else {
                    error_type t = std::move(rhs.contained_error());
                    new(rhs.value_ptr()) value_type(std::move(contained_value()));
                    new(error_ptr()) error_type(t);
                    std::swap(contained_has_value(), rhs.contained_has_value());
                }
            } else {
                if (rhs.valid()) {
                    rhs.swap(*this);
                } else {
                    std::swap(contained_error(), rhs.contained_error());
                }
            }
        }

        [[nodiscard]] constexpr bool valid() const {
            return contained_has_value();
        }

        constexpr explicit operator bool() const {
            return valid();
        }

        constexpr const value_type &value() const {
            return contained_value();
        }

        constexpr const value_type &operator*() const {
            return contained_value();
        }

        value_type &operator*() {
            return contained_value();
        }

        constexpr const value_type *operator->() const {
            return value_ptr();
        }

        value_type *operator->() {
            return value_ptr();
        }

        constexpr const error_type &error() const {
            return contained_error();
        }

        constexpr bool has_error(const error_type &e) const {
            return contained_error() == e;
        }

        constexpr exceptional<error_type> get_exceptional() const {
            return exceptional<error_type>(contained_error());
        }
    };

    template<typename E>
    struct expected<void, E> : expected_base<void, E> {
        using value_type = void;
        using error_type = E;

        using this_type = expected<void, E>;
        using base_type = expected_base<void, E>;

    private:
        error_type *error_ptr() {
            return std::addressof(base_type::storage.error);
        }

        [[nodiscard]] constexpr const error_type *error_ptr() const {
            return static_addressof(base_type::storage.error);
        }

        [[nodiscard]] constexpr const bool &contained_has_value() const &{
            return base_type::has_value;
        }

        bool &contained_has_value() &{
            return base_type::has_value;
        }

        bool &&contained_has_value() &&{
            return std::move(base_type::has_value);
        }

        [[nodiscard]] constexpr const error_type &contained_error() const &{
            return base_type::storage.error;
        }

        error_type &contained_error() &{
            return base_type::storage.error;
        }

        error_type &&contained_error() &&{
            return std::move(base_type::storage.error);
        }

    public:
        expected(const expected &rhs)
                : base_type(only_set_valid, rhs.valid()) {
            if (!rhs.valid()) {
                ::new(error_ptr()) error_type(rhs.contained_error());
            }
        }

        expected(expected &&rhs)
                : base_type(only_set_valid, rhs.valid()) {
            if (!rhs.valid()) {
                new(error_ptr()) error_type(std::move(rhs.contained_error()));
            }
        }

        expected(const exceptional<error_type> &e)
                : base_type(e) {
        }

        expected()
                : base_type() {}

        ~expected() = default;

        expected &operator=(const expected &rhs) {
            this_type(rhs).swap(*this);
            return *this;
        }

        expected &operator=(expected &&rhs) {
            this_type(std::move(rhs)).swap(*this);
            return *this;
        }

        void swap(expected &rhs) {
            if (valid()) {
                if (!rhs.valid()) {
                    error_type t = std::move(rhs.contained_error());
                    new(error_ptr()) error_type(t);
                }
            } else {
                if (!rhs.valid()) {
                    std::swap(contained_error(), rhs.contained_error());
                }
            }
        }

        [[nodiscard]] constexpr bool valid() const {
            return contained_has_value();
        }

        constexpr explicit operator bool() const {
            return valid();
        }

        [[nodiscard]] constexpr const error_type &error() const {
            return contained_error();
        }

        [[nodiscard]] constexpr bool has_error(const error_type &e) const {
            return contained_error() == e;
        }

        [[nodiscard]] constexpr exceptional<error_type> get_exceptional() const {
            return exceptional<error_type>(contained_error());
        }
    };

    template<typename T>
    inline expected<T> make_expected(T &&v) {
        return expected<T>(std::forward<T>(v));
    }

    template<typename T>
    inline expected<T> make_expected(const T &v) {
        return expected<T>(v);
    }

    template<typename T, typename U, typename E>
    inline expected<T, U> make_expected_from_error(E v) {
        return expected<T, U>(exceptional<U>(v));
    }

    template<typename T, typename E>
    inline expected<T, E> make_expected_from_error(E v) {
        return expected<T, E>(exceptional<E>(v));
    }

    template<typename T, typename E>
    inline expected<T, E> make_unexpected(E v) {
        return expected<T, E>(exceptional<E>(v));
    }

    template<typename E>
    inline expected<void, E> make_expected_zero(E v) {
        if (v) {
            return expected<void, E>(exceptional<E>(v));
        } else {
            return expected<void, E>();
        }
    }

    inline expected<void> make_expected() {
        return {};
    }
}
