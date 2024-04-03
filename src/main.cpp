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
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#include "cosc/song_data.hpp"

#if FULLSCREEN == 0
static constexpr int WIDTH = 1600;
static constexpr int HEIGHT = 900;
#endif

cosc::CameraPersp camera;
cosc::CameraAnimationManager animationManager(camera);

/// List of bar models
std::vector<cosc::Model> barModels;
/// Capture cursor
bool isCursorCapture = true;
/// Freecam: Allows free movement for debugging
bool isFreeCam = false;
/// Application status
cosc::AppStatus appStatus = cosc::AppStatus::RUNNING;

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
        // make the bars a bit wider
        model.scale.z *= BAR_WIDTH_MULT;
        model.scale.x *= BAR_WIDTH_MULT;
        // y is of course scaled by the visualiser itself

        barModels.push_back(model);
    }
}

static void addAnimations() {
    using namespace cosc;

    // clang-format off
    animationManager.addAnimations({
        CameraAnimation(
            CameraPose(glm::vec3(5.8471, 9.6994, 38.2727), glm::quat(0.9874, -0.1557, -0.0280, -0.0044)),
            CameraPose(glm::vec3(7.0729, 0.8221, 12.5933), glm::quat(0.9998, -0.0140, -0.0130, -0.0002)),
            8.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(-18.6687, 2.6341, -26.7555), glm::quat(0.3377, -0.0160, -0.9401, -0.0447)),
            CameraPose(glm::vec3(-7.5872, 2.6341, 28.1043), glm::quat(0.9736, -0.0354, -0.2255, -0.0082)),
            5.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(-7.8656, 0.0000, 11.5408), glm::quat(0.9156, 0.0051, -0.4020, 0.0022)),
            CameraPose(glm::vec3(22.7046, 0.0000, 10.8279), glm::quat(0.9193, 0.0051, 0.3935, -0.0022)),

            10.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(0.0428, 0.7932, 7.3945), glm::quat(0.9940, -0.0431, -0.1007, -0.0044)),
            CameraPose(glm::vec3(17.3785, 0.7932, 15.6179), glm::quat(0.9738, -0.0313, 0.2251, 0.0072)),
            10.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(-1.4514, -4.1163, 9.5621), glm::quat(0.9471, 0.1828, -0.2591, 0.0500)),
            CameraPose(glm::vec3(14.7786, -1.4914, 8.8891), glm::quat(0.9655, 0.0778, 0.2476, -0.0199)),
            10.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(26.3049, 4.7166, 1.8017), glm::quat(0.6935, -0.1665, 0.6815, 0.1636)),
            CameraPose(glm::vec3(16.4088, 0.9681, 10.5837), glm::quat(0.9584, -0.0264, 0.2839, 0.0078)),
            10.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(7.3091, -16.7948, 16.1069), glm::quat(0.9257, 0.3782, -0.0047, 0.0019)),
            CameraPose(glm::vec3(7.1402, 18.2470, 16.4410), glm::quat(0.9196, -0.3929, -0.0047, -0.0020)),
            10.f
        ),
        CameraAnimation(
            CameraPose(glm::vec3(6.7887, -0.4598, 16.6968), glm::quat(0.9985, 0.0544, 0.0014, -0.0001)),
            CameraPose(glm::vec3(6.6380, -10.7140, 27.1954), glm::quat(0.9787, 0.2052, -0.0082, 0.0017)),
            10.f
        )
    });
    // clang-format on
}

