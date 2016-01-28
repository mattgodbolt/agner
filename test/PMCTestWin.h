//                       PMCTestWin.h                    2012-03-02 Agner Fog
//
//          Multithread PMC Test program
//          System-specific definitions for Windows
//
// See PMCTest.txt for instructions.
//
// (c) Copyright 2000-2012 by Agner Fog. GNU General Public License www.gnu.org/licences
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <windef.h>
#include <winnt.h>
#include <winsvc.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

// comment out next line if compiler doesn't support intrinsic functions:
//#include <intrin.h>

#ifdef _WIN64
//#include <intrin.h>  // intrinsics needed in 64 bit Windows because inline asm not supported by MS compiler
#include "intrin1.h"   // short version of intrin.h
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  Definitions due to different compiler syntax
//
//////////////////////////////////////////////////////////////////////////////

// define 64 bit integer
typedef __int64 int64;
typedef unsigned __int64 uint64;

// Define macro for aligned structure, MS Visual Studio syntax
#define ALIGNEDSTRUCTURE(Name, Align) __declspec(align(Align)) struct Name


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
// This version is for 32-bit, MASM syntax

static void Cpuid (int output[4], int functionnumber) {	
    __asm {
        mov eax, functionnumber;
        cpuid;
        mov esi, output;
        mov [esi],    eax;
        mov [esi+4],  ebx;
        mov [esi+8],  ecx;
        mov [esi+12], edx;
    }
}

static inline void Serialize () {
    // serialize CPU
    __asm {
        xor eax, eax
            cpuid
    }
    // Note: ebx is changed by cpuid.
    // The compiler will save ebx automatically in most cases, but bugs have been observed.
}

#pragma warning(disable:4035)
static inline int Readtsc() {
    // read performance monitor counter number nPerfCtr
    __asm {
        rdtsc
    }
}

static inline int Readpmc(int nPerfCtr) {
    // read performance monitor counter number nPerfCtr
    __asm {
        mov ecx, nPerfCtr
            rdpmc
    }
}

#pragma warning(default:4035)

#endif  // __INTRIN_H_ 

//////////////////////////////////////////////////////////////////////////////
//
//  Definitions due to different OS calls
//
//////////////////////////////////////////////////////////////////////////////

// Function declaration for thread procedure
#define ThreadProcedureDeclaration(Name) extern "C" DWORD WINAPI Name(LPVOID parm)

ThreadProcedureDeclaration(ThreadProc1);

namespace SyS {  // system-specific process and thread functions

    typedef DWORD_PTR ProcMaskType;          // Type for processor mask
    //typedef unsigned int ProcMaskType;     // If DWORD_PTR not defined

    // Get mask of possible CPU cores
    static inline ProcMaskType GetProcessMask() {
        ProcMaskType ProcessAffMask = 0, SystemAffMask = 0;
        GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffMask, &SystemAffMask);
        return ProcessAffMask;
    }

    // Set CPU to run on specified CPU core number (0-based)
    static inline void SetProcessMask(int p) {
        int r = (int)SetThreadAffinityMask(GetCurrentThread(), (ProcMaskType)1 << p);   
        if (r == 0) {
            int e = GetLastError();
            printf("\nFailed to lock thread to processor %i. Error = %i\n", p, e);
        }
    }

    // Test if specified CPU core is available
    static inline int TestProcessMask(int p, ProcMaskType * m) {
        return ((ProcMaskType)1 << p) & *m;
    }

    // MainThreadProcNum = GetCurrentProcessorNumber(); // only available in Vista and above

    // Sleep for the rest of current timeslice
    static inline void Sleep0() {
        Sleep(0);
    }

    // Set process (all threads) to high priority
    static inline void SetProcessPriorityHigh() {
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    }

    // Set process (all threads) to normal priority
    static inline void SetProcessPriorityNormal() {
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
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
            hThreads[i] = 0;
            ThreadData[i] = 0;
        }
    }

    void Start(int Num) { // start threads
        int t;
        if (Num > MAXTHREADS) Num = MAXTHREADS;
        NumThreads = Num;
        for (t = 0; t < NumThreads-1; t++) {
            ThreadData[t] = t;
            // create and start thread
            hThreads[t] = CreateThread( 
                NULL,                   // default security attributes
                ThreadStackSize,        // stack size  
                ThreadProc1,            // thread function name
                &ThreadData[t],         // argument to thread function 
                0,                      // use default creation flags 
                NULL); 
            if (hThreads[t] == 0) {
                printf("\nFailed to create thread %i", t);
            }
        }
        // last thread = this thread
        t = NumThreads-1;
        ThreadData[t] = t;
        ThreadProc1(&ThreadData[t]);
    }

    void Stop() {
        // wait for threads to finish
        if (NumThreads == 0) return;
        WaitForMultipleObjects(NumThreads, hThreads, TRUE, INFINITE);
        for (int t = 0; t < NumThreads; t++) {
            if (hThreads[t]) CloseHandle(hThreads[t]);
        }
        NumThreads = 0;
    }

    ~ThreadHandler() {  // destructor
        Stop();
    }

