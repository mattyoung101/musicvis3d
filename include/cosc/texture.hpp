#pragma once
#include <string>

namespace cosc {
// FIXME: This may be unused (only used by mesh.cpp commented out block)
struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};
} // namespace cosc
