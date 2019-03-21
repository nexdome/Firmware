// 
// 
// 

#include <ArduinoSTL.h>
#include "LimitSwitch.h"

MicrosteppingMotor * LimitSwitch::motor;

LimitSwitch::LimitSwitch(MicrosteppingMotor * stepper, uint8_t openLimit, uint8_t closeLimit)
	: openLimitPin(openLimit), closedLimitPin(closeLimit)
{
	LimitSwitch::motor = stepper;
}

void LimitSwitch::onCloseLimitReached()
{
	if (motor->CurrentVelocity() < 0)
	{
		motor->HardStop();
		std::cout << "STOPPED: ";
	}
	std::cout << "Closed limit" << std::endl;
	//ToDo: learn fully close position
}
void LimitSwitch::onOpenLimitReached()
{
	if (motor->CurrentVelocity() > 0)
	{
		motor->HardStop();
		auto position = motor->CurrentPosition();
		if (position < motor->LimitOfTravel())
			motor->SetLimitOfTravel(motor->CurrentPosition());
		std::cout << "STOPPED at: " << position << " ";
	}
	std::cout << "Open limit" << std::endl;
	//ToDo: save limit of travel in persistent settings
}

void LimitSwitch::init()
{
	pinMode(openLimitPin, INPUT_PULLUP);
	pinMode(closedLimitPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(openLimitPin), onOpenLimitReached, FALLING);
	attachInterrupt(digitalPinToInterrupt(closedLimitPin), onCloseLimitReached, FALLING);
}
