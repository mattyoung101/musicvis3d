#!/usr/bin/env python3
import argparse
import numpy
import sys
from pathlib import Path
import audiofile
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq, rfft, rfftfreq, fftshift
from matplotlib.animation import FuncAnimation
import librosa

# This script is used to process music data in the FLAC file format, compute the FFT, and serialise it to
# Protocol Buffers.
# Matt Young, 2024

# References:
# - https://realpython.com/python-scipy-fft/
# - https://stackoverflow.com/a/67294512/5007892
# - https://spatialthoughts.com/2022/01/14/animated-plots-with-matplotlib/
# - https://www.youtube.com/watch?v=aQKX3mrDFoY

# Number of samples that constitutes one spectrum block
BLOCK_SIZE = 2048

# Number of bars we want to draw
NUM_BARS = 32

# Human hearing range
FREQ_MIN = 20 # 20 Hz
FREQ_MAX = 20_000 # 20 KHz

# 44100 KHz = 44100 samples per second
# = 44.1 samples per millisecond

# so if we want 10 milliseconds we should put 44.1 * 10 = 441


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} song_name")
        exit(1)

    song_name = sys.argv[1]
    song_path = Path(f"data/songs/{song_name}/audio.flac")
    print(f"Processing: {song_name} ({song_path})")

    # load audio
    signal, sampling_rate = audiofile.read(song_path, always_2d=True)
    print(f"sampling rate: {sampling_rate}")
    print(f"signal shape: {signal.shape}")

    # assume a stereo signal, let's mix it down to mono
    mono = np.mean(signal, axis=0)
    print(f"mono signal shape: {mono.shape}")

    # testing
    # audiofile.write("/tmp/block.flac", mono[0:BLOCK_SIZE], sampling_rate)

    # split signal into chunks of BLOCK_SIZE
    blocks = np.split(mono, range(BLOCK_SIZE, len(mono), BLOCK_SIZE))
    print(f"num blocks: {len(blocks)} block shape: {blocks[0].shape}")

    # compute FFT for each block - we use rfft which is the real valued fft
    # for block in blocks:
    #     yf = rfft(block)
    #     xf = rfftfreq(len(block), 1 / sampling_rate)
    #
    #     plt.hist(np.abs(yf), bins=NUM_BARS, range=[FREQ_MIN, FREQ_MAX])
    #     plt.show()

    fig, ax = plt.subplots(1, 1)
    fig.set_size_inches(5, 5)

    def animate(i):
        ax.clear()
        block = blocks[i]
        yf = rfft(block) #**2 # this should be the power spectrum -> FIXME looks kinda weird??
        xf = rfftfreq(len(block), 1 / sampling_rate)

        # convert power spectrum to dB
        # FIXME looks weird
        #db = librosa.power_to_db(np.abs(yf), ref=np.max)

        # apply perceptual weighting (make the graph look like how it would sound, I guess)
        # weighted = librosa.perceptual_weighting(np.abs(yf), xf, ref=np.max, kind="A")
        # print(weighted)

        # we want to sample along yf logarithmically, just like it's graphed
        samples = np.geomspace(FREQ_MIN, FREQ_MAX, NUM_BARS)
        # print(samples)

        plt.semilogx(xf, np.abs(yf))

        # draw marks where we would sample
        for sample in samples:
            plt.axvline(x=sample, color="red")

        # plt.hist(np.abs(yf), bins=NUM_BARS)
        # plt.plot(xf, np.abs(yf))
        # plt.scatter(xf, np.abs(yf))
        # plt.title(f"frame {i}")

    ani = FuncAnimation(fig, animate, frames=len(blocks), interval=30, repeat=False)
    plt.show()


if __name__ == "__main__":
    main()
