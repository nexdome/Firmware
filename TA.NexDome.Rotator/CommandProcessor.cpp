#include "CommandProcessor.h"
#include "NexDome.h"
#include "../TA.NexDome.Shutter/CommandProcessor.h"
#include "Version.h"

CommandProcessor::CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine,
                                   HomeSensor& homeSensor)
	: rotator(rotator), settings(settings), machine(machine), homeSensor(homeSensor) { }

Response CommandProcessor::HandleHR(Command& command) const
	{
	return Response::FromInteger(command, microstepsToSteps(settings.home.position));
	}

Response CommandProcessor::HandleHW(Command& command) const
	{
	auto position = stepsToMicrosteps(command.StepPosition);
	if (position < 0 || position > settings.home.microstepsPerRotation)
		return Response::Error();
	settings.home.position = position;
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::ForwardToShutter(Command& command) const
	{
	machine.SendToRemoteXbee(command.RawCommand);
	//ToDo: should the response always be successful?
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleCommand(Command& command) const
	{
	if (command.IsShutterCommand())
		{
		ForwardToShutter(command);
		return Response::NoResponse(command);
		}

	if (command.IsRotatorCommand())
		{
		if (command.Verb == "AR") return HandleAR(command); // Read firmware version
		if (command.Verb == "AW") return HandleAW(command); // Read firmware version
		if (command.Verb == "FR") return HandleFR(command); // Read firmware version
		if (command.Verb == "GA") return HandleGA(command); // Goto Azimuth (rotator only)
		if (command.Verb == "GH") return HandleGH(command); // Goto Home Sensor (rotator only)
		if (command.Verb == "HR") return HandleHR(command); // Home position Read (rotator only)
		if (command.Verb == "HW") return HandleHW(command); // Home position Write (rotator only)
		if (command.Verb == "SW") return HandleSW(command); // Stop motor (emergency stop)
		if (command.Verb == "PR") return HandlePR(command); // Position read
		if (command.Verb == "PW") return HandlePW(command); // Position write (sync)
		if (command.Verb == "RR") return HandleRR(command); // Range Read (circumference in steps)
		if (command.Verb == "RW") return HandleRW(command); // Range Write (circumference in steps)
		if (command.Verb == "VR") return HandleVR(command); // Velocity Read (motor speed, steps/s)
		if (command.Verb == "VW") return HandleVW(command); // Velocity Write (motor speed steps/s)
		if (command.Verb == "ZD") return HandleZD(command); // Reset to factory settings (load defaults).
		if (command.Verb == "ZR") return HandleZR(command); // Load settings from persistent storage
		if (command.Verb == "ZW") return HandleZW(command); // Write settings to persistent storage
		}
	if (command.IsSystemCommand())
		{
		// There are currently no system commands
		}
	return Response::Error();
	}


Response CommandProcessor::HandleAR(Command& command) const
	{
	const auto rampTime = settings.motor.rampTimeMilliseconds;
	return Response::FromInteger(command, rampTime);
	}


Response CommandProcessor::HandleGA(Command& command) const
	{
	const auto microstepsPerDegree = settings.home.microstepsPerRotation / 360.0;
	const auto target = targetStepPosition(command.StepPosition * microstepsPerDegree);
	std::cout << "Target " << target << std::endl;
	rotator.MoveToPosition(target);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleGH(Command& command) const
	{
	const auto delta = deltaSteps(settings.home.position);
	if (delta != 0)
		HomeSensor::findHome(sgn(delta));
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleAW(Command& command) const
	{
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		return Response::Error();
	rotator.SetRampTime(rampTime);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleSW(Command& command) const
	{
	rotator.HardStop();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleZW(Command& command) const
	{
	settings.Save();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleZR(Command& command) const
	{
	settings = PersistentSettings::Load();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleZD(Command& command) const
	{
	settings = PersistentSettings();
	settings.Save();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandlePR(Command& command) const
	{
	auto position = microstepsToSteps(rotator.CurrentPosition());
	auto response = Response::FromPosition(command, position);
	return response;
	}

Response CommandProcessor::HandlePW(Command& command) const
	{
	auto microsteps = stepsToMicrosteps(command.StepPosition);
	rotator.SetCurrentPosition(microsteps);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleRW(Command& command) const
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
	settings.home.microstepsPerRotation = microsteps;
	//rotator.SetLimitOfTravel(microsteps);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleRR(Command& command) const
	{
	auto range = microstepsToSteps(settings.home.microstepsPerRotation);
	return Response::FromPosition(command, range);
	}

Response CommandProcessor::HandleFR(Command& command) const
	{
	std::string message;
	message.append("FR");
	message.append(SemanticVersion);
	return Response{message};
	}

Response CommandProcessor::HandleVR(Command& command) const
	{
	auto maxSpeed = rotator.MaximumSpeed();
	return Response::FromPosition(command, microstepsToSteps(maxSpeed));
	}

Response CommandProcessor::HandleVW(Command& command) const
	{
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < rotator.MinimumSpeed())
		return Response::Error();
	rotator.SetMaximumSpeed(speed);
	return Response::FromSuccessfulCommand(command);
	}


/*
 * Computes the final target step position taking into account the shortest movement direction.
 */
int32_t CommandProcessor::targetStepPosition(const uint32_t toMicrostepPosition) const
	{
	return getNormalizedPositionInMicrosteps() + deltaSteps(toMicrostepPosition);
	}

/*
 * Computes the change in step position to reach a target taking into account the shortest movement direction.
 */
int32_t CommandProcessor::deltaSteps(const uint32_t toMicrostepPosition) const
	{
	const auto circumferenceMicrosteps = settings.home.microstepsPerRotation;
	const int32_t halfway = circumferenceMicrosteps / 2;
	const uint32_t fromMicrostepPosition = getNormalizedPositionInMicrosteps();
	int32_t delta = toMicrostepPosition - fromMicrostepPosition;
	if (delta == 0)
		return 0;
	if (delta > halfway)
		delta -= circumferenceMicrosteps;
	if (delta < -halfway)
		delta += circumferenceMicrosteps;
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
	const auto circumferenceMicrosteps = settings.home.microstepsPerRotation;
	auto position = rotator.CurrentPosition();
	while (position < 0)
		position += circumferenceMicrosteps;
	position %= circumferenceMicrosteps;
	return position;
	}

inline int32_t CommandProcessor::getPositionInWholeSteps() const
	{
	return microstepsToSteps(getNormalizedPositionInMicrosteps());
	}

float CommandProcessor::getAzimuth() const
	{
	const auto degreesPerStep = 360.0 / settings.home.microstepsPerRotation;
	return getPositionInWholeSteps() * degreesPerStep;
	}
