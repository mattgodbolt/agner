//                       MSRdrv.c                     © 2012-03-02 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Linux (32 and 64 bit x86 platform) 
// This version has an older version of IOCTL

// © 2010-2012 GNU General Public License www.gnu.org/licences

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "MSRdrvL.h"

MODULE_LICENSE("GPL");

static int MSRdrv_open(struct inode *MSRdrv_inode, struct file *MSRdrv_file );
static int MSRdrv_release(struct inode *MSRdrv_inode, struct file *MSRdrv_file );
static ssize_t MSRdrv_read(struct file *p_file, char *u_buffer, size_t count, loff_t *ppos );
static ssize_t MSRdrv_write(struct file *p_file, const char *u_buffer, size_t count, loff_t *ppos);
static int MSRdrv_ioctl(struct inode *inode, struct file *file, unsigned ioctl_num, unsigned long ioctl_param);
static long MSRdrv_ioctl32(struct file *file, unsigned ioctl_num, unsigned long ioctl_param);

dev_t MSRdrv_dev;
struct cdev *MSRdrv_cdev;
struct file_operations MSRdrv_fops = {
owner : THIS_MODULE,
read : MSRdrv_read,
ioctl : MSRdrv_ioctl,
compat_ioctl : MSRdrv_ioctl32,
               .write = MSRdrv_write,
open : MSRdrv_open,
release : MSRdrv_release,
};

static int MSRdrv_open(struct inode *p_inode, struct file *p_file ) {
    return 0;
}

static int MSRdrv_release(struct inode *p_inode, struct file *p_file ) {
    return 0;
}

static long MSRdrv_ioctl32(struct file *file, unsigned ioctl_num, unsigned long ioctl_param) {
    unsigned long param = (unsigned long)((void*)(ioctl_param));

    MSRdrv_ioctl(file->f_dentry->d_inode, file, ioctl_num, param);
    return 0;
}

static long int ReadCR(int num) {
    // read control register
    long int val = 0;
    switch (num) {
    case 0:
        __asm__ __volatile__("mov %%cr0, %0" : "=r"(val));
        break;
    case 2:
        __asm__ __volatile__("mov %%cr2, %0" : "=r"(val));
        break;
    case 3:
        __asm__ __volatile__("mov %%cr3, %0" : "=r"(val));
        break;
    case 4:
        __asm__ __volatile__("mov %%cr4, %0" : "=r"(val));
        break;
    }
    return val;
}

static void WriteCR(int num, long int val) {
    // write control register
    switch (num) {
    case 0:
        __asm__ __volatile__("mov %0, %%cr0" : : "r"(val));
        break;
    case 2:
        __asm__ __volatile__("mov %0, %%cr2" : : "r"(val));
        break;
    case 3:
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(val));
        break;
    case 4:
        __asm__ __volatile__("mov %0, %%cr4" : : "r"(val));
        break;
    }
}

static long long ReadMSR(int num) {
    // read model specific register
    int hi = 0, lo = 0;
    __asm__ __volatile__("rdmsr" : "=a"(lo), "=d"(hi) : "c"(num));
    return lo | (long long)hi << 32;
}

static void WriteMSR(int num, int low, int high) {
    // write model specific register
    __asm__ __volatile__("wrmsr" : : "c"(num), "a"(low), "d"(high));
}


static int MSRdrv_ioctl(struct inode *inode, struct file *file, unsigned ioctl_num, unsigned long ioctl_param) {
    struct SMSRInOut *commandp = (struct SMSRInOut*)ioctl_param;
    int i;
    long int cr4val;

    if (ioctl_num == IOCTL_PROCESS_LIST) {
        for (i = 0; i <= MAX_QUE_ENTRIES; i++, commandp++) {
            switch (commandp->msr_command) {
            case MSR_IGNORE:
                break;

            case MSR_STOP: default:       // end of command list
                i = MAX_QUE_ENTRIES + 1;
                break;

            case MSR_READ:                // read model specific register
                commandp->value = ReadMSR(commandp->register_number);
                break;

            case MSR_WRITE:               // write model specific register
                WriteMSR(commandp->register_number, commandp->val[0], commandp->val[1]);
                break;

            case CR_READ:                 // read control register
                commandp->value = (long long)ReadCR(commandp->register_number);
                break;

            case CR_WRITE:                // write control register
                WriteCR(commandp->register_number, (long int)commandp->value);
                break;

            case PMC_ENABLE:              // Enable RDPMC and RDTSC instructions
                cr4val = ReadCR(4);        // Read CR4
                cr4val |= 0x100;           // Enable RDPMC
                cr4val &= ~4;              // Enable RDTSC
                WriteCR(4, cr4val);        // Write CR4
                break;

            case PMC_DISABLE:             // Disable RDPMC instruction (RDTSC remains enabled)
                cr4val = ReadCR(4);        // Read CR4
                cr4val &= ~0x100;          // Disable RDPMC
                //cr4val |= 4;             // Disable RDTSC
                WriteCR(4, cr4val);        // Write CR4
                break;

            case PROC_GET:                // get processor number. 
                // not implemented in Linux version
                break;

            case PROC_SET:                // set processor number. 
                // not implemented in Linux version
                break;
            }
        }
        return 0;
    }
    else {  // unknown command
        return 1;
    }
}

static ssize_t MSRdrv_read( struct file *p_file, char *u_buffer, size_t count, loff_t *ppos ) {
    return 0;
}

static ssize_t MSRdrv_write(struct file *p_file, const char *u_buffer, size_t count, loff_t *ppos) {
    return 0;
}

static int MSRdrv_init(void) {
    MSRdrv_dev = MKDEV( DEV_MAJOR, DEV_MINOR );
    register_chrdev_region( MSRdrv_dev, 1, DEV_NAME );

    MSRdrv_cdev = cdev_alloc();
    MSRdrv_cdev->owner = THIS_MODULE;
    MSRdrv_cdev->ops = &MSRdrv_fops;
    cdev_init( MSRdrv_cdev, &MSRdrv_fops );
    cdev_add( MSRdrv_cdev, MSRdrv_dev, 1 );

    return 0;
}

static void MSRdrv_exit(void) {
    cdev_del( MSRdrv_cdev );
    unregister_chrdev_region( MSRdrv_dev, 1 );
}

module_init(MSRdrv_init);
module_exit(MSRdrv_exit);
