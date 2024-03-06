
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




/*platform is useful for the probe and remove function which has automatic matching*/




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

struct pcdrv_private_data pcdrv_data;



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

struct device_configure pcdev_configure[] = {
        [PCDEV_A_CONF] = {.configure_num1 = 40, .configure_num2 = 254 },
        [PCDEV_B_CONF] = {.configure_num1 = 50, .configure_num2 = 253 },
        [PCDEV_C_CONF] = {.configure_num1 = 60, .configure_num2 = 252 },
        [PCDEV_D_CONF] = {.configure_num1 = 70, .configure_num2 = 251 },
};



int check_permission(int dev_perm , int access_mode ){

    /*return 0 if permission is granted*/

    if (dev_perm == RD_WR ){
        return 0;
    }
    if ((dev_perm == RD_ONLY) && (access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE)){
        return 0;
    }
    if ((dev_perm == WR_ONLY) && (access_mode & FMODE_WRITE) && !(access_mode & FMODE_READ)){
        return 0;
    }

    return -EPERM;
}




static int pcdev_open(struct inode *inode, struct file *filp){



    int ret;
    int minor_n;
    struct pcdev_private_data *pcdev_data;

    /*find out on which device file was opened*/
    minor_n = MINOR(inode->i_rdev);
    printk("minor access = %d\n", minor_n);
    /*get device's private data structure*/
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

    /*to supply device private data to other methods of the driver*/
    filp->private_data = pcdev_data;

    /*check permission*/
    ret = check_permission(pcdev_data->pdata.perm, filp->f_mode);


    if(ret == 0){
        printk("open was successful\n");
    } else {
        printk("open was unsuccessful\n");
    }
    printk("open was called\n");
    return ret;
}



static int pcdev_release(struct inode *inode, struct file *filp){

    printk("release was called\n");
    return 0;


}


