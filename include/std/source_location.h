/*
 * source_location.h
 *
 *  Created on: 4/11/24.
 *      Author: Cezar PP
 */

#pragma once

#include "../util/types.h"

namespace std {
    class source_location {
    public:
        // consteval since this MUST be evaluated at compile-time
        static consteval source_location current(const char *fileName = __builtin_FILE(),
                                                 const char *functionName = __builtin_FUNCTION(),
                                                 const uint_least32_t lineNumber = __builtin_LINE(),
                                                 const uint_least32_t columnOffset = 0) noexcept {
            return {fileName, functionName, lineNumber, columnOffset};
        }

        source_location(const source_location &) = default;

        source_location(source_location &&) = default;

        [[nodiscard]] constexpr const char *file_name() const noexcept {
            return fileName;
        }

        [[nodiscard]] constexpr const char *function_name() const noexcept {
            return functionName;
        }

        [[nodiscard]] constexpr uint32_t line() const noexcept {
            return lineNumber;
        }

        [[nodiscard]] constexpr uint32_t column() const noexcept {
            return columnOffset;
        }

    private:
        constexpr source_location(const char *fileName, const char *functionName, const uint32_t lineNumber,
                                  const uint32_t columnOffset) noexcept
                : fileName(fileName), functionName(functionName), lineNumber(lineNumber), columnOffset(columnOffset) {
        }

        const char *fileName;
        const char *functionName;
        const uint32_t lineNumber;
        const uint32_t columnOffset;
    };
}

