/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * toholed daemon, TOH Low level control funtions
 *
 *
 *
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "toh.h"


int controlVdd(int state)
{
	
	int fd;
    int retval = 0;

	fd = open("/sys/devices/platform/reg-userspace-consumer.0/state", O_WRONLY);
	
    if (!(fd < 0))
	{
        retval += write (fd, state ? "1" : "0", 1);
		close(fd);
	}
	
	return fd;
}

int releaseTohInterrupt(int fdGpio)
{

    int fd;
    int retval = 0;

    close(fdGpio);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, GPIO_INT, strlen(GPIO_INT));
        close(fd);
    }
    return fd;

}


int getTohInterrupt()
{

    int fd;
    int retval = 0;

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, GPIO_INT, strlen(GPIO_INT));
        close(fd);
    }

    fd = open("/sys/class/gpio/gpio" GPIO_INT "/edge", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, GPIO_INT_EDGE, strlen(GPIO_INT_EDGE));
        close(fd);
    }
    else
        return -1; /* error */


    fd = open("/sys/class/gpio/gpio" GPIO_INT "/value", O_RDONLY | O_NONBLOCK);

    return fd;
}

int getProximityInterrupt()
{
    return open("/dev/input/event10", O_RDONLY | O_NONBLOCK);  // /sys/devices/virtual/input/input10/prx_raw_polling
}

void releaseProximityInterrupt(int fdProx)
{
    close(fdProx);
}

bool getProximityStatus()
{
    int fd;
    int retval = 0;
    char buf[2] = {0};

    fd = open("/sys/devices/virtual/input/input10/prx_detect", O_RDONLY);
    if (!(fd < 0))
        retval += read(fd, buf, 1);
    close(fd);

    return (buf[0]=='1');

}

/*
 *  Reads eeprom contents.
 *  Configuration values are stored as 16-bit big-endian
 *
 */

unsigned int getEepromConfig(int number)
{
    int fd;
    int retval = 0;
    char buf[64] = { 0xFF };

    fd = open("/sys/devices/platform/toh-core.0/config_data", O_RDONLY);
    if (!(fd < 0))
        retval += read(fd, buf, 64);
    close(fd);

    printf("EEPROM parameter %d value %d\n", number, ((buf[number*2]<<8) | (buf[(number*2)+1])));

    return ((buf[number*2]<<8) | (buf[(number*2)+1]));

}
