// Counter definitions for all supported CPUs
// Extracted from PMCTestA.cpp for reuse

#include "PMCTest.h"

SCounterDefinition CounterDefinitions[] = {
    //  id   scheme cpu    countregs eventreg event  mask   name
    {100,  S_P4, P_ALL,  4,   7,     0,      9,      7,  "Uops"     }, // uops from any source
    {101,  S_P4, P_ALL,  4,   7,     0,      9,      2,  "UopsTC"   }, // uops from trace cache
    {102,  S_P4, P_ALL,  4,   7,     0,      9,      1,  "UopsDec"  }, // uops directly from decoder
    {103,  S_P4, P_ALL,  4,   7,     0,      9,      4,  "UopsMCode"}, // uops from microcode ROM
    {110,  S_P4, P_ALL, 12,  17,     4,      1,      1,  "UopsNB"   }, // uops non-bogus
    {111,  S_P4, P_ALL, 12,  17,     4,      2,   0x0c,  "UopsBogus"}, // uops bogus
    {150,  S_P4, P_ALL,  8,  11,     1,      4, 0x8000,  "UopsFP"   }, // uops floating point, except move etc.
    {151,  S_P4, P_ALL,  8,  11,     1,   0x2e,      8,  "UopsFPMov"}, // uops floating point and SIMD move
    {152,  S_P4, P_ALL,  8,  11,     1,   0x2e,   0x10,  "UopsFPLd" }, // uops floating point and SIMD load
    {160,  S_P4, P_ALL,  8,  11,     1,      2, 0x8000,  "UopsMMX"  }, // uops 64-bit MMX
    {170,  S_P4, P_ALL,  8,  11,     1,   0x1a, 0x8000,  "UopsXMM"  }, // uops 128-bit integer XMM
    {200,  S_P4, P_ALL, 12,  17,     5,      6,   0x0f,  "Branch"   }, // branches
    {201,  S_P4, P_ALL, 12,  17,     5,      6,   0x0c,  "BrTaken"  }, // branches taken
    {202,  S_P4, P_ALL, 12,  17,     5,      6,   0x03,  "BrNTaken" }, // branches not taken
    {203,  S_P4, P_ALL, 12,  17,     5,      6,   0x05,  "BrPredict"}, // branches predicted
    {204,  S_P4, P_ALL, 12,  17,     4,      3,   0x01,  "BrMispred"}, // branches mispredicted
    {210,  S_P4, P_ALL,  4,   7,     2,      5,   0x02,  "CondJMisp"}, // conditional jumps mispredicted
    {211,  S_P4, P_ALL,  4,   7,     2,      5,   0x04,  "CallMisp" }, // indirect call mispredicted
    {212,  S_P4, P_ALL,  4,   7,     2,      5,   0x08,  "RetMisp"  }, // return mispredicted
    {220,  S_P4, P_ALL,  4,   7,     2,      5,   0x10,  "IndirMisp"}, // indirect calls, jumps and returns mispredicted
    {310,  S_P4, P_ALL,  0,   3,     0,      3,   0x01,  "TCMiss"   }, // trace cache miss
    {320,  S_P4, P_ALL,  0,   3,     7,   0x0c,  0x100,  "Cach2Miss"}, // level 2 cache miss
    {321,  S_P4, P_ALL,  0,   3,     7,   0x0c,  0x200,  "Cach3Miss"}, // level 3 cache miss
    {330,  S_P4, P_ALL,  0,   3,     3,   0x18,   0x02,  "ITLBMiss" }, // instructions TLB Miss
    {340,  S_P4, P_ALL,  0,   3,     2,      3,   0x3a,  "LdReplay" }, // memory load replay


    //  id   scheme cpu    countregs eventreg event  mask   name
    {  9,  S_P1, P_ALL,  0,   1,     0,   0x16,        2,  "Instruct" }, // instructions executed
    { 11,  S_P1, P_ALL,  0,   1,     0,   0x17,        2,  "InstVpipe"}, // instructions executed in V-pipe
    {202,  S_P1, P_ALL,  0,   1,     0,   0x15,        2,  "Flush"    }, // pipeline flush due to branch misprediction or serializing event   
    {310,  S_P1, P_ALL,  0,   1,     0,   0x0e,        2,  "CodeMiss" }, // code cache miss
    {311,  S_P1, P_ALL,  0,   1,     0,   0x29,        2,  "DataMiss" }, // data cache miss


    //  id   scheme  cpu     countregs eventreg event  mask   name
    {  9, S_P2MC, P_ALL,    0,   1,     0,   0xc0,     0,  "Instruct" }, // instructions executed
    { 10, S_P2MC, P_ALL,    0,   1,     0,   0xd0,     0,  "IDecode"  }, // instructions decoded
    { 20, S_P2MC, P_ALL,    0,   1,     0,   0x80,     0,  "IFetch"   }, // instruction fetches
    { 21, S_P2MC, P_ALL,    0,   1,     0,   0x86,     0,  "IFetchStl"}, // instruction fetch stall
    { 22, S_P2MC, P_ALL,    0,   1,     0,   0x87,     0,  "ILenStal" }, // instruction length decoder stalls
    {100, S_P2MC, INTEL_PM, 0,   1,     0,   0xc2,     0,  "Uops(F)"  }, // microoperations in fused domain
    {100, S_P2MC, P_ALL,    0,   1,     0,   0xc2,     0,  "Uops"     }, // microoperations
    {110, S_P2MC, INTEL_PM, 0,   1,     0,   0xa0,     0,  "Uops(UF)" }, // unfused microoperations submitted to execution units (Undocumented counter!)
    {104, S_P2MC, INTEL_PM, 0,   1,     0,   0xda,     0,  "UopsFused"}, // fused uops
    {115, S_P2MC, INTEL_PM, 0,   1,     0,   0xd3,     0,  "SynchUops"}, // stack synchronization uops
    {121, S_P2MC, P_ALL,    0,   1,     0,   0xd2,     0,  "PartRStl" }, // partial register access stall
    {130, S_P2MC, P_ALL,    0,   1,     0,   0xa2,     0,  "Rs Stall" }, // all resource stalls
    {201, S_P2MC, P_ALL,    0,   1,     0,   0xc9,     0,  "BrTaken"  }, // branches taken
    {204, S_P2MC, P_ALL,    0,   1,     0,   0xc5,     0,  "BrMispred"}, // mispredicted branches
    {205, S_P2MC, P_ALL,    0,   1,     0,   0xe6,     0,  "BTBMiss"  }, // static branch prediction made
    {310, S_P2MC, P_ALL,    0,   1,     0,   0x28,  0x0f,  "CodeMiss" }, // level 2 cache code fetch
    {311, S_P2MC, INTEL_P23,0,   1,     0,   0x29,  0x0f,  "L1D Miss" }, // level 2 cache data fetch

    // Core 2:
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same two counter registers.
    //  id   scheme cpu      countregs eventreg event  mask   name
    {1,   S_ID2, P_ALL,   0x40000001,  0,0,   0,      0,   "Core cyc"  }, // core clock cycles
    {2,   S_ID2, P_ALL,   0x40000002,  0,0,   0,      0,   "Ref cyc"   }, // Reference clock cycles
    {9,   S_ID2, P_ALL,   0x40000000,  0,0,   0,      0,   "Instruct"  }, // Instructions (reference counter)
    {10,  S_ID2, P_ALL,   0,   1,     0,   0xc0,     0x0f, "Instruct"  }, // Instructions
    {11,  S_ID2, P_ALL,   0,   1,     0,   0xc0,     0x01, "Read inst" }, // Instructions involving read, fused count as one
    {12,  S_ID2, P_ALL,   0,   1,     0,   0xc0,     0x02, "Write ins" }, // Instructions involving write, fused count as one
    {13,  S_ID2, P_ALL,   0,   1,     0,   0xc0,     0x04, "NonMem in" }, // Instructions without memory
    {20,  S_ID2, P_ALL,   0,   1,     0,   0x80,      0,   "Insfetch"  }, // instruction fetches. < instructions ?
    {21,  S_ID2, P_ALL,   0,   1,     0,   0x86,      0,   "IFetchStl" }, // instruction fetch stall
    {22,  S_ID2, P_ALL,   0,   1,     0,   0x87,      0,   "ILenStal"  }, // instruction length decoder stalls (length changing prefix)
    {23,  S_ID2, P_ALL,   0,   1,     0,   0x83,      0,   "IQue ful"  }, // instruction queue full
    {100, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x0f, "Uops"      }, // uops retired, fused domain
    {101, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x01, "Fused Rd"  }, // fused read uops
    {102, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x02, "Fused Wrt" }, // fused write uops
    {103, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x04, "Macrofus"  }, // macrofused uops
    {104, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x07, "FusedUop"  }, // fused uops, all kinds
    {105, S_ID2, P_ALL,   0,   1,     0,   0xc2,     0x08, "NotFusUop" }, // uops, not fused
    {110, S_ID2, P_ALL,   0,   1,     0,   0xa0,        0, "Uops UFD"  }, // uops dispatched, unfused domain. Imprecise
    {111, S_ID2, P_ALL,   0,   1,     0,   0xa2,        0, "res.stl."  }, // any resource stall
    {115, S_ID2, P_ALL,   0,   1,     0,   0xab,     0x01, "SP synch"  }, // Stack synchronization uops
    {116, S_ID2, P_ALL,   0,   1,     0,   0xab,     0x02, "SP engine" }, // Stack engine additions
    {121, S_ID2, P_ALL,   0,   1,     0,   0xd2,     0x02, "Part.reg"  }, // Partial register synchronization, clock cycles
    {122, S_ID2, P_ALL,   0,   1,     0,   0xd2,     0x04, "part.flag" }, // partial flags stall, clock cycles
    {123, S_ID2, P_ALL,   0,   1,     0,   0xd2,     0x08, "FP SW stl" }, // floating point status word stall
    {130, S_ID2, P_ALL,   0,   1,     0,   0xd2,     0x01, "R Rd stal" }, // ROB register read stall
    {140, S_ID2, P_ALL,   0,   1,     0,   0x19,     0x00, "I2FP pass" }, // bypass delay to FP unit from int unit
    {141, S_ID2, P_ALL,   0,   1,     0,   0x19,     0x01, "FP2I pass" }, // bypass delay to SIMD/int unit from fp unit (These counters cannot be used simultaneously)
    {150, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x01, "uop p0"    }, // uops port 0. Can only use first counter
    {151, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x02, "uop p1"    }, // uops port 1. Can only use first counter
    {152, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x04, "uop p2"    }, // uops port 2. Can only use first counter
    {153, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x08, "uop p3"    }, // uops port 3. Can only use first counter
    {154, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x10, "uop p4"    }, // uops port 4. Can only use first counter
    {155, S_ID2, P_ALL,   0,   0,     0,   0xa1,     0x20, "uop p5"    }, // uops port 5. Can only use first counter
    {201, S_ID2, P_ALL,   0,   1,     0,   0xc4,     0x0c, "BrTaken"   }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID2, P_ALL,   0,   1,     0,   0xc4,     0x0a, "BrMispred" }, // mispredicted branches
    {205, S_ID2, P_ALL,   0,   1,     0,   0xe6,      0,   "BTBMiss"   }, // static branch prediction made
    {210, S_ID2, P_ALL,   0,   1,     0,   0x97,      0,   "BranchBu1" }, // branch taken bubble 1
    {211, S_ID2, P_ALL,   0,   1,     0,   0x98,      0,   "BranchBu2" }, // branch taken bubble 2 (these two values must be added)
    {310, S_ID2, P_ALL,   0,   1,     0,   0x28,     0x0f, "CodeMiss"  }, // level 2 cache code fetch
    {311, S_ID2, P_ALL,   0,   1,     0,   0x29,     0x0f, "L1D Miss"  }, // level 2 cache data fetch
    {320, S_ID2, P_ALL,   0,   1,     0,   0x24,     0x00, "L2 Miss"   }, // level 2 cache miss

    {410, S_ID2,  P_ALL, 0,  3,     0,   0xe6,     0x1, "BaClrAny"  }, // MG added

    // Nehalem, Sandy Bridge, Ivy Bridge
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same counter registers.
    // id   scheme  cpu       countregs eventreg event  mask   name
    {1,   S_ID3,  INTEL_7I,  0x40000001,  0,0,   0,      0,   "Core cyc"   }, // core clock cycles
    {2,   S_ID3,  INTEL_7I,  0x40000002,  0,0,   0,      0,   "Ref cyc"    }, // Reference clock cycles
    {9,   S_ID3,  INTEL_7I,  0x40000000,  0,0,   0,      0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3,  INTEL_7I,  0,   3,     0,   0xc0,     0x01, "Instruct"   }, // Instructions
    {22,  S_ID3,  INTEL_7I,  0,   3,     0,   0x87,      0,   "ILenStal"   }, // instruction length decoder stalls (length changing prefix)
    {24,  S_ID3,  INTEL_7I,  0,   3,     0,   0xA8,     0x01, "Loop uops"  }, // uops from loop stream detector
    {25,  S_ID3,  INTEL_7I,  0,   3,     0,   0x79,     0x04, "Dec uops"   }, // uops from decoders. (MITE = Micro-instruction Translation Engine)
    {26,  S_ID3,  INTEL_7I,  0,   3,     0,   0x79,     0x08, "Cach uops"  }, // uops from uop cache. (DSB = Decoded Stream Buffer)
    {100, S_ID3,  INTEL_7I,  0,   3,     0,   0xc2,     0x01, "Uops"       }, // uops retired, unfused domain
    {103, S_ID3,  INTEL_7,   0,   3,     0,   0xc2,     0x04, "Macrofus"   }, // macrofused uops, Sandy Bridge
    {104, S_ID3,  INTEL_7I,  0,   3,     0,   0x0E,     0x01, "Uops F.D."  }, // uops, fused domain, Sandy Bridge
    {105, S_ID3,  INTEL_7,   0,   3,     0,   0x0E,     0x02, "fused uop"  }, // microfused uops 
    {110, S_ID3,  INTEL_7,   0,   3,     0,   0xa0,        0, "Uops UFD?"  }, // uops dispatched, unfused domain. Imprecise, Sandy Bridge
    {111, S_ID3,  INTEL_7I,  0,   3,     0,   0xa2,        1, "res.stl."   }, // any resource stall
    {121, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x02, "Part.reg"   }, // Partial register synchronization, clock cycles, Sandy Bridge
    {122, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x01, "part.flag"  }, // partial flags stall, clock cycles, Sandy Bridge
    {123, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x04, "R Rd stal"  }, // ROB register read stall, Sandy Bridge
    {124, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x0F, "RAT stal"   }, // RAT stall, any, Sandy Bridge
    {150, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x01, "uop p0"     }, // uops port 0.
    {151, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x02, "uop p1"     }, // uops port 1.
    {152, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x04, "uop p2"     }, // uops port 2.
    {153, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x08, "uop p3"     }, // uops port 3.
    {154, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x10, "uop p4"     }, // uops port 4.
    {155, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x20, "uop p5"     }, // uops port 5. Unreliable!
    {156, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x40, "uop p015"   }, // uops port 0,1,5. Unreliable!
    {157, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x80, "uop p234"   }, // uops port 2,3,4.
    {201, S_ID2,  INTEL_IVY, 0,   1,     0,   0xc4,     0x20, "BrTaken"    }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID3,  INTEL_7,   0,   3,     0,   0xc5,     0x0a, "BrMispred"  }, // mispredicted branches
    {207, S_ID3,  INTEL_7I,  0,   3,     0,   0xc5,     0x0,  "BrMispred"  }, // mispredicted branches
    {201, S_ID3,  INTEL_7I,  0,   3,     0,   0xc4,     0x0,  "BrTaken"  }, // MG added (not sure this is right)
    {400, S_ID3,  INTEL_7I,  0,   3,     0,   0xa7,     0x01,  "BaClrFIq"  }, // MG added
    {401, S_ID3,  INTEL_7I,  0,   3,     0,   0xe6,     0x01,  "BaClrClr"  }, // MG added
    {402, S_ID3,  INTEL_7I,  0,   3,     0,   0xe6,     0x02,  "BaClrBad"  }, // MG added
    {403, S_ID3,  INTEL_7I,  0,   3,     0,   0xe8,     0x01,  "BaClrEly"  }, // MG added
    {404, S_ID3,  INTEL_7I,  0,   3,     0,   0xe8,     0x02,  "BaClrL8"  }, // MG added
    {410, S_ID3,  INTEL_7I, 0,  3,     0,   0xe6,     0x1f, "BaClrAny"  }, // MG added
    {205, S_ID3,  INTEL_7,   0,   3,     0,   0xe6,      2,   "BTBMiss"    }, // static branch prediction made, Sandy Bridge
    {220, S_ID3,  INTEL_IVY, 0,   3,     0,   0x58,     0x03, "Mov elim"   }, // register moves eliminated
    {221, S_ID3,  INTEL_IVY, 0,   3,     0,   0x58,     0x0C, "Mov elim-"  }, // register moves elimination unsuccessful
    {311, S_ID3,  INTEL_7I,  0,   3,     0,   0x28,     0x0f, "L1D Miss"   }, // level 1 data cache miss
    {312, S_ID3,  INTEL_7,   0,   3,     0,   0x24,     0x0f, "L1 Miss"    }, // level 2 cache requests
    {150, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x01, "uop p0"     }, // uops port 0
    {151, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x02, "uop p1"     }, // uops port 1
    {152, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x0c, "uop p2"     }, // uops port 2
    {153, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x30, "uop p3"     }, // uops port 3
    {154, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x40, "uop p4"     }, // uops port 4
    {155, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x80, "uop p5"     }, // uops port 5
    {160, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0xFF, "uop p05"    }, // uops port 0 - 5

    // Haswell
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same four counter registers.
    // id   scheme  cpu       countregs eventreg event  mask   name
    {1,   S_ID3,  INTEL_HASW, 0x40000001,  0,0,   0,     0,   "Core cyc"   }, // core clock cycles
    {2,   S_ID3,  INTEL_HASW, 0x40000002,  0,0,   0,     0,   "Ref cyc"    }, // Reference clock cycles
    {9,   S_ID3,  INTEL_HASW, 0x40000000,  0,0,   0,     0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3,  INTEL_HASW, 0,  3,     0,   0xc0,     0x01, "Instruct"   }, // Instructions
    {22,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x87,     0x01, "ILenStal"   }, // instruction length decoder stall due to length changing prefix
    {24,  S_ID3,  INTEL_HASW, 0,  3,     0,   0xA8,     0x01, "Loop uops"  }, // uops from loop stream detector
    {25,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x79,     0x04, "Dec uops"   }, // uops from decoders. (MITE = Micro-instruction Translation Engine)
    {26,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x79,     0x08, "Cach uops"  }, // uops from uop cache. (DSB = Decoded Stream Buffer)
    {100, S_ID3,  INTEL_HASW, 0,  3,     0,   0xc2,     0x01, "Uops"       }, // uops retired, unfused domain
    {104, S_ID3,  INTEL_HASW, 0,  3,     0,   0x0e,     0x01, "uops RAT"   }, // uops from RAT to RS
    {111, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa2,     0x01, "res.stl."   }, // any resource stall
    {131, S_ID3,  INTEL_HASW, 0,  3,     0,   0xC1,     0x18, "AVX trans"  }, // VEX - non-VEX transition penalties
    {201, S_ID3,  INTEL_HASW, 0,  3,     0,   0xC4,     0x20, "BrTaken"    }, // branches taken
    {207, S_ID3,  INTEL_HASW, 0,  3,     0,   0xc5,     0x00, "BrMispred"  }, // mispredicted branches
    {220, S_ID3,  INTEL_HASW, 0,  3,     0,   0x58,     0x03, "Mov elim"   }, // register moves eliminated
    {221, S_ID3,  INTEL_HASW, 0,  3,     0,   0x58,     0x0C, "Mov elim-"  }, // register moves elimination unsuccessful
    {310, S_ID2,  INTEL_HASW, 0,  3,     0,   0x80,     0x02, "CodeMiss"   }, // code cache misses
    {311, S_ID3,  INTEL_HASW, 0,  3,     0,   0x24,     0xe1, "L1D Miss"   }, // level 1 data cache miss
    {320, S_ID3,  INTEL_HASW, 0,  3,     0,   0x24,     0x27, "L2 Miss"    }, // level 2 cache misses
    {150, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x01, "uop p0"     }, // uops port 0.
    {151, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x02, "uop p1"     }, // uops port 1.
    {152, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x04, "uop p2"     }, // uops port 2.
    {153, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x08, "uop p3"     }, // uops port 3.
    {154, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x10, "uop p4"     }, // uops port 4.
    {155, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x20, "uop p5"     }, // uops port 5.
    {156, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x40, "uop p6"     }, // uops port 6.
    {157, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x80, "uop p7"     }, // uops port 7.
    {160, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0xFF, "uop p07"    }, // uops port 0 - 7
    {410, S_ID3,  INTEL_HASW, 0,  3,     0,   0xe6,     0x1f, "BaClrAny"  }, // MG added
    // Following are ones MG added; spec is vague on whether these exist in Haswell
    {400, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa7,     0x01,  "BaClrFIq"  }, // MG added
    {403, S_ID3,  INTEL_HASW, 0,  3,     0,   0xe8,     0x01,  "BaClrEly"  }, // MG added
    {404, S_ID3,  INTEL_HASW, 0,  3,     0,   0xe8,     0x02,  "BaClrL8"  }, // MG added

    // Intel Broadwell, Skylake, Kaby/Coffee/Comet Lake, Ice Lake, Tiger Lake:
    // These architectures share the same PMC architecture (S_ID3) and similar event encodings
    // Based on Intel perfmon JSON data from https://github.com/intel/perfmon
    //  id   scheme  cpu         countregs eventreg event  mask   name
    {1,   S_ID3, (EProcFamily)(INTEL_BROADWELL|INTEL_SKYLAKE|INTEL_KABYLAKE|INTEL_ICELAKE|INTEL_TIGERLAKE), 0x40000002, 0, 0, 0, 0, "Core cyc"}, // Core clock cycles (fixed counter)
    {9,   S_ID3, (EProcFamily)(INTEL_BROADWELL|INTEL_SKYLAKE|INTEL_KABYLAKE|INTEL_ICELAKE|INTEL_TIGERLAKE), 0x40000000, 0, 0, 0, 0, "Instruct"}, // Instructions retired (fixed counter)
    {207, S_ID3, (EProcFamily)(INTEL_BROADWELL|INTEL_SKYLAKE|INTEL_KABYLAKE|INTEL_ICELAKE|INTEL_TIGERLAKE), 0, 3, 0, 0xc5, 0x00, "BrMispred"}, // BR_MISP_RETIRED.ALL_BRANCHES
    {410, S_ID3, (EProcFamily)(INTEL_BROADWELL|INTEL_SKYLAKE|INTEL_KABYLAKE|INTEL_ICELAKE|INTEL_TIGERLAKE), 0, 3, 0, 0xe6, 0x01, "BaClrAny"}, // BACLEARS.ANY
    {411, S_ID3, (EProcFamily)(INTEL_BROADWELL|INTEL_SKYLAKE|INTEL_KABYLAKE|INTEL_ICELAKE|INTEL_TIGERLAKE), 0, 3, 0, 0x0d, 0x80, "ClrRestr"}, // INT_MISC.CLEAR_RESTEER_CYCLES

    // Intel Atom:
    // The first counter is fixed-function counter having its own register,
    // The rest of the counters are competing for the same two counter registers.
    //  id   scheme  cpu         countregs eventreg event  mask   name
    {9,   S_ID3, INTEL_ATOM,  0x40000000, 0,0,    0,      0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3, INTEL_ATOM,  0,   1,     0,   0xc0,     0x00, "Instr"      }, // Instructions retired
    {20,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x80,     0x03, "Insfetch"   }, // instruction fetches
    {21,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x80,     0x02, "I miss"     }, // instruction cache miss
    {30,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x40,     0x21, "L1 read"    }, // L1 data cache read
    {31,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x40,     0x22, "L1 write"   }, // L1 data cache write
    {100, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc2,     0x10, "Uops"       }, // uops retired
    {200, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x00, "Branch"     }, // branches
    {201, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x0c, "BrTaken"    }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x0a, "BrMispred"  }, // mispredicted branches
    {205, S_ID3, INTEL_ATOM,  0,   1,     0,   0xe6,     0x01, "BTBMiss"    }, // Baclear
    {310, S_ID3, INTEL_ATOM,  0,   1,     0,   0x28,     0x4f, "CodeMiss"   }, // level 2 cache code fetch
    {311, S_ID3, INTEL_ATOM,  0,   1,     0,   0x29,     0x4f, "L1D Miss"   }, // level 2 cache data fetch
    {320, S_ID3, INTEL_ATOM,  0,   1,     0,   0x24,     0x00, "L2 Miss"    }, // level 2 cache miss
    {501, S_ID3, INTEL_ATOM,  0,   1,     0,   0xC0,     0x00, "inst re"    }, // instructions retired
    {505, S_ID3, INTEL_ATOM,  0,   1,     0,   0xAA,     0x02, "CISC"       }, // CISC macro instructions decoded
    {506, S_ID3, INTEL_ATOM,  0,   1,     0,   0xAA,     0x03, "decoded"    }, // all instructions decoded
    {601, S_ID3, INTEL_ATOM,  0,   1,     0,   0x02,     0x81, "st.forw"    }, // Successful store forwards
    {640, S_ID3, INTEL_ATOM,  0,   1,     0,   0x12,     0x81, "mul"        }, // Int and FP multiply operations
    {641, S_ID3, INTEL_ATOM,  0,   1,     0,   0x13,     0x81, "div"        }, // Int and FP divide and sqrt operations
    {651, S_ID3, INTEL_ATOM,  0,   1,     0,   0x10,     0x81, "fp uop"     }, // Floating point uops


    //  id   scheme  cpu         countregs eventreg event  mask   name
    {  9, S_AMD, AMD_ALL,      0,   3,     0,   0xc0,      0,  "Instruct" }, // x86 instructions executed
    {100, S_AMD, AMD_ALL,      0,   3,     0,   0xc1,      0,  "Uops"     }, // microoperations
    {204, S_AMD, AMD_ALL,      0,   3,     0,   0xc3,      0,  "BrMispred"}, // mispredicted branches
    {201, S_AMD, AMD_BULLD,    0,   3,     0,   0xc4,   0x00,  "BrTaken"  }, // branches taken
    {209, S_AMD, AMD_BULLD,    0,   3,     0,   0xc2,   0x00,  "RSBovfl"  }, // return stack buffer overflow
    {310, S_AMD, AMD_ALL,      0,   3,     0,   0x81,      0,  "CodeMiss" }, // instruction cache misses
    {311, S_AMD, AMD_ALL,      0,   3,     0,   0x41,      0,  "L1D Miss" }, // L1 data cache misses
    {320, S_AMD, AMD_ALL,      0,   3,     0,   0x43,   0x1f,  "L2 Miss"  }, // L2 cache misses
    {150, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x3f,  "UopsFP"   }, // microoperations in FP pipe
    {151, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x09,  "FPADD"    }, // microoperations in FP ADD unit
    {152, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x12,  "FPMUL"    }, // microoperations in FP MUL unit
    {153, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x24,  "FPMISC"   }, // microoperations in FP Store unit
    {150, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x01,  "UopsFP0"  }, // microoperations in FP pipe 0
    {151, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x02,  "UopsFP1"  }, // microoperations in FP pipe 1
    {152, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x04,  "UopsFP2"  }, // microoperations in FP pipe 2
    {153, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x08,  "UopsFP3"  }, // microoperations in FP pipe 3
    {110, S_AMD, AMD_BULLD,    0,   3,     0,   0x04,   0x0a,  "UopsElim" }, // move eliminations and scalar op optimizations
    {120, S_AMD, AMD_BULLD,    0,   3,     0,   0x2A,   0x01,  "Forwfail" }, // load-to-store forwarding failed
    {160, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x01,  "x87"      }, // FP x87 instructions
    {161, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x02,  "MMX"      }, // MMX instructions
    {162, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x04,  "XMM"      }, // XMM and YMM instructions

    // VIA Nano counters are undocumented
    // These are the ones I have found that counts. Most have unknown purpose
    //  id      scheme cpu    countregs eventreg event  mask   name
    {0x1000, S_VIA, P_ALL,   0,   1,     0,   0x000,    0,  "Instr" }, // Instructions
    {0x0001, S_VIA, P_ALL,   0,   1,     0,   0x001,    0,  "uops"  }, // micro-ops?
    {0x0002, S_VIA, P_ALL,   0,   1,     0,   0x002,    0,  "2"     }, // 
    {0x0003, S_VIA, P_ALL,   0,   1,     0,   0x003,    0,  "3"     }, // 
    {0x0004, S_VIA, P_ALL,   0,   1,     0,   0x004,    0,  "bubble"}, // Branch bubble clock cycles?
    {0x0005, S_VIA, P_ALL,   0,   1,     0,   0x005,    0,  "5"     }, // 
    {0x0006, S_VIA, P_ALL,   0,   1,     0,   0x006,    0,  "6"     }, // 
    {0x0007, S_VIA, P_ALL,   0,   1,     0,   0x007,    0,  "7"     }, // 
    {0x0008, S_VIA, P_ALL,   0,   1,     0,   0x008,    0,  "8"     }, // 
    {0x0009, S_VIA, P_ALL,   0,   1,     0,   0x000,    0,  "Instr" }, // Instructions
    {0x0010, S_VIA, P_ALL,   0,   1,     0,   0x010,    0,  "10"    }, // 
    {0x0014, S_VIA, P_ALL,   0,   1,     0,   0x014,    0,  "14"    }, // 
    {0x0020, S_VIA, P_ALL,   0,   1,     0,   0x020,    0,  "Br NT" }, // Branch not taken
    {0x0021, S_VIA, P_ALL,   0,   1,     0,   0x021,    0,  "Br NT Pr"}, // Branch not taken, predicted
    {0x0022, S_VIA, P_ALL,   0,   1,     0,   0x022,    0,  "Br Tk"   }, // Branch taken
    {0x0023, S_VIA, P_ALL,   0,   1,     0,   0x023,    0,  "Br Tk Pr"}, // Branch taken, predicted
    {0x0024, S_VIA, P_ALL,   0,   1,     0,   0x024,    0,  "Jmp"    }, // Jump or call
    {0x0025, S_VIA, P_ALL,   0,   1,     0,   0x025,    0,  "Jmp"    }, // Jump or call, predicted
    {0x0026, S_VIA, P_ALL,   0,   1,     0,   0x026,    0,  "Ind.Jmp"}, // Indirect jump or return
    {0x0027, S_VIA, P_ALL,   0,   1,     0,   0x027,    0,  "Ind.J. Pr"}, // Indirect jump or return, predicted
    {0x0034, S_VIA, P_ALL,   0,   1,     0,   0x034,    0,  "34"    }, // 
    {0x0040, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "40"    }, // 
    {0x0041, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "41"    }, // 
    {0x0042, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "42"    }, // 
    {0x0043, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "43"    }, // 
    {0x0044, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "44"    }, // 
    {0x0046, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "46"    }, // 
    {0x0048, S_VIA, P_ALL,   0,   1,     0,   0x040,    0,  "48"    }, // 
    {0x0082, S_VIA, P_ALL,   0,   1,     0,   0x082,    0,  "82"    }, // 
    {0x0083, S_VIA, P_ALL,   0,   1,     0,   0x083,    0,  "83"    }, // 
    {0x0084, S_VIA, P_ALL,   0,   1,     0,   0x084,    0,  "84"    }, // 
    {0x00B4, S_VIA, P_ALL,   0,   1,     0,   0x0B4,    0,  "B4"    }, // 
    {0x00C0, S_VIA, P_ALL,   0,   1,     0,   0x0C0,    0,  "C0"    }, // 
    {0x00C4, S_VIA, P_ALL,   0,   1,     0,   0x0C4,    0,  "C4"    }, // 
    {0x0104, S_VIA, P_ALL,   0,   1,     0,   0x104,    0, "104"    }, // 
    {0x0105, S_VIA, P_ALL,   0,   1,     0,   0x105,    0, "105"    }, // 
    {0x0106, S_VIA, P_ALL,   0,   1,     0,   0x106,    0, "106"    }, // 
    {0x0107, S_VIA, P_ALL,   0,   1,     0,   0x107,    0, "107"    }, // 
    {0x0109, S_VIA, P_ALL,   0,   1,     0,   0x109,    0, "109"    }, // 
    {0x010A, S_VIA, P_ALL,   0,   1,     0,   0x10A,    0, "10A"    }, // 
    {0x010B, S_VIA, P_ALL,   0,   1,     0,   0x10B,    0, "10B"    }, // 
    {0x010C, S_VIA, P_ALL,   0,   1,     0,   0x10C,    0, "10C"    }, // 
    {0x0110, S_VIA, P_ALL,   0,   1,     0,   0x110,    0, "110"    }, // 
    {0x0111, S_VIA, P_ALL,   0,   1,     0,   0x111,    0, "111"    }, // 
    {0x0116, S_VIA, P_ALL,   0,   1,     0,   0x116,    0, "116"    }, // 
    {0x0120, S_VIA, P_ALL,   0,   1,     0,   0x120,    0, "120"    }, // 
    {0x0121, S_VIA, P_ALL,   0,   1,     0,   0x121,    0, "121"    }, // 
    {0x013C, S_VIA, P_ALL,   0,   1,     0,   0x13C,    0, "13C"    }, // 
    {0x0200, S_VIA, P_ALL,   0,   1,     0,   0x200,    0, "200"    }, // 
    {0x0201, S_VIA, P_ALL,   0,   1,     0,   0x201,    0, "201"    }, // 
    {0x0206, S_VIA, P_ALL,   0,   1,     0,   0x206,    0, "206"    }, // 
    {0x0207, S_VIA, P_ALL,   0,   1,     0,   0x207,    0, "207"    }, // 
    {0x0301, S_VIA, P_ALL,   0,   1,     0,   0x301,    0, "301"    }, // 
    {0x0302, S_VIA, P_ALL,   0,   1,     0,   0x302,    0, "302"    }, // 
    {0x0303, S_VIA, P_ALL,   0,   1,     0,   0x303,    0, "303"    }, // 
    {0x0304, S_VIA, P_ALL,   0,   1,     0,   0x304,    0, "304"    }, // 
    {0x0305, S_VIA, P_ALL,   0,   1,     0,   0x305,    0, "305"    }, // 
    {0x0306, S_VIA, P_ALL,   0,   1,     0,   0x306,    0, "306"    }, // 
    {0x0502, S_VIA, P_ALL,   0,   1,     0,   0x502,    0, "502"    }, // 
    {0x0507, S_VIA, P_ALL,   0,   1,     0,   0x507,    0, "507"    }, // 
    {0x0508, S_VIA, P_ALL,   0,   1,     0,   0x508,    0, "508"    }, // 
    {0x050D, S_VIA, P_ALL,   0,   1,     0,   0x50D,    0, "50D"    }, // 
    {0x0600, S_VIA, P_ALL,   0,   1,     0,   0x600,    0, "600"    }, // 
    {0x0605, S_VIA, P_ALL,   0,   1,     0,   0x605,    0, "605"    }, // 
    {0x0607, S_VIA, P_ALL,   0,   1,     0,   0x607,    0, "607"    }, // 

    //  end of list   
    {0, S_UNKNOWN, P_UNKNOWN, 0,  0,     0,      0,     0,    0     }  // list must end with a record of all 0
};
