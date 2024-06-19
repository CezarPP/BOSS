/*
 * path.h
 *
 *  Created on: 5/19/24.
 *      Author: Cezar PP
 */

#pragma once

#include "std/string.h"

/**
 * @brief Structure to represent a path on the file system
 */
struct Path {
    using position_t = uint16_t; ///< The position type

    /**
     * @brief Construct an empty path.
     * Such path is invalid
     */
    Path() = default;

    explicit Path(std::string_view path);

    Path(const Path &base_path, std::string_view path);

    Path(const Path &base_path, const Path &p);

    Path(const Path &) = default;

    Path(Path &&) = default;

    Path &operator=(const Path &) = default;

    Path &operator=(Path &&) = default;

    Path &operator=(std::string_view rhs);

    // Conversion functions

    [[nodiscard]] std::string_view string() const;

    // Modifiers

    /*
     * \brief Makes the path invalid
     */
    void invalidate();

    // Accessors

    /**
     * @brief Returns true if the path is empty.
     * Such a path will be invalid
     */
    [[nodiscard]] bool empty() const;

    [[nodiscard]] bool is_root() const;

    [[nodiscard]] bool is_valid() const;

    /**
     * @brief Returns true if the path is absolute and has only 2 components.
     */
    [[nodiscard]] bool is_sub_root() const;

    /**
     * @brief Returns the number of elements of the path
     */
    [[nodiscard]] size_t size() const;

    /**
     * @brief Returns the base name (the last part)
     */
    [[nodiscard]] std::string_view base_name() const;

    /**
     * @brief Returns the root name (the first part)
     */
    [[nodiscard]] std::string_view root_name() const;

    /**
     * @brief Returns the sub root name (the second part)
     */
    [[nodiscard]] std::string_view sub_root_name() const;

    [[nodiscard]] bool is_absolute() const;

    [[nodiscard]] bool is_relative() const;

    // Accessors to sub parts

    /**
     * @brief Returns the ith part of the path
     */
    [[nodiscard]] std::string_view name(size_t i) const;

    /**
     * @brief Returns the ith part of the path
     */
    std::string_view operator[](size_t i) const;

    // Decomposition functions

    /**
     * @brief Returns the path minus the i first elements
     */
    Path sub_path(size_t i) const;

    /**
     * @brief Returns the path minus the last element
     */
    [[nodiscard]] Path branch_path() const;

    bool operator==(const Path &p) const;

    bool operator!=(const Path &p) const;

    bool operator==(std::string_view p) const;

    bool operator!=(std::string_view p) const;

private:
    std::string base;
    std::vector<position_t> positions;
};

Path operator/(const Path &lhs, const Path &rhs);

Path operator/(const Path &lhs, std::string_view rhs);

Path operator/(const Path &lhs, const char *rhs);

Path operator/(std::string_view lhs, const Path &rhs);
