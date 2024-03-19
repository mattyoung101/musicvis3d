#pragma once
#include <SDL2/SDL_audio.h>
#include <cstdint>
#include <memory>
#include <string>
#include "cosc/lib/dr_flac.h"
#include "proto/MusicVis.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

namespace cosc {
/// Encapsulates decoded song data.
class SongData {
public:
    /// Song name
    std::string name;
    
    /// Cap'n Proto serialised spectrum bars
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

    /// Current audio position in samples
    size_t audioPos = 0;
    
    /// Current audio position in spectrum blocks
    size_t blockPos = 0;

private:
    std::unique_ptr<::capnp::PackedFdMessageReader> message;
    int fd = -1;
    unsigned int channels;
    unsigned int sampleRate;
    drflac_int32 *audio;
    /// Audio size in samples
    drflac_uint64 audioLen = 0;
    SDL_AudioStream *audioStream;
};
}
