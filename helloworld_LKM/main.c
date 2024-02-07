#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>


static int __init helloworld_init(void){
    int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, const char *name);
    pr_info("Hello World\n");


    return 0;

}
static void __exit helloworld_exit(void){

    pr_info("Goodbye World\n");

}


module_init(helloworld_init);
module_exit(helloworld_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem");
MODULE_DESCRIPTION("A simple hello world module");
MODULE_INFO(board, "BeagleBone Black Rev.C");
