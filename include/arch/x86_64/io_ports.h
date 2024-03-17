/*
 * IO_ports.h
 *
 *  Created on: 3/13/24.
 *      Author: Cezar PP
 */
#pragma once

#include "util/types.h"

Byte getPortByte(uint16_t port);

void setPortByte(uint16_t port, Byte value);

uint16_t getPortWord(uint16_t port);

void setPortWord(uint16_t port, uint16_t value);