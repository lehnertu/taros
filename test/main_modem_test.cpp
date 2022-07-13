#include <cstdlib>
#include <cstdint>
#include <list>

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h

#include "global.h"
#include "display.h"
#include "module.h"
#include "message.h"
#include "system.h"

#include "HardwareSerial.h"

// this is the RTS pin for the modem, used for M0 and M1 wired in parallel
#define MODEM_M0_M1 22

extern "C" int main(void)
{
    // output to USB serial
    Serial.begin(9600);
    Serial.println("Modem test : ");
    // output to modem
    // void begin(uint32_t baud, uint16_t format=0);
    // default SERIAL_8N1  == 0x00
    Serial1.begin(9600);
    // clear the receive buffer
    Serial1.clear();
    
    // configure modem
    // mode = 0x64 : 9600,8N1 air rate 9.6k
    // freq = 0x12 : ch18 = 868.125 MHz
    // command : 0xC0 0x00 0x05 0x00 0x00 0x64 0x00 0x12
    // pull M0/M1 high (sleep/config mode)
    pinMode(MODEM_M0_M1, OUTPUT);
    digitalWriteFast(MODEM_M0_M1, HIGH);
    delay(10);
    Serial1.write(0xc0);
    Serial1.write(0x00);
    Serial1.write(0x05);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x64);
    Serial1.write(0x00);
    Serial1.write(0x12);
    
    delay(500);
    
    // read the resonse
    // output to USB serial
    while (Serial1.available() > 0) {
        int incomingByte = Serial1.read();
        Serial.print("UART received: ");
        Serial.println(incomingByte, HEX);
    }    

    delay(500);
    
    // pull M0/M1 low (transmit mode)
    digitalWriteFast(MODEM_M0_M1, LOW);
    // clear the receive buffer
    Serial1.clear();

    // inifnite loop
    for (int i=234; i>0; i++)
    {
        std::string buffer("ping.\r\n");
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        Serial1.write(buffer.c_str(), buffer.size());

        // read the resonse
        // output to USB serial
        while (Serial1.available() > 0) {
            int incomingByte = Serial1.read();
            Serial.print("UART received: ");
            Serial.println(incomingByte, HEX);
        }    
        
        delay(1000);
    };
}

