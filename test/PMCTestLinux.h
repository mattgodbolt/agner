//                     PMCTestLinux.h                    © 2012-03-02 Agner Fog
//
//          Multithread PMC Test program
//          System-specific definitions for Linux
//
// See PMCTest.txt for instructions.
//
// (c) Copyright 2000-2012 by Agner Fog. GNU General Public License www.gnu.org/licences
//////////////////////////////////////////////////////////////////////////////

#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/unistd.h>  // __NR_gettid
#include <stdio.h>

#include "MSRdrvL.h" // shared with driver

// comment out next line if compiler doesn't support intrinsic functions:
//#include <intrin.h>


//////////////////////////////////////////////////////////////////////////////
//
//  Definitions due to different compiler syntax
//
//////////////////////////////////////////////////////////////////////////////

// define 64 bit integer
typedef long long int64;
typedef unsigned long long uint64;

// Define macro for aligned structure, gcc syntax
#define ALIGNEDSTRUCTURE(Name, Align) struct __attribute__((aligned(Align))) Name


// Define low level functions
#ifdef __INTRIN_H_  // Use intrinsics for low level functions

static inline void Serialize () {
    // serialize CPU by cpuid function 0
    int dummy[4];
    __cpuid(dummy, 0);
    // Prevent the compiler from optimizing away the whole Serialize function:
    volatile int DontSkip = dummy[0];
}
#define Cpuid __cpuid
#define Readtsc __rdtsc
#define Readpmc __readpmc

#else // Intrinsics not supported, use inline assembly
// This version is for gas/AT&T syntax

