#include "cosc/animation.hpp"
#include "cosc/camera.hpp"
#include "cosc/cubemap.hpp"
#include "cosc/model.hpp"
#include "cosc/shader.hpp"
#include "cosc/util.hpp"
#include "glad/gl.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL_audio.h>
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>
#include "cosc/song_data.hpp"
#include <random>

static constexpr int WIDTH = 1600;
static constexpr int HEIGHT = 900;

cosc::Camera camera;
cosc::CameraAnimationManager animationManager(camera);

/// List of bar models
std::vector<cosc::Model> barModels;
/// Capture cursor
bool isCursorCapture = true;
/// Freecam: Allows free movement for debugging
bool isFreeCam = true;
/// SDL event loop control
bool shouldQuit = false;

/// Last frame delta time (seconds)
float delta;
//// Total elapsed time (seconds)
float deltaSum;

/// SDL audio callback
static void audio_callback(void *userData, uint8_t *stream, int len) {
    cosc::SongData *songData = static_cast<cosc::SongData *>(userData);
    songData->mixAudio(stream, len);
}

/// Load and construct bar models. The bar model is based on a unit cube exported from Blender.
static void constructBars(const cosc::SongData &songData, const std::string &dataDir) {
    for (size_t i = 0; i < songData.spectrum.getNumBars(); i++) {
        SPDLOG_DEBUG("Adding bar {}/{}", i, songData.spectrum.getNumBars() - 1);
        auto model = cosc::Model(dataDir + "/cube.dae");
        // apply initial transform
        model.pos.x = BAR_SPACING * i;
        // initial uniform scaling
        model.scale = glm::vec3(BAR_SCALING, BAR_SCALING, BAR_SCALING);
        model.scale.z *= 2;

        barModels.push_back(model);
    }
}

/// Poll SDL events
static void pollInputs() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            shouldQuit = true;
        }
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                // press escape or close window to quit
                shouldQuit = true;
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_G) {
                SPDLOG_INFO("Camera position: {} {} {}", camera.pos.x, camera.pos.y, camera.pos.z);
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                isCursorCapture = !isCursorCapture;
                SDL_SetRelativeMouseMode(isCursorCapture ? SDL_TRUE : SDL_FALSE);
                SPDLOG_INFO("Toggle cursor capture");
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                isFreeCam = !isFreeCam;
                SPDLOG_INFO("Toggle freecam");
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_C) {
                SPDLOG_INFO("Camera pos:\nx: {}, y: {}, z: {}", camera.pos.x, camera.pos.y, camera.pos.z);
            }
        }
        if (event.type == SDL_MOUSEMOTION && isCursorCapture && isFreeCam) {
            camera.processMouseInput(event.motion.xrel, -event.motion.yrel);
        }
    }

    // process continuous held down keys
    auto keyState = SDL_GetKeyboardState(NULL);
    if (isCursorCapture) {
        if (keyState[SDL_SCANCODE_W]) {
            camera.processKeyboardInput(cosc::MOVE_FORWARD, delta);
        }
        if (keyState[SDL_SCANCODE_A]) {
            camera.processKeyboardInput(cosc::MOVE_LEFT, delta);
        }
        if (keyState[SDL_SCANCODE_S]) {
            camera.processKeyboardInput(cosc::MOVE_BACKWARD, delta);
        }
        if (keyState[SDL_SCANCODE_D]) {
            camera.processKeyboardInput(cosc::MOVE_RIGHT, delta);
        }
    }
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    SPDLOG_INFO("COSC3000 Major Project (Computer Graphics) - Matt Young, 2024");

    if (argc < 3) {
        SPDLOG_ERROR("Usage: {} [data_dir_path] [song_name]", argv[0]);
        return 1;
    }

    fs::path dataDir = argv[1];
    std::string songName = argv[2];
    SPDLOG_INFO("Data dir: {}", dataDir.string());
    SPDLOG_INFO("Song name: {}", songName);

    // load song data
    cosc::SongData songData(dataDir, songName);

    // init SDL2
    SPDLOG_DEBUG("Initialising SDL2");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
        SPDLOG_ERROR("Failed to init SDL: {}", SDL_GetError());
        return 1;
    }

    // open audio, reference: https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideaudioexamples.html
    SDL_AudioSpec audioSpec = {
        .freq = static_cast<int>(songData.spectrum.getSampleRate()),
        .format = AUDIO_S32,
        .channels = 2,
        // This needs to be set to a small value, otherwise the number of samples that mixAudio() copies into
        // its output buffer is too many, and we can't figure out which block we're in! (i.e. we skip blocks
        // because we're diving by a larger number).
        // Just don't make this too small otherwise you'll eventually run into choppy audio.
        .samples = 128,
        .callback = audio_callback,
        .userdata = static_cast<void *>(&songData),
    };
    SDL_AudioSpec obtained;

    int audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audioDevice < 0) {
        SPDLOG_ERROR("Failed to initialise SDL audio: {}", SDL_GetError());
        return 1;
    }
    SPDLOG_INFO("Obtained audio config with freq {} Hz, format {}, channels {}, samples {}", obtained.freq,
        obtained.format, obtained.channels, obtained.samples);

    // request OpenGL 4.5, double buffering, and a depth buffer
    // source: https://news.ycombinator.com/item?id=6204597
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // OpenGL 4.5
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // enable MSAA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8); // 8x MSAA
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // use Core profile
    SPDLOG_DEBUG("Loading OpenGL");
    // source: https://bcmpinc.wordpress.com/2015/08/18/creating-an-opengl-4-5-context-using-sdl2-and-glad/
    if (SDL_GL_LoadLibrary(nullptr) < 0) {
        SPDLOG_ERROR("Failed to load OpenGL: {}", SDL_GetError());
        return 1;
    }

