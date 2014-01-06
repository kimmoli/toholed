#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "toh.h"


int control_vdd(int state) 
{
	
	int fd;

	fd = open("/sys/devices/platform/reg-userspace-consumer.0/state", O_WRONLY);
	
    if (!(fd < 0))
	{
		write (fd, state ? "1" : "0", 1);
		close(fd);
	}
	
	return fd;
}
