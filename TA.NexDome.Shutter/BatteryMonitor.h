// This file is part of the TA.NexDome project
// Copyright © 2019-2019 Tigra Astronomy, all rights reserved.

#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <Arduino.h>
#include <Timer.h>
#include <XBeeStatemachine.h>
#include "MovingAverage.h"

struct BatteryMonitorSettings
	{
	uint8_t sampleWindow;
	uint64_t sampleInterval;
	uint16_t threshold;
	uint64_t notifyInterval;
	BatteryMonitorSettings() : sampleWindow(10), sampleInterval(3000), threshold(0), notifyInterval(30000) { }
	};

class BatteryMonitor
	{
public:
	explicit BatteryMonitor(XBeeStateMachine& machine, uint8_t analogPin, BatteryMonitorSettings& settings);
	void initialize(unsigned long initialDelay);
	void loop();
private:
	void checkThresholdAndSendNotification();
	XBeeStateMachine& machine;
	MovingAverage<uint16_t> movingAverageVoltage;
	Timer sampleTimer;
	Timer notificationTimer;
	unsigned analogPin;
	BatteryMonitorSettings& settings;
	};

#endif // BATTERYMONITOR_H
