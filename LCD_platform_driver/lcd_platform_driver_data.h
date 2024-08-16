//
// Created by kareem on 3/13/24.
//

#ifndef LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
#define LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H




/*holds gpio device specific data*/
struct lcd_gpio_dev_data{
    char label[20];
    struct gpio_desc *desc;
};


/*holds lcd device specific data*/
struct lcd_dev_data {

    char text[30];
    int x;
    int y;
    long cmd;

};


/*holds driver specific data*/
struct lcd_drv_data {

    int total_devices;
    struct class *class_gpio;
    struct lcd_gpio_dev_data **gpio_devices;
    struct device *lcd_device;
    struct lcd_dev_data *lcd_dev_data;

};

#endif //LCD_PLATFORM_DRIVER_LCD_PLATFORM_DRIVER_H
