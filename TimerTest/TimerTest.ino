/*
 Name:		TimerTest.ino
 Created:	3/10/2019 6:01:18 PM
 Author:	Tim
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Timer.h>

Timer stopwatch = Timer();

// the setup function runs once when you press reset or power the board
void setup() {
	stopwatch.SetDuration(1000);
}

// the loop function runs over and over again until power down or reset
void loop() {
	auto now = millis();
	if (stopwatch.Expired())
	{
		Serial.println(now);
		stopwatch.Repeat();
	}
}
