#ifndef OLED_H
#define OLED_H

#define OLEDWIDTH 128
#define OLEDHEIGHT 64
#define SCREENBUFFERSIZE ((OLEDWIDTH/8)*OLEDHEIGHT)

#define BRIGHTNESS_HIGH 0xcff1
#define BRIGHTNESS_MED 0x4040
#define BRIGHTNESS_LOW 0x1010



int initOled(unsigned int level); /* Initializes OLED SSD1306 chip, brightness level arg */
int deinitOled();
int clearOled(char *screenBuffer); /* Clears screen buffer */
int updateOled(const char *screenBuffer); /* Draws screem buffer to OLED */
void drawTime(const char *tNow, char *screenBuffer); /* Draws clock to screen buffer */
void drawBatteryLevel(const char *batLevel, char *screenBuffer);
void drawIcon(int icon, char *screenBuffer);
void clearIcon(int icon, char *screenBuffer);
void clearIcons(char *screenBuffer);
int setContrastOled(unsigned int level); /* set contrast to BRIGHTNESS_HIGH _MED or _LOW */
void blinkOled(int count); /* Blinks screen for 'count' times */
void invertOled(bool invert); /* Select between invert and normal image */
void drawDerp(char *screenBuffer); /* Draw derp image to screen */
void drawPixel(int x, int y, int color, char *screenBuffer); /* Draw single pixel to screen */
void drawCircle(int x0, int y0, int r,  int color, char *screenBuffer); /* Draw circle to screen r=radius */
void drawBitmap(int x, int y, int height, int width, int offset, int rowsize, const char *bitmap, char *screenBuffer);


#endif
