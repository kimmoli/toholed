#ifndef TSL2772_H
#define TSL2772_H

#define ALSLIM_BRIGHTNESS_LOW  (500)
#define ALSLIM_BRIGHTNESS_HIGH (10000)

int tsl2772_initialize(int file);
int tsl2772_initComms(unsigned char addr);
int tsl2772_closeComms(int file);
int tsl2772_clearInterrupt(int file);
unsigned long tsl2772_getADC(int file, int ch);
int tsl2772_setAlsThresholds(int file, unsigned int high, unsigned int low);



#endif // TSL2772_H
