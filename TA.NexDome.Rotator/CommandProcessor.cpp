#include "CommandProcessor.h"
#include "NexDome.h"

CommandProcessor::CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine)
	: rotator(rotator), settings(settings), machine(machine)
	{
	}

inline MicrosteppingMotor& CommandProcessor::GetMotor(Command& command)
	{
	return rotator;
	}

Response CommandProcessor::ForwardToShutter(Command& command)
{
	machine.SendToRemoteXbee(command.RawCommand);
	//ToDo: should the response always be successful?
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleCommand(Command& command)
	{
	if (command.IsShutterCommand())
	{
		ForwardToShutter(command);
		return Response::FromSuccessfulCommand(command);	// ToDo - maybe return a result from the shutter?
	}

	if (command.IsRotatorCommand())
		{
		if (command.Verb == "GA") return HandleGA(command);
		if (command.Verb == "SW") return HandleSW(command);	// Stop motor
		if (command.Verb == "PR") return HandlePR(command);	// Position read
		if (command.Verb == "PW") return HandlePW(command);	// Position write (sync)
		if (command.Verb == "RR") return HandleRR(command);	// Range Read (get limit of travel)
		if (command.Verb == "RW") return HandleRW(command);	// Range Write (set limit of travel)
		if (command.Verb == "VR") return HandleVR(command);	// Read maximum motor speed
		if (command.Verb == "VW") return HandleVW(command);	// Read maximum motor speed

		}
	if (command.IsSystemCommand())
		{
		if (command.Verb == "FR") return HandleFR(command);	// Read firmware version
		if (command.Verb == "X") return HandleX(command);	// Get movement status
		if (command.Verb == "ZD") return HandleZD(command);	// Reset to factory settings (load defaults).
		if (command.Verb == "ZR") return HandleZR(command);	// Load settings from persistent storage
		if (command.Verb == "ZW") return HandleZW(command);	// Write settings to persistent storage
		}
	return Response::Error();
	}

Response CommandProcessor::HandleGA(Command& command)
{
	//ToDo: This is temporary code for testing and needs to be re-done
	const double degreesPerStep = 1.2 / 15.0 / 100.0;
	const double stepsPerDegree = 1 / degreesPerStep;
	auto wholeSteps = command.StepPosition * stepsPerDegree;
	auto microsteps = StepsToMicrosteps(wholeSteps);
	rotator.MoveToPosition(microsteps);
	return Response::FromSuccessfulCommand(command);
}

//Response CommandProcessor::HandleMI(Command& command)
//	{
//	// Commands are in whole steps, motors operate in microsteps, so we must convert.
//	auto motor = GetMotor(command);
//	auto microStepsToMove = StepsToMicrosteps(command.StepPosition);
//	auto targetPosition = motor->CurrentPosition() - microStepsToMove;
//	if (targetPosition < 0)
//		return Response::Error();
//	motor->MoveToPosition(targetPosition);
//	return Response::FromSuccessfulCommand(command);
//	}
//
//Response CommandProcessor::HandleMO(Command& command)
//	{
//	// Commands are in whole steps, motors operate in microsteps, so we must convert.
//	auto motor = GetMotor(command);
//	auto microStepsToMove = StepsToMicrosteps(command.StepPosition);
//	auto targetPosition = motor->CurrentPosition() + microStepsToMove;
//	if (targetPosition > motor->LimitOfTravel())
//		return Response::Error();
//	motor->MoveToPosition(targetPosition);
//	return Response::FromSuccessfulCommand(command);
//	}

Response CommandProcessor::HandleAW(Command & command)
	{
	auto motor = GetMotor(command);
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		return Response::Error();
	motor.SetRampTime(rampTime);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleSW(Command & command)
	{
	auto motor = GetMotor(command);
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
	auto motor = GetMotor(command);
	auto position = MicrostepsToSteps(motor.CurrentPosition());
	auto response = Response::FromPosition(command, position);
	return response;
	}

Response CommandProcessor::HandlePW(Command & command)
{
	auto microsteps = StepsToMicrosteps(command.StepPosition);
	auto motor = GetMotor(command);
	motor.SetCurrentPosition(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRW(Command & command)
{
	auto microsteps = StepsToMicrosteps(command.StepPosition);
	auto motor = GetMotor(command);
	motor.SetLimitOfTravel(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRR(Command & command)
	{
	auto motor = GetMotor(command);
	auto range = MicrostepsToSteps(motor.LimitOfTravel());
	return Response::FromPosition(command, range);
	}

Response CommandProcessor::HandleFR(Command & command)
	{
	std::string message;
	message.append("FR");
	message.append(FIRMWARE_MAJOR_VERSION);
	message.push_back('.');
	message.append(FIRMWARE_MINOR_VERSION);
	message.append(Response::Terminator);
	return Response{ message };
	}

Response CommandProcessor::HandleVR(Command & command)
{
	auto motor = GetMotor(command);
	auto maxSpeed = motor.MaximumSpeed();
	return Response::FromPosition(command, MicrostepsToSteps(maxSpeed));
}

Response CommandProcessor::HandleVW(Command & command)
{
	auto motor = GetMotor(command);
	uint16_t speed = StepsToMicrosteps(command.StepPosition);
	if (speed < motor.MinimumSpeed())
		return Response::Error();
	motor.SetMaximumSpeed(speed);
	return Response::FromSuccessfulCommand(command);
}


Response CommandProcessor::HandleX(Command & command)
	{
	if (rotator.IsMoving())
		return Response::FromInteger(command, 2);
	return Response::FromInteger(command, 0);
	}

int32_t CommandProcessor::MicrostepsToSteps(int32_t microsteps)
	{
	return microsteps / MICROSTEPS_PER_STEP;
	}

int32_t CommandProcessor::StepsToMicrosteps(int32_t wholesteps)
	{
	return wholesteps * MICROSTEPS_PER_STEP;
	}