#if FULLSCREEN == 0
    SDL_Window *window
        = SDL_CreateWindow("COSC3000 Major Project (Computer Graphics)", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
#else
    SDL_Window *window
        = SDL_CreateWindow("COSC3000 Major Project (Computer Graphics)", 0,
            0, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
    if (window == nullptr) {
        SPDLOG_ERROR("Failed to create window: {}", SDL_GetError());
        return 1;
    }
    SPDLOG_INFO("Using video driver: {}", SDL_GetCurrentVideoDriver());

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        SPDLOG_ERROR("Failed to create SDL GL context: {}", SDL_GetError());
    }

    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    SPDLOG_INFO("GL vendor: {}", (const char *) glGetString(GL_VENDOR));
    SPDLOG_INFO("GL renderer: {}", (const char *) glGetString(GL_RENDERER));
    SPDLOG_INFO("GL version: {}", (const char *) glGetString(GL_VERSION));

    // setup baseline GL stuff
    int realWidth;
    int realHeight;
    SDL_GetWindowSizeInPixels(window, &realWidth, &realHeight);
    glViewport(0, 0, realWidth, realHeight);
    glEnable(GL_DEPTH_TEST);
#if WIREFRAME == 1
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    // setup our custom GL objects
    constructBars(songData, dataDir);
    //cosc::Cubemap cubemap(dataDir, "skybox");
    cosc::Shader barShader(dataDir / "bar.vert.glsl", dataDir / "bar.frag.glsl");

    // basically capture mouse, for FPS controls
    // note this is different from SDL_CaptureMouse though, but we are emulating the behaviour of what, for
    // example, libGDX would call "capture mouse"
    SDL_SetRelativeMouseMode(isCursorCapture ? SDL_TRUE : SDL_FALSE);

    // setup song data audio stream - after this, audio should be good to go
    songData.setupAudio(audioSpec.format, obtained.format);
    SDL_PauseAudioDevice(audioDevice, 0);

    // manually calculated :skull:
    // x: 3.7500107, y: 0, z: 7.958207
    camera.pos.x = 3.7500107;
    camera.pos.z = 7.958207;

    while (!shouldQuit) {
        auto begin = std::chrono::steady_clock::now();

        // process SDL input
        pollInputs();

        // update camera animations
        if (!isFreeCam) {
            animationManager.update(delta);
        }

        // clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable our shader program (before we push uniforms)
        barShader.use();

        // push camera matrices to vertex shader (model transform is pushed later in draw)
        barShader.setMat4("projection", camera.projectionMatrix(WIDTH, HEIGHT));
        barShader.setMat4("view", camera.viewMatrix());
        barShader.setVec3("viewPos", camera.pos);

        // current bar we're editing
        size_t barIdx = 0;
        // current spectrum block
        // note that songData.blockPos gets updated by mixAudio() (FIXME possible race condition?)
        auto block = songData.spectrum.getBlocks()[songData.blockPos];
        for (auto &bar : barModels) {
            // first, get bar height from 0-255 directly from the Cap'n Proto
            auto barHeight = block[barIdx];
            // map that 0 to 255 to BAR_MIN_HEIGHT to BAR_MAX_HEIGHT
            auto scale = cosc::util::mapRange(0., 255., BAR_MIN_HEIGHT, BAR_MAX_HEIGHT, barHeight);
            // apply scale, also applying our baseline BAR_SCALING factor!
            bar.scale.y = scale * BAR_SCALING;
            //bar.pos.y = scale;
            barIdx++;

            // now update transforms, and off to the GPU we go!
            bar.applyTransform();
            bar.draw(barShader);
        }

        SDL_GL_SwapWindow(window);

        // calculate delta time
        auto end = std::chrono::steady_clock::now();
        // compute in nanoseconds (high resolution) then convert to seconds for delta time
        delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e9;
        deltaSum += delta;
        SPDLOG_TRACE("Delta: {:.2f} ms", delta * 1000.0);
    }

    SPDLOG_DEBUG("Quitting");
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_CloseAudio();
    SDL_VideoQuit();
    SDL_AudioQuit();
    SDL_Quit();

    return 0;
}
