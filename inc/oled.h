#ifndef OLED_H
#define OLED_H

#define OLEDWIDTH 128
#define OLEDHEIGHT 64
#define SCREENBUFFERSIZE ((OLEDWIDTH/8)*OLEDHEIGHT)


int initOled(); /* Initializes OLED SSD1306 chip */
int deinitOled();
int clearOled(char *screenBuffer); /* Clears screen buffer */
int updateOled(const char *screenBuffer); /* Draws screem buffer to OLED */
void drawTime(const char *tNow, char *screenBuffer); /* Draws clock to screen buffer */
void drawBatteryLevel(const char *batLevel, char *screenBuffer);
void drawIcon(int location, int icon, char *screenBuffer);
void clearIcons(char *screenBuffer);

#endif
