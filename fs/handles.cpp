/*
 * handles.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

#include "fs/handles.h"
#include "std/vector.h"

namespace handles {
    std::vector<fd_t> handles;
    constexpr fd_t INVALID_HANDLE = 0xFFFFFFFF;

    size_t register_new_handle(const fd_t &p) {
        handles.push_back(p);
        return handles.size();
    }

    void release_handle(size_t fd) {
        handles[fd - 1] = INVALID_HANDLE;
    }

    bool has_handle(size_t fd) {
        return fd > 0 && fd <= handles.size() && handles[fd - 1] != INVALID_HANDLE;
    }

    const fd_t &get_handle(size_t fd) {
        return handles[fd - 1];
    }
}
