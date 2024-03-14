//
// Created by kareem on 3/13/24.
//
#include "lcd_platform_driver.h"




struct lcd_dev_data *dev_data;



static DEVICE_ATTR_RW(lcdcmd);
static DEVICE_ATTR_RW(lcdscroll);
static DEVICE_ATTR_RW(lcdpxy);




/*gets called when names are matched*/
int lcd_probe(struct platform_device *pdev){

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



}



void __exit lcd_platform_driver_exit(void) {

}


module_init(lcd_platform_driver_init);
module_exit(lcd_platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kareem Abdo");
MODULE_DESCRIPTION("A simple platform driver for LCD");
