// Colour links blue like LaTeX
#show cite: set text(fill: blue)
#show link: set text(fill: blue)
#show ref: set text(fill: blue)

#set list(indent: 12pt)
#set heading(numbering: "1.")

#align(center, text(18pt)[
    *A 3D music visualisation in OpenGL using the Discrete Fourier Transform*
])

#align(center, text(12pt)[
    Matt Young

    46972495

    m.young2\@uqconnect.edu.au

    May 2024
])

#set par(justify: true)
#align(center)[
    *Abstract*

    Constructing computer graphics from music has important implications in the field of live entertainment.
    The Discrete Fourier Transform (DFT), often computed via the Fast Fourier Transform (FFT), is the typical
    method to convert time domain audio signals to a frequency domain spectrum. With this spectral data comes
    an almost unlimited number of ways to interpret it and construct a visualisation. In this paper, I
    investigate applying the DFT to construct a semi real-time audio visualisation using OpenGL. The
    visualisation consists of offline spectral data that is rendered in real-time in the form of 3D bars, and
    a multitude of graphics techniques including quaternion camera animation, camera shake using fractal
    Brownian motion Simplex noise, and a post-processing stage that implements chromatic aberration. The
    application is written in a mix of C++ and Python.
]

// Create a scope for this style: https://typst.app/docs/reference/styling/
#[
    // https://typst.app/docs/reference/model/outline/#definitions-entry
    #show outline.entry.where(
      level: 1
    ): it => {
      v(12pt, weak: true)
      strong(it)
    }

    #outline(
        // https://www.reddit.com/r/typst/comments/1bp8zty/how_to_make_dots_in_outline_spaced_the_same_for/
        fill: box(width: 1fr, repeat(h(5pt) + "." + h(5pt))) + h(8pt),
        indent: auto
    )
]

#outline(
    title: [List of Figures],
    fill: box(width: 1fr, repeat(h(5pt) + "." + h(5pt))) + h(8pt),
    target: figure,
)
#pagebreak()

= Introduction
A _music visualiser_ is a computer program that takes as input an audio signal, typically music, and
produces as output a graphical visualisation. Technically, this can be an entirely offline process, but most
often music visualisers run in real-time. Music visualisers have an important role in the live entertainment
scene @mccarthy2022live, particularly for Electronic Dance Music (EDM). With an almost infinite number
of ways to interpret an audio signal, music visualisation is an eclectic mix of art and engineering.

The particular music visualiser I aim to construct is an improved version of the "spectrum of bars" once used
by Canadian record label Monstercat, shown in @fig:monstercat.
The label has since transitioned away from this computational music visualiser, and instead use custom music videos.

#figure(
    image("img/monstercat.png", width: 80%),
    caption: [ Visualisation of the song "Pure Sunlight" @youtubeElectronicFijiWiji ]
) <fig:monstercat>

The visualisation in @fig:monstercat is a common type of visualisation that visualises the audio spectrum - it
shows the magnitudes of the varying frequencies that make up the song. This spectral data can be interpreted
in a number of ways, for example, the MilkDrop @milkdrop software includes visualisations that encode this in
a number of complex ways. However, for our use case, bars are visually appealing and an intuitive approach to
visualising the music.

= Architecture overview
== Overall description of sub-applications
The visualiser itself is developed and tested in a Linux environment, and is split into two sub-applications:
the visualiser itself (written in C++20), and the analysis script (written in Python). Signal processing is a
very complex subject, and real-time ("online") signal processing in C++ is even more so. Python generally has
simpler tools to address signal processing problems, such as NumPy and SciPy, so the signal processing part
was moved offline into a Python script. This is shown in @fig:block:

#figure(
    image("img/diagram.drawio.svg", width: 60%),
    caption: [Block diagram of music visualisation application]
) <fig:block>

