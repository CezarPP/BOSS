/*
 * IO_ports.h
 *
 *  Created on: 3/13/24.
 *      Author: Cezar PP
 */
#pragma once

#include "util/types.h"

// Forward declaration of the Port class template
template<typename PortType, uint16_t port>
class Port;

// Specialization for Byte operations
template<uint16_t port>
class Port<Byte, port> {
public:
    static Byte read() {
        Byte ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
        return ret;
    }

    static void write(Byte value) {
        asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
    }
};

// Specialization for Word operations
template<uint16_t port>
class Port<uint16_t, port> {
public:
    static uint16_t read() {
        uint16_t ret;
        asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
        return ret;
    }

    static void write(uint16_t value) {
        asm volatile("outw %1, %0" : : "dN"(port), "a"(value));
    }
};

[[maybe_unused]] static Byte inb(uint16_t port) {
    Byte ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

[[maybe_unused]] static void outb(uint16_t port, Byte value) {
    asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

/*
Byte getPortByte(uint16_t port);

void setPortByte(uint16_t port, Byte value);

uint16_t getPortWord(uint16_t port);

void setPortWord(uint16_t port, uint16_t value);*/
