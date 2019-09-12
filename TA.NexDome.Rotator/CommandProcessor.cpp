#include "PersistentSettings.h"
#include "CommandProcessor.h"
#include "NexDome.h"
#include "Version.h"
#include <sstream>


CommandProcessor::CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine)
	: rotator(rotator), settings(settings), machine(machine) { }

Response CommandProcessor::HandleDR(Command& command) const
	{
	const auto deadZone = getDeadZoneWholeSteps();
	return Response::FromInteger(command, deadZone);
	}

Response CommandProcessor::HandleDW(Command& command) const
	{
	const auto deadZone = stepsToMicrosteps(command.StepPosition);
	if (deadZone < 0 || deadZone > (1530 * 8))
		return Response::Error();
	settings.deadZone = deadZone;
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleHR(Command& command) const
	{
	return Response::FromInteger(command, microstepsToSteps(settings.home.position));
	}

Response CommandProcessor::HandleHW(Command& command) const
	{
	const auto position = stepsToMicrosteps(command.StepPosition);
	if (position < 0 || position > settings.home.microstepsPerRotation)
		return Response::Error();
	settings.home.position = position;
	return Response::FromSuccessfulCommand(command);
	}

void CommandProcessor::sendStatus() const
	{
	const char separator = ',';
	std::ostringstream status;
	status << std::dec << ":SER,"
		<< getPositionInWholeSteps() << separator
		<< getCircumferenceInWholeSteps() << separator
		<< getHomePositionWholeSteps() << separator
		<< getDeadZoneWholeSteps()
		<< Response::terminator;
	std::cout << status.str() << std::endl;
	}

void CommandProcessor::sendDirection(const int direction)
	{
	static const std::string clockwise = "right";
	static const std::string counterClockwise = "left";
	if (direction < 0)
		std::cout << Response::header << counterClockwise << Response::terminator << std::endl;
	else
		std::cout << Response::header << clockwise << Response::terminator << std::endl;
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
		if (command.Verb == "DR") return HandleDR(command); // Read dead zone (rotator only)
		if (command.Verb == "DW") return HandleDW(command); // Write dead zone (rotator only)
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
		if (command.Verb == "SR") return HandleSR(command);	// Status Request
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
	const auto currentPosition = rotator.getCurrentPosition();
	const auto delta = target - currentPosition;
	const auto direction = sgn(delta);
	std::cout << delta << " [" << settings.deadZone << "]" << std::endl;
	if (abs(delta) >= settings.deadZone)
		{
		sendDirection(direction);
		rotator.moveToPosition(target);
		}
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleGH(Command& command) const
	{
	const auto delta = deltaSteps(settings.home.position);
	const auto direction = sgn(delta);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleAW(Command& command) const
	{
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		return Response::Error();
	rotator.setRampTime(rampTime);
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleSW(Command& command) const
	{
	rotator.hardStop();
	return Response::FromSuccessfulCommand(command);
	}

Response CommandProcessor::HandleSR(Command& command) const
	{
	sendStatus();
	return Response::NoResponse(command);
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
	const auto position = microstepsToSteps(rotator.getCurrentPosition());
	auto response = Response::FromInteger(command, position);
	return response;
	}

Response CommandProcessor::HandlePW(Command& command) const
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
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
	const auto range = microstepsToSteps(settings.home.microstepsPerRotation);
	return Response::FromInteger(command, range);
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
	auto maxSpeed = rotator.getMaximumSpeed();
	return Response::FromInteger(command, microstepsToSteps(maxSpeed));
	}

Response CommandProcessor::HandleVW(Command& command) const
	{
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < rotator.getMinimumSpeed())
		return Response::Error();
	rotator.setMaximumSpeed(speed);
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
	auto position = rotator.getCurrentPosition();
	while (position < 0)
		position += circumferenceMicrosteps;
	position %= circumferenceMicrosteps;
	return position;
	}

inline int32_t CommandProcessor::getPositionInWholeSteps() const
	{
	return microstepsToSteps(getNormalizedPositionInMicrosteps());
	}

int32_t CommandProcessor::getCircumferenceInWholeSteps() const
	{
	return microstepsToSteps(settings.home.microstepsPerRotation);
	}

int32_t CommandProcessor::getHomePositionWholeSteps() const
	{
	return microstepsToSteps(settings.home.position);
	}

float CommandProcessor::getAzimuth() const
	{
	const auto degreesPerStep = 360.0 / settings.home.microstepsPerRotation;
	return getPositionInWholeSteps() * degreesPerStep;
	}

int32_t CommandProcessor::getDeadZoneWholeSteps() const
	{
	return microstepsToSteps(settings.deadZone);
	}

