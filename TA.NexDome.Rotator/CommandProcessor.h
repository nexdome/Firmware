
#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include "PersistentSettings.h"
#include "Command.h"
#include "Response.h"

class CommandProcessor
	{
	public:
		CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings);
		Response HandleCommand(Command& command);
		static int32_t MicrostepsToSteps(int32_t microsteps);
		static int32_t StepsToMicrosteps(int32_t wholesteps);

	private:
		MicrosteppingMotor * GetMotor(Command& command);		// Gets the motor addressed by the command
		Response HandleGA(Command& command);	// GA - GoTo Azimuth (in degrees).
		Response HandleAW(Command & command);	// AW - Acceleration ramp time write
		Response HandleFR(Command & command);	// Firmware version read
		Response HandlePR(Command & command);	// Step position read
		Response HandlePW(Command & command);	// Step position write (sync)
		Response HandleRR(Command & command);	// Range (limit of travel) read
		Response HandleRW(Command & command);	// Range (limit of travel) write
		Response HandleSW(Command & command);	// Stop write (motor emergency stop)
		Response HandleVR(Command & command);	// Velocity [sic] read (motor maximum speed in microsteps/sec)
		Response HandleVW(Command & command);	// Velocity [sic] write (microsteps/sec)
		Response HandleX(Command & command);	// Movement status read
		Response HandleZW(Command & command);	// EEPROM write (save settings)
		Response HandleZR(Command & command);	// EEPROM read (load settings)
		Response HandleZD(Command & command);	// Reset to factory settings (clears both EEPROM and working settings)
		MicrosteppingMotor *rotator;
		PersistentSettings *settings;
	};


extern Response DispatchCommand(Command& command);
extern Response DispatchCommand(char *buffer, unsigned int charCount);

#endif
