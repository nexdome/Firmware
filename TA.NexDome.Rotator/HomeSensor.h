// HomeSensor.h

#ifndef _HOMESENSOR_h
#define _HOMESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AdvancedStepper.h>
#include "CommandProcessor.h"

enum HomingPhase
	{
	Idle = 0,
	Detecting,
	Stopping,
	Reversing,
	AtHome,
	};

struct Home
	{
	int32_t position;	// Home sensor azimuth in microsteps
	unsigned int width;
	int32_t microstepsPerRotation;
	Home(int32_t stepPosition, unsigned width, int32_t circumferenceMicrosteps)
		: position(stepPosition), width(width), microstepsPerRotation(circumferenceMicrosteps) {}
	};

class HomeSensor
	{
public:
	HomeSensor(MicrosteppingMotor* stepper, Home* settings, uint8_t sensorPin, CommandProcessor & processor);
	static void init();
	static bool atHome();
	static void findHome(int direction);
	static void cancelHoming();
	void onMotorStopped() const;
	static bool homingInProgress();
private:
	static volatile HomingPhase phase;
	static uint8_t sensorPin;
	static MicrosteppingMotor* motor;
	static Home* homeSettings;
	CommandProcessor& commandProcessor;
	static void foundHome();
	static void onHomeSensorChanged();
	static void setPhase(HomingPhase newPhase);
	};


#endif
