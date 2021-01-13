#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <AdvancedStepper.h>
#include <XBeeStateMachine.h>
#include "Command.h"
#include "Response.h"

struct PersistentSettings;

class CommandProcessor
	{
public:
	CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine);
	static void responseToHost(const std::string& rxMessage);
	void HandleCommand(const Command& command) const;
	uint32_t getNormalizedPositionInMicrosteps() const;
	int32_t getPositionInWholeSteps() const;
	int32_t getCircumferenceInWholeSteps() const;
	int32_t getHomePositionWholeSteps() const;
	float getAzimuth() const;
	static int32_t microstepsToSteps(int32_t microsteps);
	static int32_t stepsToMicrosteps(int32_t wholeSteps);
	void sendStatus() const;
	static void sendDirection(int direction);
	int32_t targetStepPosition(uint32_t toMicrostepPosition) const;

private:

	void rotateToMicrostepPosition(int32_t target) const;
	void ForwardToShutter(const Command& command) const;
	void HandleZZ(const Command& command) const;
	void HandleDR(const Command& command) const;
	void HandleDW(const Command& command) const;
	void HandleAR(const Command& command) const;
	void HandleAW(const Command& command) const; // AW - Acceleration ramp time write
	void HandleFR(const Command& command) const; // Firmware version read
	void HandleGA(const Command& command) const; // GA - GoTo Azimuth (in degrees).
	void HandleGS(const Command& command) const; // GS - GoTo Step Position
	void HandleGH(const Command& command) const; // Go to home sensor
	void HandleHR(const Command& command) const; // Read Home position (steps clockwise from true north)
	void HandleHW(const Command& command) const; // Write home position (steps clockwise from true north)
	void HandlePR(const Command& command) const; // Step position read
	void HandlePW(const Command& command) const; // Step position write (sync)
	void HandleRR(const Command& command) const; // Range (limit of travel) read
	void HandleRW(const Command& command) const; // Range (limit of travel) write
	void HandleSW(const Command& command) const; // Stop write (motor emergency stop)
	void HandleSR(const Command& command) const; // Status Request
	void HandleVR(const Command& command) const; // Velocity [sic] read (motor maximum speed in microsteps/sec)
	void HandleVW(const Command& command) const; // Velocity [sic] write (microsteps/sec)
	void HandleZW(const Command& command) const; // EEPROM write (save settings)
	void HandleZR(const Command& command) const; // EEPROM read (load settings)
	void HandleZD(const Command& command) const; // Reset to factory settings (clears both EEPROM and working settings)
	// Calculates the target microstep position using the shortest direction.
	int32_t deltaSteps(uint32_t toMicrostepPosition) const;
	int32_t getDeadZoneWholeSteps() const;
	MicrosteppingMotor& rotator;
	PersistentSettings& settings;
	XBeeStateMachine& machine;
	};

#endif
