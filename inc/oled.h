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
void drawSMS(bool visible, int location, char *screenBuffer); /* Draws SMS symbol */


#endif
