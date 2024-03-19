#!/usr/bin/env python3
import sys
from pathlib import Path
import capnp


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

    # we have to hold the capnp object open while reading from the file
    # I'm somewhat regretting using this library, I should have just msgpack/CBOR'd it ;_;
    # reference: https://github.com/capnproto/pycapnp/issues/82
    f = open(data_path, "rb")
    music_vis = musicvis_capnp.MusicVisBars.read_packed(f)

    for i, block in enumerate(music_vis.blocks):
        print(f"Block {i}: {block}")

    print(f"Num bars: {music_vis.numBars}\nSample rate: {music_vis.sampleRate} Hz\nBlock size: "
          f"{music_vis.blockSize} samples")

    # Ok, **now** we should be able to close the bastard. Pray we don't get a bad fd error.
    f.close()


if __name__ == "__main__":
    main()
