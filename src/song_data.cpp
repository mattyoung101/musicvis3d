#include "cosc/song_data.hpp"
#include <SDL2/SDL_audio.h>
#include <cpptrace/cpptrace.hpp>
#include <cstring>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include "cosc/lib/dr_flac.h"
#include <fcntl.h>
#include "proto/MusicVis.capnp.h"
#include <algorithm>

namespace fs = std::filesystem;

cosc::SongData::SongData(const std::string &dataDir, const std::string &songName) {
    fs::path flacFile = dataDir + "/songs/" + songName + "/audio.flac";
    fs::path spectrumFile = dataDir + "/songs/" + songName + "/spectrum.bin";

    if (!fs::exists(flacFile)) {
        SPDLOG_ERROR("FLAC file does not exist! Tried: {}", flacFile.string());
        throw cpptrace::runtime_error("FLAC file not found");
    }
    if (!fs::exists(spectrumFile)) {
        SPDLOG_ERROR("Spectrum data does not exist! Tried: {}", spectrumFile.string());
        throw cpptrace::runtime_error("Spectrum file not found");
    }

    SPDLOG_INFO("FLAC file: {}", flacFile.string());
    SPDLOG_INFO("Spectrum file: {}", spectrumFile.string());

    // load FLAC file with dr_flac
    SPDLOG_DEBUG("Decoding FLAC file");
    audio = drflac_open_file_and_read_pcm_frames_s32(
        flacFile.c_str(), &channels, &sampleRate, &audioLen, nullptr);
    if (audio == nullptr) {
        // Failed to open and decode FLAC file.
        throw cpptrace::runtime_error("Failed to decode FLAC file");
    }
    SPDLOG_DEBUG("Audio len: {} samples", audioLen);

    // load spectrum data with capnp
    // FIXME: MOVE these into a MallocMessageBuilder so that capnp stops blowing up
    // see: https://capnproto.org/cxx.html (Security Tips)
    SPDLOG_DEBUG("Opening spectrum fd");
    fd = open(spectrumFile.c_str(), O_RDONLY);
    if (fd == -1) {
        SPDLOG_ERROR("Failed to open() spectrum data: {}", strerror(errno));
        throw cpptrace::runtime_error("Failed to open spectrum data");
    }

    SPDLOG_DEBUG("Decoding capnp message");
    // this is only a unique_ptr to keep it alive and since they are annoying and disabled copy&move
    // there's probably a better way to do this but seriously do you know how many assignments I have rn???
    message = std::make_unique<::capnp::PackedFdMessageReader>(fd);
    spectrum = message->getRoot<MusicVisBars>();

    SPDLOG_INFO("===== Decoded spectrum data =====");
    SPDLOG_INFO("Num bars: {}", spectrum.getNumBars());
    SPDLOG_INFO("Sample rate: {} Hz", spectrum.getSampleRate());
    SPDLOG_INFO("Block size: {} samples", spectrum.getBlockSize());
    SPDLOG_INFO("Num blocks: {}", spectrum.getBlocks().size());
}

void cosc::SongData::setupAudio(SDL_AudioFormat wanted, SDL_AudioFormat obtained) {
    audioStream = SDL_NewAudioStream(wanted, channels, sampleRate, obtained, channels, sampleRate);
    // shove all the data through the stream
    // seems as though we have to do x2 since there are two stereo channels? and audioLen is PCM frames?
    SDL_AudioStreamPut(audioStream, audio, audioLen * sizeof(int32_t) * channels);
    // ask it to resample for us while we're not under realtime
    SDL_AudioStreamFlush(audioStream);
}

void cosc::SongData::mixAudio(uint8_t *stream, int len) {
    // make sure 'len' isn't larger than the amount of data we can pull from the stream
    auto maxInStream = SDL_AudioStreamAvailable(audioStream);
    len = std::min(len, maxInStream);

    if (len == 0) {
        // this should be OK as long as the song is actually finished - SDL requires us to fill the buffer
        // even if the song is done
        //SPDLOG_WARN("Writing zero bytes to audio stream! Glitch will occur! len: {}, maxInStream: {}", len, maxInStream);
        std::memset(stream, 0, len);
        return;
    }
    
    // send data to the sound driver
    SDL_AudioStreamGet(audioStream, stream, len);
    
    // we are given len in bytes, and since if we have sint16 samples, we just divide by the sizeof(sint16)
    // this may also require a divide by channels?
    audioPos += len / 2 / sizeof(int16_t);
    // and the block position should then be that divided by the block size
    blockPos = audioPos / spectrum.getBlockSize();
    SPDLOG_DEBUG("Sample position: {}, Block position: {}/{}", audioPos, blockPos, spectrum.getBlocks().size());
}

cosc::SongData::~SongData() {
    SPDLOG_DEBUG("Closing spectrum fd");
    // Because cap'n proto is stupid, we have to keep the fd alive for the duration of the entire application.
    // Since this ~SongData() destructor gets called and we free the fd **before** we the message unique_ptr
    // destructor runs, we have to dispose it here **manually**, then hopefully it'll quit being mad at us.
    message.reset();
    close(fd);
    SDL_FreeAudioStream(audioStream);
    drflac_free(audio, nullptr);
}
