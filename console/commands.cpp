/*
 * commands.cpp
 *
 *  Created on: 6/20/24.
 *      Author: Cezar PP
 */

#include "console/commands.h"
#include "std/cstring.h"
#include "arch/x86_64/system_calls.h"
#include "console/console_printer.h"

const char *commandsHelp[] = {"touch <fileName>",
                              "open <fileName>",
                              "close <fileDescriptor>",
                              "read <fileDescriptor>",
                              "write <fileDescriptor> <data> <offset>",
                              "mkdir <dirName>",
                              "rmdir <dirName>",
                              "rm <fileName>"};

void commands::doCommand(char *commandText) {
    const char *command = strtok(commandText, " ");
    const char *args = strtok(nullptr, " ");

    int64_t result;
    if (strcmp(command, "pwd") == 0) {
        char buffer[256];
        result = sys_calls::issueSyscall(0x4A, reinterpret_cast<uint64_t>(buffer), 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error printing working directory!");
            return;
        }

        Console::instance().println("Current directory: %s", buffer);
    } else if (strcmp(command, "touch") == 0) {
        const char *filename = args; // open with OPEN_CREATE
        result = sys_calls::issueSyscall(0x02, reinterpret_cast<uint64_t>(filename), 0x1, 0, 0);
        if (result < 0) {
            Console::instance().println("Error creating file!");
            return;
        }
    } else if (strcmp(command, "open") == 0) {
        const char *filename = args;
        result = sys_calls::issueSyscall(0x02, reinterpret_cast<uint64_t>(filename), 0x0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error printing opening file!");
            return;
        }
        Console::instance().println("File descriptor: %X", result);
    } else if (strcmp(command, "close") == 0) {
        uint64_t fd = strtoul(args, nullptr, 10);
        result = sys_calls::issueSyscall(0x03, fd, 0, 0, 0);
        Console::instance().println("Successfully closed file descriptor");
    } else if (strcmp(command, "read") == 0) {
        uint64_t fd = strtoul(args, nullptr, 10);
        uint8_t buffer[1024];
        memset(buffer, 0, 1024);
        // uint64_t size = strtoul(strtok(nullptr, " "), nullptr, 10);
        // uint64_t offset = strtoul(strtok(nullptr, " "), nullptr, 10);

        Logger::instance().println("[COMMANDS] Reading fd %X, data %s", fd);
        result = sys_calls::issueSyscall(0x00, fd, reinterpret_cast<uint64_t>(buffer), 0, 0);
        if (result < 0) {
            Console::instance().println("Error reading file!");
            return;
        }
        Console::instance().println("Read data: %s", buffer);
    } else if (strcmp(command, "write") == 0) {
        uint64_t fd = strtoul(args, nullptr, 10);
        const char *data = strtok(nullptr, " ");
        uint64_t size = strlen(data);
        uint64_t offset = strtoul(strtok(nullptr, " "), nullptr, 10);
        Logger::instance().println("[COMMANDS] Writing fd %X, data %s, size %X, offset %X", fd, data, size, offset);
        result = sys_calls::issueSyscall(0x01, fd, reinterpret_cast<uint64_t>(data), size, offset);
        if (result < 0) {
            Console::instance().println("Error writing file!");
            return;
        }
    } else if (strcmp(command, "cd") == 0 || strcmp(command, "cwd") == 0) {
        const char *path = args;
        result = sys_calls::issueSyscall(0x4B, reinterpret_cast<uint64_t>(path), 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error changing directory!");
            return;
        }
    } else if (strcmp(command, "mkdir") == 0) {
        const char *dirName = args;
        result = sys_calls::issueSyscall(0x4E, reinterpret_cast<uint64_t>(dirName), 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error creating directory!");
            return;
        }
    } else if (strcmp(command, "rm") == 0) {
        const char *fileName = args;
        result = sys_calls::issueSyscall(0xAA, reinterpret_cast<uint64_t>(fileName), 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error removing file!");
            return;
        }
    } else if (strcmp(command, "rmdir") == 0) {
        const char *dirName = args;
        result = sys_calls::issueSyscall(0x4F, reinterpret_cast<uint64_t>(dirName), 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error removing directory!");
            return;
        }
    } else if (strcmp(command, "ls") == 0) {
        // Assuming no arguments are needed for ls
        result = sys_calls::issueSyscall(0xAB, 0, 0, 0, 0);
        if (result < 0) {
            Console::instance().println("Error listing directory!");
            return;
        }
    } else if (strcmp(command, "clear") == 0) {
        Console::instance().print_clear();
    } else if (strcmp(command, "help") == 0) {
        Console::instance().print_clear();
        for (const auto it: commandsHelp)
            Console::instance().println(it);
    } else {
        Console::instance().println("Unknown command!");
    }
}
