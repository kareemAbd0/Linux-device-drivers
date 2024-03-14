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
struct gpio_dev_data{
    char label[20];

    struct gpio_desc *desc;
};

/*holds driver specific data*/
struct gpio_drv_data {

    int total_devices;
    struct class *class_gpio;
    struct device **devices;

};


struct gpio_drv_data gpio_drv_data;



/*show store functions for the attributes*/
ssize_t direction_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gpio_dev_data *dev_data = dev_get_drvdata(dev);

    int dir ;
    char *direction;

    dir = gpiod_get_direction(dev_data->desc);
    if (dir < 0){
        return dir;
    }

    direction = (dir == 0) ? "out" : "in";

    return sprintf(buf,"%s\n",direction);

}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

    int ret;
    struct gpio_dev_data *dev_data = dev_get_drvdata(dev);

    if(sysfs_streq(buf,"in")){
        ret = gpiod_direction_input(dev_data->desc);
    } else if (sysfs_streq(buf,"out")){
        ret = gpiod_direction_output(dev_data->desc, 0);
    } else {

        ret = -EINVAL;
    }

    return ret ? ret: count;

}

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{

    struct gpio_dev_data *dev_data = dev_get_drvdata(dev);
    int  value;
    value = gpiod_get_value(dev_data->desc);
    return sprintf(buf,"%d\n",value);

}

ssize_t value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{


    struct gpio_dev_data *dev_data = dev_get_drvdata(dev);
    int ret;
    long value;

    ret = kstrtol(buf,0,&value);

    if (ret){
        return  ret;
    }

    gpiod_set_value(dev_data->desc, value);
    return  count ;
}


ssize_t label_show(struct device *dev, struct device_attribute *attr, char *buf)
{

    struct gpio_dev_data *dev_data = dev_get_drvdata(dev);

    return sprintf(buf,"%s\n",dev_data->label);
}


/* see definition of DEVICE_ATTR_RW and DEVICE_ATTR_RO in include/linux/device.h */
static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

/*attribute structure*/
struct attribute *gpio_attrs[] = {
        &dev_attr_direction.attr,
        &dev_attr_value.attr,
        &dev_attr_label.attr,
        NULL,
};

/*attribute group structure*/

struct attribute_group gpio_attr_group = {
        .attrs = gpio_attrs,
};

/*attribute groups structure*/

const struct attribute_group *gpio_attr_groups[] = {
        &gpio_attr_group,
        NULL,
};






/*gets called when names are matched*/
int gpio_probe(struct platform_device *pdev)
{

    const char *name;

    int i = 0;
    int ret;
    /*get the parent device tree node*/
    struct device_node *parent = pdev->dev.of_node;
    /*get the child device tree node*/
    struct device_node *child = NULL;

    struct device *dev = &pdev->dev;

    struct gpio_dev_data *dev_data;

    gpio_drv_data.total_devices = of_get_child_count(parent);

    if (gpio_drv_data.total_devices == 0){
        pr_err("no child nodes found\n");
        return -ENODEV;
    }

    pr_info("total devices: %d\n", gpio_drv_data.total_devices);

    gpio_drv_data.devices = devm_kzalloc(dev, sizeof(struct device *) * gpio_drv_data.total_devices, GFP_KERNEL);


    /*allocate memory for driver data, this function checks for enabled children of a parent*/

    for_each_child_of_node(parent,child){


        /*allocate memory for device data*/
        dev_data = devm_kzalloc(&pdev->dev, sizeof(struct gpio_dev_data), GFP_KERNEL);
        if(!dev_data)
        {
            pr_err("memory allocation failed\n");
            return -ENOMEM;
        }

        /*get the label from the device tree*/
        if(of_property_read_string(child, "label", &name))
        {
            pr_err("label not found\n");
            snprintf(dev_data->label, sizeof(dev_data->label), "unknown-gpio%d", i);
        } else{
            strcpy(dev_data->label,name);
            pr_err("gpio label: %s\n", dev_data->label);


        }
        /*get the gpio descriptor from the device tree*/

        dev_data->desc = devm_fwnode_get_gpiod_from_child(dev, "bone", &child->fwnode, GPIOD_ASIS, dev_data->label);
        if(IS_ERR(dev_data->desc)){
            ret = PTR_ERR(dev_data->desc);
            if (ret == -ENOENT) {
                pr_err("no GPIO has been assigned\n");
                return ret;
            }
        }

        /*set direction of the gpio*/
        ret = gpiod_direction_output(dev_data->desc, 0);
        if(ret){
            pr_err("failed to set direction\n");
            return ret;
        }


        /*create devices under /sys/class/bone-gpio*/

       gpio_drv_data.devices[i] = device_create_with_groups(gpio_drv_data.class_gpio, NULL, 0, dev_data, gpio_attr_groups, dev_data->label);
       if(IS_ERR(gpio_drv_data.devices[i])){
            pr_err("device creation failed\n");
            return PTR_ERR(gpio_drv_data.devices[i]);
        }


        i++;
    }


    return 0;
}






/*gets called when devices are removed*/
int gpio_remove(struct platform_device *pdev)
{
    int i;

    pr_info("remove called\n");
    for(i = 0 ; i < gpio_drv_data.total_devices; i++){
        device_unregister(gpio_drv_data.devices[i]);

    }
    return 0;
}

/*matches the compatible string with the device tree*/
struct of_device_id gpio_of_match[] = {
    {
        .compatible = "org,bone-gpio-sysfs",
    },
    {},
};



/*platform driver structure*/
struct platform_driver gpio_platform_driver = {
    .probe = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = "gpio-sysfs",
        .of_match_table = gpio_of_match,
    },
};

//init

int __init gpio_sysfs_init(void)
{

    gpio_drv_data.class_gpio = class_create(THIS_MODULE,"bone-gpio");
    if(IS_ERR(gpio_drv_data.class_gpio))
    {
        pr_err("class creation failed\n");
        return PTR_ERR(gpio_drv_data.class_gpio);
    }
    pr_info("module loaded\n");

    return platform_driver_register(&gpio_platform_driver);

    return 0;
}


void __exit gpio_sysfs_exit(void)
{
    platform_driver_unregister(&gpio_platform_driver);
    class_destroy(gpio_drv_data.class_gpio);
    pr_info("module unloaded\n");
}


module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Avdo");
MODULE_DESCRIPTION("GPIO sysfs driver");