/*
	Accelerated microstepping stepper motor control
	using a 3-pin Step/Direction/Enable interface.
	Works with most stepper drivers e.g. Gecko, Big Easy. etc.

	Note: within this file the following definitions are used:
	Speed - measured in steps per second.
	Velocity - speed in a specified direction. Positive velocity results in increasing step position.
	Distance - measured in motor steps
	Time - measured in seconds
	Acceleration - measured in steps per second per second
*/

#include <ArduinoSTL.h>
#include "MicrosteppingMotor.h"
#include <climits>

// Configures the I/O pins and sets a safe starting state.
void MicrosteppingMotor::InitializeHardware()
	{
	pinMode(stepPin, OUTPUT);
	pinMode(directionPin, OUTPUT);
	pinMode(enablePin, OUTPUT);
	digitalWrite(enablePin, HIGH);
	}

// Creates a new motor instance with the specified I/O pins and step generator.

MicrosteppingMotor::MicrosteppingMotor(uint8_t stepPin, uint8_t enablePin, uint8_t directionPin, IStepGenerator& stepper, MotorSettings& settings)
	{
	configuration = &settings;
	stepGenerator = &stepper;
	currentVelocity = 0;
	this->stepPin = stepPin;
	this->enablePin = enablePin;
	this->directionPin = directionPin;
	minSpeed = MIN_SPEED;
	InitializeHardware();
	stopHandler = nullptr;
	}

/*
The Step method will be called from an interrupt service routine, so
operations must be as short as possible and modify as little state as possible.
*/
void MicrosteppingMotor::Step(bool state)
	{
	digitalWrite(stepPin, state ? HIGH : LOW);
	if (state)
		{
		// Increment position on leading edge.
		configuration->currentPosition += direction;
		}
	else
		{
		// Check hard limits on falling edge
		if (configuration->currentPosition == targetPosition)
			{
			HardStop();
			}
		}
	}

// Energizes the motor coils (applies holding torque) and prepares for stepping.
// Takes account of direction reversal.
void MicrosteppingMotor::EnergizeMotor()
	{
	uint8_t forward = configuration->directionReversed ? HIGH : LOW;
	uint8_t backward = configuration->directionReversed ? LOW : HIGH;
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	digitalWrite(directionPin, direction >= 0 ? forward : backward);
	digitalWrite(enablePin, LOW);	// Active low, so energize the coils.
	}

// Disables the motor coils (releases holding torque).
void MicrosteppingMotor::ReleaseMotor()
	{
	digitalWrite(enablePin, HIGH);	// active low, so de-energize the coils
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	}

/*
 * Registers a method to be called whenever the motor stops.
 */
void MicrosteppingMotor::registerStopHandler(StopHandler handler)
	{
	this->stopHandler = handler;
	}

void MicrosteppingMotor::SetRampTime(uint16_t milliseconds)
	{
	configuration->rampTimeMilliseconds = milliseconds;
	}

/*
	Configures the motor to move to an absolute step position. Unless interrupted,
	the motor will commence stepping at minSpeed and will accelerate uniformly
	to maxSpeed. When nearing the target position, the motor will decelerate uniformly
	down to minSpeed and upon reaching the target position, will perform a hard stop.
	Note: for short moves the motor may never reach maxSpeed.
*/
void MicrosteppingMotor::MoveToPosition(int32_t position)
	{
	int32_t deltaPosition = position - configuration->currentPosition;
	targetPosition = position;
	direction = sgn(deltaPosition);
	targetVelocity = configuration->maxSpeed * direction;
	currentAcceleration = AccelerationFromRampTime() * direction;
	EnergizeMotor();
	startTime = millis();

	if (abs(currentVelocity) < minSpeed)
		{
		// Starting from rest
		startVelocity = minSpeed * direction;
		currentVelocity = startVelocity;
		stepGenerator->Start(minSpeed, this);
		}
	else
		{
		// Starting with the motor already in motion
		startVelocity = currentVelocity;
		stepGenerator->SetStepRate(abs(startVelocity));
		}
	}

/*
	Sets the motor's current step position. This does not cause any movement.
*/
void MicrosteppingMotor::SetCurrentPosition(int32_t position)
	{
	configuration->currentPosition = position;
	}

/*
	Sets the limit of travel (maximum step position) of the motor.
*/
	void MicrosteppingMotor::SetLimitOfTravel(uint32_t limit)
	{
		configuration->maxPosition = limit;
	}

	void MicrosteppingMotor::SetMaximumSpeed(uint16_t speed)
	{
		configuration->maxSpeed = speed;
	}

/*
	Gets the current motor velocity in steps per second.
*/
const float MicrosteppingMotor::CurrentVelocity()
	{
	return currentVelocity;
	}

