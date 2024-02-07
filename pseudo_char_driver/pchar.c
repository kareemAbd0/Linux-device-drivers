#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define DEV_MEM_SIZE 512

char __user *pbuffer;


/*pseudo device memory*/
char  device_buffer[DEV_MEM_SIZE];

//holds device number
dev_t device_number;

/*holds cdev instance*/
struct cdev pchar_cdev;
/*holds device*/
struct device *pchar_device;
/*holds device class*/
struct class *pchar_class;
// file operations implementation
loff_t pchar_lseek(struct file *filp, loff_t offset, int whence){
    printk("lseek was called\n");
    printk("current file position = %lld\n", filp->f_pos);
    switch (whence) {
case SEEK_SET:

            if ((offset > DEV_MEM_SIZE) || (offset < 0)) {
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            if (((filp->f_pos + offset) > DEV_MEM_SIZE) || ((filp->f_pos + offset) < 0)) {
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos += offset;
            break;
        case SEEK_END:
            if (((DEV_MEM_SIZE + offset) > DEV_MEM_SIZE) || ((DEV_MEM_SIZE + offset) < 0)) {
                printk("Invalid offset\n");
                return -EINVAL;
            }
            filp->f_pos = DEV_MEM_SIZE + offset;
            break;
        default:
            printk("Invalid whence\n");
            return -EINVAL;

    }
    printk("updated file position = %lld\n", filp->f_pos);
    return filp->f_pos;

}
ssize_t pchar_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){

    printk("read was called for %zu bytes\n", count);
    printk("current file position = %lld\n", *f_pos);
    /*adjust count*/
    if ((*f_pos + count )> DEV_MEM_SIZE){
        count = DEV_MEM_SIZE - *f_pos;
    }
    /*copy to user*/

    if(copy_to_user(buff, device_buffer +*f_pos, count)){
        return -EFAULT;
    }

    /*update file position*/
    *f_pos += count;

    printk("Number of bytes successfully read = %zu\n", count);
    printk("updated file position = %lld\n", *f_pos);
    /*return number of bytes read*/
    return count;

}
ssize_t pchar_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
    printk("write was called for %zu bytes\n", count);
    printk("current file position = %lld\n", *f_pos);
    /*adjust count*/
    if ((*f_pos + count )> DEV_MEM_SIZE){
        count = DEV_MEM_SIZE - *f_pos;
    }
    if (count == 0) {
        printk("No space left on device\n");
        return -ENOMEM;
    }
    /*copy from user*/
    if (copy_from_user(device_buffer + *f_pos, buff, count)) {
        return -EFAULT;
    }
    /*update file position*/
    *f_pos += count;
    printk("Number of bytes successfully written = %zu\n", count);
    printk("updated file position = %lld\n", *f_pos);
    /*return number of bytes written*/
    return count;

}
int pchar_open(struct inode *inode, struct file *filp){
    printk("open was called\n");
    return 0;
}
int pchar_release(struct inode *inode, struct file *filp){
    printk("release was called\n");
    return 0;
}

/*holds file operations*/

struct file_operations pchar_fops = {
    .llseek = pchar_lseek,
    .read = pchar_read,
    .write = pchar_write,
    .open = pchar_open,
    .release = pchar_release,
    .owner = THIS_MODULE,
};

static int __init pchar_driver_init(void){
    int ret;
/*dynamically allocate device number*/
 ret = alloc_chrdev_region(&device_number, 0, 1, "pchar_driver");
 if(ret < 0) {
        printk("failed to allocate device number\n");
     goto out ;
 }

printk("%s :Major number = %d, Minor number = %d\n",__func__, MAJOR(device_number), MINOR(device_number));


/*initialize cdev structure with fops*/
cdev_init(&pchar_cdev, &pchar_fops);

/*register cdev structure with VFS*/
pchar_cdev.owner = THIS_MODULE;
ret = cdev_add(&pchar_cdev, device_number, 1);
    if(ret < 0) {
        printk("failed to add cdev to VFS\n");
        goto unreg_chrdev;
    }


/*create device class under /sys/class*/
pchar_class = class_create(THIS_MODULE, "pchar_class");
if(IS_ERR(pchar_class)){
    printk("class creation failed\n");
    ret = PTR_ERR(pchar_class);
    goto cdev_del;
}

/*populate sysfs with device information*/
pchar_device =  device_create(pchar_class,NULL,device_number,NULL,"pchar_driver");
if(IS_ERR(pchar_device)){
    printk("device creation failed\n");
    ret = PTR_ERR(pchar_device);
    goto class_del;
}
    printk("module init was successful\n");

//return 0 on success
return 0;


    out :
    printk("module init was unsuccessful\n");
    unreg_chrdev:
    unregister_chrdev_region(device_number, 1);

    cdev_del:
    cdev_del(&pchar_cdev);

    class_del:
    class_destroy(pchar_class);

    return ret;


}


static void __exit pchar_driver_exit(void){

/*cleanup device*/
    device_destroy(pchar_class, device_number);
/*cleanup class*/
    class_destroy(pchar_class);
/*unregister cdev*/
    cdev_del(&pchar_cdev);
/*unregister device number*/
    unregister_chrdev_region(device_number, 1);
    printk("module unloaded\n");
}
module_init(pchar_driver_init);
module_exit(pchar_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem");
MODULE_DESCRIPTION("A simple pseudo char driver");

