// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#pragma once
#include "cosc/lib/dr_flac.h"
#include "cosc/util.hpp" // this is used, but clang-tidy cannot detect it correctly
#include "proto/MusicVis.capnp.h"
#include <SDL2/SDL_audio.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <cstdint>
#include <memory>
#include <string>

namespace cosc {
/// Encapsulates decoded song data.
class SongData {
public:
    /**
     * Load song data. This will both load the FLAC file and the Cap'n Proto serialised spectrum.
     * @param dataDir path to data dir
     * @param songName song name
     */
    explicit SongData(const fs::path &dataDir, const fs::path &songName);
    ~SongData();

    // TODO other constructors

    /// Sets up the SDL audio stream with the obtained audio config from the sound driver.
    void setupAudio(SDL_AudioFormat wanted, SDL_AudioFormat obtained);

    /**
     * Requests the mixing of 'len' bytes into the buffer `stream`.
     * If the audio playback is finished, no bytes will be mixed.
     */
    void mixAudio(uint8_t *stream, int len);

    /// Song name
    std::string name;

    /// Deserialised music vis spectrum data
    MusicVisBars::Reader spectrum;

    /// Current audio position in samples
    size_t audioPos = 0;

    /// Current audio position in spectrum blocks
    size_t blockPos = 0;

private:
    /// Capnp message reader
    std::unique_ptr<::capnp::PackedFdMessageReader> reader;
    /// Cap'n Proto serialised spectrum bars
    ::capnp::MallocMessageBuilder message;
    /// Capnp file FD
    int fd = -1;

    unsigned int channels;
    unsigned int sampleRate;
    drflac_int32 *audio;
    /// Audio size in samples
    drflac_uint64 audioLen = 0;
    SDL_AudioStream *audioStream;
};
} // namespace cosc
