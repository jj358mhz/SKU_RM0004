/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/
#include <stdio.h>
#include "st7735.h"
#include "rpiInfo.h"
#include "time.h"
#include <unistd.h>



int main(void)
{
	uint8_t symbol = 0;

	load_display_config();     // LCD_TEMP_UNIT / LCD_IP_SWITCH / LCD_CUSTOM_TEXT env overrides

	if(lcd_begin())      //LCD Screen initialization
	{
		return 0;
	}
	sleep(1);
	while(1)
	{
		lcd_display(symbol);
		sleep(2);
		symbol++;
		if(symbol==4)
        {
          symbol=0;
        }
	}
	return 0;
}
