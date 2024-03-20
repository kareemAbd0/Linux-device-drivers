//
// Created by kareem on 3/13/24.
//

#ifndef PLATFORM_MPCHAR_LCD_H
#define PLATFORM_MPCHAR_LCD_H

// 4 bit mode


#define RS_PIN 0
#define RW_PIN 1
#define EN_PIN 2
#define D0_PIN 3
#define D1_PIN 4
#define D2_PIN 5
#define D3_PIN 6


ssize_t LCD_init(struct lcd_drv_data *);
ssize_t LCD_send_command(unsigned char u8_command,struct lcd_drv_data *);
ssize_t LCD_display_char(unsigned u8_data,struct lcd_drv_data *);


#endif //PLATFORM_MPCHAR_LCD_H
