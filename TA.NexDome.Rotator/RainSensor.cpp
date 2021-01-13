//
//
//

#include <ArduinoSTL.h>
#include "RainSensor.h"
#include "CommandProcessor.h"

extern void DispatchCommand(const Command& command);

/*
 * Initialize the rain sensor.
 * @param repeat - how often to repeat closing the shutter while rain persists.
 */
void RainSensor::init(Duration repeat)
	{
	pinMode(pin, INPUT_PULLUP);
	closing = false;
	closeTimer.Stop();
	repeatDuration = repeat;
	}

void RainSensor::closeShutter()
	{
	closeTimer.SetDuration(repeatDuration);
	closing = true;
	std::cout << ":Rain#" << std::endl;
	const std::string closeShutter = "@CLS";
	const auto closeCommand = Command(closeShutter);
	DispatchCommand(closeCommand);
	}

void RainSensor::loop()
	{
	const bool rainDetected = digitalRead(pin) == 0;
	if (!rainDetected)
		{
		if (closing) std::cout << ":RainStopped#" << std::endl;
		closing = false;
		closeTimer.Stop();
		return;
		}

	// If we get here, it's raining, either persistently or a new detection.
	// If it is a new detection (!closing) or the time has expired, we send the close command.
	// This also (re)starts the timer.
	if (!closing || closeTimer.Expired())
		{
		// This is a new detection. Issue the close command and start a repeat timer.
		closeShutter();
		}
	}
