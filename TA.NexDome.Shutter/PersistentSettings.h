// PersistentSettings.h

#ifndef _PERSISTENTSETTINGS_h
#define _PERSISTENTSETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "NexDome.h"
#include "AdvancedStepper.h"
#include "Version.h"
#include "BatteryMonitor.h"

constexpr uint16_t fingerprint = 0x4AFB;
struct PersistentSettings
	{
	uint16_t fingerprintHead = fingerprint;
	char majorVersion = MajorVersion;
	char minorVersion = MinorVersion;

	struct MotorSettings motor;
	struct BatteryMonitorSettings batteryMonitor;
	uint16_t fingerprintTail = fingerprint;
	PersistentSettings();
	static PersistentSettings Load();
	void Save();
};


#endif

