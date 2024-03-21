#pragma once

#include <stddef.h>
#include <stdarg.h>
#include "../std/runtime.h"
#include "../std/concepts.h"


template<typename T>
concept Printable = requires(T a, char ch, const char *str) {
    { a.printChar(ch) } -> Std::SameAs<void>;
    { a.printStr(str) } -> Std::SameAs<void>;
};

// The PrinterInterface using CRTP
template<typename Derived>
class PrinterInterface {
public:
    void println(const char *format, ...) {
        va_list args;
        va_start(args, format);
        // Reuse printf for the main formatted output
        this->vprintf(format, args);
        // Print a newline character after the formatted output
        static_cast<Derived *>(this)->printChar('\n');
        va_end(args);
    }

    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        // Reuse printf for the main formatted output
        this->vprintf(format, args);
        va_end(args);
    }

    void vprintf(const char *format, va_list args) {
        for (const char *p = format; *p != '\0'; p++) {
            if (*p != '%') {
                static_cast<Derived *>(this)->printChar(*p);
                continue;
            }
            switch (*++p) {
                case 'c': {
                    char val = va_arg(args, int); // Promoted to int when passed through ...
                    static_cast<Derived *>(this)->printChar(val);
                    break;
                }
                case 's': {
                    char *val = va_arg(args, char*);
                    static_cast<Derived *>(this)->printStr(val);
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    printInt(val);
                    break;
                }
                case 'X': {
                    // 64-bit hex
                    unsigned long long val = va_arg(args, unsigned long long);
                    printHex(val);
                    break;
                }
                case 'x': {
                    // 32-bit hex
                    unsigned int val = va_arg(args, unsigned int);
                    printHex(val);
                    break;
                }
                default:
                    // If the specifier is not recognized, print it as is.
                    static_cast<Derived *>(this)->printChar('%');
                    static_cast<Derived *>(this)->printChar(*p);
                    break;
            }
        }
    }

    void printInt(int value) {
        char buffer[12]; // Enough for 32-bit int, -2147483648
        char *p = buffer + sizeof(buffer) - 1; // Start at the end
        *p = '\0'; // Null terminator

        if (value == 0) {
            // Handle 0 explicitly, simplest case
            static_cast<Derived *>(this)->printChar('0');
            return;
        }

        bool isNegative = value < 0;
        if (isNegative) {
            value = -value; // Make positive
        }

        while (value > 0) {
            p--;
            *p = (char) ('0' + (value % 10)); // Convert last digit to char
            value /= 10; // Move to the next digit
        }

        if (isNegative) {
            p--;
            *p = '-'; // Prefix with minus sign for negative values
        }

        static_cast<Derived *>(this)->printStr(p); // Print the string
    }

    void printHex(unsigned int value) {
        static_cast<Derived *>(this)->printStr("0x");
        char buffer[9]; // Enough for 32-bit hex, 0xFFFFFFFF
        char *p = buffer + sizeof(buffer) - 1; // Start at the end
        *p = '\0'; // Null terminator

        if (value == 0) {
            // Handle 0 explicitly, simplest case
            static_cast<Derived *>(this)->printChar('0');
            return;
        }

        while (value != 0) {
            p--;
            unsigned int digit = value % 16;
            if (digit < 10) {
                *p = (char) ('0' + digit);
            } else {
                *p = (char) ('A' + (digit - 10));
            }
            value /= 16; // Move to the next digit
        }

        static_cast<Derived *>(this)->printStr(p); // Print the string
    }

    void printHex(unsigned long long value) {
        static_cast<Derived *>(this)->printStr("0x");
        char buffer[17]; // Enough for 64-bit hex, FFFFFFFFFFFFFFFF
        char *p = buffer + sizeof(buffer) - 1; // Start at the end
        *p = '\0'; // Null terminator

        if (value == 0) {
            static_cast<Derived *>(this)->printChar('0');
            return;
        }

        while (value != 0) {
            p--;
            unsigned int digit = value % 16;
            if (digit < 10) {
                *p = (char) ('0' + digit);
            } else {
                *p = (char) ('A' + (digit - 10));
            }
            value /= 16; // Move to the next digit
        }

        static_cast<Derived *>(this)->printStr(p); // Print the string
    }
};