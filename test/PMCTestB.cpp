//                       PMCTestB.cpp                    © 2012-03-02 Agner Fog
//
//          Multithread PMC Test program for Windows and Linux
//
// This program is intended for testing the performance of a little piece of 
// code written in C or C++. The code to test is inserted at the place marked
// "Test code start".
// All sections that can be modified by the user are marked with ###########. 
//
// The code to test will be executed REPETITIONS times and the test results
// will be output for each repetition. This program measures how many clock
// cycles the code to test takes in each repetition. Furthermore, it is 
// possible to set a number of Performance Monitor Counters (PMC) to count 
// the number of micro-operations (uops), cache misses, branch mispredictions,
// etc.
// 
// The setup of the Performance Monitor Counters is microprocessor-specific.
// The specifications for PMC setup for each microprocessor family is defined
// in the tables CounterDefinitions and CounterTypesDesired.
//
// See PMCTest.txt for instructions.
//
// © 2000-2012 GNU General Public License www.gnu.org/licences
//////////////////////////////////////////////////////////////////////////////

#include "PMCTest.h"


/*############################################################################
#
#        Define constants
#
############################################################################*/

// number of repetitions of test. You may change this up to MAXREPEAT
#define REPETITIONS  8

// Number of threads
#define NUM_THREADS  1

// Use performance monitor counters. Set to 0 if not used
#define USE_PERFORMANCE_COUNTERS  1

// Subtract overhead from counts (0 if not)
#define SUBTRACT_OVERHEAD 1

// Number of repetitions in loop to find overhead
#define OVERHEAD_REPETITIONS  5

// Cache line size (for preventing threads using same cache lines)
#define CACHELINESIZE  64


/*############################################################################
#
#        list of desired counter types
#
############################################################################*/
// 
// Here you can select which performance monitor counters you want for your test.
// Select id numbers from the table CounterDefinitions[] in PMCTestA.cpp.
// The maximum number of counters you can have is MAXCOUNTERS.
// Insert zeroes if you have less than MAXCOUNTERS counters.

extern "C" {
    int CounterTypesDesired[MAXCOUNTERS] = {
        1,    // core clock cycles (Intel Core 2 and later)
        9,    // instructions (not P4)
        100,    // micro-operations
        310     // data cache mises
    };
}


/*############################################################################
#
#        Thread data
#
############################################################################*/
// Align SThreadData structure by cache line size to avoid multiple threads
// writing to the same cache line
ALIGNEDSTRUCTURE(SThreadData, CACHELINESIZE) {
    //__declspec(align(CACHELINESIZE)) struct SThreadData {
    // Data for each thread
    int CountTemp[MAXCOUNTERS+1];      // temporary storage of clock counts and PMC counts
    int CountOverhead[MAXCOUNTERS+1];  // temporary storage of count overhead
    int ClockResults[REPETITIONS];     // clock count results
    int PMCResults[REPETITIONS*MAXCOUNTERS]; // PMC count results
};

extern "C" {
    SThreadData ThreadData[NUM_THREADS];// Results for all threads
    int NumThreads = NUM_THREADS;       // Number of threads
    int NumCounters = 0;                // Number of valid PMC counters in Counters[]
    int MaxNumCounters = MAXCOUNTERS;   // Maximum number of PMC counters
    int UsePMC = USE_PERFORMANCE_COUNTERS;// 0 if no PMC counters used
    int *PThreadData = (int*)ThreadData;// Pointer to measured data for all threads
    int ThreadDataSize = sizeof(SThreadData);// Size of per-thread counter data block (bytes)
    // offset of clock results of first thread into ThreadData (bytes)
    int ClockResultsOS = (ThreadData[0].ClockResults-ThreadData[0].CountTemp)*sizeof(int);
    // offset of PMC results of first thread into ThreadData (bytes)
    int PMCResultsOS = (ThreadData[0].PMCResults-ThreadData[0].CountTemp)*sizeof(int);
    // counter register numbers used
    int Counters[MAXCOUNTERS] = {0};
    int EventRegistersUsed[MAXCOUNTERS] = {0};
    // optional extra output
    int RatioOut[4] = {0};              // See PMCTest.h for explanation
    int TempOut = 0;                    // See PMCTest.h for explanation
}

