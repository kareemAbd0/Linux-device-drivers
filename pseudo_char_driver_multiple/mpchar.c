#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

#define WR_ONLY 0x10
#define RD_ONLY 0x01
#define RD_WR 0x11
#define NUMBER_OF_DEVICES 4



/*pseudo device memory*/
char  device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char  device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char  device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char  device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];


/*device private data structure*/

struct pcdev_private_data {

	char * buffer;
	unsigned int size;
	const char *serial_number;
	int perm;
	/*holds cdev instance*/
	struct cdev cdev;

};


/*driver private data structure*/
struct pcdrv_private_data {
    	
    int total_devices;

   	/*holds device number, it is specific to driver not device */
	dev_t device_number;
	/*holds device*/
	struct device *mpchar_device;
	/*holds device class*/
	struct class *mpchar_class;
	struct pcdev_private_data pcdev_data[];
};


/*initialize driver private data structure*/

struct pcdrv_private_data pcdrv_data =
{

	.total_devices = NUMBER_OF_DEVICES,
	.pcdev_data = 
	{
		[0] = { .buffer = device_buffer_pcdev1, .size = MEM_SIZE_MAX_PCDEV1, .serial_number = "PCDEV1", .perm = RD_ONLY /*means read only*/},
		[1] = { .buffer = device_buffer_pcdev2, .size = MEM_SIZE_MAX_PCDEV2, .serial_number = "PCDEV2", .perm = WR_ONLY /*means write only */},
		[2] = { .buffer = device_buffer_pcdev3, .size = MEM_SIZE_MAX_PCDEV3, .serial_number = "PCDEV3", .perm = RD_WR/*means read and write*/},
		[3] = { .buffer = device_buffer_pcdev4, .size = MEM_SIZE_MAX_PCDEV4, .serial_number = "PCDEV4", .perm = RD_WR /*means read and write*/}
	}
	


};




// file operations implementation
loff_t pchar_lseek(struct file *filp, loff_t offset, int whence){

    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;

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




ssize_t pchar_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){


    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;


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



ssize_t pchar_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){


    /*get device's private data structure from shared file*/
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;

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


int pchar_open(struct inode *inode, struct file *filp){

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
    ret = check_permission(pcdev_data->perm, filp->f_mode);


    if(ret == 0){
        printk("open was successful\n");
    } else {
        printk("open was unsuccessful\n");
    }

    printk("open was called\n");
    return ret;
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
static int __init mpchar_driver_init(void){
    int ret;



    /*dynamically allocate device number*/
    ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NUMBER_OF_DEVICES, "mpchar_driver");
    if(ret < 0) {
        printk("failed to allocate device number\n");
        goto out ;
     }



    /*create device class under /sys/class*/
    pcdrv_data.mpchar_class = class_create(THIS_MODULE, "mpchar_class");
    //pcdrv_data.mpchar_class = class_create("mpchar_class");
    if(IS_ERR(pcdrv_data.mpchar_class)){
        printk("class creation failed\n");
        ret = PTR_ERR(pcdrv_data.mpchar_class);
        goto unreg_chrdev;
    }

    int i = 0;
    for( i = 0 ; i < NUMBER_OF_DEVICES; i++){
 	    printk("%s :Major number = %d, Minor number = %d\n",__func__, MAJOR(pcdrv_data.device_number +i), MINOR(pcdrv_data.device_number+i));
	


    	/*initialize cdev structure with fops*/
	    cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pchar_fops);

	    /*register cdev structure with VFS*/
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
	    ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number+i, 1);
    	    if(ret < 0) {
    	         printk("failed to add cdev to VFS\n");
    	        goto cdev_del;
    	        }

	/*populate sysfs with device information*/

    pcdrv_data.mpchar_device = device_create(pcdrv_data.mpchar_class,NULL,pcdrv_data.device_number+i,NULL,"mpchar_device_%d",i+1);
	if(IS_ERR(pcdrv_data.mpchar_device)) {
        printk("device creation failed\n");
        ret = PTR_ERR(pcdrv_data.mpchar_device);
        goto class_del;
    }
 }



    printk("module init was successful\n");
    
//return 0 on success

    return 0;



    cdev_del:

    class_del:
        for (;i>=0;i--){
            device_destroy(pcdrv_data.mpchar_class, pcdrv_data.device_number+i);
            cdev_del(&pcdrv_data.pcdev_data[i].cdev);
            }
        class_destroy(pcdrv_data.mpchar_class);


    unreg_chrdev:
        unregister_chrdev_region(pcdrv_data.device_number, NUMBER_OF_DEVICES);



    out :
        printk("module init was unsuccessful\n");

       return ret;


}


static void __exit mpchar_driver_exit(void){


    int i = 0;
    for( i = 0 ; i < NUMBER_OF_DEVICES; i++){
        device_destroy(pcdrv_data.mpchar_class, pcdrv_data.device_number+i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.mpchar_class);
    unregister_chrdev_region(pcdrv_data.device_number, NUMBER_OF_DEVICES);
    printk("module unloaded\n");

}


module_init(mpchar_driver_init);
module_exit(mpchar_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem");
MODULE_DESCRIPTION("A simple multi pseudo char driver");

