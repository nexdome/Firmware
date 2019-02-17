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

struct PersistentSettings
{
	static const uint16_t fingerprint = 0x4AFB;
	uint8_t majorVersion = FIRMWARE_MAJOR_VERSION;
	uint8_t minorVersion = FIRMWARE_MINOR_VERSION;

	struct MotorSettings motor;

	PersistentSettings();
	static PersistentSettings Load();
	void Save();
};


#endif

