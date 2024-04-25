/*
 * bitset.h
 *
 *  Created on: 4/17/24.
 *      Author: Cezar PP
 */

#pragma once

#include <stdint.h>

typedef uint64_t size_t;

namespace std {
    template<size_t N>
    class bitset {
    private:
        uint64_t bits[(N + 63) / 64] = {0};
    public:
        constexpr bitset() = default;

        // Set a bit to 1
        void set(size_t pos) {
            if (pos >= N) {
                // Here usually an exception is thrown
                kPanic("bitset index out of range");
            }
            bits[pos / 64] |= (1ULL << (pos % 64));
        }

        // Reset a bit to 0
        void reset(size_t pos) {
            if (pos >= N) {
                // Here usually an exception is thrown
                kPanic("bitset index out of range");
            }
            bits[pos / 64] &= ~(1ULL << (pos % 64));
        }

        // Access and manipulate bits
        class bit_reference {
        private:
            bitset &bset;
            size_t pos;

        public:
            constexpr bit_reference(bitset &bs, size_t p) : bset(bs), pos(p) {}

            // Implicit conversion to bool (for read access)
            constexpr operator bool() const {
                return bset.bits[pos / 64] & (1ULL << (pos % 64));
            }

            // Assignment operator (for write access)
            constexpr bit_reference &operator=(bool x) {
                if (x) bset.set(pos);
                else bset.reset(pos);
                return *this;
            }
        };

        // Overload the index operator
        constexpr bit_reference operator[](size_t pos) {
            return bit_reference(*this, pos);
        }
    };
}