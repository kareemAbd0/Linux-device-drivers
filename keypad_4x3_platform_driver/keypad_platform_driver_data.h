//
// Created by kareem on 12/08/24.
//

#ifndef KEYPAD_PLATFORM_DRIVER_DATA_H
#define KEYPAD_PLATFORM_DRIVER_DATA_H


/*holds gpio device specific data*/
struct keypad_gpio_dev_data {
    char label[20];
    struct gpio_desc *desc;
};


struct keypad_dev_data {
    char current_char_input;
    uint8_t state;
    uint8_t polling;
};


/*holds driver specific data*/
struct keypad_drv_data {
    int total_devices;
    struct class *class_gpio;
    struct keypad_gpio_dev_data **gpio_devices;
    struct device *keypad_device;
    struct keypad_dev_data *keypad_dev_data;
};


#endif //KEYPAD_PLATFORM_DRIVER_DATA_H
