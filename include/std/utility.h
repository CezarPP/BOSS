/*
 * utility.h
 *
 *  Created on: 4/9/24.
 *      Author: Cezar PP
 */

#pragma once


namespace std {
    template<typename T1, typename T2>
    class pair {
    public:
        T1 first;
        T2 second;

        // Default constructor
        constexpr pair() : first(T1()), second(T2()) {}

        // Initialized constructor
        constexpr pair(const T1 &a, const T2 &b) : first(a), second(b) {}

        // Copy constructor (optional, since the default copy constructor works fine for most cases)
        constexpr pair(const pair<T1, T2> &other) : first(other.first), second(other.second) {}

        // Assignment operator (optional, since the default assignment operator works fine for most cases)
        constexpr pair &operator=(const pair<T1, T2> &other) {
            if (this != &other) { // self-assignment check
                this->first = other.first;
                this->second = other.second;
            }
            return *this;
        }
    };
}