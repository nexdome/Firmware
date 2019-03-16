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
	bool IsRotatorCommand();
	bool IsShutterCommand();
	bool IsSystemCommand();
};

#endif