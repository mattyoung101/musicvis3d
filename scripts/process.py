#!/usr/bin/env python3
import argparse
import numpy
import sys
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
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
BLOCK_SIZE = 1024

# Number of bars we want to draw
NUM_BARS = 128

# Human hearing range
FREQ_MIN = 20 # 20 Hz
FREQ_MAX = 20_000 # 20 KHz

# FFT window function in scipy.signal.get_window
WINDOW = "blackmanharris"


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} song_name")
        exit(1)

    song_name = sys.argv[1]
    song_path = Path(f"data/songs/{song_name}/audio.flac")
    print(f"Processing: {song_name} ({song_path})")

    # load audio
    signal, sampling_rate = librosa.load(song_path, sr=44100)
    print(f"sampling rate: {sampling_rate}")
    print(f"signal shape: {signal.shape}")

    # assume a stereo signal, let's mix it down to mono
    # mono = np.mean(signal, axis=0)
    # print(f"mono signal shape: {mono.shape}")

    # compute mel spectrogram
    # PROBLEM: too short (only 128 entries). we need to go back to FFT.
    spectrogram = librosa.feature.melspectrogram(y=signal, sr=sampling_rate, fmin=FREQ_MIN, fmax=FREQ_MAX,
                                                 )#n_fft=BLOCK_SIZE)
    # convert power spectrogram to decibels
    db = librosa.power_to_db(spectrogram, ref=np.max)
    # plt.plot(db[64])
    # plt.show()
    print(len(spectrogram))

    # fig, ax = plt.subplots(1, 1)
    # fig.set_size_inches(5, 5)
    #
    # def animate(i):
    #     ax.clear()
    #     ax.plot(db[i])
    # ani = FuncAnimation(fig, animate, frames=len(db), interval=100, repeat=False)
    # plt.show()

    # plot
    fig, ax = plt.subplots()
    S_dB = librosa.power_to_db(spectrogram, ref=np.max)
    img = librosa.display.specshow(S_dB, x_axis='time',
                                   y_axis='mel', sr=sampling_rate,
                                   fmax=8000, ax=ax)
    fig.colorbar(img, ax=ax, format='%+2.0f dB')
    ax.set(title='Mel-frequency spectrogram')
    plt.show()

if __name__ == "__main__":
    main()
