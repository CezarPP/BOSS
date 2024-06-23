/*
 * console_printer.h
 *
 *  Created on: 3/20/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/print.h"
#include "std/circular_buffer.h"

enum class PRINTER_COLORS : uint8_t {
    PRINT_COLOR_BLACK = 0,
    PRINT_COLOR_BLUE = 1,
    PRINT_COLOR_GREEN = 2,
    PRINT_COLOR_CYAN = 3,
    PRINT_COLOR_RED = 4,
    PRINT_COLOR_MAGENTA = 5,
    PRINT_COLOR_BROWN = 6,
    PRINT_COLOR_LIGHT_GRAY = 7,
    PRINT_COLOR_DARK_GRAY = 8,
    PRINT_COLOR_LIGHT_BLUE = 9,
    PRINT_COLOR_LIGHT_GREEN = 10,
    PRINT_COLOR_LIGHT_CYAN = 11,
    PRINT_COLOR_LIGHT_RED = 12,
    PRINT_COLOR_PINK = 13,
    PRINT_COLOR_YELLOW = 14,
    PRINT_COLOR_WHITE = 15,
};


struct Char {
    uint8_t character;
    uint8_t color;
};

class Console : public PrinterInterface<Console> {
    constexpr static size_t NUM_COLS = 80;
    constexpr static size_t NUM_ROWS = 25;

    Char *const vgaBuffer = (Char *) 0xb8000; // Video memory for the VGA text mode

    size_t col_ = 0;
    size_t row_ = 0;

    uint8_t color = static_cast<uint8_t>(PRINTER_COLORS::PRINT_COLOR_WHITE) |
                    (static_cast<uint8_t>(PRINTER_COLORS::PRINT_COLOR_BLACK) << 4);

    circular_buffer<char, 128> charBuffer{};

    char crtDir[128]{};

    // Private constructor
    Console() = default;

    void printNewline();

    void printBackspace();

public:
    Console(const Console &) = delete;

    Console &operator=(const Console &) = delete;

    // Public method to access the singleton instance
    static Console &instance() {
        static Console instance;
        return instance;
    }

    void addKeyboardInput(char c);

    void executeCommand();

    void clear_row(size_t row);

    void print_clear();

    void printChar(char character);

    void printStr(const char *str);

    void printSetColor(PRINTER_COLORS foreground, PRINTER_COLORS background);

    void setCrtDir();

    void setPrompt();
};