The analysis script is first run that reads a FLAC @xiphFLACWhat audio file, and produces a Cap'n Proto
@capnp encoded binary file containing the necessary data to render a spectrum for the chosen song. Each
song consists of a directory containing one audio file, `audio.flac`, and one spectral data file,
`spectrum.bin`. These are stored in the `data` directory. For example:
`data/songs/LauraBrehm_PureSunlight/{audio.flac},{spectrum.bin}`.

Free Lossless Audio Codec (FLAC) @xiphFLACWhat is a lossless audio compression and container format. It was mainly selected
due to its ability to be easy loaded in C++ through open-source libraries like `dr_flac`, the fact that
it's entirely royalty and patent free, as well as its lossless nature. Although codecs such as MPEG-3 and
Vorbis are transparent at around 320 Kbps (meaning there are no perceptible differences between the compressed
codec and uncompressed audio), there may still be subtle artefacts introduced in the spectrum that could be
picked up by the visualiser. In any case, storing the audio in an uncompressed format introduces very little
overhead and can always be transcoded later down the track if marginally smaller file size is desired.

Cap'n Proto @capnp is a binary serialisation format that is regarded as a faster successor to Protocol Buffers.
Compared to Protocol Buffers, which are also a widely used binary serialisation format, Cap'n Proto has the
added benefit of requiring zero decode time and supporting very fast in-memory access. This is perfect for the
spectral data, which needs to be written once by the Python code, and re-read continuously by the C++
rendering code. Some of this reading takes place in the audio callback, which could be regarded as a soft
real-time function, and hence requiring very minimal overhead.

The FLAC file for a typical song weighs in at around 10 MB, and the spectral data is only around 200 KiB
thanks to Cap'n Proto's packed stream writing feature. Due to its speed, Cap'n P has to make some
trade-offs in regards to message size vs. speed, usually preferring larger messages. Since I'm checking the
data directory into the Git version control system, I used packed message writing to prefer smaller messages
with an ever so slightly longer decode time.

== Visualiser
The visualiser is what we see on the screen, the real-time rendering system that displays the bars, once the
spectral data has been computed. It is written in C++20, uses the OpenGL graphics API, and is built using
industry standard tools CMake, Ninja and the Clang compiler. This code runs "online", meaning it runs in
real-time and we would ideally like to spend no longer than 16 ms per frame (for 60 FPS). It uses a number of
open-source libraries:

- *SDL2*: Platform window management, keyboard/mouse inputs, OpenGL context creation
- *glad*: For OpenGL function loading and feature queries
- *glm*: The OpenGL maths library, used for computing transforms and its matrix/vector types
- *Cap'n Proto*: An extremely fast data serialisation format, used to transport data between Python and C++.
- *dr_flac*: A fast single-file FLAC decoder.
- *stb_image*: An image file decoder for many file formats.

== Analysis script
The analysis script is written in Python, and computes the spectral data used by the visualiser to display the
bars. It computes this data "offline", meaning it does not run in real-time, unlike the C++ code. It also uses
a relatively standard setup of Python libraries, with some additions due to the audio work involved:

- *NumPy*: Numerical computing library.
- *SciPy*: Scientific computing library.
- *spectrum.py*: Used to compute the periodogram.
- *Cap'n Proto*: Data inter-change format, see above.
- *audiofile.py*: For loading the FLAC file.
- *Matplotlib*: For graphing the waveform, debugging

= Signal processing
The overarching goal of the signal processing is to turn a time-domain audio signal into a frequency domain
spectrogram, similar to the Monstercat visualiser shown in @fig:monstercat. This turns out to actually be
quite an involved process, so this section will essentially a whirlwind tour of audio signal processing!

== Background: Computer audio
A raw audio signal consists of a number of digital _samples_ that are sent through
an audio system at a particular _sampling frequency_, typically 44.1 KHz.
#footnote[The exact reasons why are out of scope for this paper, but it has to do with the Nyquist-Shannon
theorem and history with CDs.]
Most audio is stereo, and hence there are typically two channels (although cinemas, for example, may use many
more channels). Eventually making its way out of userspace and into the kernel, these samples are converted
into an analogue signal using a Digital-to-Analogue (DAC) converter, which is typically located on most PC
motherboards or as a separate peripheral. The samples cause the speaker _driver_ (the membrane, usually
driven by magnets) to vibrate, which creates the sensation of sound. A typical digital audio signal is shown
in @fig:audiosignal.

