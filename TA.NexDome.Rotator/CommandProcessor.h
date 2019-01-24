
#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>

struct Command
	{
	String Verb;
	char TargetDevice;
	int32_t StepPosition;	// Target step position for a move command
	bool IsMotorCommand();
	bool IsSystemCommand();
	};

struct Response
	{
	static const String Terminator;
	String Message;
	bool success;

	/*
		Creates an error response.
	*/
	static Response Error();

	/*
		Creates a success response by echoing the command verb,
		terminated with a '#'.
	*/
	static Response FromSuccessfulCommand(Command& command);

	/*
		Creates a response consisting of the command verb,
		plus an unsigned integer (usually a motor step position).
	*/
	static Response FromPosition(Command& command, uint32_t position);

	/*
		Creates a response consisting of just an integer and the terminator.
	*/
	static Response FromInteger(Command& command, int i);
	};

class CommandProcessor
	{
	public:
		CommandProcessor(MicrosteppingMotor& rotator);
		Response HandleCommand(Command& command);
		static int32_t MicrostepsToSteps(int32_t microsteps);
		static int32_t StepsToMicrosteps(int32_t wholesteps);

	private:
		MicrosteppingMotor * GetMotor(Command& command);		// Gets the motor addressed by the command
		Response HandleAW(Command & command);	// AW - Acceleration ramp time write
		Response HandleBR(Command & command);	// Backlash steps read
		Response HandleBW(Command & command);	// Backlash steps write
		Response HandleCS(Command & command);	// Calibration start
		Response HandleCR(Command & command);	// Calibration state read
		Response HandleCE(Command & command);	// Calibration end (abort/cancel)
		Response HandleCl(Command & command);	// Calibration sensor first contact threshold
		Response HandleCL(Command & command);	// Calibration sensor hard stop threshold
		Response HandleCv(Command & command);	// Calibration slow motion velocity [sic] steps/sec
		Response HandleCW(Command & command);	// Calibration state write
		Response HandleER(Command & command);	// FSR value read
		Response HandleFR(Command & command);	// Firmware version read
		Response HandleMI(Command & command);	// Move motor in
		Response HandleMO(Command & command);	// Move motor out
		Response HandlePR(Command & command);	// Step position read
		Response HandlePW(Command & command);	// Step position write (sync)
		Response HandleRR(Command & command);	// Range (limit of travel) read
		Response HandleRW(Command & command);	// Range (limit of travel) write
		Response HandleSW(Command & command);	// Stop write (motor emergency stop)
		Response HandleTR(Command & command);	// Temperature read (in Celsius)
		Response HandleVR(Command & command);	// Velocity [sic] read (motor maximum speed in microsteps/sec)
		Response HandleVW(Command & command);	// Velocity [sic] write (microsteps/sec)
		Response HandleX(Command & command);	// Movement status read
		Response HandleZW(Command & command);	// EEPROM write (save settings)
		Response HandleZR(Command & command);	// EEPROM read (load settings)
		Response HandleZD(Command & command);	// Reset to factory settings (clears both EEPROM and working settings)
		MicrosteppingMotor *rotator;
	};

extern Response DispatchCommand(Command& command);
#endif
