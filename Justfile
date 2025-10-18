default:
    @just -l

conan:
    conan install . --output-folder=build --build=missing

cmake: conan
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"

build: cmake
    cd build; ninja