In order to build the visualiser, we will need to convert this time-domain collection of samples into a
frequency-domain spectrogram.

#figure(
    image("img/audiosignal.png", width: 80%),
    caption: [ An audio signal loaded in Audacity. Top is zoomed out, bottom is zoomed in showing samples. ]
) <fig:audiosignal>

== Fourier transforms, DFT and FFT
The history of the Fourier transform dates back to 1822 when mathematician Joseph Fourier understood that any
function could be represented as a sum of sines @wolframFourierSeries. From that theory, the Fourier
transform was developed.

The core equation for the Fourier transform is given by @wolframFourierTransform:

// These equations converted using: https://github.com/mitex-rs/mitex (https://mitex-rs.github.io/mitex/)

//$$\widehat{f}(\xi) = \int_{-\infty}^{\infty} f(x)\ e^{-i 2\pi \xi x}\,dx.$$
#align(center)[
    $display(hat(f )\(xi \) =  integral _(- oo )^(oo ) f \(x \)thick  e ^(- i  2 pi  xi  x )thin d x .)$
]

In simplistic terms, the Fourier transform converts a time-domain signal, where the $x$ axis is time and the
$y$ axis is magnitude, into the frequency-domain, where the $x$ axis is frequency and the $y$ axis is the
magnitude of that particular frequency. A diagram of this transform is shown in @fig:ftdiagram:

#figure(
    image("img/fouriertransform.png", width: 50%),
    caption: [  Diagram showing the time-domain to frequency-domain conversion of the Fourier transform ]
) <fig:ftdiagram>

From the Fourier transform comes the concept of the Discrete Fourier Transform (DFT). The equation for the DFT
is similar, but slightly different to, the equation for the regular continuous Fourier transform:

//$$X_k = \sum_{n=0}^{N-1} x_n \cdot e^{-i2\pi \tfrac{k}{N}n}$$
#align(center)[
    $display(X _(k ) =  sum _(n = 0 )^(N - 1 ) x _(n ) dot.c  e ^(- i 2 pi  t frac(k ,N )n ))$
]

The DFT itself has many applications outside of audio analysis. A close cousin of the DFT, the Discrete Cosine
Transform (DCT), is used in many image compression algorithms, for example.

Computing the DFT naively has a time complexity of $O(n^2)$ @Rajaby2022 , which is typically considered
unfavourable in CS. This led to what has been described as "the most important numerical algorithm of our
lifetime" @Strang1994: the Fast Fourier Transform (FFT). The modern FFT was invented by James Cooley and
John Tukey in 1965 @Cooley1965AnAF, although Carl Freidrich Gauss had earlier work of a similar nature
from 1805 @Heideman1985. This algorithm reduces the complexity to $O(n log n)$, which is much more
palatable.

It's also important to note here that evaluating the general DFT operates on complex numbers, and its result
is also complex. However, since we are using a real-valued audio signal, we can instead make use of
specialised variants of the FFT that use purely real signals @Sorensen1987. The complex output is then
just converted to its magnitude, e.g. given $z = x + y i$ we take $r = sqrt(x^2 + y^2)$

== Decoding and chunking audio
In order for the Fourier transform to be possible, a _chunk_ of audio needs to be processed. In other words,
we can't just process individual samples since we're doing a time-domain to frequency-domain transform. After
decoding the FLAC audio using the `audiofile` library, the audio samples are then split into chunks/blocks of
1024 samples using NumPy. Additionally, the stereo signal is transformed into a mono signal by averaging the
left/right channels, since multi-channel FFTs are much more complicated.

This is all just a few lines of Python:

```python
# load audio
signal, sampling_rate = audiofile.read(song_path, always_2d=True)
# assume a stereo signal, let's mix it down to mono
mono = np.mean(signal, axis=0)
# split signal into chunks of BLOCK_SIZE
blocks = np.split(mono, range(BLOCK_SIZE, len(mono), BLOCK_SIZE))
```

