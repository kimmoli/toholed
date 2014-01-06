#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "vincent.h"
#include "oled.h"
#include "charger.h"


int oledPutChar(int file, unsigned char merkki)
{
    int n;
    unsigned char buf[9] = {0};
    buf[0] = 0x40; // data reg

    merkki = merkki & 0x7f; // only lower 128 chars supported

    for (n=0; n<8 ; n++)
    {
        buf[n+1] = vincent_data[merkki][n];
    }

    if (write(file, buf, 9) != 9)
    {
        return -4;
    }

    return 0;

}

int oledPuts(const char *s)
{
	int file;
    int i, len;
	
    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }

    len = strlen(s);

    for (i=0 ; i < len ; i++)
        oledPutChar(file, s[i]);
    if ( (len % 16) != 0)
        for (i=(len % 16) ; i < 16 ; i++)
            oledPutChar(file, ' ');

    close(file);

    return 0;
}

int clearOled()
{
	int file;
    unsigned char buf[1025] = {0};
	
    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }

    buf[0] = 0x40; // data reg
	
    if (write(file, buf, 1025) != 1025)
    {
        close(file);
        return -4;
    }
	
	close(file);

    return 0;
}

int initOled()
{
    unsigned char init_seq[28] = {0xae, /* display off */
                                  0x20,0x00, /* memory addressing mode = Horizontal */
                                  0xb0, /* page start address */
                                  0xc0, /* scan direction */
                                  0x00, /* lower column start address */
                                  0x10, /* higher column start address */
                                  0x40, /* display start line */
                                  0x81,0xaf, /* contrast */
                                  0xa0, /* segment remap */
                                  0xa6, /* normal display  (a7 = inverse) */
                                  0xa8,0x3f, /* mux ratio = 64 */
                                  0xa4, /* display follows gdram */
                                  0xd3,0x00, /* display offset = 0*/
                                  0xd5,0xf0, /* oscillator */
                                  0xd9,0x22, /* precharge period */
                                  0xda,0x12, /* configure COM pins */
                                  0xdb,0x20, /* set VCOM level */
                                  0x8d,0x10, /* disable charge pump. (0x14 enables) */
                                  0xaf}; /* display on*/
								  
    int i, file;
    unsigned char buf[2] = {0};

    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }

    buf[0] = 0x80; // control reg

    // send init sequence
    for (i=0; i<28; i++)
    {
        buf[1] = init_seq[i];
        if (write(file, buf, 2) != 2)
        {
            close(file);
            return -3;
        }
    }

	close(file);
	
	return 0;
}



void updateOledScreen(int dot)
{
    char ts[20];

    time_t t;
    struct tm *tnow;

    /* 1st row - date and time */

    t = time(NULL);
    tnow = localtime(&t);

    strftime(ts, sizeof(ts), "%d.%m.%Y %H:%M", tnow);
    oledPuts(ts);

    /* 2nd row */
    oledPuts(" ");

    /* 3rd row */
    sprintf(ts, "Battery: %d %%", chargerGetCapacity() );
    oledPuts(ts);

    /* 4th row */
    oledPuts(" ");

    /* 5th row */
    oledPuts(" ");

    /* 6th row */
    oledPuts(" ");

    /* 7th row */
    sprintf(ts, "%d mW", chargerGetPowerMW() );
    oledPuts(ts);

    /* 8th row */
    oledPuts(dot ? "               ." : "                ");

}
