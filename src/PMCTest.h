////// PMCTest.h ///////////////////////////////////////// 2013-08-26 //
//
//            Header file for multithreaded PMC Test program
//
// This header file is included by PMCTestA.cpp and PMCTestB.cpp. 
// Please see PMCTest.txt for description of the program.
//
// This header file contains class declarations, function prototypes, 
// constants and other definitions for this program.
//
// Copyright 2005-2013 by Agner Fog. GNU General Public License www.gnu.org/licences
//////////////////////////////////////////////////////////////////////////////
#pragma once

// maximum number of threads. Must be 4 or 8.
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64)
#define MAXTHREADS  8
#else
#define MAXTHREADS  4
#endif

#include "MSRDriver.h"

// maximum number of performance counters used
const int MAXCOUNTERS = 6;

// maximum number of repetitions
const int MAXREPEAT = 128;

// max name length of counters
const int COUNTERNAMELEN = 10; 

class CMSRInOutQue {
public:
    // constructor
    CMSRInOutQue();
    // put record in queue
    int put (EMSR_COMMAND msr_command, unsigned int register_number,
        unsigned int value_lo, unsigned int value_hi = 0);
    // list of entries
    SMSRInOut queue[MAX_QUE_ENTRIES+1];
    // get size of queue
    int GetSize () {return n;}
protected:
    // number of entries
    int n;
};

#if defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64) 
// System-specific definitions for Windows
#include "PMCTestWin.h"
#elif defined(__unix__) || defined(__linux__)
// System-specific definitions for Linux
#include "PMCTestLinux.h"
#else
#error Unknown platform
#endif

// codes for processor vendor
enum EProcVendor {VENDOR_UNKNOWN = 0, INTEL, AMD, VIA};

// codes for processor family
enum EProcFamily {
    P_UNKNOWN    = 0,                       // unknown. can't do performance monitoring
    P_ALL = 0xFFFFFFFF,                     // All processors with the specified scheme
    INTEL_P1MMX  = 1,                       // Intel Pentium 1 or Pentium MMX
    INTEL_P23    = 2,                       // Pentium Pro, Pentium 2, Pentium 3
    INTEL_PM     = 4,                       // Pentium M, Core, Core 2
    INTEL_P4     = 8,                       // Pentium 4 and Pentium 4 with EM64T
    INTEL_CORE   = 0x10,                    // Intel Core Solo/Duo
    INTEL_P23M   = 0x16,                    // Pentium Pro, Pentium 2, Pentium 3, Pentium M, Core1
    INTEL_CORE2  = 0x20,                    // Intel Core 2
    INTEL_7      = 0x40,                    // Intel Core i7, Nehalem, Sandy Bridge
    INTEL_IVY    = 0x80,                    // Intel Ivy Bridge
    INTEL_7I     = 0xC0,                    // Nehalem, Sandy Bridge, Ivy bridge
    INTEL_HASW   = 0x100,                   // Intel Haswell
    INTEL_BROADWELL = 0x200,                // Intel Broadwell (5th gen)
    INTEL_SKYLAKE = 0x400,                  // Intel Skylake (6th gen)
    INTEL_KABYLAKE = 0x800,                 // Intel Kaby/Coffee/Comet Lake (7th-10th gen, 14nm)
    INTEL_ATOM   = 0x1000,                  // Intel Atom
    INTEL_ICELAKE = 0x2000,                 // Intel Ice Lake (10th gen, 10nm)
    INTEL_TIGERLAKE = 0x4000,               // Intel Tiger Lake (11th gen)

    AMD_ATHLON   = 0x10000,                 // AMD Athlon
    AMD_ATHLON64 = 0x20000,                 // AMD Athlon 64 or Opteron
    AMD_BULLD    = 0x80000,                 // AMD Family 15h (Bulldozer)
    AMD_ALL      = 0xF0000,                 // AMD any processor

    VIA_NANO     = 0x100000,                 // VIA Nano (Centaur)
};

// codes for PMC scheme
enum EPMCScheme {
    S_UNKNOWN = 0,                           // unknown. can't do performance monitoring
    S_P1   = 0x0001,                         // Intel Pentium, Pentium MMX
    S_P4   = 0x0002,                         // Intel Pentium 4, Netburst
    S_P2   = 0x0010,                         // Intel Pentium 2, Pentium M
    S_ID1  = 0x0020,                         // Intel Core solo/duo
    S_ID2  = 0x0040,                         // Intel Core 2
    S_ID3  = 0x0080,                         // Intel Core i7 and later and Atom
    S_P2MC = 0x0030,                         // Intel Pentium 2, Pentium M, Core solo/duo
    S_ID23 = 0x00C0,                         // Intel Core 2 and later
    S_INTL = 0x00F0,                         // Most Intel schemes
    S_AMD  = 0x1000,                         // AMD processors
    S_VIA  = 0x2000                          // VIA Nano processor and later
};


