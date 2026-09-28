#ifndef  __RPIINFO_H
#define  __RPIINFO_H

#include <stdint.h>

#define CELSIUS       0
#define FAHRENHEIT    1

#define IP_DISPLAY_OPEN     0
#define IP_DISPLAY_CLOSE    1

/*
* Runtime-configurable display preferences, loaded once at startup by load_display_config()
* (called from project/display.c's main() before the display loop starts). Overriding these
* no longer requires a rebuild -- set the environment variable and restart lcd_display.service:
*   LCD_TEMP_UNIT=F|C          (default: C)
*   LCD_IP_SWITCH=open|close   (default: open)
*   LCD_CUSTOM_TEXT=<text>     (default: "UCTRONICS", shown on the IP line when LCD_IP_SWITCH=close)
*/
typedef struct {
    int temperature_type;   // CELSIUS or FAHRENHEIT
    int ip_switch;          // IP_DISPLAY_OPEN or IP_DISPLAY_CLOSE
    char custom_display[32];
} display_config_t;

extern display_config_t g_display_config;

void load_display_config(void);
char* get_ip_address_new(void);
void get_sd_memory(uint32_t *MemSize, uint32_t *freesize);
void get_cpu_memory(float *Totalram, float *freeram);
uint8_t get_temperature(void);
uint8_t get_cpu_message(void);
uint8_t get_hard_disk_memory(uint32_t *diskMemSize, uint32_t *useMemSize);

#endif /*__RPIINFO_H*/