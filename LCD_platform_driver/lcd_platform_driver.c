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

#include "lcd_platform_driver.h"




struct lcd_drv_data lcd_drv_data;



ssize_t lcdcmd_show(struct device *dev, struct device_attribute *attr, char *buf){

    return 0;
}
ssize_t lcdcmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    return 0;
}

ssize_t lcdscroll_show(struct device *dev, struct device_attribute *attr, char *buf){

    return 0;
}


ssize_t lcdscroll_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    return 0;
}


ssize_t lcdpxy_show(struct device *dev, struct device_attribute *attr, char *buf){

    return 0;
}


ssize_t lcdpxy_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    return 0;
}


ssize_t lcdtxt_show(struct device *dev, struct device_attribute *attr, char *buf){

    return 0;
}


ssize_t lcdtxt_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){

    return 0;
}






/* see definition of DEVICE_ATTR_RW and DEVICE_ATTR_RO in include/linux/device.h */
static DEVICE_ATTR_RW(lcdcmd);
static DEVICE_ATTR_RW(lcdscroll);
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
    /*get the parent device tree node*/
    struct device_node *parent = pdev->dev.of_node;
    /*get the child device tree node*/
    struct device_node *child = NULL;

    struct device *dev = &pdev->dev;

    struct lcd_dev_data *dev_data;

    lcd_drv_data.total_devices = of_get_child_count(parent);

    if(lcd_drv_data.total_devices < 0){
        dev_err(dev, "No child nodes found\n");
        return -ENODEV;
    }

    dev_info(dev,"Total child nodes found: %d\n", lcd_drv_data.total_devices);

    /*allocate memory for the devices*/
    lcd_drv_data.devices = devm_kzalloc(dev, sizeof(struct device *) * lcd_drv_data.total_devices, GFP_KERNEL);
    if(!lcd_drv_data.devices){
        dev_err(dev, "Memory allocation failed\n");
        return -ENOMEM;
    }


    /*allocate memory for driver data, this function checks for enabled children of a parent*/
    for_each_child_of_node(parent,child){

        /*allocate memory for device data*/
        dev_data = devm_kzalloc(dev, sizeof(struct lcd_dev_data), GFP_KERNEL);
        if (!dev_data){
            dev_info(dev,"Memory allocation failed ");
        }

        /*get the label from the device tree*/
        if(of_property_read_string(child, "label", &name))
        {
            pr_err("label not found\n");
            snprintf(dev_data->label, sizeof(dev_data->label), "unknown-pin%d", i);
        } else{
            strcpy(dev_data->label,name);
            pr_err("pin label: %s\n", dev_data->label);
        }

        /*get the gpio descriptor from the device tree*/
        dev_data->desc = devm_fwnode_get_gpiod_from_child(dev, "lcd", &child->fwnode, GPIOD_ASIS, dev_data->label);





    }



    return  0;


}


/*gets called when devices are removed*/
int lcd_remove(struct platform_device *pdev){



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
struct platform_driver gpio_platform_driver = {
        .probe = lcd_probe,
        .remove = lcd_remove,
        .driver = {
                .name = "lcd_16x2",
                .of_match_table = lcd_of_match,
        },
};


int __init lcd_platform_driver_init(void){


return 0;
}



void __exit lcd_platform_driver_exit(void) {



}


module_init(lcd_platform_driver_init);
module_exit(lcd_platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Abdo");
MODULE_DESCRIPTION("A simple platform driver for LCD");
