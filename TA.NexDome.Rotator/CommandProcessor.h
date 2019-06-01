#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <sstream>
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
	Response HandleCommand(Command& command) const;
	uint32_t getNormalizedPositionInMicrosteps() const;
	int32_t getPositionInWholeSteps() const;
	int32_t getCircumferenceInWholeSteps() const;
	int32_t getHomePositionWholeSteps() const;
	float getAzimuth() const;
	static int32_t microstepsToSteps(int32_t microsteps);
	static int32_t stepsToMicrosteps(int32_t wholeSteps);
	void sendStatus() const;

private:
	Response ForwardToShutter(Command& command) const;
	Response HandleAR(Command& command) const;
	Response HandleAW(Command& command) const; // AW - Acceleration ramp time write
	Response HandleFR(Command& command) const; // Firmware version read
	Response HandleGA(Command& command) const; // GA - GoTo Azimuth (in degrees).
	Response HandleGH(Command& command) const; // Go to home sensor
	Response HandleHR(Command& command) const; // Read Home position (steps clockwise from true north)
	Response HandleHW(Command& command) const; // Write home position (steps clockwise from true north)
	Response HandlePR(Command& command) const; // Step position read
	Response HandlePW(Command& command) const; // Step position write (sync)
	Response HandleRR(Command& command) const; // Range (limit of travel) read
	Response HandleRW(Command& command) const; // Range (limit of travel) write
	Response HandleSW(Command& command) const; // Stop write (motor emergency stop)
	Response HandleSR(Command& command) const; // Status Request
	Response HandleVR(Command& command) const; // Velocity [sic] read (motor maximum speed in microsteps/sec)
	Response HandleVW(Command& command) const; // Velocity [sic] write (microsteps/sec)
	Response HandleZW(Command& command) const; // EEPROM write (save settings)
	Response HandleZR(Command& command) const; // EEPROM read (load settings)
	Response HandleZD(Command& command) const; // Reset to factory settings (clears both EEPROM and working settings)
	int32_t targetStepPosition(uint32_t toMicrostepPosition) const;
	// Calculates the target microstep position using the shortest direction.
	int32_t deltaSteps(uint32_t toMicrostepPosition) const;
	MicrosteppingMotor& rotator;
	PersistentSettings& settings;
	XBeeStateMachine& machine;
	HomeSensor& homeSensor;
	};
#endif
