#ifndef _COMMAND_h
#define _COMMAND_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

struct Command
{
    explicit Command(const std::string &buffer);
    std::string RawCommand;
    std::string Verb;
    char TargetDevice;
    int32_t StepPosition; // Target step position for a move command
    bool IsRotatorCommand() const { return TargetDevice == 'R'; }
    bool IsShutterCommand() const { return TargetDevice == 'S'; }
    bool IsSystemCommand() const { return TargetDevice == '0'; }
};

/*
 * Expects a well-formed command in the format "@VVT,nnnn"
 * The comma and everything following is optional.
 * VV is a 2-character command verb
 * T is the target address ('R' for rotator, 'S' for shutter)
 */
inline Command::Command(const std::string &buffer)
{
    const auto bufferLength = buffer.length();
    RawCommand = buffer;
    StepPosition = 0;
    Verb.push_back(buffer[1]);
    if (bufferLength > 2)
        Verb.push_back(buffer[2]);
    // Use the device address from the command, otherwise '0'
    TargetDevice = (bufferLength < 4) ? '0' : buffer[3];
    // If the parameter was present, then parse it as an integer and use as the command payload.
    // This should return 0 if conversion was unsuccessful.
    if (bufferLength > 5 && buffer[4] == ',')
    {
        const auto position = buffer.substr(5);
        const auto wholeSteps = std::strtoul(position.begin(), nullptr, 10);
        StepPosition = (int32_t)wholeSteps;
    }
}

#endif
