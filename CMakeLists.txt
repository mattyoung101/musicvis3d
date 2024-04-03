cmake_minimum_required(VERSION 3.20)
project(musicvis C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11) # C11
set(CMAKE_CXX_STANDARD 20) # C++20
Set(FETCHCONTENT_QUIET FALSE)

# cap'n proto
# based on https://github.com/capnproto/capnproto/blob/v2/c%2B%2B/samples/CMakeLists.txt
find_package(CapnProto CONFIG REQUIRED)
capnp_generate_cpp(musicVisProtoSources musicVisProtoHeaders proto/MusicVis.capnp)

add_executable(musicvis 
    src/main.cpp 
    src/lib/gl.c
    src/util.cpp
    src/mesh.cpp
    src/shader.cpp
    src/model.cpp
    src/lib/dr_flac.c
    src/song_data.cpp
    src/animation.cpp
    src/lib/stb_image.c
    src/cubemap.cpp
    src/camera.cpp
    ${musicVisProtoSources}
)
target_include_directories(musicvis PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(musicvis CapnProto::capnp)
target_compile_definitions(musicvis PRIVATE "-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
message(STATUS "Music vis headers: ${musicVisProtoHeaders}")

# SDL2
find_package(SDL2 REQUIRED)
target_include_directories(musicvis PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(musicvis ${SDL2_LIBRARIES})

# spdlog
find_package(spdlog REQUIRED)
target_link_libraries(musicvis spdlog::spdlog)

# assimp
find_package(assimp REQUIRED)
target_link_libraries(musicvis assimp::assimp)
target_include_directories(musicvis PRIVATE ${ASSIMP_INCLUDE_DIRS})

# glm
find_package(glm REQUIRED)
target_link_libraries(musicvis glm::glm)
target_include_directories(musicvis PRIVATE ${GLM_INCLUDE_DIRS})

target_compile_options(musicvis PRIVATE "-Wall" "-Wextra" "-Wno-unused-parameter" "-ggdb")
target_compile_options(musicvis PRIVATE "-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")

if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
    message(STATUS "Release build")
    target_compile_options(musicvis PRIVATE "-O3" "-march=native" "-mtune=native")
    target_link_options(musicvis PRIVATE "-flto=thin")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    message(STATUS "Debug build, adding sanitizers")
    target_compile_options(musicvis PRIVATE "-fsanitize=address" "-fsanitize=undefined"
        "-fno-omit-frame-pointer")
    target_link_options(musicvis PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif()

# Force LLD
add_link_options("-fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
target_link_options(musicvis PRIVATE "-fuse-ld=lld")