loff_t pchar_lseek(struct file *filp, loff_t offset, int whence){

    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->pdata.size;

    printk("lseek was called\n");
    printk("current file position = %lld\n", filp->f_pos);
    switch (whence) {
        case SEEK_SET:

            if ((offset > max_size || (offset < 0)) ){
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            if (((filp->f_pos + offset) > max_size || ((filp->f_pos + offset) < 0))) {
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos += offset;
            break;
        case SEEK_END:
            if (((max_size + offset) > max_size) || ((max_size + offset) < 0)) {
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos = max_size + offset;
            break;
        default:
            printk("Invalid whence\n");
            return -EINVAL;

    }
    printk("updated file position = %lld\n", filp->f_pos);
    return filp->f_pos;


}




static ssize_t pcdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{


    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->pdata.size;


    printk("read was called for %zu bytes\n", count);
    printk("current file position = %lld\n", *f_pos);
    /*adjust count*/
    if ((*f_pos + count )> max_size){
        count = max_size - *f_pos;
    }
    /*copy to user*/

    if(copy_to_user(buff, pcdev_data->buffer +(*f_pos), count)){
        return -EFAULT;
    }

    /*update file position*/
    *f_pos += count;

    printk("Number of bytes successfully read = %zu\n", count);
    printk("updated file position = %lld\n", *f_pos);
    /*return number of bytes read*/
    return count;


}
static ssize_t pcdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->pdata.size;

    printk("write was called for %zu bytes\n", count);
    printk("current file position = %lld\n", *f_pos);
    /*adjust count*/
    if ((*f_pos + count )> max_size){
        count = max_size - *f_pos;
    }
    if (count == 0) {
        printk("No space left on device\n");
        return -ENOMEM;
    }
    /*copy from user*/
    if (copy_from_user( pcdev_data->buffer + *f_pos, buff, count)) {
        return -EFAULT;
    }
    /*update file position*/
    *f_pos += count;
    printk("Number of bytes successfully written = %zu\n", count);
    printk("updated file position = %lld\n", *f_pos);
    /*return number of bytes written*/
    return count;
}


/* structure to hold file operations */
static struct file_operations pcdev_fops = {
        .open = pcdev_open,
        .release = pcdev_release,
        .read = pcdev_read,
        .write = pcdev_write,
        /* add more file operations as needed */
};



/* gets called when device is removed from the system */

int pcd_platform_driver_remove(struct platform_device *pdev)
{
    /*1- remove device that was created with device_create() */
    struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);
    device_destroy(pcdrv_data.mpchar_class, dev_data->device_number);

    /*2- remove cdev entry */
    cdev_del(&dev_data->cdev);

    pcdrv_data.total_devices--;
    pr_info("driver: A device is removed\n");
    return 0;
}


// if dev->of_node is not NULL, the device was from device tree instead of device setup

struct pcdev_platform_data* pcdev_get_platdata_from_dt (struct device *dev){

    struct device_node *device_node  = dev->of_node;
    struct pcdev_platform_data *pdata;

    if(!device_node) return NULL;

    pdata = devm_kzalloc(dev,sizeof(*pdata),GFP_KERNEL);
    if (!pdata){
        pr_info("Cannot allocate memory");
        return ERR_PTR( -ENOMEM);
    }
    if(of_property_read_string(device_node, "org,device-serial-num", &pdata->serial_number)){
        pr_info("missing serial number\n");
        return ERR_PTR(-EINVAL);
    }
    if(of_property_read_u32(device_node, "org,size", &pdata->size)){
        pr_info("missing size\n");
        return ERR_PTR(-EINVAL);
    }
    if(of_property_read_u32(device_node, "org,perm", &pdata->perm)){
        pr_info("missing permission\n");
        return ERR_PTR(-EINVAL);
    }

    return pdata;




}



/* the platform device data is copied to the dev data here, dev has a platform device member which is populated by the data in pdev that is coming from another driver */

/* gets called when match device is found */
int pcd_platform_driver_probe(struct platform_device *pdev)
{


    int ret;
    struct pcdev_private_data *dev_data = NULL;
    struct pcdev_platform_data *pdata;

    int driver_data;
    pr_info("driver: A device is detected\n");


    /* 1- get platform data */


    pdata = pcdev_get_platdata_from_dt(&pdev->dev);
    if(IS_ERR(pdata)){
        return PTR_ERR(pdata);
    }


    if (!pdata) {
        // non device tree method
        pdata = (struct pcdev_platform_data *) dev_get_platdata(&pdev->dev);
        if (!pdata) {
            pr_info("No platform data available\n");
            return -EINVAL;
        }

        driver_data = pdev->id_entry->driver_data;

    } else {

        driver_data = (int)device_get_match_data(&pdev->dev);

    }


    /* 2- allocate memory for device private data */
    dev_data = devm_kzalloc(&pdev->dev,sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        pr_info("Cannot allocate memory\n");
        return -ENOMEM;
    }


    /* save device private data pointer in platform device structure, used to share data between prop and remove */
    dev_set_drvdata(&pdev->dev, dev_data);

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.perm = pdata->perm;
    dev_data->pdata.serial_number = pdata->serial_number;
    pr_info("driver: device number = %d\n", pcdrv_data.total_devices);
    pr_info("driver: Device serial number %s\n", dev_data->pdata.serial_number);
    pr_info("driver: Device size %d\n", dev_data->pdata.size);
    pr_info("driver: Device permission %d\n", dev_data->pdata.perm);
    pr_info("driver: Driver data %d\n",pcdev_configure[driver_data].configure_num1);
    pr_info("driver: Driver data %d\n",pcdev_configure[driver_data].configure_num2);

    /* 3- dynamically allocate memory for the device buffer */
    dev_data->buffer = devm_kzalloc(&pdev->dev,dev_data->pdata.size, GFP_KERNEL);
    if (!dev_data->buffer) {
        pr_info("Cannot allocate memory\n");
        return -ENOMEM;
    }

    /* 4- get device number */
    dev_data->device_number = pcdrv_data.device_number_base + pcdrv_data.total_devices;

    /* 5- cdev init and cdev add  */
    cdev_init(&dev_data->cdev, &pcdev_fops);
    dev_data->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->cdev, dev_data->device_number, 1);
    if (ret < 0) {
        pr_info("cdev add failed\n");
        return ret;
    }


    /* 6- create device file for detected platform device */
    pcdrv_data.mpchar_device = device_create(pcdrv_data.mpchar_class,&pdev->dev , dev_data->device_number, NULL, "pcdev-%d", pcdrv_data.total_devices);
    if (IS_ERR(pcdrv_data.mpchar_device)) {
        pr_info("device create failed\n");
        ret = PTR_ERR(pcdrv_data.mpchar_device);
        cdev_del(&dev_data->cdev);
        return ret;
    }

    pcdrv_data.total_devices++;

    pr_info("driver: the probe was successful\n");

    return 0;


}


