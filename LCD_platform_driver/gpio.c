//
// Created by kareem on 3/13/24.
//


#include "gpio.h"


ssize_t GPIO_set_value(unsigned char u8_value,struct gpio_desc *gpio_desc){

    gpiod_set_value(gpio_desc,u8_value);
    return 0;
}

ssize_t GPIO_get_value(unsigned char *u8_value,struct gpio_desc *gpio_desc){

    *u8_value = gpiod_get_value(gpio_desc);
    return 0;
}

ssize_t GPIO_set_direction(unsigned char u8_direction,struct gpio_desc *gpio_desc){

    gpiod_direction_output(gpio_desc,u8_direction);
    return 0;
}


ssize_t GPIO_get_direction(unsigned char *u8_direction,struct gpio_desc *gpio_desc){

    *u8_direction = gpiod_get_direction(gpio_desc);
    return 0;
}