// record specifying how to count a particular event on a particular CPU family
struct SCounterDefinition {
    int          CounterType;                // ID identifying what to count
    EPMCScheme   PMCScheme;                  // PMC scheme. values may be OR'ed
    EProcFamily  ProcessorFamily;            // processor family. values may be OR'ed
    int          CounterFirst, CounterLast;  // counter number or a range of possible alternative counter numbers
    int          EventSelectReg;             // event select register
    int          Event;                      // event code
    int          EventMask;                  // event mask
    char         Description[COUNTERNAMELEN];// name of counter. length must be < COUNTERNAMELEN
};


// class CCounters defines, starts and stops MSR counters
class CCounters {
public:
    CCounters();                             // constructor
    const char * DefineCounter(int CounterType);   // request a counter setup
    const char * DefineCounter(SCounterDefinition & CounterDef); // request a counter setup
    void LockProcessor();                    // Make program and driver use the same processor number
    void QueueCounters();                    // Put counter definitions in queue
    int  StartDriver();                      // Install and load driver
    void StartCounters(int ThreadNum);       // start counting
    void StopCounters (int ThreadNum);       // stop and reset counters
    void CleanUp();                          // Any required cleanup of driver etc
    CMSRDriver msr;                          // interface to MSR access driver
    char * CounterNames[MAXCOUNTERS];        // name of each counter
    void Put1 (int num_threads,              // put record into multiple start queues
        EMSR_COMMAND msr_command, unsigned int register_number,
        unsigned int value_lo, unsigned int value_hi = 0);
    void Put2 (int num_threads,              // put record into multiple stop queues
        EMSR_COMMAND msr_command, unsigned int register_number,
        unsigned int value_lo, unsigned int value_hi = 0);
    void GetProcessorVendor();               // get microprocessor vendor
    void GetProcessorFamily();               // get microprocessor family
    void GetPMCScheme();                     // get PMC scheme
protected:
    CMSRInOutQue queue1[MAXTHREADS];         // que of MSR commands to do by StartCounters()
    CMSRInOutQue queue2[MAXTHREADS];         // que of MSR commands to do by StopCounters()
    // translate event select number to register address for P4 processor:
    static int GetP4EventSelectRegAddress(int CounterNr, int EventSelectNo); 
    int NumCounterDefinitions;               // number of possible counter definitions in table CounterDefinitions
    EProcVendor MVendor;                     // microprocessor vendor
    EProcFamily MFamily;                     // microprocessor type and family
    EPMCScheme  MScheme;                     // PMC monitoring scheme
    int NumPMCs;                             // Number of general PMCs
    int NumFixedPMCs;                        // Number of fixed function PMCs
    int ProcessorNumber;                     // main thread processor number in multiprocessor systems
};


extern "C" {

    // Link to PMCTestB.cpp, PMCTestB32.asm or PMCTestB64.asm:
    // The basic test loop containing the code to test
    int TestLoop (int thread);              // test loop

}


//////////////////////////////////////////////////////////////////////////////
//    Global variables imported from PMCTestBxx module
//////////////////////////////////////////////////////////////////////////////

extern "C" {

    extern SCounterDefinition CounterDefinitions[];

    extern int NumThreads;                  // number of threads
    // performance counters used
    extern int NumCounters;                // Number of PMC counters defined Counters[]
    extern int MaxNumCounters;             // Maximum number of PMC counters
    extern int UsePMC;                     // 0 if no PMC counters used
    extern int CounterTypesDesired[MAXCOUNTERS];// list of desired counter types
    extern int EventRegistersUsed[MAXCOUNTERS]; // index of counter registers used
    extern int Counters[MAXCOUNTERS];      // PMC register numbers

    // count results (all threads)
    extern int * PThreadData;              // Pointer to measured data for all threads
    extern int ThreadDataSize;             // Size of per-thread counter data block (bytes)
    extern int ClockResultsOS;             // offset of clock results of first thread into ThreadData (bytes)
    extern int PMCResultsOS;               // offset of PMC results of first thread into ThreadData (bytes)

    // optional extra output of ratio between two performance counts
    extern int RatioOut[4];                // RatioOut[0] = 0: no ratio output, 1 = int, 2 = float
                                           // RatioOut[1] = numerator (0 = clock, 1 = first PMC, etc., -1 = none)
                                           // RatioOut[2] = denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                                           // RatioOut[3] = factor, int or float according to RatioOut[0]

    extern int TempOut;                    // Use CountTemp (possibly extended into CountOverhead) for arbitrary output
                                           // 0 = no extra output
                                           // 2 = signed 32-bit integer
                                           // 3 = signed 64-bit integer
                                           // 4 = 32-bit integer, hexadecimal
                                           // 5 = 64-bit integer, hexadecimal
                                           // 6 = float
                                           // 7 = double
    extern const char * RatioOutTitle;     // Column heading for optional extra output of ratio
    extern const char * TempOutTitle;      // Column heading for optional arbitrary output
}
