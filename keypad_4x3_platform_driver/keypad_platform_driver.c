


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
#include <linux/delay.h>
#include <linux/kthread.h>
#include "keypad_platform_driver_data.h"
#include "keypad.h"

/*this driver is desgined to work with kernel 5.15*/
/* kernel driver for 4x3 keypad*/


struct keypad_drv_data keypad_drv_data;
static struct task_struct *keypad_thread;


int keypad_thread_func(void *data) {
    struct keypad_drv_data *keypad_drv_data = (struct keypad_drv_data *) data;
    while (!kthread_should_stop()) {
        if (keypad_drv_data->keypad_dev_data->polling) {
            pr_info("Polling...\n");
            keypad_read(keypad_drv_data);
        }
        msleep(2);
    }

    pr_info("Polling thread exiting...\n");
    return 0;
}


ssize_t keypadChar_show(struct device *dev, struct device_attribute *attr, char *buf) {
    //get the driver data

    struct keypad_drv_data *keypad_drv_data = dev_get_drvdata(dev);

    if (!keypad_drv_data) {
        pr_err("keypad_drv_data is null\n");
        return -1;
    }


    struct keypad_dev_data *keypad_dev_data = keypad_drv_data->keypad_dev_data;

    if (!keypad_dev_data) {
        pr_err("keypad_dev_data is null\n");
        return -1;
    }

    return sprintf(buf, "%c\n", keypad_dev_data->current_char_input);
}

ssize_t keypadChar_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    return -EPERM;
}


ssize_t keypadState_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct keypad_drv_data *keypad_drv_data = dev_get_drvdata(dev);
    if (!keypad_drv_data) {
        pr_err("keypad_drv_data is null\n");
        return -1;
    }
    struct keypad_dev_data *keypad_dev_data = keypad_drv_data->keypad_dev_data;
    if (!keypad_dev_data) {
        pr_err("keypad_dev_data is null\n");
        return -1;
    }

    /*check if the input is valid*/
    if (buf[0] != '0' && buf[0] != '1') {
        pr_err("Invalid input\n");
        return -EINVAL;
    }


    int ret;
    ret = kstrtou8(buf, 10, &keypad_dev_data->state);
    if (ret) {
        pr_err("Failed to convert string to integer\n");
        return ret;
    }
    return count;
}

ssize_t keypadState_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct keypad_drv_data *keypad_drv_data = dev_get_drvdata(dev);
    if (!keypad_drv_data) {
        pr_err("keypad_drv_data is null\n");
        return -1;
    }
    struct keypad_dev_data *keypad_dev_data = keypad_drv_data->keypad_dev_data;
    if (!keypad_dev_data) {
        pr_err("keypad_dev_data is null\n");
        return -1;
    }

    return sprintf(buf, "%d\n", keypad_dev_data->state);
}

ssize_t keypadPolling_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct keypad_drv_data *keypad_drv_data = dev_get_drvdata(dev);
    if (!keypad_drv_data) {
        pr_err("keypad_drv_data is null\n");
        return -1;
    }
    struct keypad_dev_data *keypad_dev_data = keypad_drv_data->keypad_dev_data;
    if (!keypad_dev_data) {
        pr_err("keypad_dev_data is null\n");
        return -1;
    }

    int ret;

    /*check if the input is valid*/
    if (buf[0] != '0' && buf[0] != '1') {
        pr_err("Invalid input\n");
        return -EINVAL;
    }

    ret = kstrtou8(buf, 10, &keypad_dev_data->polling);
    if (ret) {
        pr_err("Failed to convert string to integer\n");
        return ret;
    }

    if (keypad_dev_data->polling == 1) {
        if (!keypad_thread) {
            keypad_thread = kthread_run(keypad_thread_func, keypad_drv_data, "keypad_thread");
            if (IS_ERR(keypad_thread)) {
                pr_err("Failed to create kernel thread\n");
                keypad_thread = NULL;
                return PTR_ERR(keypad_thread);
            }
        } else {
            pr_info("Polling thread is already running.\n");
        }
    } else {
        if (keypad_thread) {
            kthread_stop(keypad_thread);
            pr_info("Polling thread stopped.\n");
            keypad_thread = NULL;
        }
    }

    return count;
}

ssize_t keypadPolling_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct keypad_drv_data *keypad_drv_data = dev_get_drvdata(dev);
    if (!keypad_drv_data) {
        pr_err("keypad_drv_data is null\n");
        return -1;
    }
    struct keypad_dev_data *keypad_dev_data = keypad_drv_data->keypad_dev_data;
    if (!keypad_dev_data) {
        pr_err("keypad_dev_data is null\n");
        return -1;
    }

    return sprintf(buf, "%d\n", keypad_dev_data->polling);
}


/* unique unclear macros, check implementation */

static DEVICE_ATTR_RO(keypadChar);
static DEVICE_ATTR_RW(keypadState);
static DEVICE_ATTR_RW(keypadPolling);


/*attributes for the device*/
static struct attribute *keypad_attrs[] = {
    &dev_attr_keypadChar.attr,
    &dev_attr_keypadState.attr,
    &dev_attr_keypadPolling.attr,
    NULL
};

/*attribute group structure*/
static struct attribute_group keypad_attr_group = {
    .attrs = keypad_attrs,
};


/*attribute groups structure*/
static const struct attribute_group *keypad_attr_groups[] = {
    &keypad_attr_group,
    NULL
};


