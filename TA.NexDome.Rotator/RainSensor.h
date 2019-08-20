// RainSensor.h

#ifndef _RAINSENSOR_h
#define _RAINSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Timer.h"

class RainSensor
	{
public:
	explicit RainSensor(uint8_t pin) : pin(pin), closing(false), repeatDuration(Timer::Seconds(30)) { }
	void init(Duration repeat);
	void closeShutter();
	void loop();
private:
	uint8_t pin;
	bool closing;
	Timer closeTimer;
	Duration repeatDuration;
	};

#endif

