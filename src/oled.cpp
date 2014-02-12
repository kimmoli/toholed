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
#include "pienifontti.h"
#include "icons.h"
#include "derp.h"

#define DBGPRINT

void drawDerp(char *screenBuffer)
{
    char* sb = screenBuffer;

    int i,d,off,s,n,o,x,h,t;

    h = 0;
    o = 0; // rivi mistä tulostus alkaa

    d = 0; // byte offset
    s = 0; // bit offset

    for (i=0 ; i<128 ; i++) // merkin leveys
    {
        for (n=0; n<derpHeightPixels ; n++) // merkin korkeus
        {
            off = ( n * derpWidthPages ) + d;
            t = derpBitmaps[off] & ( 0x80 >> ( (s + i) % 8) );
            if (t)
                (*(sb+((o+n)/8)+((h+i)*8))) = (*(sb+((o+n)/8)+((h+i)*8))) | ( 0x01 << ( (o+n) % 8 ) );
        }

        if ( ((s+i) % 8) == 7 )
            d++;

    }

}

void drawIcon(int location, int icon, char *screenBuffer)
{
    char* sb = screenBuffer;

    int i,d,off,s,n,o,x,h,t;

    h=location;
    o = 50; // rivi mistä tulostus alkaa

    for (x=0; x < 3 ; x++)
    {
        if ( icon == iconsMap[x] )
        {
            d = iconsStart[x] / 8; // byte offset
            s = iconsStart[x] - d*8; // bit offset

            for (i=0 ; i<iconsWidth[x] ; i++) // merkin leveys
            {
                for (n=0; n<iconsHeightPixels ; n++) // merkin korkeus
                {
                    off = ( n * iconsWidthPages ) + d;
                    t = iconsBitmaps[off] & ( 0x80 >> ( (s + i) % 8) );
                    if (t)
                        (*(sb+((o+n)/8)+((h+i)*8))) = (*(sb+((o+n)/8)+((h+i)*8))) | ( 0x01 << ( (o+n) % 8 ) );
                }

                if ( ((s+i) % 8) == 7 )
                    d++;

            }
        }
    }
}

void clearIcons(char *screenBuffer)
{
    char *sb = screenBuffer;
    int i;

    for (i=518 ; i < 1024 ; i=i+8 )
    {
        *(sb+i) = 0x00;
        *(sb+i+1) = 0x00;
    }

}



/* Draw Battery percentage level */
void drawBatteryLevel(const char *batLevel, char *screenBuffer)
{
    char* sb = screenBuffer;

    int i,d,off,s,n,o,x,h,t;
    unsigned m;

    h=0;
    o = 50; // rivi mistä tulostus alkaa

    for (m = 0; m<strlen(batLevel) ; m++)
    {
        if ( batLevel[m] == ' ') // space. just proceed cursor
        {
            h = h + 4;
        }
        else
        {
            for (x=0; x < 11 ; x++)
            {
                if ( batLevel[m] == pieniFonttiMap[x] )
                {
                    d = pieniFonttiStart[x] / 8; // byte offset
                    s = pieniFonttiStart[x] - d*8; // bit offset

                    for (i=0 ; i<pieniFonttiWidth[x] ; i++) // merkin leveys
                    {
                        for (n=0; n<pieniFonttiHeightPixels ; n++) // merkin korkeus
                        {
                            off = ( n * pieniFonttiWidthPages ) + d;
                            t = pieniFonttiBitmaps[off] & ( 0x80 >> ( (s + i) % 8) );
                            if (t)
                                (*(sb+((o+n)/8)+((h+i)*8))) = (*(sb+((o+n)/8)+((h+i)*8))) | ( 0x01 << ( (o+n) % 8 ) );
                        }

                        if ( ((s+i) % 8) == 7 )
                            d++;

                    }
                    h = h + pieniFonttiWidth[x] +0; // vaakakoordinaatti johon seuraava merkkitulee
                }
            }
        }
    }
}


/* Draws clock to screen buffer */
void drawTime(const char *tNow, char *screenBuffer)
{
    char* sb = screenBuffer;

    int i,d,off,s,n,x,h,t;
    unsigned m;

    h=0;

    for (m = 0; m<strlen(tNow) ; m++)
    {
        if ( tNow[m] == ' ') // space. just proceed cursor
        {
            h = h + 15;
        }
        else
        {
            for (x=0; x < 11 ; x++)
            {
                if ( tNow[m] == jollaFonttiMap[x] )
                {
                    d = jollaFonttiStart[x] / 8; // byte offset
                    s = jollaFonttiStart[x] - d*8; // bit offset

                    for (i=0 ; i<jollaFonttiWidth[x] ; i++) // merkin leveys
                    {
                        for (n=0; n<jollaFonttiHeightPixels ; n++) // merkin korkeus
                        {
                            off = ( n * jollaFonttiWidthPages ) + d;
                            t = jollaFonttiBitmaps[off] & ( 0x80 >> ( (s + i) % 8) );
                            if (t)
                                (*(sb+(n/8)+((h+i)*8))) = (*(sb+(n/8)+((h+i)*8))) | ( 0x01 << ( n % 8 ) );
                        }

                        if ( ((s+i) % 8) == 7 )
                            d++;

                    }
                    h = h + jollaFonttiWidth[x] +2; // vaakakoordinaatti johon seuraava merkkitulee
                }
            }
        }
    }
}


