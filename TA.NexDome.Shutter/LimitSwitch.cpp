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
		motor->SoftStop();
		std::cout << "STOPPED: ";
	}
	std::cout << "Closed limit" << std::endl;
	//ToDo: learn fully close position
}
void LimitSwitch::onOpenLimitReached()
{
	if (motor->CurrentVelocity() > 0)
	{
		motor->SoftStop();
		auto stoppingDistance = motor->distanceToStop();
		auto position = motor->CurrentPosition();
		auto stopPosition = motor->CurrentPosition() + stoppingDistance;
		if (stopPosition < motor->LimitOfTravel())
			motor->SetLimitOfTravel(stopPosition);
		std::cout << "STOPPED at: " << stopPosition << " ";
	}
	std::cout << "Open limit" << std::endl;
}

void LimitSwitch::init()
{
	pinMode(openLimitPin, INPUT_PULLUP);
	pinMode(closedLimitPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(openLimitPin), onOpenLimitReached, FALLING);
	attachInterrupt(digitalPinToInterrupt(closedLimitPin), onCloseLimitReached, FALLING);
}
