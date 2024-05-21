/*
 * string_view.h
 *
 *  Created on: 5/19/24.
 *      Author: Cezar PP
 */


#pragma once

#include "iterator.h"
#include "algorithm.h"
#include "cstring.h"

namespace std {
    template<typename CharT>
    struct basic_string_view {
        using value_type = CharT;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = size_t;
        using iterator = value_type *;
        using const_iterator = const value_type *;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static constexpr const size_t npos = -1;

        constexpr basic_string_view() noexcept
                : data_(nullptr), size_(0) {
        }

        constexpr basic_string_view(const basic_string_view &) noexcept = default;

        basic_string_view &operator=(const basic_string_view &) noexcept = default;

        constexpr basic_string_view(const CharT *str)
                : data_(str), size_(strlen(str)) {
        }

        constexpr basic_string_view(const CharT *str, size_t len)
                : data_(str), size_(len) {
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return data_;
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
            return data_;
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(&data_[int64_t(size_) - 1]);
        }

        [[nodiscard]] const_reverse_iterator rend() const noexcept {
            return reverse_iterator(&data_[-1]);
        }

        [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(&data_[int64_t(size_) - 1]);
        }

        [[nodiscard]] const_reverse_iterator crend() const noexcept {
            return reverse_iterator(&data_[-1]);
        }

        // capacity

        [[nodiscard]] constexpr size_t size() const noexcept {
            return size_;
        }

        [[nodiscard]] constexpr size_t length() const noexcept {
            return size_;
        }

        [[nodiscard]] constexpr size_t max_size() const noexcept {
            return 18446744073709551615UL;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return !size_;
        }

        // Element access

        constexpr const_reference operator[](size_t pos) const {
            return data_[pos];
        }

        [[nodiscard]] constexpr const_reference at(size_t pos) const {
            return data_[pos];
        }

        [[nodiscard]] constexpr const_reference front() const {
            return data_[0];
        }

        [[nodiscard]] constexpr const_reference back() const {
            return data_[size_ - 1];
        }

        [[nodiscard]] constexpr const_pointer data() const noexcept {
            return data_;
        }

        void remove_prefix(size_t n) {
            data_ += n;
            size_ -= n;
        }

        void remove_suffix(size_t n) {
            size_ -= n;
        }

        void swap(basic_string_view &s) noexcept {
            std::swap(size_, s.size_);
            std::swap(data_, s.data_);
        }

        [[nodiscard]] constexpr basic_string_view substr(size_t pos = 0, size_t n = npos) const {
            return {data() + pos, std::min(n, size() - pos)};
        }

        [[nodiscard]] int compare(const basic_string_view &rhs) const noexcept {
            const size_t min_size = std::min(size_, rhs.size_);
            int result = strncmp(data_, rhs.data_, min_size);
            if (result == 0) {
                if (size_ == rhs.size_) {
                    return 0;
                }
                return size_ < rhs.size_ ? -1 : 1;
            }
            return result;
        }

        std::strong_ordering operator<=>(const basic_string_view &rhs) const noexcept {
            return compare(rhs) <=> 0;
        }

    private:
        const CharT *data_;
        size_t size_;
    };

    using string_view = basic_string_view<char>;

    static_assert(sizeof(string_view) == 16, "The size of a string_view must always be 16 bytes");

    template<typename CharT>
    bool operator==(const basic_string_view<CharT> &x, const basic_string_view<CharT> &y) noexcept {
        return (x <=> y) == 0;
    }
}