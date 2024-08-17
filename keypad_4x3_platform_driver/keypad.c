//
// Created by kareem on 12/08/24.
//


#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include "keypad_platform_driver_data.h"
#include "keypad.h"

/*user application should manually set state to 0 to indiacate that a key has been read*/

/*when a key is pressed the state is set to 1 and the new current_char_input is set to the key pressed*/


char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

uint8_t keypad_init(struct keypad_drv_data *KeypadDrvData) {
    int error = 0;
    /*initialize the gpio devices*/
    gpiod_direction_input(KeypadDrvData->gpio_devices[R1_PIN]->desc);
    gpiod_direction_input(KeypadDrvData->gpio_devices[R2_PIN]->desc);
    gpiod_direction_input(KeypadDrvData->gpio_devices[R3_PIN]->desc);
    gpiod_direction_input(KeypadDrvData->gpio_devices[R4_PIN]->desc);


    gpiod_direction_output(KeypadDrvData->gpio_devices[C1_PIN]->desc, 1);
    gpiod_direction_output(KeypadDrvData->gpio_devices[C2_PIN]->desc, 1);
    gpiod_direction_output(KeypadDrvData->gpio_devices[C3_PIN]->desc, 1);


    /*intialize the keypad device data*/
    KeypadDrvData->keypad_dev_data->current_char_input = ' ';
    KeypadDrvData->keypad_dev_data->state = 0;
    KeypadDrvData->keypad_dev_data->polling = 0;

    return error;
}


uint8_t keypad_read(struct keypad_drv_data *KeypadDrvData) {
    char coloms[COLS] = {C1_PIN, C2_PIN, C3_PIN};
    char rows[ROWS] = {R1_PIN, R2_PIN, R3_PIN, R4_PIN};
    int i = 0, j = 0;


    for (i = 0; i < COLS; i++) {
        gpiod_set_value(KeypadDrvData->gpio_devices[coloms[i]]->desc, 0);
        for (j = 0; j < ROWS; j++) {
            if (gpiod_get_value(KeypadDrvData->gpio_devices[rows[j]]->desc) == 0) {
                msleep(DEBOUNCE_DELAY);
                if (gpiod_get_value(KeypadDrvData->gpio_devices[rows[j]]->desc) == 0) {
                    KeypadDrvData->keypad_dev_data->state = 1;

                    KeypadDrvData->keypad_dev_data->current_char_input = keys[j][i];

                    /*wait for the key to be released*/

                    /*uncomment when hardware and pullup resistors are available*/

                    while (gpiod_get_value(KeypadDrvData->gpio_devices[rows[j]]->desc) == 0) {
                        pr_info("inside release loop\n");

                        msleep(DEBOUNCE_DELAY);
                    }

                    pr_info("read key %c\n", KeypadDrvData->keypad_dev_data->current_char_input);

                    gpiod_set_value(KeypadDrvData->gpio_devices[coloms[i]]->desc, 1);


                    return 0;
                }
            }
        }

        gpiod_set_value(KeypadDrvData->gpio_devices[coloms[i]]->desc, 1);
    }


    pr_info("finished reading keypad\n");


    return 1;
}
