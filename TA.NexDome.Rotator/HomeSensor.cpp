/*
 * Provides interrupt processing for the home sensor.
 * 
 * The home sensor synchronizes the dome rotation step position when it is triggered.
 * When rotating in the positive direction, we synchronise to the falling edge.
 * When rotating in the negative direction, we synchronise to the rising edge.
 * When not rotating, activity is ignored.
 * 
 * Note: some fields have to be static because they are used during interrupts
 */

#include <ArduinoSTL.h>
#include <limits.h>
#include "HomeSensor.h"

#pragma region static fields used within interrupt service routines
MicrosteppingMotor* HomeSensor::motor;
Home* HomeSensor::settings;
uint8_t HomeSensor::sensorPin;
volatile bool HomeSensor::state;
volatile bool HomeSensor::homingInProgress;
#pragma endregion

/*
 * Creates a new HomeSensor instance.
 * Note: sensorPin must correspond to a digital input pin that is valid for attaching interrupts.
 * Not all pins on all platforms support attaching interrupts.
 * Arduino Leonardo supports pins 0, 1, 2, 3, 7
 */
HomeSensor::HomeSensor(MicrosteppingMotor* stepper, Home* settings, const uint8_t sensorPin)
	{
	motor = stepper;
	HomeSensor::settings = settings;
	HomeSensor::sensorPin = sensorPin;
	}


/*
 * Triggered as an interrupt whenever the home sensor pin changes state.
 * Synchronizes the current motor stop position to the calibrated home position.
 */
void HomeSensor::onHomeSensorChanged()
	{
	state = digitalRead(sensorPin);
	if (!motor->isMoving()) // Ignore state change if rotator not moving
		return;
	const auto direction = motor->getCurrentDirection();
	if ((state && direction < 0) || (!state && direction > 0))
		{
		// sync position on either the rising or falling edge, depending on rotation direction.
		motor->SetCurrentPosition(settings->position);
		if (homingInProgress)
			cancelHoming();
		}
	}

/*
 * Configures the hardware pin ready for use and attaches the interrupt.
 */
void HomeSensor::init()
	{
	pinMode(sensorPin, INPUT_PULLUP);
	state = digitalRead(sensorPin);
	attachInterrupt(digitalPinToInterrupt(sensorPin), onHomeSensorChanged, CHANGE);
	}


bool HomeSensor::atHome()
	{
	return !state;
	}

void HomeSensor::findHome(int direction)
	{
	homingInProgress = true;
	motor->moveToPosition(direction ? INT32_MAX : INT32_MIN);
	}

void HomeSensor::cancelHoming()
	{
	homingInProgress = false;
	if (motor->isMoving())
		motor->SoftStop();
	}
