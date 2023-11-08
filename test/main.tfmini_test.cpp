#include <cstdlib>
#include <cstdint>
#include <queue>

// ************************************************************
// Test a TFmini-S fime-of-flight distance sensor
// ************************************************************

// default communication setup is 115200 baud serial
// sensor is connected to TX5/RX5 (pin 20/21)

// 1 - black, GND
// 2 - red, +5V
// 3 - white, RXD/SDA -> pin 20 (TX5)
// 4 - green, TXD/SCL -> pin 21 (RX5)

// that port is not connected on the controller base board,
// there we'd use the vision port (TX3/RX3)

#include "Arduino.h"

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h

extern "C" int main(void)
{

	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect.
	}

	Serial.println("\ntesting TFmini sensor ...");

	HardwareSerial *comm = &Serial5;
	std::queue<int> in_queue;
	
	comm->begin(115200);
	// send command requesting firmware version
	// comm->write(0x5a);
	// comm->write(0x04);
	// comm->write(0x01);
	// comm->write(0x5f);
	
	uint32_t last_print = systick_millis_count;
	int frame_count = 0;
	double dist = 0.0;
	int RSI = 0;
	
	while (true)
	{
		while (in_queue.size()>=9)
		{
			if (in_queue.front()==0x59)
			{
				in_queue.pop();
				if (in_queue.front()==0x59)
				{
					// found frame start
					frame_count++;
					in_queue.pop();
					int dist_L = in_queue.front();
					in_queue.pop();
					int dist_H = in_queue.front();
					in_queue.pop();
					dist = 2.56*dist_H + 0.01*dist_L;
					int strength_L = in_queue.front();
					in_queue.pop();
					int strength_H = in_queue.front();
					in_queue.pop();
					RSI = 256*strength_H+strength_L;
					// skip the temp
					in_queue.pop();
					in_queue.pop();
					// skip the checksum
					in_queue.pop();
				}
			}
			else
			{	
				// cannot be frame start - discard
				in_queue.pop();
			}
		}
		while (comm->available() > 0)
			in_queue.push(comm->read());
		
		uint32_t now = systick_millis_count;
		if (now-last_print>1000)
		{
			char prt[80];
			sprintf(prt, "%d frames/s  d=%5.3f m    RSI=%d", frame_count, dist, RSI);
			Serial.println(prt);
			frame_count=0;
			last_print=now;
		}
	};

}

