/*
 * handles.h
 *
 *  Created on: 6/7/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"

namespace handles {
    using fd_t = size_t;

    size_t register_new_handle(const fd_t &p);

    void release_handle(size_t fd);

    bool has_handle(size_t fd);

    const fd_t &get_handle(size_t fd);
}