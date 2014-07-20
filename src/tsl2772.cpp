#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "tsl2772.h"
#include "toh.h"

/*
 *  ALS Value relation to brightness
 *
 *  0 < ALS C0 < 500  = Brightness low
 *  501 < ALS C0 < 10000 = Brightness Med
 *  10001 < ALS C0 < MAX = Brightness high
 *
 *
 */

unsigned int prox_limit;
bool proximityEnabled;
bool alsEnabled;

int tsl2772_setAlsThresholds(int file, unsigned int high, unsigned int low)
{
    char buf[5] = {
        0xa4, /* Command Register, Auto increment protocol, address 4 */
        (low & 0xff),         /* Reg04 AILTL  - ALS Interrupt threshold */
        ((low >> 8) & 0xff),  /* Reg05 AILTH  - */
        (high & 0xff),        /* Reg06 AIHTL */
        ((high >> 8) & 0xff)  /* Reg07 AIHTH */
    };

    if (write(file, buf, 5) != 5)
       {
           close(file);
           return -3;
       }

    return 7;

}

int tsl2772_setProxThresholds(int file, unsigned int high, unsigned int low)
{
    char buf[5] = {
        0xa8, /* Command Register, Auto increment protocol, address 8 */
        (low & 0xff),         /* Reg08 PILTL  - Proximity Interrupt threshold */
        ((low >> 8) & 0xff),  /* Reg09 PILTH  - */
        (high & 0xff),        /* Reg0A PIHTL */
        ((high >> 8) & 0xff)  /* Reg0B PIHTH */
    };

    if (write(file, buf, 5) != 5)
       {
           close(file);
           return -3;
       }

    return 7;

}



int tsl2772_initialize(int file)
{
    prox_limit = getEepromConfig(0);

    if ((prox_limit == 0) || (prox_limit > 1024)) /* Use default value if out of limits */
        prox_limit = PROX_LIMIT;

    char buf[17] = {
        0xa0, /* Command Register, Auto increment protocol, address 0 */
        0x00, /* Reg00 ENABLE - Disable all */
        0xdb, /* Reg01 ATIME  - 100ms */
        0xff, /* Reg02 PTIME  - 2.73ms */
        0xff, /* Reg03 WTIME  - 2.73ms */
        (ALSLIM_BRIGHTNESS_LOW & 0xff),         /* Reg04 AILTL  - ALS Interrupt threshold N/A */
        ((ALSLIM_BRIGHTNESS_LOW >> 8) & 0xff),  /* Reg05 AILTH  - */
        (ALSLIM_BRIGHTNESS_HIGH & 0xff),        /* Reg06 AIHTL */
        ((ALSLIM_BRIGHTNESS_HIGH >> 8) & 0xff), /* Reg07 AIHTH */
        0x00,                                   /* Reg08 PILTL  - Proximity interrupt threshod*/
        0x00,                                   /* Reg09 PILTH  - Low */
        (prox_limit & 0xff),                    /* Reg0a PIHTL */
        ((prox_limit >> 8) & 0xff),             /* Reg0b PIHTH */
        0x11, /* Reg0c PERS   - APERS = 1, PPERS = 1 */
        0x00, /* Reg0d CONFIG - AGL = 0, WLONG = 0, PLD = 0 */
        0x08, /* Reg0e PPULSE - 8 pulses during prox accum */
        0x92};/* Reg0f CONTROL- PDRIVE = 30mA, PDIODE = CH0, PGAIN = 0, AGAIN = x16 */

    if (write(file, buf, 17) != 17)
       {
           close(file);
           return -3;
       }

    /* enable clocks, Prox Int, ALS Int, power on */
    buf[0] = 0xa0;
    buf[1] = 0x0f | (proximityEnabled ? 0x20 : 0) | (alsEnabled ? 0x10 : 0);
/*
 *       7  Reserved
 *  SAI  6  Sleep after interrupt.
 *          When asserted, the device will power down at the end of a proximity or ALS cycle
 *          if an interrupt has been generated.
 *  PIEN 5  Proximity interrupt mask. When asserted, permits proximity interrupts to be generated.
 *  AIEN 4  ALS interrupt mask. When asserted, permits ALS interrupts to be generated.
 *  WEN  3  Wait Enable. This bit activates the wait feature.
 *          Writing a 1 activates the wait timer. Writing a 0 disables the wait timer.
 *  PEN  2  Proximity enable. This bit activates the proximity function. Writing a 1 enables proximity.
 *          Writing a 0 disables proximity.
 *  AEN  1  ALS Enable. This bit actives the two channel ADC. Writing a 1 activates the ALS.
 *          Writing a 0 disables the ALS.
 *  PON  0  Power ON. This bit activates the internal oscillator to permit the timers and
 *          ADC channels to operate.
 *          Writing a 1 activates the oscillator. Writing a 0 disables the oscillator.
 */

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

void tsl2772_disableInterrupts(int file)
{
    char buf[2] = {0xa0, 0x0f};
    int retval = 0;

    retval += write(file, buf, 2);
}

void tsl2772_enableInterrupts(int file)
{
    char buf[2] = {0xa0, 0x3f};
    int retval = 0;

    buf[1] = 0x0f | (proximityEnabled ? 0x20 : 0) | (alsEnabled ? 0x10 : 0);

    retval += write(file, buf, 2);
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

unsigned long tsl2772_getReg(int file, unsigned char reg)
{
    unsigned char buf[1] = {0};

    buf[0] = 0xa0 + (reg & 0x1f);

    if (write(file, buf, 1) != 1)
    {
       close(file);
       return 0xFFFF;
    }

    if (read( file, buf, 1 ) != 1)
    {
        close(file);
        return 0xFFFF;
    }

    return buf[0];
}

