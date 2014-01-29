#ifndef TSL2772_H
#define TSL2772_H

int tsl2772_initialize(int file);
int tsl2772_initComms(unsigned char addr);
int tsl2772_closeComms(int file);
int tsl2772_clearInterrupt(int file);
unsigned long tsl2772_getADC(int file, int ch);

#endif // TSL2772_H