/* Clears screen buffer */
int clearOled(char *screenBuffer)
{
    //memset(screenBuffer, 0x00, SCREENBUFFERSIZE);
    char * sb = screenBuffer;
    int i;

    for (i=0 ; i<SCREENBUFFERSIZE ; i++)
    {
        *sb = 0;
        sb++;
    }

    return 0;
}

/* Draws screem buffer to OLED */
int updateOled(const char *screenBuffer)
{

    const char * sb = screenBuffer;
    int file, i;
    char buf[1025];

    buf[0] = 0x40;
    for (i=0 ; i < SCREENBUFFERSIZE ; i++)
    {
        buf[i+1] = (*sb);
        sb++;
    }


    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }

    if (write(file, buf, SCREENBUFFERSIZE+1) != SCREENBUFFERSIZE+1)
    {
        close(file);
        return -4;
    }

    close(file);

    return 0;
}

/* Contrast control */
int setContrastOled(unsigned int level)
{

    unsigned char contrast_seq[4] = { 0xd9, 0x7f, 0x81, 0x7f };
    unsigned char buf[2] = {0};
    int i, file;

    /* check that level is one of three valid ones */
    if ( (level != BRIGHTNESS_HIGH) && (level != BRIGHTNESS_LOW) && (level != BRIGHTNESS_MED))
        return -9;

    contrast_seq[1] = ((level >> 8 ) & 0xff);
    contrast_seq[3] = (level & 0xff);

    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return -2;
    }

    buf[0] = 0x80;

    // send init sequence
    for (i=0; i<4; i++)
    {
        buf[1] = contrast_seq[i];
        if (write(file, buf, 2) != 2)
        {
            close(file);
            return -3;
        }
    }

    close(file);

}

void blinkOled(int count)
{
    int i;

    for (i=0; i<count; i++)
    {
        invertOled(true);
        setContrastOled(BRIGHTNESS_HIGH);
        usleep(150000);
        invertOled(false);
        setContrastOled(BRIGHTNESS_LOW);
        usleep(150000);
    }
}

void invertOled(bool invert)
{
    unsigned char buf[2] = {0};
    int file;


    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return;
    }
    if (ioctl( file, I2C_SLAVE, 0x3c) < 0)
    {
        close(file);
        return;
    }

    buf[0] = 0x80;
    buf[1] = (invert ? 0xa7 : 0xa6);

    // send invert change sequence
    if (write(file, buf, 2) != 2)
    {
        close(file);
        return;
    }

    close(file);

}


/* Initializes OLED SSD1306 chip */
int initOled()
{
    unsigned char init_seq[28] = {0xae, /* display off */
                                  0x20,0x01, /* memory addressing mode = Vertical 01 Horizontal 00 */
                                  0xb0, /* page start address */
                                  0xc0, /* scan direction */
                                  0x00, /* lower column start address */
                                  0x10, /* higher column start address */
                                  0x40, /* display start line */
                                  0x81,0xcf, /* contrast (was af)*/
                                  0xa0, /* segment remap */
                                  0xa6, /* normal display  (a7 = inverse) */
                                  0xa8,0x3f, /* mux ratio = 64 */
                                  0xa4, /* display follows gdram */
                                  0xd3,0x00, /* display offset = 0*/
                                  0xd5,0xf0, /* oscillator */
                                  0xd9,0xf1, /* precharge period (was 22)*/
                                  0xda,0x12, /* configure COM pins */
                                  0xdb,0x40, /* set VCOM level (was 20)*/
                                  0x8d,0x14, /* enable charge pump. (0x10 disables) */
                                  0xaf}; /* display on*/
								  
    int i, file;
    unsigned char buf[2] = {0};

    usleep(150000); /* Wait for 150 ms */

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

/* Shutdown OLED - Must be done before disabling VDD */
int deinitOled()
{
    unsigned char init_seq[3] = {0xae, /* display off */
                                 0x8d,0x14}; /* disable charge pump. (0x14 enables) */


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
    for (i=0; i<3; i++)
    {
        buf[1] = init_seq[i];
        if (write(file, buf, 2) != 2)
        {
            close(file);
            return -3;
        }
    }

    close(file);

    usleep(100000);

    return 0;

}

