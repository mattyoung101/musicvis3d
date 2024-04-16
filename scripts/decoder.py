#!/usr/bin/env python3
import sys
from pathlib import Path
import capnp
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} song_name")
        exit(1)

    # load cap'n proto spec
    print("Loading capnp spec")
    capnp.remove_import_hook()
    musicvis_capnp = capnp.load("proto/MusicVis.capnp")

    # read capnp message
    song_name = sys.argv[1]
    data_path = Path(f"data/songs/{song_name}/spectrum.bin")
    print(f"Reading capnp message from {data_path}")
    assert data_path.exists()

    with open(data_path, "rb") as f:
        music_vis = musicvis_capnp.MusicVisBars.read_packed(f)

        for i, block in enumerate(music_vis.blocks):
            print(f"Block {i}: {block}")

        print(f"Num bars: {music_vis.numBars}\nSample rate: {music_vis.sampleRate} Hz\nBlock size: "
              f"{music_vis.blockSize} samples\nMax spectral energy: {music_vis.maxSpectralEnergy}")

        plt.plot(music_vis.spectralEnergyBlocks)
        plt.show()


if __name__ == "__main__":
    main()
