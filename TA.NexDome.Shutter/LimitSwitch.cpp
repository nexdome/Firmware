//
//
//

#include <ArduinoSTL.h>
#include "LimitSwitch.h"
#include "NexDome.h"


MicrosteppingMotor* LimitSwitch::motor;
volatile bool LimitSwitch::closeTriggered; // static and volatile because accessed in ISR

LimitSwitch::LimitSwitch(MicrosteppingMotor* stepper, uint8_t openLimit, uint8_t closeLimit)
	: openLimitPin(openLimit), closedLimitPin(closeLimit)
	{
	LimitSwitch::motor = stepper;
	}

bool LimitSwitch::isOpen() const
	{
	return digitalRead(openLimitPin) == 0;
	}

bool LimitSwitch::isClosed() const
	{
	return digitalRead(closedLimitPin) == 0;
	}

void LimitSwitch::onCloseLimitReached()
	{
	if (closeTriggered)
		return;
	if (motor->getCurrentVelocity() < 0)
		{
		closeTriggered = true;
		motor->SetCurrentPosition(SHUTTER_LIMIT_STOPPING_DISTANCE);
		motor->moveToPosition(0);
		}
	}

void LimitSwitch::onOpenLimitReached()
	{
	if (motor->getCurrentVelocity() > 0)
		{
		const auto stopPosition = motor->getCurrentPosition() + SHUTTER_LIMIT_STOPPING_DISTANCE;
		motor->moveToPosition(stopPosition);
		//if (stopPosition < motor->limitOfTravel())
		//	motor->SetLimitOfTravel(stopPosition);
		}
	}

void LimitSwitch::onMotorStopped()
	{
	closeTriggered = false;
	}

void LimitSwitch::init() const
	{
	pinMode(openLimitPin, INPUT_PULLUP);
	pinMode(closedLimitPin, INPUT_PULLUP);
	closeTriggered = false;
	attachInterrupt(digitalPinToInterrupt(openLimitPin), onOpenLimitReached, FALLING);
	attachInterrupt(digitalPinToInterrupt(closedLimitPin), onCloseLimitReached, FALLING);
	}
