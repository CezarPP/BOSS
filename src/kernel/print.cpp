#include "print.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color;
};

class Printer {
    Char *buffer = (Char *) 0xb8000;
    size_t col_ = 0;
    size_t row_ = 0;
    uint8_t color = static_cast<uint8_t>(PRINTER_COLORS::PRINT_COLOR_WHITE) |
                    (static_cast<uint8_t>(PRINTER_COLORS::PRINT_COLOR_BLACK) << 4);

public:
    void clear_row(size_t row) {
        Char empty = {
                .character =  ' ',
                .color =  color,
        };

        for (size_t col = 0; col < NUM_COLS; col++) {
            buffer[col + NUM_COLS * row] = empty;
        }
    }


    void print_clear() {
        for (size_t i = 0; i < NUM_ROWS; i++) {
            clear_row(i);
        }
    }

    void print_newline() {
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

    void print_char(char character) {
        if (character == '\n') {
            print_newline();
            return;
        }

        if (col_ > NUM_COLS) {
            print_newline();
        }

        buffer[col_ + NUM_COLS * row_] = {
                .character =  static_cast<uint8_t>(character),
                .color =  color,
        };

        col_++;
    }

    void print_str(const char *str) {
        for (size_t i = 0; true; i++) {
            char character = static_cast<uint8_t>(str[i]);

            if (character == '\0') {
                return;
            }

            print_char(character);
        }
    }

    void print_set_color(PRINTER_COLORS foreground, PRINTER_COLORS background) {
        color = static_cast<uint8_t>(foreground) + (static_cast<uint8_t>(background) << 4);
    }
};