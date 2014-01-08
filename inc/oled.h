#ifndef OLED_H
#define OLED_H

#define OLEDWIDTH 128
#define OLEDHEIGHT 64
#define SCREENBUFFERSIZE ((OLEDWIDTH/8)*OLEDHEIGHT)

extern char screenBuffer[];

int initOled(); /* Initializes OLED SSD1306 chip */
int clearOled(); /* Clears screen buffer */
int updateOled(); /* Draws screem buffer to OLED */
void drawTime(const char *tNow); /* Draws clock to screen buffer */


#endif
