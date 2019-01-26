#include "NexDome.h"
#include <XBee.h>



auto& xbee = Serial1;
auto& host = Serial;

// the setup function runs once when you press reset or power the board
void setup()
	{
	Serial.begin(115200);
	xbee.begin(9600);
	}

// the loop function runs over and over again until power down or reset
void loop() { }
