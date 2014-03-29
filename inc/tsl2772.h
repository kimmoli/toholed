#ifndef TSL2772_H
#define TSL2772_H

#define ALSLIM_BRIGHTNESS_LOW  (500)
#define ALSLIM_BRIGHTNESS_HIGH (10000)
#define PROX_LIMIT (650)

int tsl2772_initialize(int file);
int tsl2772_initComms(unsigned char addr);
int tsl2772_closeComms(int file);
int tsl2772_clearInterrupt(int file);
void tsl2772_disableInterrupts(int file);
void tsl2772_enableInterrupts(int file);
unsigned long tsl2772_getADC(int file, int ch);
int tsl2772_setAlsThresholds(int file, unsigned int high, unsigned int low);
int tsl2772_setProxThresholds(int file, unsigned int high, unsigned int low);


#endif // TSL2772_H
