//
// Created by kareem on 3/13/24.
//

#ifndef LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
#define LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H




/*holds device specific data*/
struct lcd_dev_data{
    char label[20];
    struct gpio_desc *desc;
};

/*holds driver specific data*/
struct lcd_drv_data {

    int total_devices;
    struct class *class_gpio;
    struct device **devices;

};

#endif //LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
