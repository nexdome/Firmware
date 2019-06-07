
#include <sstream>
#include "BatteryMonitor.h"
#include "Response.h"
#include "CommandProcessor.h"

BatteryMonitor::BatteryMonitor(XBeeStateMachine& machine, uint8_t analogPin, BatteryMonitorSettings& settings)
	: machine(machine), analogPin(analogPin), settings(settings),
	movingAverageVoltage(settings.sampleWindow)
	{
	sampleTimer.Stop();
	notificationTimer.Stop();
	}

void BatteryMonitor::initialize(unsigned long initialDelay)
	{
	sampleTimer.SetDuration(initialDelay);
	notificationTimer.SetDuration(settings.notifyInterval);
	movingAverageVoltage.init(65535);
	}


void BatteryMonitor::loop()
	{
	if (!sampleTimer.Expired())
		return;
	sampleTimer.SetDuration(settings.sampleInterval);
	movingAverageVoltage.addSample(static_cast<const int>(analogRead(analogPin)));
	checkThresholdAndSendNotification();
	}

void BatteryMonitor::checkThresholdAndSendNotification()
	{
	if (!notificationTimer.Expired())
		return;
	notificationTimer.SetDuration(settings.notifyInterval);
	std::ostringstream message;
	message << Response::header << "BVS" << movingAverageVoltage.average << Response::terminator;
	machine.SendToRemoteXbee(message.str());
	std::cout << message.str() << std::endl;
	if (movingAverageVoltage.average < settings.threshold)
		{
		//ToDo: take action on low battery condition
		std::cout << "Auto-close" << std::endl;
		}
	}
