@0x9dfabc7dfb1b949b;

struct MusicVisBars {
    # Number of bars displayed
    numBars @0 : UInt8; 

    # Actual audio sample rate
    sampleRate @1 : UInt32; 

    # Block size in samples
    blockSize @2 : UInt32; 
    
    # List of blocks. Each list will have numBars and should last from (i * blockSize) 
    # to ((i+1) * blockSize) samples.
    blocks @3 : List(List(UInt8));
}
