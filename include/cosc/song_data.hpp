#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "proto/MusicVis.capnp.h"

namespace cosc {
/// Encapsulates decoded song data.
class SongData {
    /// Song name
    std::string name;

    /// Decoded FLAC audio samples
    std::vector<int32_t> audio;
    
    /// Cap'n Proto serialised spectrum bars
    MusicVisBars bars;
    
    /**
     * Load song data. This will both load the FLAC file and the Cap'n Proto serialised spectrum.
     * @param dataDir path to data dir
     * @param songName song name
     */
    explicit SongData(const std::string &dataDir, const std::string &songName); 
};
}
