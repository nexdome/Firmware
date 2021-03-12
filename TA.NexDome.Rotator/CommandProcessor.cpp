#include "PersistentSettings.h"
#include "CommandProcessor.h"
#include "NexDome.h"
#include "Version.h"
#include <sstream>


CommandProcessor::CommandProcessor(MicrosteppingMotor& rotator, PersistentSettings& settings, XBeeStateMachine& machine)
	: rotator(rotator), settings(settings), machine(machine) { }

/*
 * Sends an encapsulated response to the host application
 */
void CommandProcessor::responseToHost(const std::string& rxMessage)
	{
	auto length = rxMessage.length();
	if (length < 1) return;
	if (rxMessage[0] != ':')
		std::cout << ':';
	std::cout << rxMessage;
	if (rxMessage[length - 1] != '#')
		std::cout << '#';
	std::cout << std::endl;
	}

void CommandProcessor::HandleZZ(const Command& command) const
	{
	void(*resetFunc) (void) = nullptr; //declare reset function at address 0
	resetFunc(); //call reset
	}

void CommandProcessor::HandleDR(const Command& command) const
	{
	const auto deadZone = getDeadZoneWholeSteps();
	ResponseBuilder::FromInteger(command, deadZone);
	}

void CommandProcessor::HandleDW(const Command& command) const
	{
	const auto deadZone = stepsToMicrosteps(command.StepPosition);
	if (deadZone < 0 || deadZone > (1530 * 8))
		ResponseBuilder::Error();
	settings.deadZone = deadZone;
	ResponseBuilder::FromSuccessfulCommand(command);
	}

void CommandProcessor::HandleHR(const Command& command) const
	{
	ResponseBuilder::FromInteger(command, microstepsToSteps(settings.home.position));
	}

void CommandProcessor::HandleHW(const Command& command) const
	{
	const auto position = stepsToMicrosteps(command.StepPosition);
	if (position < 0 || position > settings.home.microstepsPerRotation)
		ResponseBuilder::Error();
	settings.home.position = position;
	ResponseBuilder::FromSuccessfulCommand(command);
	}

void CommandProcessor::sendStatus() const
	{
	if (HomeSensor::homingInProgress())
		return;
	const char separator = ',';
	std::ostringstream status;
	status << std::dec << ":SER,"
		<< getPositionInWholeSteps() << separator
		<< HomeSensor::atHome() << separator
		<< getCircumferenceInWholeSteps() << separator
		<< getHomePositionWholeSteps() << separator
		<< getDeadZoneWholeSteps()
		<< ResponseBuilder::terminator;
	std::cout << status.str() << std::endl;
	}

void CommandProcessor::sendDirection(const int direction)
	{
	static const std::string clockwise = "right";
	static const std::string counterClockwise = "left";
	if (direction < 0)
		std::cout << ResponseBuilder::header << counterClockwise << ResponseBuilder::terminator << std::endl;
	else
		std::cout << ResponseBuilder::header << clockwise << ResponseBuilder::terminator << std::endl;
	}



void CommandProcessor::ForwardToShutter(const Command& command) const
	{
	machine.SendToRemoteXbee(command.RawCommand);
	ResponseBuilder::FromSuccessfulCommand(command);
	}

void CommandProcessor::HandleCommand(const Command& command) const
	{
	ResponseBuilder::FromSuccessfulCommand(command); // This is the default response unless overwritten below
	if (command.IsShutterCommand())
		{
		ForwardToShutter(command);
		ResponseBuilder::NoResponse(command);	// The shutter will send a response asynchronously.
		}

	if (command.IsRotatorCommand())
		{
		if (command.Verb == "AR") return HandleAR(command); // Read acceleration ramp time (ms)
		if (command.Verb == "AW") return HandleAW(command); // Write acceleration ramp time (ms)
		if (command.Verb == "DR") return HandleDR(command); // Read dead zone (rotator only)
		if (command.Verb == "DW") return HandleDW(command); // Write dead zone (rotator only)
		if (command.Verb == "FR") return HandleFR(command); // Read firmware version
		if (command.Verb == "GA") return HandleGA(command); // Goto Azimuth (rotator only)
		if (command.Verb == "GH") return HandleGH(command); // Goto Home Sensor (rotator only)
		if (command.Verb == "GS") return HandleGS(command); // Goto step position (rotator only)
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
		if (command.Verb == "ZZ") return HandleZZ(command); // Reboot immediately
		}
	}


