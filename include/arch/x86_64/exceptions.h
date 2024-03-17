/*
 * exceptions.h
 *
 *  Created on: 3/16/24.
 *      Author: Cezar PP
 */

#pragma once

#include "interrupts.h"

void dump_regstate(RegistersState *reg_state);

void kPanicAt(const char *msg, const char *file, unsigned int line);

inline void kPanic [[gnu::always_inline]](const char *msg) {
    kPanicAt(msg, __FILE__, __LINE__);
}

void kException(RegistersState *reg_state, const char *exceptionMsg);