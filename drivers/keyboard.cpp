/*
 * keyboard.cpp
 *
 *  Created on: 3/17/24.
 *      Author: Cezar PP
 */

#include "drivers/keyboard.h"
#include "util/console_printer.h"


void KeyboardDriver::activate() {
    while (commandPort.read() & 0x1)
        dataPort.read();
    commandPort.write(0xae); // activate interrupts
    commandPort.write(0x20); // command 0x20 = read controller command byte
    uint8_t status = (dataPort.read() | 1) & ~0x10;
    commandPort.write(0x60); // command 0x60 = set controller command byte
    dataPort.write(status);
    dataPort.write(0xf4);
    Logger::instance().println("[DRIVERS] Keyboard driver initialized");
}

// Key states for modifiers
bool shiftPressed = false;
bool ctrlPressed = false;

void KeyboardDriver::handleInterrupt_() {
    // Logger::instance().println("Handling keyboard interrupt");
    uint8_t key = dataPort.read();
    bool released = key & 0x80; // Check if the key was released
    key &= 0x7F; // Mask out the release bit to get the scancode

    // Handle special keys (Ctrl, Shift, etc.)
    switch (key) {
        case 0x1D: // Ctrl key down
            ctrlPressed = !released;
            return;
        case 0x2A: // Left Shift down
        case 0x36: // Right Shift down
            shiftPressed = !released;
            return;
        default:
            break; // Not a modifier key, proceed with normal handling
    }

    if (released) {
        // For keys that we only care about when pressed, not released
        return;
    }

    KeyMapIndex mapIndex = KeyMapIndex::Normal;
    if (shiftPressed) {
        mapIndex = KeyMapIndex::Shift;
    } else if (ctrlPressed) {
        mapIndex = KeyMapIndex::Ctrl;
    }

    if (key < sizeof(keyboardMap) / sizeof(keyboardMap[0])) {
        char outputChar = keyboardMap[key][(size_t) mapIndex];
        if (outputChar != 0) { // Check if there's a mapped character
            handler.onKeyDown(outputChar);
        }
    }

}

void KeyboardDriver::handleInterrupt() {
    KeyboardDriver::instance().handleInterrupt_();
}
