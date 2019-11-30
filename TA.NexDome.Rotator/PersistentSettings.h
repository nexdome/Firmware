// PersistentSettings.h

#ifndef _PERSISTENTSETTINGS_h
#define _PERSISTENTSETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "AdvancedStepper.h"
#include "HomeSensor.h"
#include "Version.h"

constexpr uint16_t fingerprint = 0x4AFB;
struct PersistentSettings
{
	uint16_t fingerprintHead = fingerprint;
	uint8_t majorVersion = MajorVersion;
	uint8_t minorVersion = MinorVersion;
	struct MotorSettings motor{};
	struct Home home;
	int deadZone = 1000;
	uint16_t fingerprintTail = fingerprint;
	PersistentSettings();
	static PersistentSettings Load();
	void Save();
};


#endif

