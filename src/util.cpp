// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#include "cosc/util.hpp"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace fs = std::filesystem;

std::string cosc::util::readPathToString(const fs::path &path) {
    SPDLOG_DEBUG("Read path {} to string", path.string());
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path.string());

    std::stringstream stream;
    stream << file.rdbuf();
    file.close();

    return stream.str();
}
