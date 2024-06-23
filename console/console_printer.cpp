/*
 * console_printer.cpp
 *
 *  Created on: 3/20/24.
 *      Author: Cezar PP
 */



#include "console/console_printer.h"
#include "std/array.h"
#include "arch/x86_64/logging.h"
#include "arch/x86_64/exceptions.h"
#include "std/cstring.h"
#include "console/commands.h"
#include "fs/vfs.h"


void Console::clear_row(size_t row) {
    Char empty = {
            .character = ' ',
            .color = color,
    };

    for (size_t col = 0; col < NUM_COLS; col++) {
        vgaBuffer[col + NUM_COLS * row] = empty;
    }
}

void Console::print_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }
    row_ = 0;
    charBuffer.clear();
}

void Console::printNewline() {
    col_ = 0;

    if (row_ < NUM_ROWS - 1) {
        row_++;
        return;
    }

    for (size_t row = 1; row < NUM_ROWS; row++) {
        for (size_t col = 0; col < NUM_COLS; col++) {
            auto character = vgaBuffer[col + NUM_COLS * row];
            vgaBuffer[col + NUM_COLS * (row - 1)] = character;
        }
    }

    clear_row(NUM_COLS - 1);
}

void Console::printBackspace() {
    if (col_ > 0) {
        // Move cursor back one column
        col_--;
    } else if (row_ > 0) {
        col_ = NUM_COLS - 1;
        // Move cursor back to the end of the previous row
        row_--;
    } else {
        // At the start of the buffer, no effect
        return;
    }
    vgaBuffer[col_ + NUM_COLS * row_] = {
            .character = ' ',  // Replace the character with a space
            .color = color,
    };
}

void Console::printChar(char character) {
    if (character == '\n') {
        printNewline();
        return;
    } else if (character == '\b') {
        printBackspace();
        return;
    }

    if (col_ >= NUM_COLS) {
        printNewline();
    }

    vgaBuffer[col_ + NUM_COLS * row_] = {
            .character = static_cast<uint8_t>(character),
            .color = color,
    };

    col_++;
}

void Console::printStr(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        printChar(str[i]);
    }
}

void Console::printSetColor(PRINTER_COLORS foreground, PRINTER_COLORS background) {
    color = static_cast<uint8_t>(foreground) | (static_cast<uint8_t>(background) << 4);
}

void Console::addKeyboardInput(char c) {
    Console::instance().printf("%c", c);
    if (c == '\n') {
        executeCommand();
        setPrompt();
        charBuffer.clear();
    } else if (c == '\b')
        charBuffer.pop_last();
    else
        charBuffer.push(c);
}

void Console::executeCommand() {
    std::array<char, 128> buff{};
    int ptr = 0;
    while (!charBuffer.empty()) {
        buff[ptr++] = charBuffer.pop();
        kAssert(ptr < 128, "[CONSOLE] Command can't be longer than 128 chars");
    }
    buff[ptr] = '\0';

    if (strlen(buff.data()) > 0) {
        Logger::instance().println("Executing command %s", buff.data());

        commands::doCommand(buff.data());
    }
}

void Console::setCrtDir() {
    strcpy(this->crtDir, vfs::pwd().c_str());
}

void Console::setPrompt() {
    setCrtDir();
    Console::instance().printf("boss@boss %s > ", this->crtDir);
}

/*void Printer::print_hex(uint64_t num) {
    printStr("0x");

    bool leading = true;
    for (int i = 60; i >= 0; i -= 4) {
        auto temp = (num >> i) & 0xF;
        if (leading && temp == 0 && i != 0) {
            continue;
        }
        leading = false;
        if (temp >= 0xA) {
            printChar(temp - 0xA + 'A');
        } else {
            printChar(temp + '0');
        }
    }

    if (leading) {
        printChar('0');
    }
}*/
