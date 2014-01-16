#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "tca8424.h"

int file = 0;

int tca8424_reset(int file)
{
    char buf[4] = {0x00, 0x06, 0x00, 0x01};

    if (write(file, buf, 4) != 4)
       {
           close(file);
           return -3;
       }
    return 3;
}

int tca8424_leds(int file, char leds)
{
    char buf[9] = {0x00, 0x06, 0x20, 0x03, 0x00, 0x07, 0x01, 0x00, 0x00};

    buf[8] = leds;

    if (write(file, buf, 9) != 9)
       {
           close(file);
           return -4;
       }
    return file;
}


int tca8424_initComms(unsigned char addr)
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

int tca8424_closeComms(int file)
{
    close(file);
    return file;
}

int tca8424_readInputReport(int file, char* report)
{
    char buf[13] = { 0x00, 0x06, 0x11, 0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int i;

    if (write(file, buf, 6) != 6)
    {
       close(file);
       return -5;
    }

    if (read( file, buf, 11 ) != 11)
    {
        close(file);
        return -6;
    }

    for (i=0 ; i< 11 ; i++)
        report[i] = buf[i];

    return file;

}

int tca8424_readMemory(int file, int start, int len, char* data)
{
    char buf[300] = {0};
    int i;

    buf[0] = start & 0xff;
    buf[1] = (start>>8) & 0xff;

    if (write(file, buf, 2) != 2)
    {
       close(file);
       return -11;
    }

    if (read( file, buf, len ) != len)
    {
        close(file);
        return -12;
    }

    for (i=0 ; i< len ; i++)
        data[i] = buf[i];

    return file;
}

