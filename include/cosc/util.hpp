#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace cosc::util {

/// Reads the contents of path to a string.
std::string readPathToString(const fs::path &path);

};
