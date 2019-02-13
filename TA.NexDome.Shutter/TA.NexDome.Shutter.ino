#include "NexDome.h"
#include <XBee.h>



auto& xbeeSerial = Serial1;
auto& host = Serial;
const char* xbeeAttentionString = "+++";
const char* xbeeInitString = "ATCE0,ID7734,CH0C,MY1,DH0,DL0,AP0,SM0,WR,BD7,CN";

// the setup function runs once when you press reset or power the board
void setup()
	{
	host.begin(115200);
	xbeeSerial.begin(9600);
	delay(5000);
	xbeeSerial.write(xbeeAttentionString);
	delay(1000);
	xbeeSerial.write(xbeeInitString);
	delay(1000);
	xbeeSerial.begin(115200);
}

// the loop function runs over and over again until power down or reset
void loop() { }
