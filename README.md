# COSC3000 Major Project: Computer Graphics
_by Matt Young_

This is a semi real-time 3D audio visualisation using OpenGL. The visualisation consists of offline spectral
data that is rendered in real-time in the form of 3D bars. A multitude of graphics techniques are used,
including: quaternion camera animation, camera shake using Simplex noise under fractal Brownian motion, a
skybox, and a post-processing stage that implements chromatic aberration. The application is written in a mix
of C++ (for rendering) and Python (for DSP). The spectrum of bars is computed using the Fast Fourier
Transform.

This is my major project in computer graphics for COSC3000, done during UQ Sem 1 2024. The code itself is
based on a fork of the graphics minor project, completed earlier in the semester.

For more information, see [paper/major.pdf](paper/major.pdf).

## Building and running
You will need the following tools/libraries: 
- CMake 3.20+
- Ninja
- LLVM
- Clang
- LLD
- SDL2
- glm
- Assimp
- spdlog
- Cap'n Proto

All the above dependencies are available on the AUR or main repos for Arch Linux. If you're using another
distro, you may have to compile from source to get the newer versions (especially if on Ubuntu). If you're
not on Linux, you're on your own! In particular, this may be challenging to build on MacOS due to their
deprecation of OpenGL.

Generate the project using:
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release # or Debug
```

Build (in `build` directory): `ninja`

Run: `./musicvis ../data BandName_SongName`

In the fish shell, use `set -x ASAN_OPTIONS detect_leaks=0` to silence LeakSanitizer which has false positives
under SDL2.

## Importing new audio
New songs can be imported into the visualiser.

First, make a new directory in `data/songs`, for example, `mkdir data/songs/Band_Name_Song_Name`.

Your song needs to be available as a FLAC file. Copy this into `data/songs/Band_Name_Song_Name/audio.flac`.

Finally, you can activate the virtual environment and run the process script:

```
python -m venv env
source env/bin/activate.fish
pip install -r requirements.txt
./scripts/process.py Band_Name_Song_Name
```

This will then write the `spectrum.bin` file in the Cap'n Proto format.

To double check this worked, run `./scripts/decoder.py Band_Name_Song_Name`. This will load the capnp message
in `spectrum.bin` and display it.

## Compiling the paper
The paper is written in [Typst](https://github.com/typst/typst). In the `paper` directory, you can run `typst
compile major.typ` to produce `major.pdf`.

## Libraries used
This project makes use of the following open-source libraries:

- SDL2: zlib licence
- glm: MIT licence
- Assimp: BSD 3-Clause
- spdlog: MIT licence
- Cap'n Proto: MIT licence
- dr_flac: Public domain
- stb_image: Public domain
- `camera.cpp`/`camera.hpp` derived from Cinder Project: BSD 2-Clause
    - Upstream: https://github.com/cinder/Cinder/blob/master/src/cinder/Camera.cpp
- `Simplex.h`: BSD 3-Clause
    - Upstream: https://github.com/simongeilfus/SimplexNoise

Some code based on code from LearnOpenGL https://learnopengl.com/ - however, given the generic nature of the
code (i.e. there is really only one way to instantiate a GL shader), I do not believe that their CC-BY-NC
licence applies to this repo. The LOGL content was used _as reference_ only.

## Assets used
The space skybox was generated from: https://tools.wwwtyro.net/space-3d/index.html

The rest of the assets are made by me.

## Licence
TBA
