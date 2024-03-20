//
// Created by kareem on 3/13/24.
//

#ifndef PLATFORM_MPCHAR_GPIO_H
#define PLATFORM_MPCHAR_GPIO_H
#include "linux/gpio/consumer.h"


ssize_t GPIO_set_value(unsigned char u8_value,struct gpio_desc *gpio_desc);
ssize_t GPIO_get_value(unsigned char *u8_value,struct gpio_desc *gpio_desc);
ssize_t GPIO_set_direction(unsigned char u8_direction,struct gpio_desc *gpio_desc);
ssize_t GPIO_get_direction(unsigned char *u8_direction,struct gpio_desc *gpio_desc);










#endif //PLATFORM_MPCHAR_GPIO_H
