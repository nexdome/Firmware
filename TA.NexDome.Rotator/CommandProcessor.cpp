#include "CommandProcessor.h"
#include "NexDome.h"
#include "../TA.NexDome.Shutter/CommandProcessor.h"
#include "Version.h"

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
		return Response::NoResponse(command);
	}

	if (command.IsRotatorCommand())
		{
		if (command.Verb == "FR") return HandleFR(command);	// Read firmware version
		if (command.Verb == "GA") return HandleGA(command);	// Goto Azimuth (rotator only)
		if (command.Verb == "SW") return HandleSW(command);	// Stop motor
		if (command.Verb == "PR") return HandlePR(command);	// Position read
		if (command.Verb == "PW") return HandlePW(command);	// Position write (sync)
		if (command.Verb == "RR") return HandleRR(command);	// Range Read (get limit of travel)
		if (command.Verb == "RW") return HandleRW(command);	// Range Write (set limit of travel)
		if (command.Verb == "VR") return HandleVR(command);	// Read maximum motor speed
		if (command.Verb == "VW") return HandleVW(command);	// Read maximum motor speed
		if (command.Verb == "ZD") return HandleZD(command);	// Reset to factory settings (load defaults).
		if (command.Verb == "ZR") return HandleZR(command);	// Load settings from persistent storage
		if (command.Verb == "ZW") return HandleZW(command);	// Write settings to persistent storage
		}
	if (command.IsSystemCommand())
		{
		// There are currently no system commands
		}
	return Response::Error();
	}

Response CommandProcessor::HandleGA(Command& command) const
	{
	//ToDo: This is temporary code for testing and needs to be re-done
	const auto microstepsPerDegree = ROTATOR_FULL_REVOLUTION_MICROSTEPS / 360.0;
	const auto target = targetStepPosition(command.StepPosition * microstepsPerDegree);
	rotator.MoveToPosition(target);
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
	auto position = microstepsToSteps(motor.CurrentPosition());
	auto response = Response::FromPosition(command, position);
	return response;
	}

Response CommandProcessor::HandlePW(Command & command)
{
	auto microsteps = stepsToMicrosteps(command.StepPosition);
	auto motor = GetMotor(command);
	motor.SetCurrentPosition(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRW(Command & command)
{
	auto microsteps = stepsToMicrosteps(command.StepPosition);
	auto motor = GetMotor(command);
	motor.SetLimitOfTravel(microsteps);
	return Response::FromSuccessfulCommand(command);
}

Response CommandProcessor::HandleRR(Command & command)
	{
	auto motor = GetMotor(command);
	auto range = microstepsToSteps(motor.LimitOfTravel());
	return Response::FromPosition(command, range);
	}

Response CommandProcessor::HandleFR(Command & command)
	{
	std::string message;
	message.append("FR");
	message.append(SemanticVersion);
	return Response{ message };
	}

Response CommandProcessor::HandleVR(Command & command)
{
	auto motor = GetMotor(command);
	auto maxSpeed = motor.MaximumSpeed();
	return Response::FromPosition(command, microstepsToSteps(maxSpeed));
}

Response CommandProcessor::HandleVW(Command & command)
{
	auto motor = GetMotor(command);
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < motor.MinimumSpeed())
		return Response::Error();
	motor.SetMaximumSpeed(speed);
	return Response::FromSuccessfulCommand(command);
}


Response CommandProcessor::HandleX(Command & command) const
	{
	if (rotator.IsMoving())
		return Response::FromInteger(command, 2);
	return Response::FromInteger(command, 0);
	}

// Computes the target step position taking into account the shortest movement direction.
int32_t CommandProcessor::targetStepPosition(const uint32_t toMicrostepPosition) const
	{
	const int32_t halfway = settings.microstepsPerRotation / 2;
	const uint32_t fromMicrostepPosition = getNormalizedPositionInMicrosteps();
	int32_t delta = toMicrostepPosition - fromMicrostepPosition;
	if (delta == 0)
		return 0;
	if (delta > halfway)
		delta -= settings.microstepsPerRotation;
	if (delta < -halfway)
		delta += settings.microstepsPerRotation;
	return delta;
	}

inline int32_t CommandProcessor::microstepsToSteps(int32_t microsteps)
	{
	return microsteps / MICROSTEPS_PER_STEP;
	}

inline int32_t CommandProcessor::stepsToMicrosteps(int32_t wholeSteps)
	{
	return wholeSteps * MICROSTEPS_PER_STEP;
	}

uint32_t CommandProcessor::getNormalizedPositionInMicrosteps() const
{
	auto position = rotator.CurrentPosition();
	while (position < 0)
		position += ROTATOR_FULL_REVOLUTION_MICROSTEPS;
	return position;
}

inline int32_t CommandProcessor::getPositionInWholeSteps() const
	{
	return CommandProcessor::microstepsToSteps(getNormalizedPositionInMicrosteps());
	}

float CommandProcessor::getAzimuth() const
	{
	const auto degreesPerStep = 360.0 / settings.microstepsPerRotation;
	return getPositionInWholeSteps() * degreesPerStep;
	}
