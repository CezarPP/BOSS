/*
 * IO_ports.cpp
 *
 *  Created on: 3/13/24.
 *      Author: Cezar PP
 */
#include "../../include/arch/x86_64/io_ports.h"

Byte getPortByte(uint16_t port) {
    Byte ret;

    __asm__ __volatile__("inb %1, %0"
            : "=a"(ret)
            : "dN"(port));
    return ret;
}

void setPortByte(uint16_t port, Byte value) {
    __asm__ __volatile__("outb %1, %0"
            :
            : "dN"(port), "a"(value));
}

uint16_t getPortWord(uint16_t port) {
    uint16_t ret;

    __asm__ __volatile__("inw %1, %0"
            : "=a"(ret)
            : "dN"(port));
    return ret;
}

void setPortWord(uint16_t port, uint16_t value) {
    __asm__ __volatile__("outw %1, %0"
            :
            : "dN"(port), "a"(value));
}