/*
 * vector.h
 *
 *  Created on: 5/2/24.
 *      Author: Cezar PP
 */

#pragma once

#include "new.h"
#include "iterator.h"
#include "algorithm.h"

namespace std {
    /* Commenting out std::allocator is a quick fix for a circular dependency where
     * new needs kalloc which needs buddy_allocator, which needs vector with std::allocator which uses new
     */
    template<class T, class Allocator/* = std::allocator<T>*/>
    class vector_early {
    public:
        using size_type = size_t;
        using iterator = T *;
        using const_iterator = const T *;
    private:

        T *data_;
        size_type capacity_;
        size_type size_;
        Allocator allocator_;

        void reallocate(std::size_t new_capacity) {
            T *new_data = allocator_.allocate(new_capacity);
            for (std::size_t i = 0; i < size_; ++i) {
                new(new_data + i) T(std::move(data_[i]));  // Use placement new
                data_[i].~T();  // Call destructor explicitly
            }
            if (capacity_ > 0)
                allocator_.deallocate(data_, capacity_);
            data_ = new_data;
            capacity_ = new_capacity;
        }

    public:
        // Default constructor
        vector_early() : data_(nullptr), capacity_(0), size_(0) {}

        explicit vector_early(size_type c) : data_(allocator_.allocate(c)), capacity_(c), size_(0) {}

        explicit vector_early(const Allocator &alloc = Allocator()) : data_(nullptr), capacity_(0), size_(0),
                                                                      allocator_(alloc) {}

        explicit vector_early(size_t c, const Allocator &alloc = Allocator()) : capacity_(c), size_(0),
                                                                                allocator_(alloc) {
            data_ = allocator_.allocate(c);
        }


        // Copy constructor
        vector_early(const vector_early &other) : capacity_(other.capacity_), size_(other.size_),
                                                  allocator_(other.allocator_) {
            data_ = allocator_.allocate(capacity_);
            for (std::size_t i = 0; i < size_; ++i) {
                new(data_ + i) T(other.data_[i]);  // Placement new
            }
        }

        // Move constructor
        vector_early(vector_early &&other) noexcept: data_(other.data_), capacity_(other.capacity_), size_(other.size_),
                                                     allocator_(std::move(other.allocator_)) {
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        // Destructor
        ~vector_early() {
            for (std::size_t i = 0; i < size_; ++i) {
                data_[i].~T();  // Explicit destructor call
            }
            if (capacity_ > 0)
                allocator_.deallocate(data_, capacity_);
        }

        // Copy assignment operator
        vector_early &operator=(const vector_early &other) {
            if (this != &other) {
                vector_early temp(other);
                std::swap(data_, temp.data_);
                std::swap(capacity_, temp.capacity_);
                std::swap(size_, temp.size_);
                std::swap(allocator_, temp.allocator_);
            }
            return *this;
        }

        // Move assignment operator
        vector_early &operator=(vector_early &&other) noexcept {
            if (this != &other) {
                for (std::size_t i = 0; i < size_; ++i) {
                    data_[i].~T();  // Explicit destructor call
                }
                if (capacity_ > 0)
                    allocator_.deallocate(data_, capacity_);

                data_ = other.data_;
                capacity_ = other.capacity_;
                size_ = other.size_;
                allocator_ = std::move(other.allocator_);

                other.data_ = nullptr;
                other.capacity_ = 0;
                other.size_ = 0;
            }
            return *this;
        }

        template<typename... Args>
        T &emplace_back(Args &&... args) {
            if (size_ == capacity_) {
                reallocate(capacity_ == 0 ? 1 : 2 * capacity_);
            }
            new(data_ + size_) T(std::forward<Args>(args)...);  // Placement new
            ++size_;
            return data_ + size_;
        }

        void push_back(const T &value) {
            if (size_ == capacity_) {
                reallocate(capacity_ == 0 ? 1 : 2 * capacity_);
            }
            data_[size_++] = value;
        }

        void push_back(T &&element) {
            if (size_ == capacity_) {
                reallocate(capacity_ == 0 ? 1 : 2 * capacity_);
            }
            data_[size_++] = std::move(element);
        }

        void pop_back() {
            --size_;

            // Call the destructor of the erased value
            data_[size_].~T();
        }

        constexpr const T &operator[](size_type pos) const {
            return data_[pos];
        }

        T &operator[](size_type index) {
            return data_[index];
        }

        [[nodiscard]] constexpr size_type size() const {
            return size_;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        [[nodiscard]] constexpr size_type capacity() const noexcept {
            return capacity_;
        }

        [[nodiscard]] constexpr T *data() noexcept {
            return data_;
        }

        [[nodiscard]] constexpr const T *data() const noexcept {
            return data_;
        }

        T &front() {
            return data_[0];
        }

        const T &front() const {
            return data_[0];
        }

        T &back() {
            return data_[size() - 1];
        }

        const T &back() const {
            return data_[size() - 1];
        }

        // Iterators

        iterator begin() {
            return iterator(&data_[0]);
        }

        constexpr const_iterator begin() const {
            return const_iterator(&data_[0]);
        }

        iterator end() {
            return iterator(&data_[size_]);
        }

        constexpr const_iterator end() const {
            return const_iterator(&data_[size_]);
        }

        auto operator<=>(const vector_early &other) const {
            return std::lexicographical_compare_three_way(this->begin(), this->end(),
                                                          other.begin(), other.end());
        }

        // Equality operator
        bool operator==(const vector_early &other) const {
            if (this->size_ != other.size_) {
                return false;
            }
            return std::equal(this->begin(), this->end(), other.begin());
        }

        void resize(size_type new_size) {
            if (new_size < size_) {
                // If new size is smaller, destroy elements beyond new size
                for (size_type i = new_size; i < size_; ++i) {
                    data_[i].~T();  // Call destructor explicitly
                }
                size_ = new_size;
            } else if (new_size > size_) {
                // If new size is larger, insert default-inserted elements
                if (new_size > capacity_) {
                    // If new size exceeds capacity, reallocate memory
                    reallocate(new_size);
                }
                for (size_type i = size_; i < new_size; ++i) {
                    new(data_ + i) T();  // Placement new for default-inserted elements
                }
                size_ = new_size;
            }
        }

        void reserve(size_type new_capacity) {
            if (new_capacity > capacity_) {
                reallocate(new_capacity);
            }
        }

        constexpr iterator erase(const_iterator pos) {
            for (size_t i = pos - begin(); i < size_ - 1; ++i) {
                data_[i] = std::move(data_[i + 1]);
            }

            size_--;

            // Call the destructor of the last value
            data_[size_].~T();
        }

        void erase(iterator first, iterator last) {
            auto n = std::distance(first, last);

            for (size_t i = first - begin(); i < size_ - n; ++i) {
                data_[i] = std::move(data_[i + n]);
            }

            // Call the destructors on the erase elements
            for (size_t i = size_ - n; i < size_; ++i) {
                data_[i].~T();
            }

            size_ -= n;
        }

        void destruct_all() {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
        }

        void clear() {
            destruct_all();
            size_ = 0;
        }
    };


    template<class T, class Alloc, class U>
    constexpr std::vector_early<T, Alloc>::size_type erase(std::vector_early<T, Alloc> &c, const U &value) {
        auto it = std::remove(c.begin(), c.end(), value);
        auto r = c.end() - it;
        c.erase(it, c.end());
        return r;
    }
}


