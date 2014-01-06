#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "toh.h"


int control_vdd(int state) 
{
	
	int fd;

	fd = open("/sys/devices/platform/reg-userspace-consumer.0/state", O_WRONLY);
	//fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8xxx-led/leds/led:rgb_blue/brightness", O_WRONLY);
	
	if (fd != -1)
	{
		write (fd, state ? "1" : "0", 1);
		close(fd);
	}
	
	return fd;
}
