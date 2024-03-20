//
// Created by kareem on 3/13/24.
//

#include "err.h"
#include "gpio.h"
#include "lcd_platform_driver.h"
#include "lcd.h"



ssize_t LCD_init(struct lcd_drv_data *lcdDrvData) {

    gpiod_set_value( lcdDrvData->devices[0],1);





}
