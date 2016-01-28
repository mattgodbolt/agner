//                       msrdriver.h                    © 2012-03-02 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Linux (32 and 64 bit x86 platform) 

// (c) Copyright 2010-2012 by Agner Fog. GNU General Public License www.gnu.org/licences

#ifndef MSRDRV_H
#define MSRDRV_H

#include <linux/types.h>
#include "MSRDriver.h"

#define DEV_MAJOR 222
#define DEV_MINOR 0
#define DEV_NAME "MSRdrv"

#define IOCTL_NOACTION _IO(DEV_MAJOR, 0)
#define IOCTL_PROCESS_LIST _IO(DEV_MAJOR, 1)

#endif
