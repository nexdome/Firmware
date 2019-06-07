// HomeSensor.h

#ifndef _HOMESENSOR_h
#define _HOMESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AdvancedStepper.h>

struct Home
	{
	int32_t position;
	unsigned int width;
	int32_t microstepsPerRotation;
	Home(int32_t stepPosition, unsigned width, int32_t circumferenceMicrosteps) 
		: position(stepPosition), width(width), microstepsPerRotation(circumferenceMicrosteps) {}
	};

class HomeSensor
	{
public:
	HomeSensor(MicrosteppingMotor* stepper, Home* settings, uint8_t sensorPin);
	static void init();
	static bool atHome();
	static void findHome(int direction);
	static void cancelHoming();
private:
	static uint8_t sensorPin;
	static volatile bool state;
	static MicrosteppingMotor* motor;
	static Home* settings;
	static void onHomeSensorChanged();
	static volatile bool homingInProgress;
	};


#endif