void CommandProcessor::HandleAR(const Command& command) const
	{
	const auto rampTime = settings.motor.rampTimeMilliseconds;
	ResponseBuilder::FromInteger(command, rampTime);
	}


void CommandProcessor::rotateToMicrostepPosition(const int32_t target) const {
const auto currentPosition = rotator.getCurrentPosition();
const auto delta = target - currentPosition;
const auto direction = sgn(delta);
if (abs(delta) >= settings.deadZone)
	{
	HomeSensor::cancelHoming();
	sendDirection(direction);
	rotator.moveToPosition(target);
	}
}

void CommandProcessor::HandleGA(const Command& command) const
	{
	const auto microstepsPerDegree = settings.home.microstepsPerRotation / 360.0;
	const auto target = targetStepPosition(command.StepPosition * microstepsPerDegree);
	rotateToMicrostepPosition(target);
	}

void CommandProcessor::HandleGS(const Command& command) const
	{
	const auto target = targetStepPosition(stepsToMicrosteps(command.StepPosition));
	rotateToMicrostepPosition(target);
	}

void CommandProcessor::HandleGH(const Command& command) const
	{
	const auto delta = deltaSteps(settings.home.position);
	const auto direction = sgn(delta);
	HomeSensor::findHome(direction == 0 ? 1 : direction);
	}

void CommandProcessor::HandleAW(const Command& command) const
	{
	auto rampTime = command.StepPosition;
	// The minimum ramp time is 100ms, fail if the user tries to set it lower.
	if (rampTime < MIN_RAMP_TIME)
		ResponseBuilder::Error();
	rotator.setRampTime(rampTime);
	}

void CommandProcessor::HandleSW(const Command& command) const
	{
	rotator.hardStop();
	}

void CommandProcessor::HandleSR(const Command& command) const
	{
	sendStatus();
	ResponseBuilder::NoResponse(command);
	}

void CommandProcessor::HandleZW(const Command& command) const
	{
	settings.Save();
	}

void CommandProcessor::HandleZR(const Command& command) const
	{
	settings = PersistentSettings::Load();
	}

void CommandProcessor::HandleZD(const Command& command) const
	{
	settings = PersistentSettings();
	settings.Save();
	}

void CommandProcessor::HandlePR(const Command& command) const
	{
	const auto position = microstepsToSteps(rotator.getCurrentPosition());
	ResponseBuilder::FromInteger(command, position);
	}

void CommandProcessor::HandlePW(const Command& command) const
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
	rotator.SetCurrentPosition(microsteps);
	}

void CommandProcessor::HandleRW(const Command& command) const
	{
	const auto microsteps = stepsToMicrosteps(command.StepPosition);
	settings.home.microstepsPerRotation = microsteps;
	}

void CommandProcessor::HandleRR(const Command& command) const
	{
	const auto range = microstepsToSteps(settings.home.microstepsPerRotation);
	ResponseBuilder::FromInteger(command, range);
	}

void CommandProcessor::HandleFR(const Command& command) const
	{
	ResponseBuilder::FromString(command, SemanticVersion);
	}

void CommandProcessor::HandleVR(const Command& command) const
	{
	auto maxSpeed = rotator.getMaximumSpeed();
	ResponseBuilder::FromInteger(command, microstepsToSteps(maxSpeed));
	}

void CommandProcessor::HandleVW(const Command& command) const
	{
	uint16_t speed = stepsToMicrosteps(command.StepPosition);
	if (speed < rotator.getMinimumSpeed())
		ResponseBuilder::Error();
	rotator.setMaximumSpeed(speed);
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

