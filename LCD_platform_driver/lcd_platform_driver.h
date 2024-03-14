//
// Created by kareem on 3/13/24.
//

#ifndef LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
#define LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>



/*holds device specific data*/
struct lcd_dev_data{
    char label[20];
    struct gpio_desc *desc;
};

/*holds driver specific data*/
struct gpio_drv_data {

    int total_devices;
    struct class *class_gpio;
    struct device **devices;

};






#endif //LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
