#pragma once
#include <SDL2/SDL_audio.h>
#include <cstdint>
#include <memory>
#include <string>
#include "cosc/lib/dr_flac.h"
#include "cosc/util.hpp"
#include "proto/MusicVis.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

namespace cosc {
/// Encapsulates decoded song data.
class SongData {
public:
    /// Song name
    std::string name;
    
    /// Deserialised music vis spectrum data
    MusicVisBars::Reader spectrum;

    /**
     * Load song data. This will both load the FLAC file and the Cap'n Proto serialised spectrum.
     * @param dataDir path to data dir
     * @param songName song name
     */
    explicit SongData(const std::string &dataDir, const std::string &songName); 
    ~SongData();
    
    /// Sets up the SDL audio stream with the obtained audio config from the sound driver.
    void setupAudio(SDL_AudioFormat wanted, SDL_AudioFormat obtained);
    
    /**
     * Requests the mixing of 'len' bytes into the buffer `stream`.
     * If the audio playback is finished, no bytes will be mixed.
     */
    void mixAudio(uint8_t *stream, int len);

    /**
     * Predicts which spectrum block we are currently in based on timings to mixAudio().
     * Must only be called if canPredictAudio is true!
     */
    uint32_t predictSpectrumBlockPos();

    /// True if we could possibly predict audio pos (i.e. we have called `mixAudio` at least once)
    bool canPredictAudio = false;
private:
    /// Capnp message reader
    std::unique_ptr<::capnp::PackedFdMessageReader> reader;
    /// Cap'n Proto serialised spectrum bars
    ::capnp::MallocMessageBuilder message;
    /// Capnp file FD
    int fd = -1;

    /// Last time mixAudio was called (beginning of the function)
    util::SteadyTimePoint_t lastMixAudioCall;
    /// Time between mixAudio() calls in nanoseconds
    util::NanoDuration_t mixAudioTime;
    /// Number of samples we provided in mixAudio()
    uint64_t mixAudioSamples;

    /// Audio pos **IN BYTES**
    uint64_t audioPos;

    unsigned int channels;
    unsigned int sampleRate;
    /// Audio samples
    drflac_int32 *audio;
    /// Audio size in samples
    drflac_uint64 audioLen = 0;
    SDL_AudioStream *audioStream;
};
}
