/*
 * runtime.h
 *
 *  Created on: 3/16/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"

typedef unsigned long long __guard;
extern "C" int __cxa_guard_acquire(__guard *g);
extern "C" void __cxa_guard_release(__guard *g);
extern "C" void __cxa_guard_abort(__guard *g);
