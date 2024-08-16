//
// Created by kareem on 3/13/24.
//

//#include "gpio.h"
#include "linux/gpio/consumer.h"
#include <linux/delay.h>
#include "lcd_platform_driver_data.h"
#include "lcd.h"



ssize_t LCD_init(struct lcd_drv_data *lcdDrvData) {



    msleep(100);
    /*set to 4 bit mode*/
    LCD_send_command(0x28,lcdDrvData);
    msleep(1);
    /*display on cursor off*/
    LCD_send_command(0x0f,lcdDrvData);
    msleep(1);
    /*clear display*/
    LCD_send_command(0x01,lcdDrvData);
    msleep(2);
    /*entry mode set*/
    LCD_send_command(0x06,lcdDrvData);

    return 0;

}



ssize_t LCD_send_command(unsigned char u8_command,struct lcd_drv_data * lcdDrvData ){


    gpiod_set_value( lcdDrvData->gpio_devices[RS_PIN]->desc,LOW);
    /*send the high nibble*/
    LCD_latch_data(u8_command >> 4,lcdDrvData);
    /*send the low nibble*/
    LCD_latch_data(u8_command & 0x0F ,lcdDrvData);

    return 0;

}



ssize_t LCD_display_char(const char data, struct lcd_drv_data *lcdDrvData) {
    gpiod_set_value( lcdDrvData->gpio_devices[RS_PIN]->desc,HIGH);

    /*send the high nibble*/
    LCD_latch_data(data >> 4, lcdDrvData);
    /*send the low nibble*/
    LCD_latch_data(data & 0x0F , lcdDrvData);

    return 0;
}

ssize_t LCD_display_string(const char *string, struct lcd_drv_data * lcdDrvData){

    int i = 0;
    while(string[i] != '\0'){
        LCD_display_char(string[i], lcdDrvData);
        i++;
    }

    return 0;
}




ssize_t LCD_latch_data(unsigned char u8_data , struct lcd_drv_data * lcdDrvData){

    gpiod_set_value( lcdDrvData->gpio_devices[RW_PIN]->desc,LOW);
    gpiod_set_value( lcdDrvData->gpio_devices[EN_PIN]->desc,LOW);

    gpiod_set_value( lcdDrvData->gpio_devices[D0_PIN]->desc, (u8_data >> 0) & 0x01);
    gpiod_set_value( lcdDrvData->gpio_devices[D1_PIN]->desc, (u8_data >> 1) & 0x01);
    gpiod_set_value( lcdDrvData->gpio_devices[D2_PIN]->desc, (u8_data >> 2) & 0x01);
    gpiod_set_value( lcdDrvData->gpio_devices[D3_PIN]->desc, (u8_data >> 3) & 0x01);

    gpiod_set_value( lcdDrvData->gpio_devices[EN_PIN]->desc,HIGH);
    msleep(10);
    gpiod_set_value( lcdDrvData->gpio_devices[EN_PIN]->desc,LOW);

    return 0;
}

/*remove this function later*/
ssize_t tester (struct lcd_drv_data *lcdDrvData){

    /*blink EN pin*/

    pr_info("debugging here LCD t 3");
    gpiod_set_value( lcdDrvData->gpio_devices[EN_PIN]->desc,HIGH);
    pr_info("debugging here LCD t 2");
    msleep(3000);
    gpiod_set_value( lcdDrvData->gpio_devices[EN_PIN]->desc,LOW);
    pr_info("debugging here LCD t 4");

    /*blink RS pin*/
    gpiod_set_value( lcdDrvData->gpio_devices[RS_PIN]->desc,HIGH);
    msleep(3000);
    gpiod_set_value( lcdDrvData->gpio_devices[RS_PIN]->desc,LOW);


    return 0;
}


