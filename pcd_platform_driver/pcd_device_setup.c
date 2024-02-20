#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"


#define WR_ONLY 0x10
#define RD_ONLY 0x01
#define RD_WR 0x11




void pcdev_release(struct device *dev){
    pr_info("device: device released");

}



/* 1 - create 2 platform data */
struct pcdev_platform_data pcdev_data[] = {
        [0] = { .size = 512 , .perm = RD_WR , .serial_number = "PCDEV1"},
        [1] = { .size = 1024, .perm = RD_WR , .serial_number = "PCDEV2"},
        [2] = { .size = 256 , .perm = RD_ONLY , .serial_number = "PCDEV3"},
        [3] = { .size = 128 , .perm = RD_WR , .serial_number = "PCDEV4"},



};


/* 2 - create 2 platform devices */

struct  platform_device platform_device_1 = {
        .name = "pseudo_platform-Z1",
        .id = 0,
        .dev = {
                .platform_data = &pcdev_data[0],
                .release = pcdev_release

        }
};

struct  platform_device platform_device_2 = {
        .name = "pseudo_platform-A1",
        .id = 1,
        .dev = {
                .platform_data = &pcdev_data[1],
                .release = pcdev_release
        }
};

struct  platform_device platform_device_3 = {
        .name = "pseudo_platform-B1",
        .id = 2,
        .dev = {
                .platform_data = &pcdev_data[2],
                .release = pcdev_release

        }
};



struct  platform_device platform_device_4 = {
        .name = "pseudo_platform-R1",
        .id = 3,
        .dev = {
                .platform_data = &pcdev_data[3],
                .release = pcdev_release

        }
};
static int __init pcd_platform_driver_init(void){
       /* register platform device */
        platform_device_register(&platform_device_1);
        platform_device_register(&platform_device_2);
        platform_device_register(&platform_device_3);
        platform_device_register(&platform_device_4);


        pr_info("device: device setup module inserted\n");
        return 0;
}


static void __exit pcd_platform_driver_cleanup(void){
         /* unregister platform device */
        platform_device_unregister(&platform_device_1);
        platform_device_unregister(&platform_device_2);
        platform_device_unregister(&platform_device_3);
        platform_device_unregister(&platform_device_4);
        pr_info("device: device setup module removed\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers 2 platform devices");