== Power spectrum and periodogram
From the DFT, a power spectrum - otherwise known as "power spectral density" (PSD) - can be computed. The
power spectrum "describes how a signal's power is distributed in frequency" @Youngworth2005. Consider
a signal $U_T(t)$, whose Fourier transform is given by $|U_T(V)|^2$. The power spectrum can be computed as
follows, using the definition in @Youngworth2005:

#align(center)[
    $display("PS" \(V \) =  frac(| U _(T )\(V \)| ^(2 ),N ^(2 )))$
]

Where $N$ is the total number of sample points.

Then, the power spectral density can then be simply computed as:

#align(center)[
    $display( "PSD"(V) = frac("PS"(V), Delta v) )$
]

Where $Delta v$ is the space between data points in frequency space (so, the sampling rate).

Critically, the result of this equation is in the units of amplitude squared per frequency unit - which is
unwieldy and not particularly useful for our type of audio analysis. Instead, it would be better if we
converted it into dBFS ("Decibels relative to full scale"). This is a unit where 0.0 dBFS is the loudest
possible sound a system can emit, and it decreases negatively from there. For example, -10 dBFS is 10 dB
quieter than the maximum possible sound a system can emit.

The conversion from PSD to dBFS can be achieved as follows, given our signal $U_T(t)$:

#align(center)[
    $display("dbFS"_T(t) = sum_t log_10 ("PSD"(t)) / (max "PSD"))$
]

Implementing the above in Python, this produces the plot in @fig:psd:

#figure(
    image("img/psd.png", width: 80%),
    caption: [  Power spectral density (PSD) in dbFS of a block of audio from the song "Saturn's Air" by
    artist Animadrop. ]
) <fig:psd>

Also to note here is that we compute a metric known as _spectral energy_, which is a measure of the
"excitement" of the audio. It's computed as the magnitude squared of the FFT:

#align(center)[
    $display(S_E = sum_(i=0)^k "PSD"(i)^2)$
]

== Binning spectral data
Now that we have spectral data for each block of audio, we need to process it into "bins", or blocks, and use
that to construct the bars. Given we know the number of bars we want (by default, 32), we can simply sample
this using NumPy:

```python
samples = np.linspace(FREQ_MIN, FREQ_MAX, NUM_BARS)
```

Where `FREQ_MIN` and `FREQ_MAX` refer to the human hearing range, 20 Hz to 20 KHz, respectively.

Then, we simply walk through the linear space considering our current and previous value, and compute the mean
amplitude inside this block. There is some additional code that maps this value, in dbFS, to a `uint8_t`
bar height from 0 to 255, for Cap'n Proto serialisation.

Once this is computed, the data is serialised and can be loaded in C++ - and the signal processing is
complete.

= Computer graphics
== Render pipeline and states overview
The visualiser uses a fairly standard, simple, forward rendering pipeline based on OpenGL 3.0. The exact
rendering pipeline depends on which one of the two states the application may be in.

The application is divided in two two states: "intro" state, and "running" state. In the intro state, a
fullscreen textured quad displays three introduction slides which display the project name, my name, and the
song name #footnote[At this time, the song name is hardcoded and has to be manually changed when the song is
changed].

Once this is completed, the application transitions into "running" mode, which performs the actual
visualisation. This mode first binds a framebuffer. Then, it draws the bars using a GLSL shader, then the
skybox as a cubemap, and finally draws the framebuffer using a special fragment shader to provide some
post-processing effects.

The diagram in @fig:renderpipe shows the rendering pipeline (next page):

#figure(
    image("img/renderpipe.drawio.svg", width: 70%),
    caption: [ Diagram of graphics render pipeline ]
) <fig:renderpipe>