protected:
    int NumThreads;
    HANDLE hThreads[MAXTHREADS];
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
protected:
    SC_HANDLE scm;
    SC_HANDLE service;
    HANDLE hDriver;
    const char * DriverFileName;
    const char * DriverSymbolicName;
    char DriverFileNameE[MAX_PATH], DriverFilePath[MAX_PATH];

public:
    CMSRDriver() {  // constructor   
        // Define Driver filename
        if (Need64BitDriver()) {
            DriverFileName = "MSRDriver64";
        }
        else {
            DriverFileName = "MSRDriver32";
        }   
        // Define driver symbolic link name
        DriverSymbolicName = "\\\\.\\slMSRDriver";

        // Get the full path of the driver file name
        strcpy(DriverFileNameE, DriverFileName);
        strcat(DriverFileNameE, ".sys");           // append .sys to DriverName
        ::GetFullPathName(DriverFileNameE, MAX_PATH, DriverFilePath, NULL);

        // Initialize
        service = NULL;
        hDriver = NULL;
        scm     = NULL;
    }

    ~CMSRDriver() {  // destructor
        // Unload driver if not already unloaded and close SCM handle
        //if (hDriver) UnloadDriver();
        if (service) {
            ::CloseServiceHandle(service); service = NULL;   
        }
        if (scm) {
            // Optionally unload driver
            // UnloadDriver();
            // Don't uninstall driver, you may need reboot before you can install it again
            // UnInstallDriver();
            ::CloseServiceHandle(scm); scm = NULL;
        }
    }

    const char * GetDriverName() {            // get name of driver
        return DriverFileName;
    }

    int LoadDriver() {                        // load MSRDriver
        int r = 0, e = 0;
        // open driver service
        r = OpenDriver();
        if (r == 1060) {
            // Driver not installed. Install it
            e = InstallDriver();
            if (e) return e;
            r = OpenDriver();
        }
        if (r) {
            printf("\nError %i loading driver\n");
            return r;
        }

        // Start the service
        r = ::StartService(service, 0, NULL);
        if (r == 0) {
            e = ::GetLastError();
            switch (e) {
            case ERROR_PATH_NOT_FOUND:
                printf("\nDriver file %s path not found\n", DriverFileNameE);
                break;

            case ERROR_FILE_NOT_FOUND:  // .sys file not found
                printf("\nDriver file %s not found\n", DriverFileNameE);
                break;

            case 577:
                // driver not signed (Vista and Windows 7)
                printf("\nThe driver %s is not signed by Microsoft\nPlease press F8 during boot and select 'Disable Driver Signature Enforcement'\n", DriverFileNameE);
                break;

            case 1056:
                // Driver already loaded. Ignore
                //printf("\nDriver already loaded\n");
                e = 0;
                break;

            case 1058:
                printf("\nError: Driver disabled\n");
                break;

            default:
                printf("\nCannot load driver %s\nError no. %i", DriverFileNameE, e);
            }
        }
        if (e == 0) {
            // Get handle to driver
            hDriver = ::CreateFile(DriverSymbolicName, GENERIC_READ + GENERIC_WRITE,
                0, NULL, OPEN_EXISTING, 0, NULL);

            if(hDriver == NULL || hDriver == INVALID_HANDLE_VALUE) {
                hDriver = NULL;
                e = ::GetLastError();
                printf("\nCannot load driver\nError no. %i", e);
            }
        }
        return e;
    }

    int UnloadDriver() {                      // unload MSRDriver
        int r = 0, e = 0;
        if(GetScm() == NULL) {
            return -6;
        }

        if (hDriver) {
            r = ::CloseHandle(hDriver); hDriver = NULL;
            if (r == 0) {
                e = ::GetLastError();
                printf("\nCannot close driver handle\nError no. %i", e);
                return e;
            }
            printf("\nUnloading driver");
        }

        if (service) {
            SERVICE_STATUS ss;
            r = ::ControlService(service, SERVICE_CONTROL_STOP, &ss);
            if (r == 0) {
                e = ::GetLastError();
                if (e == 1062) {
                    printf("\nDriver not active\n");
                }
                else {
                    printf("\nCannot close driver\nError no. %i", e);
                }
                return e;
            }
        }
        return 0;
    }

