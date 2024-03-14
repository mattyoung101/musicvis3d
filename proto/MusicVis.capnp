@0x9dfabc7dfb1b949b;

struct MusicVizBars {
    numBars @0 : UInt8;
    sampleRate @1 : UInt32;

    bars @2 : List(List(UInt8));
}
