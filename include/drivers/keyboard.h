/*
 * keyboard.h
 *
 *  Created on: 3/17/24.
 *      Author: Cezar PP
 */

#pragma once

#include "std/runtime.h"
#include "util/types.h"
#include "arch/x86_64/io_ports.h"
#include "arch/x86_64/logging.h"
#include "util/console_printer.h"

static constexpr uint16_t dataPortNr = 0x60;
static constexpr uint16_t controlPortNr = 0x64;

class KeyboardEventHandler {
public:
    void onKeyDown(char c) {
        Printer::instance().printf("%c", c);
    }
};

class KeyboardDriver {
    typedef Port<Byte, controlPortNr> commandPort;
    typedef Port<Byte, dataPortNr> dataPort;

    KeyboardEventHandler handler{};

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

// Keyboard map index constants for readability
enum class KeyMapIndex {
    Normal = 0,
    Shift = 1,
    Ctrl = 2,
    KeyMapSize = 3 // Placeholder to get the size of the key maps
};

// Define a keyboard map for the standard QWERTY layout
constexpr char keyboardMap[0x80][(size_t) KeyMapIndex::KeyMapSize] = {
        {}, // 0x00
        {}, // 0x01 ESC
        {'1',  '!',  0}, // 0x02
        {'2',  '@',  0}, // 0x03
        {'3',  '#',  0}, // 0x04
        {'4',  '$',  0}, // 0x05
        {'5',  '%',  0}, // 0x06
        {'6',  '^',  0}, // 0x07
        {'7',  '&',  0}, // 0x08
        {'8',  '*',  0}, // 0x09
        {'9',  '(',  0}, // 0x0A
        {'0',  ')',  0}, // 0x0B
        {'-',  '_',  0}, // 0x0C
        {'=',  '+',  0}, // 0x0D
        {'\b', '\b', 0x7F}, // 0x0E Backspace
        {'\t', '\t', 0x09}, // 0x0F Tab
        {'q',  'Q',  0x11}, // 0x10
        {'w',  'W',  0x17}, // 0x11
        {'e',  'E',  0x05}, // 0x12
        {'r',  'R',  0x12}, // 0x13
        {'t',  'T',  0x14}, // 0x14
        {'y',  'Y',  0x19}, // 0x15
        {'u',  'U',  0x15}, // 0x16
        {'i',  'I',  0x09}, // 0x17
        {'o',  'O',  0x0F}, // 0x18
        {'p',  'P',  0x10}, // 0x19
        {'[',  '{',  0x1B}, // 0x1A
        {']',  '}',  0x1D}, // 0x1B
        {'\n', '\n', 0x0A}, // 0x1C Enter
        {}, // 0x1D Ctrl
        {'a',  'A',  0x01}, // 0x1E
        {'s',  'S',  0x13}, // 0x1F
        {'d',  'D',  0x04}, // 0x20
        {'f',  'F',  0x06}, // 0x21
        {'g',  'G',  0x07}, // 0x22
        {'h',  'H',  0x08}, // 0x23
        {'j',  'J',  0x0A}, // 0x24
        {'k',  'K',  0x0B}, // 0x25
        {'l',  'L',  0x0C}, // 0x26
        {';',  ':',  0}, // 0x27
        {'\'', '\"', 0}, // 0x28
        {'`',  '~',  0}, // 0x29
        {}, // 0x2A Left Shift
        {'\\', '|',  0x1C}, // 0x2B
        {'z',  'Z',  0x1A}, // 0x2C
        {'x',  'X',  0x18}, // 0x2D
        {'c',  'C',  0x03}, // 0x2E
        {'v',  'V',  0x16}, // 0x2F
        {'b',  'B',  0x02}, // 0x30
        {'n',  'N',  0x0E}, // 0x31
        {'m',  'M',  0x0D}, // 0x32
        {',',  '<',  0}, // 0x33
        {'.',  '>',  0}, // 0x34
        {'/',  '?',  0}, // 0
        {'/',  '?',  0}, // 0x35
        {}, // 0x36 Right Shift
        {}, // 0x37
        {' ',  ' ',  0}, // 0x38 Space
};