== Initialisation
SDL and glad are used to initialise the system platform. SDL is used to create the window, manage the audio
context, and load the GL driver. glad is the OpenGL loader which allows the actual interfacing with the GL
driver. One particularly important note to make regarding platform initialisation is the audio stream. For a
music visualisation application, it's very important that the on-screen graphics are precisely synced to the
audio being played. This means that a low-latency audio stream is required, one that is equal or smaller to
the spectral data block size. I achieve this by using SDL's newer audio APIs, which allow resampling based on
the platform audio driver's selection of what it believes is the best format. In the code, we request a
particular low sampling rate using `SDL_AudioSpec`, but allow SDL to change the actual underlying
datatype if it desires.

At startup, the application is passed the path to the data directory and the name of the song to display. Once
platform initialisation is complete, the FLAC audio is decoded using the dr_flac library, and the Cap'n Proto
spectrum data is decoded using the C++ Cap'n Proto library. This is stored in the `cosc::SongData` class,
which also handles mixing the audio into the SDL audio stream using `cosc::SongData::mixAudio`, which
also handles resampling using SDL.

== Transforming and rendering bars
Each frame, we query the `cosc::SongData` to figure out which spectrum block we are in, and use that to
query the heights of all the bars from the decoded Cap'n Proto document. The heights are a `uint8_t`, so
range from 0 to 255 inclusive. These are remapped to a float based on a configurable minimum and maximum
height.

The spectrum itself simply consists of $N$ unit cubes. The cubes are transformed by means of a transformation
matrix, exactly as was done in the previous Graphics Minor Project. #footnote[The only difference is that
transformations are computed using glm, rather than manually.] The height of the bars is simply determined by
their scale on the Y axis.

This is summarised in the following code snippet:

```cpp
size_t barIdx = 0;
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
```

The bar shader is almost identical to the shader used in the Graphics Minor Project.

TODO copy n paste description.

== Computing camera animations
One of the goals I had in mind for the visualiser was automated and smooth camera animations, that would also
not be too hard to describe in code. In order to do this though, the camera class from the Graphics Minor
Project, which was based on code from LearnOpenGL, would need to be rewritten. There was also the need for a
bug-free "freecam" mode, which can be moved around using the normal WASD and mouse controls. This is necessary
for me to find the begin and end points for each animation and for debugging.

The new camera is based on a slightly modified version of the perspective camera from the Cinder project
@cinderCamera. The main difference compared to the previous LearnOpenGL camera, is that the new camera
represents its pose as just a 3D vector and quaternion, rather than a set of vectors. This means that the pose
of the camera can be described much more easily, and can also be interpolated to create animations. Other than
that, it still outputs the same perspective and view matrices.

To implement camera moves, I chose to represent any given move as a begin pose, end pose, and a duration. This
is encoded in the `cosc::CameraAnimation` class. Then, in turn, a `cosc::CameraAnimationManager` is
attached to the `cosc::Camera`, which enables it to animate the camera every frame.

```cpp
/// The pose of a camera, with its position and orientation.
class CameraPose {
public:
    glm::vec3 pos;
    glm::quat orientation;
    ...
};

/// A camera animation
class CameraAnimation {
public:
    /// Beginning camera pose
    CameraPose begin;
    /// Ending camera pose
    CameraPose end;
    // Duration of the animation in seconds
    float duration;
    ...
};

/// Manages a camera with a set of animations.
class CameraAnimationManager {
public:
    explicit CameraAnimationManager(Camera &camera) : camera(camera) {};

    void update(float delta, float spectralEnergyRatio);

    void addAnimations(const std::vector<CameraAnimation> &animation) {
        ...
    }
    ...
};
```

Every frame, the `cosc::CameraAnimationManager` takes the delta time of the last frame and a spectral
energy ratio, to compute the camera animations. The spectral energy ratio is simply the current spectral
energy divided by the max spectral energy across the whole song. This lets us animate effects to the
_intensity_ of the song, which we'll soon cover.

The CameraAnimationManager stores which of the animations it's currently in, and handles the logic to
transition to the next animation. When the end of the animation list is reached, it's simply wrapped around
using modulo. The animation progress is tracked by keeping a sum of the delta values provided to the
`update()` function, forming an `elapsed` value.

