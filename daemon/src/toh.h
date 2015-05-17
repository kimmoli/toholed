#ifndef TOH_H
#define TOH_H

int controlVdd(int state);
int getTohInterrupt();
int releaseTohInterrupt(int fdGpio);
int getProximityInterrupt();
void releaseProximityInterrupt(int fdProx);
bool getProximityStatus();
unsigned int getEepromConfig(int number);

#define GPIO_INT "67"
#define GPIO_INT_EDGE "falling"

#endif // TOH_H
