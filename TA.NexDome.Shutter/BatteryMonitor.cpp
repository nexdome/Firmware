#include <sstream>
#include "BatteryMonitor.h"
#include "Response.h"
#include "CommandProcessor.h"

extern void DispatchCommand(const Command& command);

BatteryMonitor::BatteryMonitor(XBeeStateMachine& machine, uint8_t analogPin, BatteryMonitorSettings& settings) :
    machine(machine),
    movingAverageVoltage(settings.sampleWindow), analogPin(analogPin), settings(settings)
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

bool BatteryMonitor::lowVolts()
	{
	return movingAverageVoltage.average < settings.threshold;
	}

void BatteryMonitor::checkThresholdAndSendNotification()
	{
	if (!notificationTimer.Expired())
		return;
	notificationTimer.SetDuration(settings.notifyInterval);
	std::ostringstream message;
	message << ResponseBuilder::header << "BV" << movingAverageVoltage.average << ResponseBuilder::terminator;
	machine.SendToRemoteXbee(message.str());
#ifdef SHUTTER_LOCAL_OUTPUT
	std::cout << message.str() << std::endl;
#endif
	if (lowVolts())
		{
		const std::string lowVoltsMessage = "Volts";
        const std::string closeShutter = "@CLS";
#ifdef SHUTTER_LOCAL_OUTPUT
		std::cout << lowVoltsMessage << std::endl;
#endif
        auto command = Command(closeShutter);
        DispatchCommand(command);
		machine.SendToRemoteXbee(lowVoltsMessage);
		}
	}
