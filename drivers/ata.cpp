/*
 * ata.cpp
 *
 *  Created on: 6/12/24.
 *      Author: Cezar PP
 */

#include "drivers/ata.h"

namespace ata {
    volatile bool Ata::primary_invoked = false;
}