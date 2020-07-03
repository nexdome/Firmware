#include <sstream>
#include <stdint.h>

#include "CommandProcessor.h"
#include "NexDome.h"
#include "Version.h"

CommandProcessor::CommandProcessor(MicrosteppingMotor& motor, PersistentSettings& settings, XBeeStateMachine& machine, LimitSwitch& limits, BatteryMonitor &monitor)
	: motor(motor), settings(settings), limitSwitches(limits), machine(machine), battery(monitor) {}

int32_t CommandProcessor::microstepsToSteps(int32_t microsteps)
	{
	return microsteps / MICROSTEPS_PER_STEP;
	}

int32_t CommandProcessor::stepsToMicrosteps(int32_t wholeSteps)
	{
	return wholeSteps * MICROSTEPS_PER_STEP;
	}

int32_t CommandProcessor::getPositionInWholeSteps() const
	{
	return microstepsToSteps(motor.getCurrentPosition());
	}

void CommandProcessor::sendStatus() const
	{
	const char separator = ',';
	static std::ostringstream converter;
	converter.clear();
	converter.str("");
	converter << ResponseBuilder::header
		<< "SES" << separator
		<< getPositionInWholeSteps() << separator
		<< microstepsToSteps(motor.limitOfTravel()) << separator
		<< limitSwitches.isOpen() << separator
		<< limitSwitches.isClosed()
		<< ResponseBuilder::terminator;
	machine.SendToRemoteXbee(converter.str());
#ifdef SHUTTER_LOCAL_OUTPUT
    std::cout << converter.str() << std::endl;
#endif
	}


void CommandProcessor::HandleCommand(const Command& command)
	{
    ResponseBuilder::FromSuccessfulCommand(command); // Default response is to echo the command.
	if (command.IsShutterCommand())
		{
		if (command.Verb == "AR") HandleAR(command); // Ramp time (acceleration) read (milliseconds)
		if (command.Verb == "AW") HandleAW(command); // Ramp time (acceleration) write (milliseconds)
		if (command.Verb == "BR") HandleBR(command); // Read battery low threshold
		if (command.Verb == "BW") HandleBW(command); // Write battery low threshold
		if (command.Verb == "CL") HandleCL(command); // Close shutter
		if (command.Verb == "FR") HandleFR(command); // Read firmware version
		if (command.Verb == "OP") HandleOP(command); // Open shutter
		if (command.Verb == "PR") HandlePR(command); // Position read
		if (command.Verb == "PW") HandlePW(command); // Position write (sync)
		if (command.Verb == "RR") HandleRR(command); // Range Read (get limit of travel)
		if (command.Verb == "RW") HandleRW(command); // Range Write (set limit of travel)
		if (command.Verb == "SR") HandleSR(command); // Send status report
		if (command.Verb == "SW") HandleSW(command); // Emergency stop
		if (command.Verb == "VR") HandleVR(command); // Read maximum motor speed
		if (command.Verb == "VW") HandleVW(command); // Read maximum motor speed
		if (command.Verb == "ZD") HandleZD(command); // Reset to factory settings (load defaults).
		if (command.Verb == "ZR") HandleZR(command); // Load settings from persistent storage
		if (command.Verb == "ZW") HandleZW(command); // Write settings to persistent storage
		}
	}

void CommandProcessor::sendOpenNotification() const
	{
	sendToLocalAndRemote("open");
	}

void CommandProcessor::sendCloseNotification() const
	{
	sendToLocalAndRemote("close");
	}

void CommandProcessor::sendToLocalAndRemote(const std::string& message) const
	{
	std::ostringstream output;
	output << ResponseBuilder::header << message << ResponseBuilder::terminator;
	machine.SendToRemoteXbee(output.str());
	#ifdef SHUTTER_LOCAL_OUTPUT
	std::cout << output.str() << std::endl;
	#endif
	}

void CommandProcessor::HandleOP(const Command& command)
	{
	if (!(limitSwitches.isOpen() || battery.lowVolts()))
		{
		sendOpenNotification();
		motor.moveToPosition(settings.motor.maxPosition);
		}
	}

void CommandProcessor::HandleCL(const Command& command)
	{
	if (!limitSwitches.isClosed())
		{
		sendCloseNotification();
		motor.moveToPosition(-1000);
		}
	}

void CommandProcessor::HandleAW(const Command& command)
	{
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		return ResponseBuilder::Error();
	motor.setRampTime(rampTime);
	}

void CommandProcessor::HandleAR(const Command& command) const
	{
	const auto rampTime = settings.motor.rampTimeMilliseconds;
	ResponseBuilder::FromInteger(command, rampTime);
	}

void CommandProcessor::HandleBR(const Command& command) const
	{
	ResponseBuilder::FromInteger(command, settings.batteryMonitor.threshold);
	}

void CommandProcessor::HandleBW(const Command& command)
	{
	const auto threshold = command.StepPosition;
	settings.batteryMonitor.threshold = threshold;
	}

void CommandProcessor::HandleSW(const Command& command)
	{
	motor.hardStop();
	}

void CommandProcessor::HandleZW(const Command& command)
	{
	settings.Save();
	}

void CommandProcessor::HandleZR(const Command& command)
	{
	settings = PersistentSettings::Load();
	}

void CommandProcessor::HandleZD(const Command& command)
	{
	settings = PersistentSettings();
	settings.Save();
	}

void CommandProcessor::HandlePR(const Command& command) const
	{
	const auto position = microstepsToSteps(motor.getCurrentPosition());
	ResponseBuilder::FromInteger(command, position);
	}

void CommandProcessor::HandlePW(const Command& command)
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
	motor.SetCurrentPosition(microsteps);
	}

void CommandProcessor::HandleRW(const Command& command)
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
	motor.SetLimitOfTravel(microsteps);
	}

void CommandProcessor::HandleSR(const Command& command)
	{
	sendStatus();
	ResponseBuilder::NoResponse(command);
	}

void CommandProcessor::HandleRR(const Command& command) const
	{
	const auto range = microstepsToSteps(motor.limitOfTravel());
	ResponseBuilder::FromInteger(command, range);
	}

void CommandProcessor::HandleFR(const Command& command) const
	{
        ResponseBuilder::FromString(command, SemanticVersion);
	}

void CommandProcessor::HandleVR(const Command& command) const
	{
	auto maxSpeed = motor.getMaximumSpeed();
	ResponseBuilder::FromInteger(command, microstepsToSteps(maxSpeed));
	}

void CommandProcessor::HandleVW(const Command& command)
	{
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < motor.getMinimumSpeed())
		return ResponseBuilder::Error();
	motor.setMaximumSpeed(speed);
	}


void CommandProcessor::HandleX(const Command& command)
	{
	if (motor.isMoving())
		return ResponseBuilder::FromInteger(command, 2);
	ResponseBuilder::FromInteger(command, 0);
	}
