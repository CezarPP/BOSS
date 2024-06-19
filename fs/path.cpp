/*
 * path.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

#include "../include/fs/path.h"
#include "arch/x86_64/exceptions.h"

void update_positions(std::string &base, std::vector<Path::position_t> &positions) {
    positions.clear();

    if (base.empty()) {
        return;
    }

    for (int i = base.size() - 2; i >= 0; --i) {
        if (base[i] == '/' && base[i + 1] == '/') {
            base.erase(size_t(i + 1));
        }
    }

    if (base.back() != '/') {
        base += '/';
    }

    for (size_t i = 0; i < base.size(); ++i) {
        if (base[i] == '/') {
            positions.push_back(i);
        }
    }
}

Path::Path(std::string_view path) : base(path.begin(), path.end()) {
    kAssert(path.size(), "[PATH] Invalid base path");

    update_positions(base, positions);
}

Path::Path(const Path &base_path, std::string_view p) : base(base_path.base) {
    kAssert(p.empty() || p[0] != '/', "[PATH] Impossible to add absolute path to another path");

    base += std::string(p.begin(), p.end());

    update_positions(base, positions);
}

Path::Path(const Path &base_path, const Path &p) : base(base_path.base) {
    kAssert(p.is_relative(), "[PATH] Impossible to add absolute path to another path");
    base += p.base;

    update_positions(base, positions);
}

Path &Path::operator=(std::string_view rhs) {
    base = rhs;

    update_positions(base, positions);

    return *this;
}

std::string_view Path::string() const {
    return {base.c_str(), base.size()};
}

void Path::invalidate() {
    base = "//";
    positions.clear();
}

bool Path::empty() const {
    return base.empty();
}

bool Path::is_root() const {
    return base == "/";
}

bool Path::is_valid() const {
    return !base.empty() && base != "//";
}

bool Path::is_sub_root() const {
    return is_absolute() && positions.size() == 2;
}

size_t Path::size() const {
    return positions.size();
}

std::string_view Path::base_name() const {
    if (empty()) {
        return "";
    }

    if (is_root()) {
        return "/";
    }

    if (is_absolute()) {
        return {base.begin() + positions[size() - 2] + 1,
                base.size() - 1 - static_cast<size_t>(positions[size() - 2] + 1)};
    }

    if (size() == 1) {
        return {base.begin(), base.size() - 1};
    } else {
        return {base.begin() + positions[size() - 2] + 1,
                base.size() - 1 - static_cast<size_t>(positions[size() - 2] + 1)};
    }
}

std::string_view Path::root_name() const {
    if (empty()) {
        return "";
    }

    if (is_absolute()) {
        return "/";
    }

    return {base.begin(), positions.front()};
}

std::string_view Path::sub_root_name() const {
    kAssert(is_absolute(), "[PATH] sub_root_name() does not make sense on relative path");

    if (size() < 2) {
        return "";
    }

    return {base.begin() + 1, static_cast<size_t>(positions[1]) - 1};
}

bool Path::is_absolute() const {
    return !base.empty() && base[0] == '/';
}

bool Path::is_relative() const {
    return !is_absolute();
}

std::string_view Path::name(size_t i) const {
    if (is_absolute()) {
        if (i == 0) {
            return "/";
        }

        return {base.begin() + positions[i - 1] + 1, size_t(positions[i] - 1) - positions[i - 1]};
    }

    if (i == 0) {
        return {base.begin(), positions[i]};
    }

    return {base.begin() + positions[i - 1] + 1, size_t(positions[i] - 1) - positions[i - 1]};
}

std::string_view Path::operator[](size_t i) const {
    return name(i);
}

Path Path::sub_path(size_t i) const {
    if (i == 0) {
        return *this;
    }

    Path p;

    if (is_absolute()) {
        if (i == size()) {
            return p;
        }

        p.base.assign(base.begin() + positions[i - 1] + 1, base.end());
    } else {
        if (i == size()) {
            return p;
        }

        p.base.assign(base.begin() + positions[i - 1] + 1, base.end());
    }

    update_positions(p.base, p.positions);

    return p;
}

Path Path::branch_path() const {
    if (empty()) {
        return *this;
    }

    if (is_root()) {
        return *this;
    }

    if (is_relative() && size() == 1) {
        return *this;
    }

    Path p;

    if (is_relative()) {
        p.base.assign(base.begin(), base.begin() + positions[size() - 2]);
    } else {
        p.base.assign(base.begin(), base.begin() + positions[size() - 2]);
    }

    update_positions(p.base, p.positions);

    return p;
}

bool Path::operator==(const Path &p) const {
    return base == p.base;
}

bool Path::operator!=(const Path &p) const {
    return base != p.base;
}

bool Path::operator==(std::string_view p) const {
    Path rhs(p);
    return *this == rhs;
}

bool Path::operator!=(std::string_view p) const {
    return !(*this == p);
}

Path operator/(const Path &lhs, const Path &rhs) {
    return {lhs, rhs};
}

Path operator/(const Path &lhs, std::string_view rhs) {
    return {lhs, rhs};
}

Path operator/(const Path &lhs, const char *rhs) {
    return {lhs, rhs};
}

Path operator/(std::string_view lhs, const Path &rhs) {
    return {Path(lhs), rhs};
}
