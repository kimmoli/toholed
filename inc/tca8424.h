
int tca8424_readInputReport(char* report);
int tca8424_initComms();
int tca8424_closeComms();
int tca8424_reset();
int tca8424_leds(char leds);
int tca8424_readMemory(int start, int len, char* data);