Animating the 3D position of the camera between the begin and end pose turns out to be very easy, using simple
linear interpolation, as follows:

```cpp
auto pos = glm::mix(anim.begin.pos, anim.end.pos, progress); // performs linear interpolation
camera.setEyePoint(pos);
```

The `progress` value ranges from 0.0 to 1.0 and is computed by `elapsed / anim.duration`.

Animating the orientation of the camera is much more difficult. There are a number of ways of
expressing 3D orientations, the two main paradigms being Euler angles and quaternions. Euler angles represent
rotations as a 3D vector of angles around axes: pitch, roll and yaw (@fig:euler).

#figure(
    image("img/euler.png", width: 65%),
    caption: [ Demonstration of Euler angle rotation system ]
) <fig:euler>

Unfortunately, Euler angles suffer from well-described problems such as ambiguity and gimbal lock
@Hemingway2018, which make their use unsuitable for freeform 3D rotations and animations as is required
in this project.

Instead, the system of quaternions are used. A quaternion is a 4D complex number system that can be used to
encode 3D rotations, and has a direct correlation to rotation matrices (TODO citation).

TODO

== Skybox cubemap
The visualiser also draws a skybox using OpenGL's cubemap system. A cubemap is a 3D texture that is mapped to
a cube volume that surrounds the environment. Assuming the texture has no visible seams on the edges of the
cube volume, the cubemap gives the impression of a detailed space environment. This is a very common technique
used in almost all video games. The space cubemap texture itself was generated using the tool in
@tyroSpace, which uses procedural techniques.

TODO talk about the depth optimisation.

== Post-processing effects
The visualiser implements a simple post-processing pass that is run after the main scene is drawn. Currently,
the only effect rendered is chromatic aberration. This is an "artistic touch", if you will, that I added to
emphasise the _intensity_ of the song at certain points. The intensity is automatically calculated by
the same metric used to drive the screen shake: spectral energy ratio (that is, current spectral energy $div$
max spectral energy).

Post-processing is achieved using OpenGL's framebuffer and renderbuffer system. The framebuffer is attached to
the colour buffer, which is sampled in the fragment shader. The colour and stencil buffers are bound to the
renderbuffer, which does not need to be sampled.

The fragment shader, which implements the post-processing effects, is passed the final rendered image along
with the spectral energy ratio. To compute chromatic aberration, the fragment sampling position is adjusted on
the horizontal axis by an amount relative to the spectral energy ratio, performed separately for the R, G and
B channels. This is implemented as follows (based on the shader in @godotChromatic):

```glsl
vec4 colour;
float amount = 0.06 * spectralEnergyRatio;
colour.r = texture2D(screenTexture, vec2(TexCoords.x + amount, TexCoords.y)).r;
colour.g = texture2D(screenTexture, TexCoords).g;
colour.b = texture2D(screenTexture, vec2(TexCoords.x - amount, TexCoords.y)).b;
colour.a = 1.0;
```

The result is shown in @fig:chromatic:

#figure(
    image("img/chromatic_small.png", width: 70%),
    caption: [ Screenshot of visualisation application showing chromatic aberration effect ]
) <fig:chromatic>

// We probably don't need this section if we include results inline
// \section{Results}

= Discussion
Future improvements, etc.

= Self-assessment
I'm really proud of the work I achieved for this project. The visualiser turned out better than I was hoping
for, and I was able to achieve everything I set out to.

= Conclusion
In this paper, I present a 3D music visualisation application using OpenGL and the Discrete Fourier
Transform. The offline signal processing, using the Fast Fourier Transform as the mechanism for computing the
DFT, is implemented in Python. The visualisation application, which loads the computed spectral data, is
implemented using C++. It has graphics features such as camera animations, camera shake, a cubemap skybox, and
a post-processing path that implements chromatic aberration, and an intro section. The application can be
configured to display any song with audio available.

#pagebreak()

#bibliography("major.bib", title: "References", style: "ieeemodified.csl")