/*
	Gets the current motor position in steps.
*/
const int32_t MicrosteppingMotor::CurrentPosition()
	{
	return configuration->currentPosition;
	}

const int32_t MicrosteppingMotor::MidpointPosition()
	{
	return configuration->maxPosition / 2;
	}

const int32_t MicrosteppingMotor::LimitOfTravel()
	{
	return configuration->maxPosition;
	}

const uint16_t MicrosteppingMotor::MaximumSpeed()
{
	return configuration->maxSpeed;
}

const uint16_t MicrosteppingMotor::MinimumSpeed()
{
	return minSpeed;
}

const bool MicrosteppingMotor::IsMoving()
	{
	return currentVelocity != 0;
	}

/*
 * Compute the distance (in steps) needed to decelerate to stop (minimum speed),
 * given the current velocity and acceleration in steps per second.
 */
int32_t MicrosteppingMotor::distanceToStop() const
{
	// v² = u² + 2as ∴ s = (v² - u²) / 2a	
	// v is final velocity
	// u is initial (current) velocity
	// a is acceleration
	// v, u, a are in steps per second
	const auto v = 0; //minSpeed * direction;
	const auto u = currentVelocity;
	const auto a = -currentAcceleration;
	const auto s = (v * v - u * u) / (2 * a);
	return int(s);
}

/*
	Computes the linear acceleration required to accelerate from rest to the maximum
	speed in the ramp time. The returned value is always positive.
	From v = u + at; since u is 0, v = at where t is the ramp time. Therefore, a = v/t.
*/
float MicrosteppingMotor::AccelerationFromRampTime()
	{
	float rampTimeSeconds = (float)(configuration->rampTimeMilliseconds) / 1000.0;
	float acceleration = (float)(configuration->maxSpeed) / rampTimeSeconds;
	return acceleration;
	}

/*
	Computes the theoretical accelerated velocity assuming uniform acceleration since start time.
	v = u + at
	u = startVelocity, a is acceleration, t is elapsed time since start
*/
float MicrosteppingMotor::AcceleratedVelocity()
	{
	float elapsedTime = (millis() - startTime) / 1000.0;
	float acceleratedVelocity = startVelocity + currentAcceleration * elapsedTime; // v = u + at
	return acceleratedVelocity;
	}

/*
	Computes the maximum velocity that will still allow the motor to decelerate to minSpeed
	before reaching the target position. We do this by computing what the velocity would have been
	if we had started at the target position and accelerated back for n steps, then changing the sign of
	that velocity to match the current direction of travel.
	v² = u² + 2as
	u = minSpeed, a = |acceleration|, s = steps still to go
	|v| = √(u² + 2as) (positive root)
	maximum velocity = v * direction
*/
float MicrosteppingMotor::DeceleratedVelocity()
	{
	auto current = (int32_t)configuration->currentPosition;
	auto target = (int32_t)targetPosition;
	int32_t deltaSteps = target - current;
	uint32_t stepsToGo = abs(deltaSteps);
	auto acceleration = fabs(currentAcceleration);
	auto uSquared = minSpeed * minSpeed;
	auto vSquared = uSquared + 2.0 * acceleration * stepsToGo;
	auto speed = sqrt(vSquared);
	auto velocity = speed * direction;
	return velocity;
	}

/*
	Brings the motor to an immediate hard stop.
*/
void MicrosteppingMotor::HardStop()
	{
	stepGenerator->Stop();
	currentAcceleration = 0;
	currentVelocity = 0;
	direction = 0;
	ReleaseMotor();
	if (stopHandler != nullptr)
		stopHandler();
	}

/*
 * Decelerate to a stop in the shortest distance allowed by the current acceleration.
 */
void MicrosteppingMotor::SoftStop()
{
	const auto current = CurrentPosition();
	const auto distance = distanceToStop();
	targetPosition = current + distance;
	std::cout << std::dec << "current " << current << " dist " << distance << " target " << targetPosition << std::endl;
}

void MicrosteppingMotor::Loop()
	{
	if (MicrosteppingMotor::IsMoving())
		ComputeAcceleratedVelocity();
	}

/*
	Recomputes the current motor velocity. Call this from within the main loop.
*/
void MicrosteppingMotor::ComputeAcceleratedVelocity()
	{
	float accelerationCurve = AcceleratedVelocity();
	float decelerationCurve = DeceleratedVelocity();
	float computedSpeed = min(abs(accelerationCurve), abs(decelerationCurve));
	float constrainedSpeed = constrain(computedSpeed, minSpeed, configuration->maxSpeed);
	currentVelocity = constrainedSpeed * direction;
	stepGenerator->SetStepRate(constrainedSpeed);	// Step rate must be positive
	}
