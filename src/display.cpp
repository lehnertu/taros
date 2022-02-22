#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

#include "global.h"
#include "display.h"

#define sclk 13
#define mosi 11
#define cs   10
#define rst  6
#define dc   9

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

void display_test()
{

    Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

    display.begin();

    display.fillScreen(BLACK);
    delay(500);

    // lcdTestPattern();
    uint8_t w,h;
    display.setAddrWindow(0, 0, 96, 64);

    for (h = 0; h < 64; h++) {
    for (w = 0; w < 96; w++) {
      if (w > 83) {
        display.writePixel(w, h, WHITE);
      } else if (w > 71) {
        display.writePixel(w, h, BLUE);
      } else if (w > 59) {
        display.writePixel(w, h, GREEN);
      } else if (w > 47) {
        display.writePixel(w, h, CYAN);
      } else if (w > 35) {
        display.writePixel(w, h, RED);
      } else if (w > 23) {
        display.writePixel(w, h, MAGENTA);
      } else if (w > 11) {
        display.writePixel(w, h, YELLOW);
      } else {
        display.writePixel(w, h, BLACK);
      }
    }
    }
    display.endWrite();

    delay(2000);

    display.fillScreen(BLACK);
    display.setCursor(0,0);
    display.print("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa");
    delay(2000);

    // tft print function!
    display.fillScreen(BLACK);
    display.setCursor(0, 5);
    display.setTextColor(RED);
    display.setTextSize(1);
    display.println("Hello World!");
    display.setTextColor(YELLOW, GREEN);
    display.setTextSize(2);
    display.print("Hello Wo");
    display.setTextColor(BLUE);
    display.setTextSize(3);
    display.print(1234.567);
    delay(2000);
    
    display.fillScreen(BLACK);
    display.setCursor(0, 5);
    display.setTextColor(WHITE);
    display.setTextSize(0);
    display.println("Hello World!");
    display.setTextSize(1);
    display.setTextColor(GREEN);
    display.print(3.14159, 5);
    display.println(" Want pi?");
    display.print(8675309, HEX); // print 8,675,309 out in HEX!
    display.print(" Print HEX");
    display.setTextColor(WHITE);
    display.println("Sketch has been");
    display.println("running for: ");
    display.setTextColor(MAGENTA);
    display.print(millis() / 1000);
    display.setTextColor(WHITE);
    display.print(" seconds.");
    delay(2000);

}
