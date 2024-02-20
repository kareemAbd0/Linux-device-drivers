//
// Created by kareem on 3/6/24.
//

#include "pcd_platform_driver_dt_sysfs.h"


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




int pcdev_open(struct inode *inode, struct file *filp){



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



 int pcdev_release(struct inode *inode, struct file *filp){

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



 ssize_t pcdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
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
 ssize_t pcdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
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

