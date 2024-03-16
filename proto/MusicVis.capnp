@0x9dfabc7dfb1b949b;

struct MusicVisBars {
    numBars @0 : UInt8; # Number of bars displayed
    sampleRate @1 : UInt32; # Actual audio sample rate
    blockSize @2 : UInt32; # Block size in samples
    blocks @3 : List(MusicVisBlock); # Blocks
}

struct MusicVisBlock {
    frameIdx @0 : UInt32; # Frame index (this block therefore covers frameIdx + blockSize samples)
    values @1 : UInt8; # Bar heights, 0-255
}