protected:
    int InstallDriver() {                     // install MSRDriver
        // install MSRDriver
        int e = 0;
        if(GetScm() == NULL) return -1;

        // Install driver in database
        service = ::CreateService(scm, DriverFileNameE, "MSR driver",
            SERVICE_START + SERVICE_STOP + DELETE, SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, DriverFilePath, 
            NULL, NULL, NULL, NULL, NULL);

        if(service == NULL) {
            e = ::GetLastError();
            printf("\nCannot install driver %s\nError no. %i", DriverFileNameE, e);
        }
        else {
            printf("\nFirst time: Installing driver %s\n", DriverFileNameE);
        }
        return e;
    } 

    int UnInstallDriver() {                   // uninstall MSRDriver
        // uninstall MSRDriver
        int r = 0, e = 0;
        GetScm();
        if (service == 0) {
            service = ::OpenService(scm, DriverFileNameE, SERVICE_ALL_ACCESS);
        }
        if(service == 0) {
            e = ::GetLastError();
            if (e == 1060) {
                printf("\nDriver %s already uninstalled or never installed\n", DriverFileNameE);
            }
            else {
                printf("\nCannot open service, failed to uninstall driver %s\nError no. %i", DriverFileNameE, e);
            }
        }
        else {
            r = ::DeleteService(service);
            if (r == 0) {
                e = ::GetLastError();
                printf("\nFailed to uninstall driver %s\nError no. %i", DriverFileNameE, e);
                if (e == 1072) printf("\nDriver already marked for deletion\n");
            }
            else {
                printf("\nUninstalling driver %s\n", DriverFileNameE);
            }
            r = ::CloseServiceHandle(service);
            if (r == 0) {
                e = ::GetLastError();
                printf("\nCannot close service\nError no. %i", e);
            }
            service = NULL;
        }
        return e;
    }

    SC_HANDLE GetScm() {                      // Make scm handle
        // Make scm handle
        if (scm) return scm;  // handle already made

        // Open connection to Windows Service Control Manager (SCM)
        scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(scm == NULL) {
            int e = ::GetLastError();
            if (e == ERROR_ACCESS_DENIED) {
                printf("\nAccess denied. Please run as administrator\n");
            }
            else if (e == 120) {  // function not implemented
                printf("\nFunction not implemented on this operating system. Windows 2000 or later required.\n");
            }
            else {
                printf("\nCannot load Windows Service Control Manager\nError no. %i", e);
            }
        }
        return scm;
    }

    int OpenDriver() {                        // open driver service
        // open driver service
        int e;
        // Open a service handle if not already open
        if (service == 0) {
            service = ::OpenService(GetScm(), DriverFileNameE, SERVICE_ALL_ACCESS);
        }
        if(service == 0) {
            e = ::GetLastError();

            switch (e) { // Any other error than driver not installed
            case 1060: // Driver not installed. Install it
                break;
            case 6:    // access denied
                printf("\nAccess denied\n");
                break;
            default:  // Any other error
                printf("\nCannot open service, failed to load driver %s\nError no. %i", DriverFileNameE, e);
            }
            return e;
        }
        return 0;
    }

    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    int Need64BitDriver() {                   // tell whether we need 32 bit or 64 bit driver
        // Tell whether we need 32 bit or 64 bit driver.
        // Return value:
        // 0: running in 32 bits Windows
        // 1: running 32 bits mode in 64 bits Windows
        // 2: running 64 bits mode in 64 bits Windows
#ifdef _WIN64
        return 2;
#else
        LPFN_ISWOW64PROCESS fnIsWow64Process = 
            (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle("kernel32"),"IsWow64Process");   
        if (fnIsWow64Process) {      
            BOOL bIsWow64 = FALSE;      
            if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)) {
                return 0;
            }
            return bIsWow64;
        }
        return 0;
#endif
    }

public:
    // send commands to driver to read or write MSR registers
    int AccessRegisters(void * pnIn, int nInLen, void * pnOut, int nOutLen) {
        if (nInLen <= 0) return 0;

        const int DeviceType = 0x22;        // FILE_DEVICE_UNKNOWN;
        const int Function = 0x800;
        const int Method = 0;               // METHOD_BUFFERED;
        const int Access = 1 | 2;           // FILE_READ_ACCESS | FILE_WRITE_ACCESS;
        const int IOCTL_MSR_DRIVER = DeviceType << 16 | Access << 14 | Function << 2 | Method;

        DWORD len = 0;

        // This call results in a call to the driver rutine DispatchControl()
        int res = ::DeviceIoControl(hDriver, IOCTL_MSR_DRIVER, pnIn, nInLen,
            pnOut, nOutLen, &len, NULL);
        if (!res) { 
            // Error
            int e = GetLastError();
            printf("\nCan't access driver. error %i", e);
            return e;
        }

        // Check return error codes from driver
        SMSRInOut * outp = (SMSRInOut*)pnOut;
        for (int i = 0; i < nOutLen/(INT)sizeof(SMSRInOut); i++) {
            if (outp[i].msr_command == PROC_SET && outp[i].val[0]) {
                printf("\nSetting processor number in driver failed, error 0x%X", outp[i].val[0]);
            }
        }
        return 0;
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
};
