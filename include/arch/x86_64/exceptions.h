/*
 * exceptions.h
 *
 *  Created on: 3/16/24.
 *      Author: Cezar PP
 */

#pragma once

#include "interrupts.h"
#include "../../std/source_location.h"

void dumpRegstate(RegistersState *reg_state);

void kPanicAt(const char *message, std::source_location location);

inline void kPanic(const char *message, std::source_location location = std::source_location::current()) {
    kPanicAt(message, location);
}

// void kException(RegistersState *reg_state, const char *exceptionMsg);

template<typename Expr>
inline void kAssert(Expr expression, const char *message,
                    std::source_location location = std::source_location::current()) {
    if (!expression) {
        kPanicAt(message, location);
    }
}