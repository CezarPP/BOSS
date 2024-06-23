/*
 * circular_buffer.h
 *
 *  Created on: 5/28/24.
 *      Author: Cezar PP
 */

#pragma once

#include "../util/types.h"
#include "utility.h"
#include "array.h"

template<typename T, size_t S>
struct circular_buffer {
private:
    static constexpr const size_t Size = S + 1;

    std::array<T, Size> buffer;
    size_t start, end;

public:
    circular_buffer() : start(0), end(0) {}

    [[nodiscard]] constexpr bool full() const {
        return (end + 1) % Size == start;
    }

    [[nodiscard]] constexpr bool empty() const {
        return end == start;
    }

    bool push(T value) {
        if (full()) {
            return false;
        } else {
            buffer[end] = value;
            end = (end + 1) % Size;

            return true;
        }
    }

    template<typename... Ts>
    bool emplace_push(Ts &&... values) {
        if (full()) {
            return false;
        } else {
            new(&buffer[end]) T{std::forward<Ts>(values)...};
            end = (end + 1) % Size;

            return true;
        }
    }

    /**
     * Returns the value at the top of the buffer
     */
    [[nodiscard]] constexpr T top() const {
        return buffer[start];
    }

    /**
     * Removes and returns the value at the top of the buffer
     */
    T pop() {
        auto value = buffer[start];
        start = (start + 1) % Size;
        return value;
    }

    /**
     * Removes all elements from the buffer
     */
    void clear() {
        start = end = 0;
    }

    /**
     * Removes the last element that was pushed
     */
    void pop_last() {
        if (end == 0) {
            end = Size - 1;
        } else {
            --end;
        }
    }

    bool contains(const T &value) {
        for (size_t i = start; i != end; i = (i + 1) % Size) {
            if (buffer[i] == value) {
                return true;
            }
        }

        return false;
    }

    void replace(const T &value, const T &new_value) {
        for (size_t i = start; i != end; i = (i + 1) % Size) {
            if (buffer[i] == value) {
                buffer[i] = new_value;
                return;
            }
        }
    }
};

