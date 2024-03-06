#include "util/print.h"


void Printer::clear_row(size_t row) {
    Char empty = {
            .character = ' ',
            .color = color,
    };

    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void Printer::print_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }
}

void Printer::print_newline() {
    col_ = 0;

    if (row_ < NUM_ROWS - 1) {
        row_++;
        return;
    }

    for (size_t row = 1; row < NUM_ROWS; row++) {
        for (size_t col = 0; col < NUM_COLS; col++) {
            auto character = buffer[col + NUM_COLS * row];
            buffer[col + NUM_COLS * (row - 1)] = character;
        }
    }

    clear_row(NUM_COLS - 1);
}

void Printer::print_char(char character) {
    if (character == '\n') {
        print_newline();
        return;
    }

    if (col_ >= NUM_COLS) {
        print_newline();
    }

    buffer[col_ + NUM_COLS * row_] = {
            .character = static_cast<uint8_t>(character),
            .color = color,
    };

    col_++;
}

void Printer::print_str(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

void Printer::print_set_color(PRINTER_COLORS foreground, PRINTER_COLORS background) {
    color = static_cast<uint8_t>(foreground) | (static_cast<uint8_t>(background) << 4);
}

void Printer::print_hex(uint64_t num) {
    print_str("0x");

    bool leading = true;
    for (int i = 60; i >= 0; i -= 4) {
        auto temp = (num >> i) & 0xF;
        if (leading && temp == 0 && i != 0) {
            continue;
        }
        leading = false;
        if (temp >= 0xA) {
            print_char(temp - 0xA + 'A');
        } else {
            print_char(temp + '0');
        }
    }

    if (leading) {
        print_char('0');
    }
}
