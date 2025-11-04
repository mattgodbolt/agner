//                       PMCTestA.cpp                2013-08-26 Agner Fog
//
//          Multithread PMC Test program for Windows and Linux
//
//
// This program is intended for testing the performance of a little piece of 
// code written in C, C++ or assembly. 
// The code to test is inserted at the place marked "Test code start" in
// PMCTestB.cpp, PMCTestB32.asm or PMCTestB64.asm.
// 
// Run as administrator, with driver signature enforcement off.
// See PMCTest.txt for further instructions.
//
// (c) 2000-2013 GNU General Public License www.gnu.org/licenses
//////////////////////////////////////////////////////////////////////////////

#include "PMCTest.h"
#include "CPUDetection.h"


//////////////////////////////////////////////////////////////////////
//
//        Thread synchronizer
//
//////////////////////////////////////////////////////////////////////

union USync {
#if MAXTHREADS > 4
    int64 allflags;                     // for MAXTHREADS = 8
#else
    int allflags;                       // for MAXTHREADS = 4
#endif
    char flag[MAXTHREADS];
};
volatile USync TSync = {0};

// processornumber for each thread
int ProcNum[MAXTHREADS] = {0};

// number of repetitions in each thread
int repetitions;

// Create CCounters instance
CCounters MSRCounters;


//////////////////////////////////////////////////////////////////////
//
//        Thread procedure
//
//////////////////////////////////////////////////////////////////////

ThreadProcedureDeclaration(ThreadProc1) {
    //DWORD WINAPI ThreadProc1(LPVOID parm) {
    // check thread number
    unsigned int threadnum = *(unsigned int*)parm;

    if (threadnum >= (unsigned int)NumThreads) {
        printf("\nThread number out of range %i", threadnum);
        return NULL;
    }

    // get desired processornumber
    int ProcessorNumber = ProcNum[threadnum];

    // Lock process to this processor number
    SyS::SetProcessMask(ProcessorNumber);

    // Start MSR counters
    MSRCounters.StartCounters(threadnum);

    // Wait for rest of timeslice
    SyS::Sleep0();

    // wait for other threads
    // Initialize synchronizer
    USync WaitTo;
    WaitTo.allflags = 0;
    for (int t = 0; t < NumThreads; t++) WaitTo.flag[t] = 1;
    // flag for this thead ready
    TSync.flag[threadnum] = 1;
    // wait for other threads to be ready
    while (TSync.allflags != WaitTo.allflags) {} // Note: will wait forever if a thread is not created

    // Run the test code
    repetitions = TestLoop(threadnum);

    // Wait for rest of timeslice
    SyS::Sleep0();

    // Start MSR counters
    MSRCounters.StopCounters(threadnum);

    return NULL;
};


//////////////////////////////////////////////////////////////////////
//
//        Main
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    int repi;                           // repetition counter
    int i;                              // loop counter
    int t;                              // thread counter
    int e;                              // error number
    int procthreads;                    // number of threads supported by processor

    // Limit number of threads
    if (NumThreads > MAXTHREADS) {
        NumThreads = MAXTHREADS;
        printf("\nToo many threads");
    }
    if (NumThreads < 1) NumThreads = 1;

    // Get mask of possible CPU cores
    SyS::ProcMaskType ProcessAffMask = SyS::GetProcessMask();
    // Count possible threads
    for (procthreads = i = 0; i < MAXTHREADS; i++) {
        if (SyS::TestProcessMask(i, &ProcessAffMask)) procthreads++;
    }

    // Fix a processornumber for each thread
    for (t = 0, i = NumThreads-1; t < NumThreads; t++, i--) {
        // make processornumbers different, and last thread = MainThreadProcNum:
        // ProcNum[t] = MainThreadProcNum ^ i;
        if (procthreads < 4) {        
            ProcNum[t] = i;
        }
        else {        
            ProcNum[t] = (i % 2) * (procthreads/2) + i / 2;
        }
        if (!SyS::TestProcessMask(ProcNum[t], &ProcessAffMask)) {
            // this processor core is not available
            printf("\nProcessor %i not available. Processors available:\n", ProcNum[t]);
            for (int p = 0; p < MAXTHREADS; p++) {
                if (SyS::TestProcessMask(p, &ProcessAffMask)) printf("%i  ", p);
            }
            printf("\n");
            return 1;
        }
    }

    // Make program and driver use the same processor number
    MSRCounters.LockProcessor();

    // Find counter defitions and put them in queue for driver
    MSRCounters.QueueCounters();

    // Install and load driver
    e = MSRCounters.StartDriver();
    if (e) return e;

    // Set high priority to minimize risk of interrupts during test
    SyS::SetProcessPriorityHigh();

    // Make multiple threads
    ThreadHandler Threads;
    Threads.Start(NumThreads);

    // Stop threads
    Threads.Stop();

    // Set priority back normal
    SyS::SetProcessPriorityNormal();

    // Clean up
    MSRCounters.CleanUp();

    // print column headings
    if (NumThreads > 1) printf("Processor,");
    printf("Clock,");
    if (UsePMC) {
        for (i = 0; i < NumCounters; i++) {
            printf("%s", MSRCounters.CounterNames[i]);
            if (i != NumCounters - 1) printf(",");
        }
    }
    printf("\n");
    // TODO: support RatioOut/TempOut?

    // Print results
    for (t = 0; t < NumThreads; t++) {
        // calculate offsets into ThreadData[]
        int TOffset = t * (ThreadDataSize / sizeof(int));
        int ClockOS = ClockResultsOS / sizeof(int);
        int PMCOS   = PMCResultsOS / sizeof(int);

        if (NumThreads > 1) printf("%i,", ProcNum[t]);
        // print counter outputs
        for (repi = 0; repi < repetitions; repi++) {
            printf("%i,", PThreadData[repi+TOffset+ClockOS]);
            if (UsePMC) {
                for (i = 0; i < NumCounters; i++) {         
                    printf("%i", PThreadData[repi+i*repetitions+TOffset+PMCOS]);
                    if (i != NumCounters - 1) printf(",");
                }
            }
            printf("\n");
        }
    }

    // Exit
    return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
