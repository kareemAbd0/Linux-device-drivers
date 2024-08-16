//
// Created by kareem on 3/13/24.
//
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
#include "lcd.h"
#include "lcd_platform_driver_data.h"

struct lcd_drv_data lcd_drv_data;





ssize_t lcdcmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    /*get the driver data*/
    struct lcd_drv_data *drv_data = dev_get_drvdata(dev);

    /*convert and store the command*/
    if  (kstrtol(buf,0,&drv_data->lcd_dev_data->cmd)){
        pr_err("wrong command\n");
        return -EINVAL;
    }

    /*send the command*/
    if (LCD_send_command(drv_data->lcd_dev_data->cmd,drv_data) < 0){
        pr_err("LCD command failed\n");
        return -ENODEV;
    }

    return count;
}


ssize_t lcdscroll_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){


    struct lcd_drv_data *drv_data = dev_get_drvdata(dev);

    if (sysfs_streq(buf,"on")){
    if (LCD_send_command(0x18,drv_data) < 0){
            pr_err("LCD scroll failed\n");
            return -ENODEV;
        }
    } else if (sysfs_streq(buf,"off")){
        if (LCD_send_command(0x1C,drv_data) < 0){
            pr_err("LCD scroll failed\n");
            return -ENODEV;
        }
    } else{
        return -EINVAL;
    }




    return count; // Acknowledge the write operation

}


ssize_t lcdpxy_show(struct device *dev, struct device_attribute *attr, char *buf){

    struct lcd_dev_data *dev_data = dev_get_drvdata(dev);
    return sprintf(buf, "x: %d\n  y: %d  ", dev_data->x, dev_data->y);
}


ssize_t lcdpxy_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){


    /*temp testing */

    int ret = 0;
    struct lcd_drv_data *drv_data = dev_get_drvdata(dev);

    /*get the driver data*/
    pr_info("debugging here LCD xy 1");



    pr_info("debugging here LCD xy 2");

    /*temp gpio test */

    if(sysfs_streq(buf, "0")) {
        tester(drv_data);
    } else{
       ret = -EINVAL;
    }

    pr_info("debugging here LCD xy 3");
    return ret? ret : count;
}


ssize_t lcdtxt_show(struct device *dev, struct device_attribute *attr, char *buf){

struct lcd_dev_data *dev_data = dev_get_drvdata(dev);
return sprintf(buf, " current text: %s\n", dev_data->text);

}


ssize_t lcdtxt_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    struct lcd_drv_data *drv_data = dev_get_drvdata(dev);

    unsigned char u8_data = *buf;
    /*set the text*/
    strcpy(drv_data->lcd_dev_data->text,buf);
    /*display the text*/

    if  ((LCD_display_string(buf,drv_data) < 0)){
        pr_err("LCD text failed\n");
        return -ENODEV;
    }

    return 0;
}






/* see definition of DEVICE_ATTR_RW and DEVICE_ATTR_RO in include/linux/device.h */
static DEVICE_ATTR_WO(lcdcmd);
static DEVICE_ATTR_WO(lcdscroll);
static DEVICE_ATTR_RW(lcdpxy);
static DEVICE_ATTR_RW(lcdtxt);


/*attribute structure*/

static struct attribute *lcd_attrs[] = {
        &dev_attr_lcdcmd.attr,
        &dev_attr_lcdscroll.attr,
        &dev_attr_lcdpxy.attr,
        &dev_attr_lcdtxt.attr,
        NULL,
};


/*attribute group structure*/

static struct attribute_group lcd_attr_group = {
        .attrs = lcd_attrs,
};


/*attribute groups structure*/

static const struct attribute_group *lcd_attr_groups[] = {
        &lcd_attr_group,
        NULL,
};





