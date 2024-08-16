//
// Created by kareem on 12/08/24.
//

#ifndef KEYPAD_H
#define KEYPAD_H


#define ROWS 4
#define COLS 3

#define HIGH 1
#define LOW 0

#define R1_PIN 0
#define R2_PIN 1
#define R3_PIN 2
#define R4_PIN 3
#define C1_PIN 4
#define C2_PIN 5
#define C3_PIN 6

#define DEBOUNCE_DELAY 10


uint8_t keypad_init(struct keypad_drv_data *KeypadDrvData);

uint8_t keypad_read(struct keypad_drv_data *KeypadDrvData);


#endif //KEYPAD_H
