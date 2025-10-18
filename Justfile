default:
    @just -l

# very annoying SDL build quirk that stupidly uses -Werror, so we have to silence that here
conan:
    CFLAGS="-Wno-incompatible-pointer-types" CXXFLAGS="-Wno-incompatible-pointer-types" conan install . \
        --output-folder=build --build=missing -c tools.system.package_manager:mode=install -c \
        tools.system.package_manager:sudo=True

cmake: conan
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"

build: cmake
    cd build; ninja
