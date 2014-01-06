#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "frontled.h"


int control_frontLed(int r, int g, int b) 
{
	
	int fd;
	char vstr[4];
	
	sprintf(vstr, "%d", r);

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8xxx-led/leds/led:rgb_red/brightness", O_WRONLY);
	
    if (!(fd < 0))
	{
		write (fd, vstr, strlen(vstr));
		close(fd);
	}

	sprintf(vstr, "%d", g);

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8xxx-led/leds/led:rgb_green/brightness", O_WRONLY);
	
    if (!(fd < 0))
	{
		write (fd, vstr, strlen(vstr));
		close(fd);
	}

	sprintf(vstr, "%d", b);

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8xxx-led/leds/led:rgb_blue/brightness", O_WRONLY);
	
    if (!(fd < 0))
	{
		write (fd, vstr, strlen(vstr));
		close(fd);
	}
	
	
	return fd;
}
