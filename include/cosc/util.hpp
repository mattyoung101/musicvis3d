#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

/// If true, mute audio (mostly for debugging so I don't get sick of the songs!)
#define MUTE 1

namespace cosc::util {

/// Reads the contents of path to a string.
std::string readPathToString(const fs::path &path);

};