/// Poll SDL events
static void pollInputs() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            appStatus = cosc::AppStatus::QUIT;
        }
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                // press escape or close window to quit
                appStatus = cosc::AppStatus::QUIT;
            }
       if (event.key.keysym.scancode == SDL_SCANCODE_C) {
                isCursorCapture = !isCursorCapture;
                SDL_SetRelativeMouseMode(isCursorCapture ? SDL_TRUE : SDL_FALSE);
                SPDLOG_INFO("Toggle cursor capture");
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                isFreeCam = !isFreeCam;
                SPDLOG_INFO("Toggle freecam");
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_G) {
                // convert euler angles to quaternion: https://gamedev.stackexchange.com/a/13441/72826
                // order is pitch, yaw, roll
                auto angle = camera.getOrientation();
                auto pos = camera.getEyePoint();

                SPDLOG_INFO("CameraPose(glm::vec3({:.4f}, {:.4f}, {:.4f}), "
                            "glm::quat({:.4f}, {:.4f}, {:.4f}, {:.4f}))",
                            pos.x, pos.y, pos.z, angle.w, angle.x, angle.y, angle.z);
            }
        }
        if (event.type == SDL_MOUSEMOTION && isCursorCapture && isFreeCam && cosc::isNotInIntro(appStatus)) {
            camera.processMouseInput(event.motion.xrel, -event.motion.yrel);
        }
    }

    // process continuous held down keys
    auto keyState = SDL_GetKeyboardState(NULL);
    bool boost = keyState[SDL_SCANCODE_LSHIFT];
    if (isCursorCapture) {
        if (keyState[SDL_SCANCODE_W]) {
            camera.processKeyboardInput(cosc::MOVE_FORWARD, delta, boost);
        }
        if (keyState[SDL_SCANCODE_A]) {
            camera.processKeyboardInput(cosc::MOVE_LEFT, delta, boost);
        }
        if (keyState[SDL_SCANCODE_S]) {
            camera.processKeyboardInput(cosc::MOVE_BACKWARD, delta, boost);
        }
        if (keyState[SDL_SCANCODE_D]) {
            camera.processKeyboardInput(cosc::MOVE_RIGHT, delta, boost);
        }
        if (keyState[SDL_SCANCODE_SPACE]) {
            camera.processKeyboardInput(cosc::MOVE_UP, delta, boost);
        }
        if (keyState[SDL_SCANCODE_LCTRL]) {
            camera.processKeyboardInput(cosc::MOVE_DOWN, delta, boost);
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
    SPDLOG_DEBUG("Obtained audio config with freq {} Hz, format {}, channels {}, samples {}", obtained.freq,
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
    SDL_Window *window = SDL_CreateWindow("COSC3000 Major Project (Computer Graphics)", 0, 0, 0, 0,
        SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
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
    //camera = cosc::CameraPersp(realWidth, realHeight, 45.f);
    constructBars(songData, dataDir);
    addAnimations();
    cosc::Cubemap skybox(dataDir, "skybox");
    cosc::Shader barShader(dataDir / "bar.vert.glsl", dataDir / "bar.frag.glsl");
    cosc::Shader quadShader(dataDir / "quad.vert.glsl", dataDir / "quad.frag.glsl");

    // basically capture mouse, for FPS controls
    // note this is different from SDL_CaptureMouse though, but we are emulating the behaviour of what, for
    // example, libGDX would call "capture mouse"
    SDL_SetRelativeMouseMode(isCursorCapture ? SDL_TRUE : SDL_FALSE);

    // setup song data audio stream - after this, audio should be good to go
    songData.setupAudio(audioSpec.format, obtained.format);
    SDL_PauseAudioDevice(audioDevice, 0);

    // manually calculated :skull:
    // x: 3.7500107, y: 0, z: 7.958207
    camera.setEyePoint(glm::vec3(3.7500107f, 0.f, 7.958207));
    camera.lookAt(glm::vec3(0.f, 0.f, 0.f));
    camera.setFov(55.f);
    camera.setNearClip(0.1f);
    camera.setFarClip(200.0f);

    while (cosc::isAppRunning(appStatus)) {
        auto begin = std::chrono::steady_clock::now();

        // process SDL input
        pollInputs();

        // update camera animations
        if (!isFreeCam && cosc::isNotInIntro(appStatus)) {
            animationManager.update(delta);
        }

        // clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable our shader program (before we push uniforms)
        barShader.use();

        // push camera matrices to vertex shader (model transform is pushed later in draw)
        barShader.setMat4("projection", camera.getProjectionMatrix());
        barShader.setMat4("view", camera.getViewMatrix());
        barShader.setVec3("viewPos", camera.getEyePoint());

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
            barIdx++;

            // now update transforms, and off to the GPU we go!
            bar.applyTransform();
            bar.draw(barShader);
        }

        // draw skybox!
        skybox.draw(camera);

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
