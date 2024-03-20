/*
 * keyboard.h
 *
 *  Created on: 3/17/24.
 *      Author: Cezar PP
 */

#pragma once

#include <stdint.h>
#include "std/runtime.h"
#include "util/types.h"
#include "arch/x86_64/io_ports.h"
#include "arch/x86_64/logging.h"

static constexpr uint16_t dataPortNr = 0x60;
static constexpr uint16_t controlPortNr = 0x64;

class KeyboardEventHandler {
public:
    void onKeyDown(char);

    void onKeyUp(char);
};

class KeyboardDriver {
    typedef Port<Byte, controlPortNr> commandPort;
    typedef Port<Byte, dataPortNr> dataPort;

    KeyboardEventHandler handler;

    void handleInterrupt_();

public:
    KeyboardDriver() = default;

    KeyboardDriver(const KeyboardDriver &) = delete;

    KeyboardDriver &operator=(const KeyboardDriver &) = delete;

    static void activate();

    // Public method to access the singleton instance
    static KeyboardDriver &instance() {
        static KeyboardDriver instance;
        return instance;
    }

    static void handleInterrupt();
};