/* platform device ids to accept multiple names instead of 1, so it probes anything with names in here, .name not necessary now */




struct platform_device_id pcdevs_ids[] = {
        {.name = "pcdev-A1x", .driver_data = PCDEV_A_CONF},
        {.name = "pcdev-B1x", .driver_data = PCDEV_B_CONF},
        {.name = "pcdev-C1x", .driver_data = PCDEV_C_CONF},
        {.name = "pcdev-D1x", .driver_data = PCDEV_D_CONF},
        {}
};



/* device tree match table (for device tree methodology only) */

struct of_device_id org_pcdev_dt_match[] = {
        {.compatible = "pcdev-A1x", .data = (void *)PCDEV_A_CONF},
        {.compatible = "pcdev-B1x", .data = (void *)PCDEV_B_CONF},
        {.compatible = "pcdev-C1x", .data = (void *)PCDEV_C_CONF},
        {.compatible = "pcdev-D1x", .data = (void *)PCDEV_D_CONF},
        {}
};


/* platform driver structure */
static struct platform_driver pcd_platform_driver = {
        .probe = pcd_platform_driver_probe,
        .remove = pcd_platform_driver_remove,
        .id_table = pcdevs_ids,
        .driver = {
                .name = "pseudo_platform_driver",
                .of_match_table = org_pcdev_dt_match,
        },
};






/* module initialization and cleanup functions */
static int __init pcdev_platform_init(void)
{

    int ret;
    /* 1- dynamic allocation of device number */

    ret = alloc_chrdev_region(&pcdrv_data.device_number_base, 0,MAX_DEVICES, "mpchar_driver");
    if(ret < 0) {
        printk("driver: failed to allocate device number\n");
        return ret;
    }
    /* 2- create device class under /sys/class */

    pcdrv_data.mpchar_class = class_create(THIS_MODULE,"mpchar_class");
    if(IS_ERR(pcdrv_data.mpchar_class)){
        printk("driver: class creation failed\n");
        ret = PTR_ERR(pcdrv_data.mpchar_class);
        unregister_chrdev_region(pcdrv_data.device_number_base, MAX_DEVICES);
        return ret;
    }

    /* 3- register platform driver */

    platform_driver_register(&pcd_platform_driver);
    pr_info("driver: pcd platform driver loaded");


    return 0;
}

static void __exit pcdev_platform_exit(void)
{

    /* 1- unregister platform driver */
    platform_driver_unregister(&pcd_platform_driver);

    /* 2-  destroy device class */
    class_destroy(pcdrv_data.mpchar_class);
    /* 3- unregister device number */
    unregister_chrdev_region(pcdrv_data.device_number_base, MAX_DEVICES);

    pr_info("driver: pcd platform driver unloaded");
    /* release resources acquired during initialization */
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_AUTHOR("Kareem Abdo");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Character Driver");
