#include "rpiInfo.h"
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include "st7735.h"
#include <stdlib.h>
#include <strings.h>
#include <sys/statvfs.h>

display_config_t g_display_config = {
    .temperature_type = CELSIUS,
    .ip_switch = IP_DISPLAY_OPEN,
    .custom_display = "UCTRONICS",
};

void load_display_config(void)
{
    const char *temp_unit = getenv("LCD_TEMP_UNIT");
    if (temp_unit != NULL && (temp_unit[0] == 'F' || temp_unit[0] == 'f'))
    {
        g_display_config.temperature_type = FAHRENHEIT;
    }

    const char *ip_switch = getenv("LCD_IP_SWITCH");
    if (ip_switch != NULL && strcasecmp(ip_switch, "close") == 0)
    {
        g_display_config.ip_switch = IP_DISPLAY_CLOSE;
    }

    const char *custom_text = getenv("LCD_CUSTOM_TEXT");
    if (custom_text != NULL)
    {
        strncpy(g_display_config.custom_display, custom_text, sizeof(g_display_config.custom_display) - 1);
        g_display_config.custom_display[sizeof(g_display_config.custom_display) - 1] = '\0';
    }
}

/*
* Get the IP address of eth0, falling back to wlan0
*/

char* get_ip_address_new(void)
{
    int fd;
    struct ifreq ifr;
    int symbol=0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    symbol=ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    if(symbol==0)
    {
      return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    }
    else
    {
      fd = socket(AF_INET, SOCK_DGRAM, 0);
      /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;
      /* I want IP address attached to "wlan0" */
      strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
      symbol=ioctl(fd, SIOCGIFADDR, &ifr);
      close(fd);    
      if(symbol==0)
      {
        return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);   
      }
      else
      {
        char* buffer="xxx.xxx.xxx.xxx";
        return buffer;
      }
    }
}



/*
* get ram memory
*
* *freeram is populated from /proc/meminfo's MemAvailable, not MemFree: MemFree excludes
* the kernel's disk cache/buffers, which Linux uses opportunistically for any unused RAM and
* reclaims instantly under pressure. Using MemFree overstates "used" memory by however much
* is sitting in cache (matches what `free -h`'s "available" column reports).
*/
void get_cpu_memory(float *Totalram,float *freeram)
{
  struct sysinfo s_info;

  unsigned int value=0;
  unsigned char buffer[100]={0};
  unsigned char famer[100]={0};
    if(sysinfo(&s_info)==0)            //Get memory information
    {
        FILE* fp=fopen("/proc/meminfo","r");
        if(fp==NULL)
        {
            return ;
        }
        while(fgets(buffer,sizeof(buffer),fp))
        {
            if(sscanf(buffer,"%s%u",famer,&value)!=2)
            {
            continue;
            }
            if(strcmp(famer,"MemTotal:")==0)
            {
             *Totalram=value/1024.0/1024.0;
            }
            else if(strcmp(famer,"MemAvailable:")==0)
            {
              *freeram=value/1024.0/1024.0;
            }
        }
        fclose(fp);
    }
}

/*
* get sd memory
*/
void get_sd_memory(uint32_t *MemSize, uint32_t *freesize)
{
    struct statfs diskInfo;
    if (statfs("/", &diskInfo) != 0) {
        *MemSize = 0;
        *freesize = 0;
        return;
    }
    unsigned long long blocksize = diskInfo.f_bsize;// The number of bytes per block
    unsigned long long totalsize = blocksize*diskInfo.f_blocks;//Total number of bytes	
    *MemSize=(unsigned int)(totalsize>>30);


    unsigned long long size = blocksize*diskInfo.f_bfree; //Now let's figure out how much space we have left
    *freesize=size>>30;
    *freesize=*MemSize-*freesize;
}


/*
* get hard disk memory
*/
uint8_t get_hard_disk_memory(uint32_t *diskMemSize, uint32_t *useMemSize)
{
    struct statvfs fs;
    if (statvfs("/", &fs) != 0) {
        *diskMemSize = 0;
        *useMemSize = 0;
        return 1; // Error
    }

    unsigned long long total = (unsigned long long) fs.f_blocks * fs.f_frsize;
    unsigned long long free  = (unsigned long long) fs.f_bfree  * fs.f_frsize;
    unsigned long long used  = total - free;

    *diskMemSize = (uint32_t)(total >> 20);  // MB
    *useMemSize  = (uint32_t)(used  >> 20);  // MB

    return 0;
}

/*
* get temperature
*/

uint8_t get_temperature(void)
{
    FILE *fd;
    unsigned int temp = 0;
    char buff[10] = {0};
    fd = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    if (fd == NULL)
    {
        return 0;
    }
    fgets(buff,sizeof(buff),fd);
    sscanf(buff, "%d", &temp);
    fclose(fd);
    return g_display_config.temperature_type == FAHRENHEIT ? temp/1000*1.8+32 : temp/1000;
}

/*
* Aggregate CPU jiffie counters from the first line of /proc/stat.
*/
typedef struct {
    unsigned long long user, nice, sys, idle, iowait, irq, softirq, steal;
} cpu_stat_t;

static int read_cpu_stat(cpu_stat_t *out)
{
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        return 0;
    }
    char label[8] = {0};
    int fields = fscanf(fp, "%7s %llu %llu %llu %llu %llu %llu %llu %llu",
                         label,
                         &out->user, &out->nice, &out->sys, &out->idle,
                         &out->iowait, &out->irq, &out->softirq, &out->steal);
    fclose(fp);
    return fields == 9;
}

/*
* Get cpu usage
*
* Reads /proc/stat directly instead of shelling out to `top`/`awk`: cheaper (no process
* spawn per read), and avoids relying on parsing another program's text output.  Two samples
* 200ms apart are needed since /proc/stat reports cumulative jiffies since boot, not an
* instantaneous load.
*/
uint8_t get_cpu_message(void)
{
    cpu_stat_t s1, s2;

    if (!read_cpu_stat(&s1))
    {
        return 0;
    }

    usleep(200000);

    if (!read_cpu_stat(&s2))
    {
        return 0;
    }

    unsigned long long idle1 = s1.idle + s1.iowait;
    unsigned long long idle2 = s2.idle + s2.iowait;
    unsigned long long total1 = s1.user + s1.nice + s1.sys + s1.idle + s1.iowait + s1.irq + s1.softirq + s1.steal;
    unsigned long long total2 = s2.user + s2.nice + s2.sys + s2.idle + s2.iowait + s2.irq + s2.softirq + s2.steal;

    if (total2 <= total1)
    {
        return 0;
    }

    unsigned long long totalDelta = total2 - total1;
    unsigned long long idleDelta = idle2 - idle1;
    unsigned long long busyDelta = totalDelta - idleDelta;

    return (uint8_t)(busyDelta * 100 / totalDelta);
}
