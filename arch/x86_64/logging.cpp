/*
 * logging.cpp
 *
 *  Created on: 3/17/24.
 *      Author: Cezar PP
 */
#include "../../include/arch/x86_64/logging.h"



bool Logger::init() {
    Port<Byte, PORT + 1>::write(0x00); // Disable all interrupts
    Port<Byte, PORT + 3>::write(0x80); // Enable DLAB (set baud rate divisor)

    Port<Byte, PORT + 0>::write(0x03); // Set divisor to 3 (lo byte) 38400 baud
    Port<Byte, PORT + 1>::write(0x00); // (hi byte)
    Port<Byte, PORT + 3>::write(0x03); // 8 bits, no parity, one stop bit
    Port<Byte, PORT + 2>::write(0xC7); // Enable FIFO, clear them, with 14-byte threshold
    Port<Byte, PORT + 4>::write(0x0B); // IRQs enabled, RTS/DSR set
    Port<Byte, PORT + 4>::write(0x1E); // Set in loopback mode, test the serial chip
    Port<Byte, PORT + 0>::write(0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (Port<Byte, PORT>::read() != 0xAE) {
        return true; // Serial is faulty
    }

    // If serial is not faulty, set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    Port<Byte, PORT + 4>::write(0x0F);
    return false; // Success
}


int Logger::isTransmitEmpty() {
    return Port<Byte, PORT + 5>::read() & 0x20;
}

void Logger::write(char a) {
    while (isTransmitEmpty() == 0);

    Port<Byte, PORT>::write(a);
}

void Logger::printChar(char c) {
    write(c);
}


void Logger::printStr(const char *s) {
    while (*s) {
        write(*s++);
    }
    // printChar('\n');
}

/*
void Logger::log_hex(uint64_t num) {
    printChar('0');
    printChar('x');

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
    printChar('\n');
}*/

template class PrinterInterface<Logger>;