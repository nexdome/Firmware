#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include <XBeeStatemachine.h>
#include "PersistentSettings.h"
#include "Command.h"
#include "Response.h"

class CommandProcessor
	{
public:
	CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine,
	                 HomeSensor& homeSensor);
	Response HandleCommand(Command& command);
	Response HandleAR(Command& command) const;
	uint32_t getNormalizedPositionInMicrosteps() const;
	int32_t getPositionInWholeSteps() const;
	float getAzimuth() const;
	static int32_t microstepsToSteps(int32_t microsteps);
	static int32_t stepsToMicrosteps(int32_t wholeSteps);

private:
	Response ForwardToShutter(Command& command);
	Response HandleAW(Command& command); // AW - Acceleration ramp time write
	Response HandleFR(Command& command); // Firmware version read
	Response HandleGA(Command& command); // GA - GoTo Azimuth (in degrees).
	Response HandleGH(Command& command); // Go to home sensor
	Response HandlePR(Command& command); // Step position read
	Response HandlePW(Command& command); // Step position write (sync)
	Response HandleRR(Command& command); // Range (limit of travel) read
	Response HandleRW(Command& command); // Range (limit of travel) write
	Response HandleSW(Command& command); // Stop write (motor emergency stop)
	Response HandleVR(Command& command); // Velocity [sic] read (motor maximum speed in microsteps/sec)
	Response HandleVW(Command& command); // Velocity [sic] write (microsteps/sec)
	Response HandleX(Command& command); // Movement status read
	Response HandleZW(Command& command); // EEPROM write (save settings)
	Response HandleZR(Command& command); // EEPROM read (load settings)
	Response HandleZD(Command& command); // Reset to factory settings (clears both EEPROM and working settings)
	int32_t targetStepPosition(uint32_t toMicrostepPosition) const;
	// Calculates the target microstep position using the shortest direction.
	int32_t deltaSteps(uint32_t toMicrostepPosition) const;
	MicrosteppingMotor& rotator;
	PersistentSettings& settings;
	XBeeStateMachine& machine;
	HomeSensor& homeSensor;
	};
#endif
