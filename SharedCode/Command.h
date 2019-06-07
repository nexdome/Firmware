#ifndef _COMMAND_h
#define _COMMAND_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

struct Command
	{
	std::string RawCommand;
	std::string Verb;
	char TargetDevice;
	int32_t StepPosition;	// Target step position for a move command
	bool IsRotatorCommand() const { return TargetDevice == 'R'; }
	bool IsShutterCommand() const { return TargetDevice == 'S'; }
	bool IsSystemCommand() const { return TargetDevice == '0'; }
	};

#endif