/*
 * IO_ports.h
 *
 *  Created on: 3/13/24.
 *      Author: Cezar PP
 */
#pragma once

#include "util/types.h"

class Port {
public:
    uint16_t portNumber;
protected:
    constexpr explicit Port(uint16_t portNumber) : portNumber(portNumber) {}
};


struct Port8Bit : public Port {
    constexpr explicit Port8Bit(uint16_t portNumber) : Port(portNumber) {}

    uint8_t read() {
        return read8(portNumber);
    }

    void write(uint8_t data) {
        write8(portNumber, data);
    }

    static inline uint8_t read8(uint16_t _port) {
        uint8_t result;
        __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (_port));
        return result;
    }

    static inline void write8(uint16_t _port, uint8_t _data) {
        __asm__ volatile("outb %0, %1" : : "a" (_data), "Nd" (_port));
    }
};


struct Port16Bit : public Port {
    constexpr explicit Port16Bit(uint16_t portNumber) : Port(portNumber) {}

    uint16_t read() {
        return read16(portNumber);
    }

    void write(uint16_t data) {
        write16(portNumber, data);
    }

    static inline uint16_t read16(uint16_t _port) {
        uint16_t result;
        __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (_port));
        return result;
    }

    static inline void write16(uint16_t _port, uint16_t _data) {
        __asm__ volatile("outw %0, %1" : : "a" (_data), "Nd" (_port));
    }
};


struct Port32Bit : public Port {
    constexpr explicit Port32Bit(uint16_t portNumber) : Port(portNumber) {}

    uint32_t read() {
        return read32(portNumber);
    }

    void write(uint32_t data) {
        write32(portNumber, data);
    }

    static inline uint32_t read32(uint16_t _port) {
        uint32_t result;
        __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));
        return result;
    }

    static inline void write32(uint16_t _port, uint32_t _data) {
        __asm__ volatile("outl %0, %1" : : "a"(_data), "Nd" (_port));
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
