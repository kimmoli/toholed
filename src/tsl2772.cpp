#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "tsl2772.h"


int tsl2772_initialize(int file)
{
    char buf[17] = {0xa0, 0x00, 0xdb, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x20, 0x03, 0x11, 0x00, 0x08, 0xa2};

    if (write(file, buf, 17) != 17)
       {
           close(file);
           return -3;
       }

    /* enable clocks, prox interrupt, power on*/
    buf[0] = 0xa0;
    buf[1] = 0x2f;

    if (write(file, buf, 2) != 2)
       {
           close(file);
           return -3;
       }
    return 3;
}



int tsl2772_initComms(unsigned char addr)
{
    int file;

    /* open file and start ioctl */
    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, addr) < 0)
    {
        close(file);
        return -2;
    }
    return file;
}

int tsl2772_closeComms(int file)
{
    close(file);
    return file;
}

int tsl2772_clearInterrupt(int file)
{
    char buf[1] = {0xe7};

    if (write(file, buf, 1) != 1)
       {
           close(file);
           return -4;
       }
    return 4;
}


unsigned long tsl2772_getADC(int file, int ch) /*CH0, CH1, CH2 for Proximity*/
{
    char buf[5] = {0};
    unsigned int i;

    buf[0] = 0xb4 + (ch*2); // ALS C0, ALS C1, Proximity

    if (write(file, buf, 1) != 1)
    {
       close(file);
       return 0x10000;
    }

    if (read( file, buf, 2 ) != 2)
    {
        close(file);
        return 0x10000;
    }

    i = buf[0] | (0xff00 & (buf[1]<<8));

    return i;
}