static void Cpuid (int Output[4], int aa) {	
    int a, b, c, d;
    __asm("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(aa),"c"(0) : );
    Output[0] = a;
    Output[1] = b;
    Output[2] = c;
    Output[3] = d;
}

static inline void Serialize () {
    // serialize CPU
    __asm__ __volatile__ ( "xorl %%eax, %%eax \n cpuid " : : : "%eax","%ebx","%ecx","%edx" );
}

static inline int Readtsc() {
    // read time stamp counter
    int r;
    __asm__ __volatile__ ( "rdtsc" : "=a"(r) : : "%edx");    
    return r;
}

static inline int Readpmc(int nPerfCtr) {
    // read performance monitor counter number nPerfCtr
    int r;
    __asm__ __volatile__ ( "rdpmc" : "=a"(r) : "c"(nPerfCtr) : "%edx");    
    return r;
}
#endif  // __INTRIN_H_ 

//////////////////////////////////////////////////////////////////////////////
//
//  Definitions due to different OS calls
//
//////////////////////////////////////////////////////////////////////////////

// Declare the gettid syscall.
//_syscall0(pid_t, gettid);
static inline pid_t gettid(void) { return syscall(__NR_gettid); }


// Function declaration for thread procedure
#define ThreadProcedureDeclaration(Name) void* Name(void * parm)
ThreadProcedureDeclaration(ThreadProc1);

namespace SyS {  // system-specific interface functions

    typedef cpu_set_t ProcMaskType;          // Type for processor mask

    // Get mask of possible CPU cores
    static inline ProcMaskType GetProcessMask() {
        ProcMaskType ProcessAffMask;
        CPU_ZERO(&ProcessAffMask);
        int e = sched_getaffinity(0, sizeof(ProcMaskType), &ProcessAffMask);
        if (e) printf("\nsched_getaffinity failed");
        return ProcessAffMask;
    }

    // Set CPU to run on specified CPU core (0 based number)
    static inline void SetProcessMask(int p) {
        ProcMaskType mask;
        CPU_ZERO(&mask);
        CPU_SET(p, &mask);
        int e = sched_setaffinity(gettid(), sizeof(ProcMaskType), &mask);
        if (e) {
            printf("\nFailed to lock thread to processor %i\n", p);
        }
    }

    // Test if specified CPU core is available
    static inline int TestProcessMask(int p, ProcMaskType * m) {
        return CPU_ISSET(p, m);
    }

    // Sleep for the rest of current timeslice
    static inline void Sleep0() {
        sched_yield();
    }

    // Set process (all threads) to high priority
    static inline void SetProcessPriorityHigh() {
        setpriority(PRIO_PROCESS, 0, PRIO_MIN);
    }

    // Set process (all threads) to normal priority
    static inline void SetProcessPriorityNormal() {
        setpriority(PRIO_PROCESS, 0, 0);
    } 

}


//////////////////////////////////////////////////////////////////////
//
//        Class ThreadHandler: Create threads
//
//////////////////////////////////////////////////////////////////////

#define ThreadStackSize  0x4000    // Stack size for each thread


class ThreadHandler {
public:
    ThreadHandler() {       // constructor
        NumThreads = 0;
        for (int i = 0; i < MAXTHREADS; i++) {
            //hThreads[i] = 0;
            ThreadData[i] = 0;
        }
    }

    void Start(int Num) { // start threads
        if (Num > MAXTHREADS) Num = MAXTHREADS;
        NumThreads = Num;
        int t;
        for (t = 0; t < NumThreads-1; t++) {
            ThreadData[t] = t;
            // create and start thread
            int e = pthread_create(&hThreads[t], NULL, &ThreadProc1, &ThreadData[t]);
            if (e) printf("\nFailed to create thread %i", t);
        }
        // last thread = this thread
        t = NumThreads-1;
        ThreadData[t] = t;
        ThreadProc1(&ThreadData[t]);
    }

    void Stop() {
        // wait for threads to finish
        if (NumThreads == 0) return;
        for (int t = 0; t < NumThreads - 1; t++) {
            int e = pthread_join(hThreads[t], NULL);
            if (e) printf("\nFailed to terminate thread %i", t);
        }
        NumThreads = 0;
    }

    ~ThreadHandler() {  // destructor
        Stop();
    }

protected:
    int NumThreads;
    pthread_t hThreads[MAXTHREADS];
    int ThreadData[MAXTHREADS];
};


//////////////////////////////////////////////////////////////////////
//
//                         class CMSRDriver
//
// Thie class encapsulates the interface to the driver MSRDriver32.sys
// or MSRDriver64.sys which is needed for privileged access to set up 
// the model specific registers in the CPU. 
// This class loads, unloads and sends commands to MSRDriver
//
//////////////////////////////////////////////////////////////////////

class CMSRDriver {
public:
    CMSRDriver() {             // constructor
        DriverFileName = "/dev/MSRdrv";
        DriverHandle = 0;
    }

    ~CMSRDriver() {            // destructor
        UnloadDriver();
    }

    int LoadDriver() {         // load MSRDriver
        DriverHandle = open(DriverFileName, 0);
        if (DriverHandle == -1) {
            printf("\nCannot open device %s\n", DriverFileName);
            DriverHandle = 0;
            return 1;
        }
        return 0;
    }

    int UnloadDriver() {       // unload MSRDriver
        if (DriverHandle) {
            close(DriverHandle);
            DriverHandle = 0;
        }
        return 0;
    }

    const char* GetDriverName() {  // get name of driver
        return DriverFileName;
    }

    // send commands to driver to read or write MSR registers
    int AccessRegisters(void * pnIn, int nInLen, void * pnOut, int nOutLen) {
        if (!DriverHandle) return -1;

        // pnIn and pnOut must be the same
        if (pnIn != pnOut) printf("\nError: driver must use same buffer for input and output");

        // nInLen and nOutLen are ignored. Rely on queue ending with MSR_STOP command
        return ioctl(DriverHandle, IOCTL_PROCESS_LIST, pnIn);
    }

    // send commands to driver to read or write MSR registers
    int AccessRegisters(CMSRInOutQue & q) {
        // Number of bytes in/out
        int n = q.GetSize() * sizeof(SMSRInOut);  
        if (n <= 0) return 0;
        return AccessRegisters(q.queue, n, q.queue, n);
    }

    // read performance monitor counter
    // send command to driver to read one MSR register
    int64 MSRRead(int r) {
        SMSRInOut a;
        a.msr_command = MSR_READ;
        a.register_number = r;
        a.value = 0;
        AccessRegisters(&a,sizeof(a),&a,sizeof(a));
        return a.val[0];
    } 

    // send command to driver to write one MSR register
    int MSRWrite(int r, int64 val) {
        SMSRInOut a;
        a.msr_command = MSR_WRITE;
        a.register_number = r;
        a.value = val;
        return AccessRegisters(&a,sizeof(a),&a,sizeof(a));
    }

    // send command to driver to read one control register, cr0 or cr4
    size_t CRRead(int r) {
        if (r != 0 && r != 4) return -11;
        SMSRInOut a;
        a.msr_command = CR_READ;
        a.register_number = r;
        a.value = 0;
        AccessRegisters(&a,sizeof(a),&a,sizeof(a));
        return size_t(a.value);
    }

    // send command to driver to write one control register, cr0 or cr4
    int CRWrite(int r, size_t val) {
        if (r != 0 && r != 4) return -12;
        SMSRInOut a;
        a.msr_command = CR_WRITE;
        a.register_number = r;
        a.value = val;
        return AccessRegisters(&a,sizeof(a),&a,sizeof(a));
    } 

protected:
    const char* DriverFileName;
    int DriverHandle;
};
