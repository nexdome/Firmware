// HomeSensor.h

#ifndef _HOMESENSOR_h
#define _HOMESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AdvancedStepper.h>

struct Home
	{
	int32_t position;
	unsigned int width;
	Home(int32_t stepPosition, unsigned width) : position(stepPosition), width(width){}
	};

class HomeSensor
{
public:
	HomeSensor(MicrosteppingMotor* stepper, Home *settings, uint8_t sensorPin);
	static void init();
	bool atHome();
private:
	static uint8_t sensorPin;
	static volatile bool state;
	static MicrosteppingMotor* motor;
	static Home* settings;
	static void onHomeSensorChanged();
};


#endif

