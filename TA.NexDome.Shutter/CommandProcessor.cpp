
#include "CommandProcessor.h"
#include "NexDome.h"
#include "Version.h"

CommandProcessor::CommandProcessor(MicrosteppingMotor& motor, PersistentSettings& settings)
	: motor(motor), settings(settings) 	{}

MicrosteppingMotor& CommandProcessor::GetMotor(Command& command)
	{
	return motor;
	}

int32_t CommandProcessor::microstepsToSteps(int32_t microsteps)
{
	return microsteps / MICROSTEPS_PER_STEP;
}

int32_t CommandProcessor::stepsToMicrosteps(int32_t wholeSteps)
{
	return wholeSteps * MICROSTEPS_PER_STEP;
}

Response CommandProcessor::HandleCommand(Command& command)
	{
	if (command.IsShutterCommand())
		{
		if (command.Verb == "AR") return HandleAR(command);		// Ramp time (acceleration) read (milliseconds)
		if (command.Verb == "AW") return HandleAW(command);		// Ramp time (acceleration) write (milliseconds)
		if (command.Verb == "CL") return HandleCL(command);		// Close shutter
		if (command.Verb == "FR") return HandleFR(command);		// Read firmware version
		if (command.Verb == "OP") return HandleOP(command);		// Open shutter
		//if (command.Verb == "MI") return HandleMI(command);	// Move motor in
		//if (command.Verb == "MO") return HandleMO(command);	// Move motor out
		if (command.Verb == "SW") return HandleSW(command);		// Stop motor (hard stop)
		if (command.Verb == "PR") return HandlePR(command);	// Position read
		if (command.Verb == "PW") return HandlePW(command);	// Position write (sync)
		if (command.Verb == "RR") return HandleRR(command);	// Range Read (get limit of travel)
		if (command.Verb == "RW") return HandleRW(command);	// Range Write (set limit of travel)
		if (command.Verb == "VR") return HandleVR(command);		// Read maximum motor speed
		if (command.Verb == "VW") return HandleVW(command);		// Read maximum motor speed
		if (command.Verb == "ZD") return HandleZD(command);		// Reset to factory settings (load defaults).
		if (command.Verb == "ZR") return HandleZR(command);		// Load settings from persistent storage
		if (command.Verb == "ZW") return HandleZW(command);		// Write settings to persistent storage
		}
	if (command.IsSystemCommand())
		{
		}
	return Response::Error();
	}

Response CommandProcessor::HandleOP(Command& command)
{
	motor.MoveToPosition(settings.motor.maxPosition);
	//ToDo: trigger some state that will monitor for the open limit switch
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleCL(Command& command)
{
	motor.MoveToPosition((uint32_t)0);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleAW(Command & command)
	{
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		return Response::Error();
	motor.SetRampTime(rampTime);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleAR(Command& command) const
	{
	const auto rampTime = settings.motor.rampTimeMilliseconds;
	return Response::FromInteger(command, rampTime);

}

Response CommandProcessor::HandleSW(Command & command)
	{
	motor.HardStop();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleZW(Command & command)
	{
	settings.Save();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleZR(Command & command)
{
	settings = PersistentSettings::Load();
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleZD(Command & command)
	{
	settings = PersistentSettings();
	settings.Save();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandlePR(Command & command)
	{
	auto position = microstepsToSteps(motor.CurrentPosition());
	auto response = Response::FromPosition(command, position);
	return response;
	}

Response CommandProcessor::HandlePW(Command & command)
{
	auto microsteps = stepsToMicrosteps(command.StepPosition);
	motor.SetCurrentPosition(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRW(Command & command)
{
	auto microsteps = stepsToMicrosteps(command.StepPosition);
	motor.SetLimitOfTravel(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRR(Command & command)
	{
	auto range = microstepsToSteps(motor.LimitOfTravel());
	return Response::FromPosition(command, range);
	}

Response CommandProcessor::HandleFR(Command& command)
{
	std::string message;
	message.append("FR");
	message.append(SemanticVersion);
	return Response{ message };
}

Response CommandProcessor::HandleVR(Command & command)
{
	auto maxSpeed = motor.MaximumSpeed();
	return Response::FromPosition(command, microstepsToSteps(maxSpeed));
}

Response CommandProcessor::HandleVW(Command & command)
{
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < motor.MinimumSpeed())
		return Response::Error();
	motor.SetMaximumSpeed(speed);
	return Response::FromSuccessfulCommand(command);
}


Response CommandProcessor::HandleX(Command & command)
	{
	if (motor.IsMoving())
		return Response::FromInteger(command, 2);
	return Response::FromInteger(command, 0);
	}