//        CMSRInOutQue class member functions
//
//////////////////////////////////////////////////////////////////////////////

// Constructor
CMSRInOutQue::CMSRInOutQue() {
    n = 0;
    for (int i = 0; i < MAX_QUE_ENTRIES+1; i++) {
        queue[i].msr_command = MSR_STOP;
    }
}

// Put data record in queue
int CMSRInOutQue::put (EMSR_COMMAND msr_command, unsigned int register_number,
                       unsigned int value_lo, unsigned int value_hi) {

                           if (n >= MAX_QUE_ENTRIES) return -10;

                           queue[n].msr_command = msr_command;
                           queue[n].register_number = register_number;
                           queue[n].val[0] = value_lo;
                           queue[n].val[1] = value_hi;
                           n++;
                           return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
//        CCounters class member functions
//
//////////////////////////////////////////////////////////////////////////////

// Constructor
CCounters::CCounters() {
    // Set everything to zero
    MVendor = VENDOR_UNKNOWN;
    MFamily = P_UNKNOWN;
    MScheme = S_UNKNOWN;
    NumPMCs = 0;
    NumFixedPMCs = 0;
    ProcessorNumber = 0;
    for (int i = 0; i < MAXCOUNTERS; i++) CounterNames[i] = 0;
}

void CCounters::QueueCounters() {
    // Put counter definitions in queue
    int n = 0, CounterType; 
    const char * err;
    while (CounterDefinitions[n].ProcessorFamily || CounterDefinitions[n].CounterType) n++;
    NumCounterDefinitions = n;

    // Get processor information
    CPUDetection cpuDetect;
    MVendor = cpuDetect.GetVendor();
    MFamily = cpuDetect.GetFamily();
    MScheme = cpuDetect.GetScheme();

    // Get additional PMC information (NumPMCs, NumFixedPMCs)
    NumPMCs = 2;
    NumFixedPMCs = 0;
    if (MVendor == AMD) {
        NumPMCs = 4;
    }
    else if (MVendor == INTEL) {
        int CpuIdOutput[4];
        Cpuid(CpuIdOutput, 0);
        if (CpuIdOutput[0] >= 0x0A) {
            Cpuid(CpuIdOutput, 0x0A);
            if (CpuIdOutput[0] & 0xFF) {
                NumPMCs = (CpuIdOutput[0] >> 8) & 0xFF;
                NumFixedPMCs = CpuIdOutput[3] & 0x1F;
            }
        }
    }

    if (UsePMC) {   
        // Get all counter requests
        for (int i = 0; i < MaxNumCounters; i++) {
            CounterType = CounterTypesDesired[i];
            err = DefineCounter(CounterType);
            if (err) {
                printf("\nCannot make counter %i. %s\n", i+1, err);
            }
        }  
    }
}

void CCounters::LockProcessor() {
    // Make program and driver use the same processor number if multiple processors
    // Enable RDMSR instruction
    int thread, procnum;

    // We must lock the driver call to the desired processor number
    for (thread = 0; thread < NumThreads; thread++) {
        procnum = ProcNum[thread];
        // lock driver to the same processor number as thread
        queue1[thread].put(PROC_SET, 0, procnum);
        queue2[thread].put(PROC_SET, 0, procnum);
        // enable readpmc instruction (for this processor number)
        queue1[thread].put(PMC_ENABLE, 0, 0);
        // disable readpmc instruction after run
        queue2[thread].put(PMC_DISABLE, 0, 0);
    }
}

int CCounters::StartDriver() {
    // Install and load driver
    // return error code
    int ErrNo = 0;

    if (UsePMC) {
        // Load driver
        ErrNo = msr.LoadDriver();
    }

    return ErrNo;
}

void CCounters::CleanUp() {
    // Any required cleanup of driver etc
    // Optionally unload driver
    //msr.UnloadDriver();
    //msr.UnInstallDriver();
}

// put record into multiple start queues
void CCounters::Put1 (int num_threads,
    EMSR_COMMAND msr_command, unsigned int register_number,
    unsigned int value_lo, unsigned int value_hi) {
    for (int t = 0; t < num_threads; t++) {
        queue1[t].put(msr_command, register_number, value_lo, value_hi);
    }
}

// put record into multiple stop queues
void CCounters::Put2 (int num_threads,
    EMSR_COMMAND msr_command, unsigned int register_number,
    unsigned int value_lo, unsigned int value_hi) {
    for (int t = 0; t < num_threads; t++) {
        queue2[t].put(msr_command, register_number, value_lo, value_hi);
    }
}

// Start counting
void CCounters::StartCounters(int ThreadNum) {
    if (UsePMC) {
        msr.AccessRegisters(queue1[ThreadNum]);
    }
}

// Stop and reset counters
void CCounters::StopCounters(int ThreadNum) {
    if (UsePMC) {
        msr.AccessRegisters(queue2[ThreadNum]);
    }
}

// Request a counter setup
// (return value is error message)
const char * CCounters::DefineCounter(int CounterType) {
    if (CounterType == 0) return NULL;
    int i;
    SCounterDefinition * p;

    // Search for matching counter definition
    for (i=0, p = CounterDefinitions; i < NumCounterDefinitions; i++, p++) {
        if (p->CounterType == CounterType && (p->PMCScheme & MScheme) && (p->ProcessorFamily & MFamily)) {
            // Match found
            break;
        }
    }
    if (i >= NumCounterDefinitions) return "No matching counter definition found"; // not found in list
    return DefineCounter(*p);
}

// Request a counter setup
// (return value is error message)
const char * CCounters::DefineCounter(SCounterDefinition & CDef) {
    int i, counternr, a, b, reg, eventreg, tag;
    static int CountersEnabled = 0, FixedCountersEnabled = 0;

    if ( !(CDef.ProcessorFamily & MFamily)) return "Counter not defined for present microprocessor family";
    if (NumCounters >= MaxNumCounters) return "Too many counters";

    if (CDef.CounterFirst & 0x40000000) { 
        // Fixed function counter
        counternr = CDef.CounterFirst;
    }
    else {
        // check CounterLast
        if (CDef.CounterLast < CDef.CounterFirst) {
            CDef.CounterLast = CDef.CounterFirst;
        }
        if (CDef.CounterLast >= NumPMCs && (MScheme & S_INTL)) {
        }   

        // Find vacant counter
        for (counternr = CDef.CounterFirst; counternr <= CDef.CounterLast; counternr++) {
            // Check if this counter register is already in use
            for (i = 0; i < NumCounters; i++) {
                if (counternr == Counters[i]) {
                    // This counter is already in use, find another
                    goto USED;
                }
            }
            if (MFamily == INTEL_P4) {
                // Check if the corresponding event register ESCR is already in use
                eventreg = GetP4EventSelectRegAddress(counternr, CDef.EventSelectReg); 
                for (i = 0; i < NumCounters; i++) {
                    if (EventRegistersUsed[i] == eventreg) {
                        goto USED;
                    }
                }
            }

            // Vacant counter found. stop searching
            break;

        USED:;
            // This counter is occupied. keep searching
        }

        if (counternr > CDef.CounterLast) {
            // No vacant counter found
            return "Counter registers are already in use";
        }
    }

    // Vacant counter found. Save name   
    CounterNames[NumCounters] = CDef.Description;

    // Put MSR commands for this counter in queues
    switch (MScheme) {

    case S_P1:
        // Pentium 1 and Pentium MMX
        a = CDef.Event | (CDef.EventMask << 6);
        if (counternr == 1) a = EventRegistersUsed[0] | (a << 16);
        Put1(NumThreads, MSR_WRITE, 0x11, a);
        Put2(NumThreads, MSR_WRITE, 0x11, 0);
        Put1(NumThreads, MSR_WRITE, 0x12+counternr, 0);
        Put2(NumThreads, MSR_WRITE, 0x12+counternr, 0);
        EventRegistersUsed[0] = a;
        break;

    case S_ID2: case S_ID3:
        // Intel Core 2 and later
        if (counternr & 0x40000000) {
            // This is a fixed function counter
            if (!(FixedCountersEnabled++)) {
                // Enable fixed function counters
                for (a = i = 0; i < NumFixedPMCs; i++) {
                    b = 2;  // 1=privileged level, 2=user level, 4=any thread
                    a |= b << (4*i);
                }
                // Set MSR_PERF_FIXED_CTR_CTRL
                Put1(NumThreads, MSR_WRITE, 0x38D, a); 
                Put2(NumThreads, MSR_WRITE, 0x38D, 0);
            }
            break;
        }
        if (!(CountersEnabled++)) {
            // Enable counters
            a = (1 << NumPMCs) - 1;      // one bit for each pmc counter
            b = (1 << NumFixedPMCs) - 1; // one bit for each fixed counter
            // set MSR_PERF_GLOBAL_CTRL
            Put1(NumThreads, MSR_WRITE, 0x38F, a, b);
            Put2(NumThreads, MSR_WRITE, 0x38F, 0);
        }
        // All other counters continue in next case:

    case S_P2: case S_ID1:
        // Pentium Pro, Pentium II, Pentium III, Pentium M, Core 1, (Core 2 continued):


        a = CDef.Event | (CDef.EventMask << 8) | (1 << 16) | (1 << 22);
        if (MScheme == S_ID1) a |= (1 << 14);  // Means this core only
        //if (MScheme == S_ID3) a |= (1 << 22);  // Means any thread in this core!

        eventreg = 0x186 + counternr;             // IA32_PERFEVTSEL0,1,..
        reg = 0xc1 + counternr;                   // IA32_PMC0,1,..
        Put1(NumThreads, MSR_WRITE, eventreg, a);
        Put2(NumThreads, MSR_WRITE, eventreg, 0);
        Put1(NumThreads, MSR_WRITE, reg, 0);
        Put2(NumThreads, MSR_WRITE, reg, 0);
        break;

    case S_P4:
        // Pentium 4 and Pentium 4 with EM64T
        // ESCR register
        eventreg = GetP4EventSelectRegAddress(counternr, CDef.EventSelectReg); 
        tag = 1;
        a = 0x1C | (tag << 5) | (CDef.EventMask << 9) | (CDef.Event << 25);
        Put1(NumThreads, MSR_WRITE, eventreg, a);
        Put2(NumThreads, MSR_WRITE, eventreg, 0);
        // Remember this event register is used
        EventRegistersUsed[NumCounters] = eventreg;
        // CCCR register
        reg = counternr + 0x360;
        a = (1 << 12) | (3 << 16) | (CDef.EventSelectReg << 13);
        Put1(NumThreads, MSR_WRITE, reg, a);
        Put2(NumThreads, MSR_WRITE, reg, 0);
        // Reset counter register
        reg = counternr + 0x300;
        Put1(NumThreads, MSR_WRITE, reg, 0);
        Put2(NumThreads, MSR_WRITE, reg, 0);
        // Set high bit for fast readpmc
        counternr |= 0x80000000;
        break;

    case S_AMD:
        // AMD Athlon, Athlon 64, Opteron
        a = CDef.Event | (CDef.EventMask << 8) | (1 << 16) | (1 << 22);
        eventreg = 0xc0010000 + counternr;
        reg = 0xc0010004 + counternr;
        Put1(NumThreads, MSR_WRITE, eventreg, a);
        Put2(NumThreads, MSR_WRITE, eventreg, 0);
        Put1(NumThreads, MSR_WRITE, reg, 0);
        Put2(NumThreads, MSR_WRITE, reg, 0);
        break;

    case S_VIA:
        // VIA Nano. Undocumented!
        a = CDef.Event | (1 << 16) | (1 << 22);
        eventreg = 0x186 + counternr;
        reg = 0xc1 + counternr;
        Put1(NumThreads, MSR_WRITE, eventreg, a);
        Put2(NumThreads, MSR_WRITE, eventreg, 0);
        Put1(NumThreads, MSR_WRITE, reg, 0);
        Put2(NumThreads, MSR_WRITE, reg, 0);
        break;

    default:
        return "No counters defined for present microprocessor family";
    }

    // Save counter register number in Counters list
    Counters[NumCounters++] = counternr;

    return NULL; // NULL = success
}


// Translate event select register number to register address for P4 processor
int CCounters::GetP4EventSelectRegAddress(int CounterNr, int EventSelectNo) {
    // On Pentium 4 processors, the Event Select Control Registers (ESCR) are
    // identified in a very confusing way. Each ESCR has both an ESCRx-number which 
    // is part of its name, an event select number to specify in the Counter 
    // Configuration Control Register (CCCR), and a register address to specify 
    // in the WRMSR instruction.
    // This function gets the register address based on table 15-6 in Intel manual 
    // 25366815, IA-32 Intel Architecture Software Developer's Manual Volume 3: 
    // System Programming Guide, 2005.
    // Returns -1 if error.
    static int TranslationTables[4][8] = {
        {0x3B2, 0x3B4, 0x3AA, 0x3B6, 0x3AC, 0x3C8, 0x3A2, 0x3A0}, // counter 0-3
        {0x3C0, 0x3C4, 0x3C2,    -1,    -1,    -1,    -1,    -1}, // counter 4-7
        {0x3A6, 0x3A4, 0x3AE, 0x3B0,    -1, 0x3A8,    -1,    -1}, // counter 8-11
        {0x3BA, 0x3CA, 0x3BC, 0x3BE, 0x3B8, 0x3CC, 0x3E0,    -1}};// counter 12-17

        unsigned int n = CounterNr;
        if (n > 17) return -1;
        if (n > 15) n -= 3;
        if ((unsigned int)EventSelectNo > 7) return -1;

        int a = TranslationTables[n/4][EventSelectNo];
        if (a < 0) return a;
        if (n & 2) a++;
        return a;
}


//////////////////////////////////////////////////////////////////////////////
//
//             list of counter definitions
//
//////////////////////////////////////////////////////////////////////////////
// How to add new entries to this list:
//
// Warning: Be sure to save backup copies of your files before you make any 
// changes here. A wrong register number can result in a crash in the driver.
// This results in a blue screen and possibly loss of your most recently
// modified file.
//
// Set CounterType to any vacant id number. Use the same id for similar events
// in different processor families. The other fields depend on the processor 
// family as follows:
//
// Pentium 1 and Pentium MMX:
//    Set ProcessorFamily = INTEL_P1MMX.
//    CounterFirst = 0, CounterLast = 1, Event = Event number,
//    EventMask = Counter control code.
//
// Pentium Pro, Pentium II, Pentium III, Pentium M, Core Solo/Duo
//    Set ProcessorFamily = INTEL_P23M for events that are valid for all these
//    processors or INTEL_PM or INTEL_CORE for events that only apply to 
//    one processor.
//    CounterFirst = 0, CounterLast = 1, 
//    Event = Event number, EventMask = Unit mask.
//
// Core 2
//    Set ProcessorFamily = INTEL_CORE2.
//    Fixed function counters:
//    CounterFirst = 0x40000000 + MSR Address - 0x309. (Intel manual Oct. 2006 is wrong)
//    All other counters:
//    CounterFirst = 0, CounterLast = 1, 
//    Event = Event number, EventMask = Unit mask.
//
// Pentium 4 and Pentium 4 with EM64T (Netburst):
//    Set ProcessorFamily = INTEL_P4.
//    Look in Software Developer's Manual vol. 3 appendix A, table of 
//    Performance Monitoring Events.
//    Set CounterFirst and CounterLast to the range of possible counter
//    registers listed under "Counter numbers per ESCR".
//    Set EventSelectReg to the value listed for "CCCR Select".
//    Set Event to the value indicated for "ESCR Event Select".
//    Set EventMask to a combination of the relevant bits for "ESCR Event Mask".
//    You don't need the table named "Performance Counter MSRs and Associated
//    CCCR and ESCR MSRs". This table is already implemented in the function
//    CCounters::GetP4EventSelectRegAddress.
//
// AMD Athlon 64, Opteron
//    Set ProcessorFamily = AMD_ATHLON64.
//    CounterFirst = 0, CounterLast = 3, Event = Event mask,
//    EventMask = Unit mask.
//