/*gets called when names are matched*/
int lcd_probe(struct platform_device *pdev){
    const char *name;
    int i = 0;
    int ret;
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;
    struct device *dev = &pdev->dev;
    struct lcd_gpio_dev_data *gpio_dev_data;

    // Get total number of child nodes
    lcd_drv_data.total_devices = of_get_child_count(parent);

    // Check if there are child nodes
    if(lcd_drv_data.total_devices < 0){
        dev_err(dev, "No child nodes found\n");
        return -ENODEV;
    }

    // Log total number of child nodes
    dev_info(dev,"Total child nodes found: %d\n", lcd_drv_data.total_devices);

    // Allocate memory for the gpio devices
    lcd_drv_data.gpio_devices = devm_kzalloc(dev, sizeof(struct lcd_gpio_dev_data *) * lcd_drv_data.total_devices, GFP_KERNEL);

    // Check if memory allocation was successful
    if(!lcd_drv_data.gpio_devices){
        dev_err(dev, "Memory allocation failed\n");
        return -ENOMEM;
    }
    // Allocate memory for driver data
    for_each_child_of_node(parent,child){
        gpio_dev_data = devm_kzalloc(dev, sizeof(struct lcd_gpio_dev_data), GFP_KERNEL);

        // Check if memory allocation was successful
        if (!gpio_dev_data){
            dev_info(dev,"Memory allocation failed ");
        }


        // Get the label from the device tree
        if(of_property_read_string(child, "label", &name))
        {
            pr_err("label not found\n");
            snprintf(gpio_dev_data->label, sizeof(gpio_dev_data->label), "unknown-pin%d", i);
        } else{
            strcpy(gpio_dev_data->label, name);
            pr_err("pin label: %s\n", gpio_dev_data->label);
        }

        // Get the gpio descriptor from the device tree
        gpio_dev_data->desc = devm_fwnode_get_gpiod_from_child(dev, "rpi", &child->fwnode, GPIOD_ASIS, gpio_dev_data->label);

        // Check if getting gpio descriptor was successful
        if(IS_ERR(gpio_dev_data->desc)){
            ret = PTR_ERR(gpio_dev_data->desc);
            if(ret == -ENOENT){
                dev_err(dev, "Failed to get gpio descriptor\n");
            }

            return ret;
        }

        // Set direction of the gpio
        ret = gpiod_direction_output(gpio_dev_data->desc, 0);

        // Check if setting direction was successful
        if(ret){
            pr_err("failed to set direction\n");
            return ret;
        }

        // Populate driver data with gpio devices
        lcd_drv_data.gpio_devices[i] = gpio_dev_data;



        i++;
    }

    /* Create lcd device under /sys/class/bone-gpio and bind everything together */
    lcd_drv_data.lcd_device = device_create_with_groups(lcd_drv_data.class_gpio, dev, 0, &lcd_drv_data, lcd_attr_groups,"lcd");

    /* Check if device creation was successful */
    if (IS_ERR(lcd_drv_data.lcd_device)){
        dev_err(dev, "device create failed\n");
        return PTR_ERR(lcd_drv_data.lcd_device);
    }


    /*initialize the lcd*/

    if (LCD_init(&lcd_drv_data) < 0){
        pr_err("LCD initialization failed\n");
        return -ENODEV;
    }


    return  0;
}

/*gets called when devices are removed*/
int lcd_remove(struct platform_device *pdev){

        device_unregister(lcd_drv_data.lcd_device);
        return  0;
}


/*matches the compatible string with the device tree*/
struct of_device_id lcd_of_match[] = {
        {
                .compatible = "org,lcd",
        },
        {},
};


/*platform driver structure*/
struct platform_driver lcd_platform_driver = {
        .probe = lcd_probe,
        .remove = lcd_remove,
        .driver = {
                .name = "lcd_16x2",
                .of_match_table = lcd_of_match,
        },
};


int __init lcd_platform_driver_init(void){

    lcd_drv_data.class_gpio = class_create(THIS_MODULE, "lcd_16x2");
    if(IS_ERR(lcd_drv_data.class_gpio)){
        pr_err("class creation failed\n");
        return PTR_ERR(lcd_drv_data.class_gpio);
    }
    pr_info("module loaded\n");
    return platform_driver_register(&lcd_platform_driver);

}



void __exit lcd_platform_driver_exit(void) {

    platform_driver_unregister(&lcd_platform_driver);
    class_destroy(lcd_drv_data.class_gpio);
    pr_info("module unloaded\n");

}


module_init(lcd_platform_driver_init);
module_exit(lcd_platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Abdo");
MODULE_DESCRIPTION("A simple platform driver for LCD");
