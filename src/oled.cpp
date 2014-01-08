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
#include "oled.h"
#include "charger.h"
#include "jollafontti.h"


char screenBuffer[SCREENBUFFERSIZE] = {0};

/* Draws clock to screen buffer */
void drawTime(const char *tNow)
{
    int i,d,off,s,n,b,x,r,c,h;
    unsigned m;

    h=0;

    for (m = 0; m<strlen(tNow) ; m++)
    {
        for (x=0; x < 11 ; x++)
            if ( jollaFonttiMap[x] == tNow[m] )
            {
                for (i=0 ; i<jollaFonttiHeightPixels ; i++) // row
                {
                    d = (jollaFonttiStart[x]/8); // byte offset
                    off = (i*jollaFonttiWidthPages) + d;
                    s = jollaFonttiStart[x] - d*8; // bit offset

                    c = (h)/8; // byte offset
                    r = (h) - c*8; // bit offset

                    for (n = s; n < (s+jollaFonttiWidth[x]) ; n++)
                    {
                        b = jollaFonttiBitmaps[off];

                        if ((b<<(n%8)) & 0x80)
                            screenBuffer[ (i*(OLEDWIDTH/8)) + c ] = screenBuffer[ (i*(OLEDWIDTH/8)) + c ] | (0x80 >> (r%8));

                        if ( r % 8 == 7 )
                            c++;
                        r++;

                        if ( n % 8 == 7 )
                            off++;
                    }
                }
                h = h + jollaFonttiWidth[x] +2;
            }
    }
}


/* Clears screen buffer */
int clearOled()
{
    int i;
    for (i=0; i<SCREENBUFFERSIZE ; i++)
        screenBuffer[i] = 0;
	
    return 0;
}

/* Draws screem buffer to OLED */
int updateOled()
{
    int file;
    char buf[1] = { 0x40 };

    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }


    if (write(file, buf, 1) != 1)
    {
        close(file);
        return -4;
    }

    if (write(file, screenBuffer, SCREENBUFFERSIZE) != SCREENBUFFERSIZE)
    {
        close(file);
        return -4;
    }

    close(file);

    return 0;
}

/* Initializes OLED SSD1306 chip */
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


