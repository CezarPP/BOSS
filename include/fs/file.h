/*
 * file.h
 *
 *  Created on: 5/28/24.
 *      Author: Cezar PP
 */


#pragma once

#include "../std/string.h"

namespace vfs {
    struct file {
        std::string fileName;
        bool isFile{};
        bool isValid{};
        uint64_t size{};

        size_t inum{};

        file() = default;

        file(std::string file_name, bool isFile, uint64_t inum)
                : fileName(std::move(file_name)), isFile(isFile), inum(inum) {
            this->isValid = true;
        };

        void invalidate() {
            this->isValid = false;
        }
    };
}