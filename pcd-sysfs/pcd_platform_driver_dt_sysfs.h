//
// Created by kareem on 3/6/24.
//

#ifndef PLATFORM_MPCHAR_PCD_PLATFORM_DRIVER_DT_SYSFS_H
#define PLATFORM_MPCHAR_PCD_PLATFORM_DRIVER_DT_SYSFS_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include "platform.h"
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>

#define WR_ONLY 0x10
#define RD_ONLY 0x01
#define RD_WR 0x11
#define MAX_DEVICES 10




int check_permission(int dev_perm , int access_mode );
int pcdev_open(struct inode *inode, struct file *filp);
int pcdev_release(struct inode *inode, struct file *filp);
ssize_t pcdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);






/* Create dummy device configure */
enum pcdev_name {
    PCDEV_A_CONF,
    PCDEV_B_CONF,
    PCDEV_C_CONF,
    PCDEV_D_CONF,
};

struct device_configure {
    int configure_num1;
    int configure_num2;
};



/* device private data structure */
struct pcdev_private_data {

    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t device_number;
    /* holds cdev instance */
    struct cdev cdev;
};

/* driver private data structure */
struct pcdrv_private_data {
    int total_devices;
    /* holds device number, it is specific to driver not device */
    dev_t device_number_base;
    /* holds device */
    struct device *mpchar_device;
    /* holds device class */
    struct class *mpchar_class;
};





#endif //PLATFORM_MPCHAR_PCD_PLATFORM_DRIVER_DT_SYSFS_H
