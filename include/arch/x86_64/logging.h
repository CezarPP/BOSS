/*
 * logging.h
 *
 *  Created on: 3/17/24.
 *      Author: Cezar PP
 */

#pragma once

#include "../../util/types.h"
#include "io_ports.h"
#include "util/print.h"


constexpr uint16_t PORT = 0x3F8;

class Logger : public PrinterInterface<Logger> {
public:
    bool init();

    void printChar(char c);

    void printStr(const char *c);

    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;


    // Public method to access the singleton instance
    static Logger &instance() {
        static Logger instance;
        return instance;
    }

private:
    Logger() = default;

    int isTransmitEmpty();

    void write(char a);
};