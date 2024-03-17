#!/usr/bin/env python3
import sys
from pathlib import Path
import audiofile
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from spectrum import Periodogram
from spectrum import tools as stools
import capnp
from typing import List, Tuple
from tqdm import tqdm
import scipy

# This script is used to process music data in the FLAC file format, compute the spectrogram, and serialise 
# it to Cap'n Proto for the C++ side.
# It computes a periodogram using the Fast Fourier Transform (FFT) (using the Kaiser windowing function), 
# converts it into decibels, then samples the frequency spectrum logarithmically to generate the bars.

# Matt Young, 2024

# References:
# - https://realpython.com/python-scipy-fft/
# - https://stackoverflow.com/a/67294512/5007892
# - https://spatialthoughts.com/2022/01/14/animated-plots-with-matplotlib/
# - https://www.youtube.com/watch?v=aQKX3mrDFoY
# - https://pyspectrum.readthedocs.io/en/latest/quickstart.html

# Number of samples that constitutes one spectrum block
BLOCK_SIZE = 1024

# Number of bars we want to draw
NUM_BARS = 32

# Human hearing range in Hz
FREQ_MIN = 20
FREQ_MAX = 20_000

# Min/max volume in dBFS
MIN_VOL = -80
MAX_VOL = 0

# TODO make these command line args, this is stupid
PLOT = False
PLOT_FFT = False
PLOT_BAR = False


def filter_freqs(freqs: List[Tuple[float, float]], min: float, max: float) -> List[float]:
    """
    Filters a list of (frequency, amplitude) tuple pairs to make sure the frequency is
    between min and max inclusive.
    """
    return [x[1] for x in freqs if x[0] >= min and x[0] <= max]


def process_block(block: np.ndarray, sampling_rate: int, ax=None) -> List[int]:
    """
    Processes a block of audio samples into a bar spectrogram.
    The output bars are in the range 0..255 and can be fed into the visualiser directly.
    """
    # compute periodogram using kaiser windowing function
    p = Periodogram(block, sampling=sampling_rate, window="kaiser")
    p.run()

    # apply perceptual weighting (make the graph look like how it would sound, I guess)
    # weighted = librosa.A_weighting(db) # FIXME doesn't work

    # we want to sample along yf logarithmically, just like it's graphed
    # This used to be logarithmic sampling using np.geomspace, but the problem is that it seems like the
    # results from both scipy.signal.periodogram and spectrum.py's Periodogram, the frequencies, are _already_
    # logarithmically distributed. so the linear sampling actually ends up looking OK? I'm not sure if this is
    # correct or not though.
    # TODO verify this is what we want
    samples = np.linspace(FREQ_MIN, FREQ_MAX, NUM_BARS)

    # convert power spectrum to dB
    # source: https://github.com/cokelaer/spectrum/blob/master/src/spectrum/psd.py#L691 (given norm=True)
    db = 10 * stools.log10(p.psd / max(p.psd))

    # Now we pair frequencies with their samples
    pairs = [x for x in zip(p.frequencies(), db)]

    # Finalised output bars
    bars = []

    # Sample bars using our samples array
    prev = samples[0]
    for cur in samples[1:]:
        # Take the mean amplitude in this block
        filtered = filter_freqs(pairs, prev, cur)
        # Sometimes the bin is empty, this just happens, we just return 0.
        if len(filtered) == 0:
            bars.append(0)
            continue

        mean = np.mean(filtered)

        # Map range MIN_VOL..MAX_VOL dB to 0..255 (where 0=MIN_DB dB; 255=MAX_DB dB)
        val = np.interp(mean, [MIN_VOL, MAX_VOL], [0, 255])
        bars.append(val)

        # Move along to the next sample
        # print(f"Sampling bar from {prev}..{cur} filtered: {filtered} dB: {mean} val: {val}")
        prev = cur

    # Idiotic method to handle the final case (cur..FREQ_MAX) (FIXME this is stupid)
    filtered = filter_freqs(pairs, prev, cur)
    if len(filtered) == 0:
        bars.append(0)
    else:
        mean = np.mean(filter_freqs(pairs, cur, FREQ_MAX))
        val = np.interp(mean, [MIN_VOL, MAX_VOL], [0, 255])
        bars.append(val)

    # plot spectrum
    if PLOT and PLOT_FFT:
        plt.semilogx(p.frequencies(), db)
        ax.set_ylim([-80, 10]) # basically 0 to -80 dB

        # draw marks where we would sample
        for sample in samples:
            plt.axvline(x=sample, color="grey")

    return [int(x) for x in bars]


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} song_name")
        exit(1)

    if PLOT:
        assert PLOT_FFT ^ PLOT_BAR, "PLOT_FFT and PLOT_BAR are mutually exclusive"

    song_name = sys.argv[1]
    song_path = Path(f"data/songs/{song_name}/audio.flac")
    print(f"Processing: {song_name} ({song_path})")

    # load cap'n proto spec
    print("Loading capnp spec")
    capnp.remove_import_hook()
    musicvis_capnp = capnp.load("proto/MusicVis.capnp")

    # load audio
    signal, sampling_rate = audiofile.read(song_path, always_2d=True)
    print(f"sampling rate: {sampling_rate}")
    print(f"signal shape: {signal.shape}")

    # assume a stereo signal, let's mix it down to mono
    mono = np.mean(signal, axis=0)
    print(f"mono signal shape: {mono.shape}")
    # audiofile.write("/tmp/block.flac", mono[0:BLOCK_SIZE], sampling_rate)

    # split signal into chunks of BLOCK_SIZE
    blocks = np.split(mono, range(BLOCK_SIZE, len(mono), BLOCK_SIZE))
    print(f"num blocks: {len(blocks)} block shape: {blocks[0].shape}")

    # construct capnp message
    music_vis = musicvis_capnp.MusicVisBars.new_message()
    music_vis.numBars = NUM_BARS
    music_vis.sampleRate = sampling_rate
    music_vis.blockSize = BLOCK_SIZE

    if PLOT:
        fig, ax = plt.subplots(1, 1)
        fig.set_size_inches(5, 5)

        def animate(i):
            ax.clear()
            bars = process_block(blocks[i], sampling_rate, ax)
            
            if PLOT and PLOT_BAR:
                # plot bar graph (final)
                plt.bar(x=range(NUM_BARS), height=bars)
                ax.set_ylim([0, 255])

        ani = FuncAnimation(fig, animate, frames=len(blocks), interval=30, repeat=False)
        plt.show()
        exit(0)

    # process blocks
    all_bars = []
    for i, block in tqdm(enumerate(blocks), total=len(blocks)):
        # calculate the bars
        all_bars.append(process_block(block, sampling_rate))
        # print(f"Block {i} bars: {bars}")
        # put the bars into the capnp message

    music_vis.blocks = all_bars

    # write capnp message
    data_path = Path(f"data/songs/{song_name}/spectrum.bin")
    print(f"Writing capnp message to {data_path}")
    f = open(data_path, "w+b")
    music_vis.write_packed(f)
    f.flush()
    f.close()
    print("Done!")


if __name__ == "__main__":
    main()
