//                       uninstall.cpp                 © 2009-08-13 Agner Fog
//
//
// Uninstaller for driver MSRDriver32.sys and MSRDriver64.sys used in 
// PMCTest program
// 
//
// Compile for console mode Windows.
// You must have sufficient user privileges to uninstall a driver.
//
// © 2009 GNU General Public License www.gnu.org/licenses
//////////////////////////////////////////////////////////////////////////////

#include "msrdriver.h"
#include "PMCTest.h"
#include <conio.h>

// main
int main(int argc, char* argv[]) {
    CMSRDriver msr;
    msr.LoadDriver();
    msr.UnloadDriver();
    msr.UnInstallDriver();

    // Optional: wait for key press
    printf("\npress any key");
    getch();

    // Exit
    return 0;
}


//////////////////////////////////////////////////////////////////////
//
//        CMSRDriver class member functions
//            Copied from PMCTestA.cpp
//
//////////////////////////////////////////////////////////////////////

// Constructor
CMSRDriver::CMSRDriver() {

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

SC_HANDLE CMSRDriver::GetScm() {
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

// Destructor
CMSRDriver::~CMSRDriver() {
    // Unload driver if not already unloaded and close SCM handle

    //if (hDriver) UnloadDriver();
    if (service) {
        ::CloseServiceHandle(service); service = NULL;   
    }
    if (scm) {
        //UnloadDriver();
        //UnInstallDriver();
        ::CloseServiceHandle(scm); scm = NULL;
    }
}

int CMSRDriver::InstallDriver() {
    // install MSRDriver
    if(GetScm() == NULL) return -1;

    // Install driver in database
    service = ::CreateService(scm, DriverFileNameE, "MSR driver",
        SERVICE_START + SERVICE_STOP + DELETE, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, DriverFilePath, 
        NULL, NULL, NULL, NULL, NULL);

    if(service == NULL) {
        int e = ::GetLastError();
        printf("\nCannot install driver %s\nError no. %i", DriverFileNameE, e);
    }
    else {
        printf("\nFirst time: Installing driver %s\n", DriverFileNameE);
    }
}

int CMSRDriver::UnInstallDriver() {
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

// Load driver
int CMSRDriver::LoadDriver() {
    int r = 0, e = 0;
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

    // Start the service
    r = ::StartService(service, 0, NULL);
    if (r == 0) {
        e = ::GetLastError();
        switch (e) {
        case ERROR_FILE_NOT_FOUND:  // .sys file not found
            printf("\nDriver file %s not found\n", DriverFileNameE);
            break;

        case 577:
            // driver not signed (Vista and Windows 7)
            printf("\nThe driver %s is not signed by Microsoft\nPlease press F8 during boot and select 'Disable Driver Signature Enforcement'\n", DriverFileNameE);
            break;

        case 1056:
            printf("\nDriver already loaded\n");
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

// Unload driver
int CMSRDriver::UnloadDriver() {
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


// Access MSR or control registers
int CMSRDriver::AccessRegisters(void * pnIn, int nInLen, void * pnOut, int nOutLen) {
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
        return e;}
    return 0;
}


// Access MSR or control registers by queue
int CMSRDriver::AccessRegisters(CMSRInOutQue & q) {
    // Number of bytes in/out
    int n = q.GetSize() * sizeof(SMSRInOut);  
    if (n <= 0) return 0;
    return AccessRegisters(q.queue, n, q.queue, n);
}


// Get driver name
LPCTSTR CMSRDriver::GetDriverName() {
    return DriverFileName;
}

// Read a MSR register
int CMSRDriver::MSRRead(int r) {
    SMSRInOut a;
    a.msr_command = MSR_READ;
    a.register_number = r;
    a.value = 0;
    AccessRegisters(&a,sizeof(a),&a,sizeof(a));
    return a.val[0];}

// Write a MSR register
int CMSRDriver::MSRWrite(int r, int val) {
    SMSRInOut a;
    a.msr_command = MSR_WRITE;
    a.register_number = r;
    a.value = val;
    return AccessRegisters(&a,sizeof(a),&a,sizeof(a));}

// Read a control register cr0 or cr4
size_t CMSRDriver::CRRead(int r) {
    if (r != 0 && r != 4) return -11;
    SMSRInOut a;
    a.msr_command = CR_READ;
    a.register_number = r;
    a.value = 0;
    AccessRegisters(&a,sizeof(a),&a,sizeof(a));
    return size_t(a.value);}

// Write a control register cr0 or cr4
int CMSRDriver::CRWrite(int r, size_t val) {
    if (r != 0 && r != 4) return -12;
    SMSRInOut a;
    a.msr_command = CR_WRITE;
    a.register_number = r;
    a.value = val;
    return AccessRegisters(&a,sizeof(a),&a,sizeof(a));}


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
int CMSRDriver::Need64BitDriver() {
    // Tell whether we need 32 bit or 64 bit driver.
    // Return value:
    // 0: running in 32 bits Windows
    // 1: running 32 bits application in 64 bits Windows
    // 2: running in 64 bits Windows

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
