/*
 * unique_ptr.h
 *
 *  Created on: 5/15/24.
 *      Author: Cezar PP
 */

#pragma once

#include "default_delete.h"
#include "tuple.h"

namespace std {
    /**
     * A unique pointer of type T
     *
     * @tparam T The type of the pointer
     * @tparam D The deleter, by default: default_delete<T>
     */
    template<typename T, typename D = default_delete<T>>
    struct unique_ptr {
    public:
        using pointer_type = T *;
        using element_type = T;
        using deleter_type = D;
    private:
        using data_impl = tuple<pointer_type, deleter_type>; /// The type of internal data

        data_impl data_;
    public:

        unique_ptr() : data_() {}

        unique_ptr(decltype(nullptr)) : unique_ptr() {}

        explicit unique_ptr(pointer_type p) : data_(make_tuple(p, deleter_type())) {}

        unique_ptr(unique_ptr &&u) : data_(make_tuple(u.unlock(), u.get_deleter())) {}

        unique_ptr &operator=(unique_ptr &&u) {
            reset(u.unlock());
            get_deleter() = std::forward<deleter_type>(u.get_deleter());
            return *this;
        }

        ~unique_ptr() {
            reset();
        }

        unique_ptr(const unique_ptr &rhs) = delete;

        unique_ptr &operator=(const unique_ptr &rhs) = delete;

        unique_ptr &operator=(decltype(nullptr)) {
            reset();
            return *this;
        }

        element_type &operator*() const {
            return *get();
        }

        pointer_type operator->() const {
            return get();
        }

        [[nodiscard]] pointer_type get() const {
            return std::get<0>(data_);
        }

        deleter_type &get_deleter() {
            return std::get<1>(data_);
        }

        [[nodiscard]] const deleter_type &get_deleter() const {
            return std::get<1>(data_);
        }

        explicit operator bool() const {
            return get() != pointer_type();
        }

        pointer_type unlock() {
            pointer_type p = get();
            std::get<0>(data_) = pointer_type();
            return p;
        }

        void reset(pointer_type p = pointer_type()) {
            if (get() != pointer_type()) {
                get_deleter()(get());
            }

            std::get<0>(data_) = p;
        }
    };


    /// Unique pointer for arrays
    template<typename T, typename D>
    struct unique_ptr<T[], D> {
        using pointer_type = T *;
        using element_type = T;
        using deleter_type = D;

        unique_ptr() : data_() {}

        unique_ptr(decltype(nullptr)) : unique_ptr() {}

        explicit unique_ptr(pointer_type p) : data_(make_tuple(p, deleter_type())) {}

        unique_ptr(unique_ptr &&u) : data_(make_tuple(u.unlock(), u.get_deleter())) {}

        unique_ptr &operator=(unique_ptr &&u) {
            reset(u.unlock());
            get_deleter() = std::forward<deleter_type>(u.get_deleter());
            return *this;
        }

        ~unique_ptr() {
            reset();
        }

        // Disable copy
        unique_ptr(const unique_ptr &rhs) = delete;

        unique_ptr &operator=(const unique_ptr &rhs) = delete;

        unique_ptr &operator=(decltype(nullptr)) {
            reset();
            return *this;
        }

        [[nodiscard]] pointer_type get() const {
            return std::get<0>(data_);
        }


        deleter_type &get_deleter() {
            return std::get<1>(data_);
        }

        [[nodiscard]] const deleter_type &get_deleter() const {
            return std::get<1>(data_);
        }

        element_type &operator[](size_t i) const {
            return get()[i];
        }

        explicit operator bool() const {
            return get() != pointer_type();
        }

        pointer_type unlock() {
            pointer_type p = get();
            std::get<0>(data_) = pointer_type();
            return p;
        }

        void reset() {
            reset(pointer_type());
        }

        void reset(pointer_type p) {
            auto tmp = get();
            std::get<0>(data_) = p;
            if (tmp) {
                get_deleter()(tmp);
            }
        }

    private:
        using data_impl = tuple<pointer_type, deleter_type>; /// The type of internal data

        data_impl data_;  /// The internal data storage
    };

    static_assert(sizeof(unique_ptr<long>) == sizeof(long), "unique_ptr must have zero overhead with default deleter");
    static_assert(sizeof(unique_ptr<long[]>) == sizeof(long),
                  "unique_ptr must have zero overhead with default deleter");

    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args &&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
