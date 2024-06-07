/*
 * functional.h
 *
 *  Created on: 6/7/24.
 *      Author: Cezar PP
 */

#pragma once

#include "unique_ptr.h"

namespace std {
    /**
     * General purpose polymorphic function wrapper
     */
    template<typename T>
    class function;


    // Implementation is inspired by Jason Turner's C++ Weekly - Ep 333
    template<typename R, typename... Args>
    class function<R(Args...)> {
    private:
        struct callableInterface {
            virtual R call(Args...) = 0;

            virtual ~callableInterface() = default;
        };

        template<typename Callable>
        struct callableImpl : callableInterface {
            explicit callableImpl(Callable callable_) : callable(std::move(callable_)) {
            }

            R call(Args... args) {
                return callable(args...);
            }

            Callable callable;
        };

        std::unique_ptr<callableInterface> callable;
    public:
        function(R (*f)(Args...)) noexcept: callable(std::make_unique<callableImpl<R(*)(Args...)>>(f)) {
        }

        R operator()(Args... args) {
            return callable->call(args...);
        }
    };

}