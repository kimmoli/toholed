#ifndef OLED_H
#define OLED_H

#define OLEDWIDTH 128
#define OLEDHEIGHT 64
#define SCREENBUFFERSIZE ((OLEDWIDTH/8)*OLEDHEIGHT)

#define BRIGHTNESS_HIGH 0xcff1
#define BRIGHTNESS_MED 0x4040
#define BRIGHTNESS_LOW 0x1010


int initOled(); /* Initializes OLED SSD1306 chip */
int deinitOled();
int clearOled(char *screenBuffer); /* Clears screen buffer */
int updateOled(const char *screenBuffer); /* Draws screem buffer to OLED */
void drawTime(const char *tNow, char *screenBuffer); /* Draws clock to screen buffer */
void drawBatteryLevel(const char *batLevel, char *screenBuffer);
void drawIcon(int location, int icon, char *screenBuffer);
void clearIcons(char *screenBuffer);
int setContrastOled(unsigned int level); /* set contrast to BRIGHTNESS_HIGH _MED or _LOW */

#endif
