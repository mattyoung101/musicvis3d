#include "cosc/camera.hpp"
#include "cosc/model.hpp"
#include "cosc/shader.hpp"
#include "cosc/util.hpp"
#include "glad/gl.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>
#include <cpptrace/cpptrace.hpp>

static constexpr int WIDTH = 1600;
static constexpr int HEIGHT = 900;

cosc::Camera camera;

/// Last frame delta time (seconds)
float delta;
//// Total elapsed time (seconds)
float deltaSum;

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    SPDLOG_INFO("COSC3000 Major Project (Computer Graphics) - Matt Young, 2024");

    if (argc < 2) {
        SPDLOG_ERROR("Usage: {} <DATA_DIR_PATH>", argv[0]);
        return 1;
    }
    std::string dataDir = argv[1];

    SPDLOG_INFO("Data dir: {}", dataDir);

    // init SDL2
    SPDLOG_DEBUG("Initialising SDL2");
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        SPDLOG_ERROR("Failed to init SDL: {}", SDL_GetError());
        return 1;
    }

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

    SDL_Window *window = SDL_CreateWindow("COSC3000 Graphics Minor Project", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SPDLOG_ERROR("Failed to create window: {}", SDL_GetError());
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        SPDLOG_ERROR("Failed to create SDL GL context: {}", SDL_GetError());
    }

    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    SPDLOG_INFO("GL vendor: {}", (const char *) glGetString(GL_VENDOR));
    SPDLOG_INFO("GL renderer: {}", (const char *) glGetString(GL_RENDERER));
    SPDLOG_INFO("GL version: {}", (const char *) glGetString(GL_VERSION));

    // setup GL viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    // setup shaders
    cosc::Shader shader(dataDir + "/vert.glsl", dataDir + "/frag.glsl");

    glEnable(GL_DEPTH_TEST);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // basically capture mouse, for FPS controls
    // note this is different from SDL_CaptureMouse though, but we are emulating the behaviour of what, for
    // example, libgDX would call "capture mouse"
    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool shouldQuit = false;
    while (!shouldQuit) {
        auto begin = std::chrono::steady_clock::now();

        // poll inputs
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
            }
            if (event.type == SDL_MOUSEMOTION) {
                camera.processMouseInput(event.motion.xrel, -event.motion.yrel);
            }
        }

        // clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable our shader program (before we push uniforms)
        shader.use();

        // push camera matrices to vertex shader (model transform is pushed later in draw)
        shader.setMat4("projection", camera.projectionMatrix(WIDTH, HEIGHT));
        shader.setMat4("view", camera.viewMatrix());
        shader.setVec3("viewPos", camera.pos);

        SDL_GL_SwapWindow(window);

        // calculate delta time
        auto end = std::chrono::steady_clock::now();
        // compute in nanoseconds (high resolution) then convert to seconds for delta time
        delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e9;
        deltaSum += delta;
    }

    SPDLOG_DEBUG("Quitting");
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_VideoQuit();
    SDL_Quit();

    return 0;
}
