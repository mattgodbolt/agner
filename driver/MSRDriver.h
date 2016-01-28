//                       msrdriver.h                     2012-03-02 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Windows 2000 and later and Linux (32 and 64 bit x86 platform) 

// (c) Copyright 2005-2012 by Agner Fog. GNU General Public License www.gnu.org/licences

#pragma once

// list of input/output data structures for MSR driver
#define MAX_QUE_ENTRIES 32                  // maximum number of entries in queue

// commands for MSR driver. Shared with application program
enum EMSR_COMMAND {
    MSR_IGNORE = 0,                // do nothing
    MSR_STOP   = 1,                // skip rest of list
    MSR_READ   = 2,                // read model specific register
    MSR_WRITE  = 3,                // write model specific register
    CR_READ    = 4,                // read control register
    CR_WRITE   = 5,                // write control register
    PMC_ENABLE = 6,                // Enable RDPMC and RDTSC instructions
    PMC_DISABLE= 7,                // Disable RDPMC instruction (RDTSC remains enabled)
    PROC_GET   = 8,                // Get processor number (In multiprocessor systems. 0-based)
    PROC_SET   = 9,                // Set processor number (In multiprocessor systems. 0-based)
    UNUSED1    = 0x7fffffff        // make sure this enum takes 32 bits
};


// input/output data structure for MSR driver
struct SMSRInOut {
    enum EMSR_COMMAND msr_command;      // command for read or write register
    unsigned int register_number;  // register number
    union {
        long long value;            // 64 bit value to read or write
        unsigned int val[2];        // lower and upper 32 bits
    };
};
