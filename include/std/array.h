/*
 * array.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */


#pragma once

#include "../util/types.h"

namespace std {
    template<typename T, size_t N>
    class array {
    public:
        using value_type = T;
        using iterator = value_type *;
        using const_iterator = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = size_t;

        constexpr T &operator[](size_type pos) {
            return data_[pos];
        }

        constexpr const T &operator[](size_type pos) const {
            return data_[pos];
        }

        [[nodiscard]] constexpr size_type size() const {
            return N;
        }

        constexpr iterator begin() {
            return iterator(&data_[0]);
        }

        constexpr const_iterator begin() const {
            return const_iterator(&data_[0]);
        }

        constexpr iterator end() {
            return iterator(&data_[N]);
        }

        constexpr const_iterator end() const {
            return const_iterator(&data_[N]);
        }

        constexpr reference front() {
            return data_[0];
        }

        constexpr const_reference front() const {
            return data_[0];
        }

        constexpr reference back() {
            return data_[N - 1];
        }

        constexpr const_reference back() const {
            return data_[N - 1];
        }

        constexpr value_type *data() {
            return data_;
        }

    private:
        T data_[N];
        static_assert(sizeof(data_) == sizeof(T) * N);
    };
}