/*
 * runtime.cpp
 *
 *  Created on: 3/16/24.
 *      Author: Cezar PP
 */

#include "../include/std/runtime.h"
/*
 * The __cxa_guard_* functions calling for static member initialization
*/

extern "C" {
// The __cxa_guard_acquire function should return 1 if the initialization has not yet been completed, and 0 otherwise.
// In a multi-threaded environment, this function is also responsible for blocking other threads until the initialization is complete.
int __cxa_guard_acquire(__guard *g) {
    return !*(char *) (g);
}

// The __cxa_guard_release function is called once the initialization has been completed.
// It marks the guarded object as initialized and releases any blocked threads.
void __cxa_guard_release(__guard *g) {
    *(char *) g = 1;
}

// The __cxa_guard_abort function is called if the initialization throws an exception.
// It marks the initialization as not completed and releases any blocked threads.
void __cxa_guard_abort(__guard *g) {
    *(char *) g = 0;
}
}
