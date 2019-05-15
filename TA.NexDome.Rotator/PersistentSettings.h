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
#include "HomeSensor.h"
#include "Version.h"

struct PersistentSettings
{
	static const uint16_t fingerprint = 0x4AFB;
	uint8_t majorVersion = MajorVersion;
	uint8_t minorVersion = MinorVersion;
	struct MotorSettings motor{};
	struct Home home;
	PersistentSettings();
	static PersistentSettings Load();
	void Save();
};


#endif