/*############################################################################
#
#        User data
#
############################################################################*/

// Put any data definitions your test code needs here:

#define ROUND_UP(A,B)  ((A+B-1)/B*B)  // Round up A to nearest multiple of B

// Make sure USER_DATA_SIZE is a multiple of the cache line size, because there
// is a penalty if multiple threads access the same cache line:
#define USER_DATA_SIZE  ROUND_UP(1000,CACHELINESIZE) 

int UserData[NUM_THREADS][USER_DATA_SIZE];



//////////////////////////////////////////////////////////////////////////////
//    Test Loop
//////////////////////////////////////////////////////////////////////////////

int TestLoop (int thread) {
    // this function runs the code to test REPETITIONS times
    // and reads the counters before and after each run:
    int i;                        // counter index
    int repi;                     // repetition index

    for (i = 0; i < MAXCOUNTERS+1; i++) {
        ThreadData[thread].CountOverhead[i] = 0x7FFFFFFF;
    }

    /*############################################################################
    #
    #        Initializations
    #
    ############################################################################*/

    // place any user initializations here:


    /*############################################################################
    #
    #        Initializations end
    #
    ############################################################################*/

    // first test loop. 
    // Measure overhead = the test count produced by the test program itself
    for (repi = 0; repi < OVERHEAD_REPETITIONS; repi++) {

        Serialize();

#if USE_PERFORMANCE_COUNTERS
        // Read counters
        for (i = 0; i < MAXCOUNTERS; i++) {
            ThreadData[thread].CountTemp[i+1] = (int)Readpmc(Counters[i]);
        }
#endif

        Serialize();
        ThreadData[thread].CountTemp[0] = (int)Readtsc();
        Serialize();

        // no test code here

        Serialize();
        ThreadData[thread].CountTemp[0] -= (int)Readtsc();
        Serialize();

#if USE_PERFORMANCE_COUNTERS
        // Read counters
        for (i = 0; i < MAXCOUNTERS; i++) {
            ThreadData[thread].CountTemp[i+1] -= (int)Readpmc(Counters[i]);
        }
#endif
        Serialize();

        // find minimum counts
        for (i = 0; i < MAXCOUNTERS+1; i++) {
            if (-ThreadData[thread].CountTemp[i] < ThreadData[thread].CountOverhead[i]) {
                ThreadData[thread].CountOverhead[i] = -ThreadData[thread].CountTemp[i];
            }
        }
    }


    // Second test loop. Includes code to test.
    // This must be identical to first test loop, except for the test code
    for (repi = 0; repi < REPETITIONS; repi++) {

        Serialize();

#if USE_PERFORMANCE_COUNTERS
        // Read counters
        for (i = 0; i < MAXCOUNTERS; i++) {
            ThreadData[thread].CountTemp[i+1] = (int)Readpmc(Counters[i]);
        }
#endif

        Serialize();
        ThreadData[thread].CountTemp[0] = (int)Readtsc();
        Serialize();


        /*############################################################################
        #
        #        Test code start
        #
        ############################################################################*/

        // Put the code to test here,
        // or a call to a function defined in a separate module
        //½½

        for (i = 0; i < 1000; i++) UserData[thread][i] *= 99;


        /*############################################################################
        #
        #        Test code end
        #
        ############################################################################*/

        Serialize();
        ThreadData[thread].CountTemp[0] -= (int)Readtsc();
        Serialize();

#if USE_PERFORMANCE_COUNTERS
        // Read counters
        for (i = 0; i < MAXCOUNTERS; i++) {
            ThreadData[thread].CountTemp[i+1] -= (int)Readpmc(Counters[i]);
        }
#endif
        Serialize();

        // subtract overhead
        ThreadData[thread].ClockResults[repi] = -ThreadData[thread].CountTemp[0] - ThreadData[thread].CountOverhead[0];
        for (i = 0; i < MAXCOUNTERS; i++) {
            ThreadData[thread].PMCResults[repi+i*REPETITIONS] = -ThreadData[thread].CountTemp[i+1] - ThreadData[thread].CountOverhead[i+1];
        }
    }

    // return
    return REPETITIONS;
}
