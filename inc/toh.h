
int controlVdd(int state);
int getTohInterrupt();
int releaseTohInterrupt(int fdGpio);

#define GPIO_INT "67"
#define GPIO_INT_EDGE "falling"
#define POLL_TIMEOUT 2000