/*probe function for the platform driver, maps devices from device tree */
int keypad_probe(struct platform_device *pdev) {
    const char *name;
    int i = 0;
    int ret;
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;
    struct device *dev = &pdev->dev;
    struct keypad_gpio_dev_data *gpio_dev_data;

    // Get total number of child nodes
    keypad_drv_data.total_devices = of_get_child_count(parent);

    // Check if there are child nodes
    if (keypad_drv_data.total_devices < 0) {
        dev_err(dev, "No child nodes found\n");
        return -ENODEV;
    }

    // Allocate memory for the gpio devices
    keypad_drv_data.gpio_devices = devm_kzalloc(
        dev, sizeof(struct keypad_gpio_dev_data *) * keypad_drv_data.total_devices, GFP_KERNEL);

    // Check if memory allocation was successful
    if (!keypad_drv_data.gpio_devices) {
        dev_err(dev, "Memory allocation failed\n");
        return -ENOMEM;
    }


    // Allocate memory for driver data
    for_each_child_of_node(parent, child) {
        gpio_dev_data = devm_kzalloc(dev, sizeof(struct keypad_gpio_dev_data), GFP_KERNEL);

        // Check if memory allocation was successful
        if (!gpio_dev_data) {
            dev_info(dev, "Memory allocation failed ");
        }

        // Get the label from the device tree
        if (of_property_read_string(child, "label", &name)) {
            pr_err("label not found\n");
            snprintf(gpio_dev_data->label, sizeof(gpio_dev_data->label), "unknown-pin%d", i);
        } else {
            strcpy(gpio_dev_data->label, name);
            pr_err("pin label: %s\n", gpio_dev_data->label);
        }

        // Get the gpio descriptor from the device tree
        gpio_dev_data->desc = devm_fwnode_get_gpiod_from_child(dev, "rpi", &child->fwnode, GPIOD_ASIS,
                                                               gpio_dev_data->label);

        // Check if getting gpio descriptor was successful
        if (IS_ERR(gpio_dev_data->desc)) {
            ret = PTR_ERR(gpio_dev_data->desc);
            if (ret == -ENOENT) {
                dev_err(dev, "Failed to get gpio descriptor\n");
            }

            return ret;
        }

        // Set direction of the gpio
        ret = gpiod_direction_output(gpio_dev_data->desc, 0);

        // Check if setting direction was successful
        if (ret) {
            pr_err("failed to set direction\n");
            return ret;
        }

        // Populate driver data with gpio devices
        keypad_drv_data.gpio_devices[i] = gpio_dev_data;


        i++;
    }


    /* Create keypad device under /sys/class/ and bind everything together */
    keypad_drv_data.keypad_device = device_create_with_groups(keypad_drv_data.class_gpio, dev, 0, &keypad_drv_data,
                                                              keypad_attr_groups, "keypad");


    if (IS_ERR(keypad_drv_data.class_gpio)) {
        dev_err(dev, "Failed to create class\n");
        return PTR_ERR(keypad_drv_data.class_gpio);
    }

    /* Check if device creation was successful */
    if (IS_ERR(keypad_drv_data.keypad_device)) {
        dev_err(dev, "device create failed\n");
        return PTR_ERR(keypad_drv_data.keypad_device);
    }


    /* Allocate memory for keypad_dev_data*/
    keypad_drv_data.keypad_dev_data = devm_kzalloc(dev, sizeof(struct keypad_dev_data), GFP_KERNEL);
    if (!keypad_drv_data.keypad_dev_data) {
        dev_err(dev, "Failed to allocate memory for keypad_dev_data\n");
        return -ENOMEM;
    }


    // Initialize kernel thread pointer
    keypad_thread = NULL;


    /*initialize the keypad*/


    if (keypad_init(&keypad_drv_data) != 0) {
        pr_err("Failed to initialize keypad\n");
        return -1;
    }


    return 0;
}

int keypad_remove(struct platform_device *pdev) {
    device_unregister(keypad_drv_data.keypad_device);


    // Stop kernel thread if running
    if (keypad_thread) {
        kthread_stop(keypad_thread);
    }


    return 0;
}


/*matches the compatible string with the device tree*/
struct of_device_id keypad_of_match[] = {
    {
        .compatible = "org,keypad",
    },
    {},
};

/*platform driver structure*/
struct platform_driver keypad_platform_driver = {
    .probe = keypad_probe,
    .remove = keypad_remove,
    .driver = {
        .name = "keypad_4x3",
        .of_match_table = keypad_of_match,
    },
};


int __init keypad_platform_driver_init(void) {
    keypad_drv_data.class_gpio = class_create(THIS_MODULE, "keypad_4x3");
    if (IS_ERR(keypad_drv_data.class_gpio)) {
        pr_err("Failed to create class\n");
        return PTR_ERR(keypad_drv_data.class_gpio);
    }
    pr_info("module loaded\n");
    return platform_driver_register(&keypad_platform_driver);
}

void __exit keypad_platform_driver_exit(void) {
    platform_driver_unregister(&keypad_platform_driver);
    class_destroy(keypad_drv_data.class_gpio);
    pr_info("module unloaded\n");
}


module_init(keypad_platform_driver_init);
module_exit(keypad_platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Abdo");
MODULE_DESCRIPTION("A simple platform driver for 4x3 keypad");

