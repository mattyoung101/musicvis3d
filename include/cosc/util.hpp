#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace cosc::util {

/// Finds loadable OBJ models in the given directory.
std::vector<fs::path> findLoadableModels(const std::string &dataDir);

/// Reads the contents of path to a string.
std::string readPathToString(const fs::path &path);

};
