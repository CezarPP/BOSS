/*
 * fs_errors.h
 *
 *  Created on: 6/7/24.
 *      Author: Cezar PP
 */


#pragma once

#include "../util/types.h"

namespace std {
    constexpr const size_t ERROR_NOT_EXISTS = 1;
    constexpr const size_t ERROR_NOT_EXECUTABLE = 2;
    constexpr const size_t ERROR_FAILED_EXECUTION = 3;
    constexpr const size_t ERROR_NOTHING_MOUNTED = 4;
    constexpr const size_t ERROR_INVALID_FILE_PATH = 5;
    constexpr const size_t ERROR_DIRECTORY = 6;
    constexpr const size_t ERROR_INVALID_FILE_DESCRIPTOR = 7;
    constexpr const size_t ERROR_FAILED = 8;
    constexpr const size_t ERROR_EXISTS = 9;
    constexpr const size_t ERROR_BUFFER_SMALL = 10;
    constexpr const size_t ERROR_INVALID_FILE_SYSTEM = 11;
    constexpr const size_t ERROR_DISK_FULL = 12;
    constexpr const size_t ERROR_PERMISSION_DENIED = 13;
    constexpr const size_t ERROR_INVALID_OFFSET = 14;
    constexpr const size_t ERROR_UNSUPPORTED = 15;
    constexpr const size_t ERROR_INVALID_COUNT = 16;
    constexpr const size_t ERROR_INVALID_REQUEST = 17;
    constexpr const size_t ERROR_INVALID_DEVICE = 18;
    constexpr const size_t ERROR_ALREADY_MOUNTED = 19;
    constexpr const size_t ERROR_UNKNOWN = 20;

    inline const char *error_message(size_t error) {
        switch (error) {
            case ERROR_NOT_EXISTS:
                return "The file does not exist";
            case ERROR_NOT_EXECUTABLE:
                return "The file is not an executable";
            case ERROR_FAILED_EXECUTION:
                return "Execution failed";
            case ERROR_NOTHING_MOUNTED:
                return "Nothing is mounted";
            case ERROR_INVALID_FILE_PATH:
                return "The file path is not valid";
            case ERROR_DIRECTORY:
                return "The file is a directory";
            case ERROR_INVALID_FILE_DESCRIPTOR:
                return "Invalid file descriptor";
            case ERROR_FAILED:
                return "Failed";
            case ERROR_EXISTS:
                return "The file exists";
            case ERROR_BUFFER_SMALL:
                return "The buffer is too small";
            case ERROR_INVALID_FILE_SYSTEM:
                return "Unknown file system";
            case ERROR_DISK_FULL:
                return "The disk is full";
            case ERROR_PERMISSION_DENIED:
                return "Permission denied";
            case ERROR_INVALID_OFFSET:
                return "The offset is not valid";
            case ERROR_UNSUPPORTED:
                return "Unsupported operation: May not be implemented yet";
            case ERROR_INVALID_COUNT:
                return "The count is not valid";
            case ERROR_INVALID_REQUEST:
                return "The request is not valid";
            case ERROR_INVALID_DEVICE:
                return "The device is not valid for this request";
            case ERROR_ALREADY_MOUNTED:
                return "Something is already mounted";
            case ERROR_UNKNOWN:
                return "Unknown error occurred";
            default:
                return "Unknown error";
        }
    }
}