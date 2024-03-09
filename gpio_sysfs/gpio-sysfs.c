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



/*holds device specific data*/
struct gpio_dev_data{
    char label[20];

};

/*holds driver specific data*/
struct gpio_drv_data {

    int total_devices;
    struct class *class_gpio;
};

/*gets called when names are matched*/
int gpio_probe(struct platform_device *pdev)
{

    /*get the parent device tree node*/
    struct device_node *parent = pdev->dev.of_node;
    /*get the child device tree node*/

    for_each_child_of_node()

    return 0;
}

/*gets called when devices are removed*/
int gpio_remove(struct platform_device *pdev)
{
    return 0;
}

/*matches the compatible string with the device tree*/
struct of_device_id gpio_of_match[] = {
    {
        .compatible = "org,bone-gpio-sysfs",
    },
    {},
};


struct gpio_drv_data *gpio_drv_data;

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

    gpio_drv_data->class_gpio = class_create("bone-gpio");
    if(IS_ERR(gpio_drv_data->class_gpio))
    {
        pr_err("class creation failed\n");
        return PTR_ERR(gpio_drv_data->class_gpio);
    }
    pr_info("module loaded\n");

    return platform_driver_register(&gpio_platform_driver);

    return 0;
}


void __exit gpio_sysfs_exit(void)
{
    platform_driver_unregister(&gpio_platform_driver);
    class_destroy(gpio_drv_data->class_gpio);
    pr_info("module unloaded\n");
}


module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Avdo");
MODULE_DESCRIPTION("GPIO sysfs driver");