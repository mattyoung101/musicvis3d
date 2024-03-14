#include "cosc/util.hpp"
#include <cpptrace/cpptrace.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

std::vector<fs::path> cosc::util::findLoadableModels(const std::string &dataDir) {
    SPDLOG_DEBUG("Finding loadable models in dir {}", dataDir);
    fs::path dataDirPath = dataDir;

    if (!fs::is_directory(dataDirPath)) {
        throw cpptrace::invalid_argument("Data dir is not directory");
    }

    std::vector<fs::path> out {};
    for (const auto &entry : fs::recursive_directory_iterator(dataDirPath)) {
        if (entry.path().extension() == ".obj") {
            SPDLOG_DEBUG("Found obj file {}", entry.path().string());
            out.emplace_back(entry.path());
        }
    }
    return out;
}

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
