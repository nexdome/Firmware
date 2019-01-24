/*
	Accelerated stepper motor control for Integra 85.

	Note: within this file the following definitions are used:
	Speed - measured in steps per second.
	Velocity - speed in a specified direction. Positive velocity results in increasing step position.
	Distance - measured in motor steps
	Time - measured in seconds
	Acceleration - measured in steps per second per second
*/

#include "Motor.h"

// Configures the I/O pins and sets a safe starting state.
void Motor::InitializeHardware()
	{
	pinMode(stepPin, OUTPUT);
	pinMode(directionPin, OUTPUT);
	pinMode(enablePin, OUTPUT);
	digitalWrite(enablePin, HIGH);
	}

// Creates a new motor instance with the specified I/O pins and step generator.
Motor::Motor(uint8_t stepPin, uint8_t enablePin, uint8_t directionPin)
	{
	this->stepPin = stepPin;
	this->enablePin = enablePin;
	this->directionPin = directionPin;
	minSpeed = MIN_SPEED;
	InitializeHardware();
	}

Motor::Motor(uint8_t stepPin, uint8_t enablePin, uint8_t directionPin, IStepGenerator& stepper, MotorSettings& settings)
	: Motor(stepPin,enablePin,directionPin)
	{
	configuration = &settings;
	stepGenerator = &stepper;
	currentVelocity = 0;
	}

/*
The Step method will be called from an interrupt service routine, so
operations must be a short as possible and modify as little state as possible.
*/
void Motor::Step(bool state)
	{
	digitalWrite(stepPin, state ? HIGH : LOW);
	if (state)
		{
		// Increment position on leading edge.
		configuration->currentPosition += state ? direction : 0;
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

// Moves the motor with the specified velocity. Movement continues until stopped or a hard limit is reached.
void Motor::MoveAtVelocity(float stepsPerSecond)
	{
	auto absoluteStepsPerSecond = abs(stepsPerSecond);
	direction = sgn(stepsPerSecond);
	targetPosition = direction > 0 ? configuration->maxPosition : 0;
	targetVelocity = stepsPerSecond;
	currentAcceleration = AccelerationFromRampTime() * direction;
	EnergizeMotor();
	startTime = millis();
	if (currentVelocity == 0)
		stepGenerator->Start(absoluteStepsPerSecond, this);
	else
		stepGenerator->SetStepRate(absoluteStepsPerSecond);
	}

// Energizes the motor coils (applies holding torque) and prepares for stepping.
void Motor::EnergizeMotor()
	{
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	digitalWrite(directionPin, direction >= 0 ? LOW : HIGH);
	digitalWrite(enablePin, LOW);	// Active low, so energize the coils.
	}

// Disables the motor coils (releases holding torque).
void Motor::ReleaseMotor()
	{
	digitalWrite(enablePin, HIGH);	// active low, so de-energize the coils
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	}

void Motor::SetRampTime(uint16_t milliseconds)
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
void Motor::MoveToPosition(int32_t position)
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
void Motor::SetCurrentPosition(int32_t position)
	{
	configuration->currentPosition = position;
	}

/*
	Sets the limit of travel (maximum step position) of the motor.
*/
	void Motor::SetLimitOfTravel(uint32_t limit)
	{
		configuration->maxPosition = limit;
	}

	void Motor::SetMaximumSpeed(uint16_t speed)
	{
		configuration->maxSpeed = speed;
	}

/*
	Gets the current motor velocity in steps per second.
*/
const float Motor::CurrentVelocity()
	{
	return currentVelocity;
	}

/*
	Gets the current motor position in steps.
*/
const int32_t Motor::CurrentPosition()
	{
	return configuration->currentPosition;
	}

const int32_t Motor::MidpointPosition()
	{
	return configuration->maxPosition / 2;
	}

const int32_t Motor::LimitOfTravel()
	{
	return configuration->maxPosition;
	}

const uint16_t Motor::MaximumSpeed()
{
	return configuration->maxSpeed;
}

const uint16_t Motor::MinimumSpeed()
{
	return minSpeed;
}

const bool Motor::IsMoving()
	{
	return currentVelocity != 0;
	}

/*
	Computes the linear acceleration required to accelerate from rest to the maximum
	speed in the ramp time. The returned value is always positive.
	From v = u + at; since u is 0, v = at where t is the ramp time. Therefore, a = v/t.
*/
float Motor::AccelerationFromRampTime()
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
float Motor::AcceleratedVelocity()
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
float Motor::DeceleratedVelocity()
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
void Motor::HardStop()
	{
	stepGenerator->Stop();
	currentAcceleration = 0;
	currentVelocity = 0;
	direction = 0;
	ReleaseMotor();
	}

void Motor::Loop()
	{
	if (Motor::IsMoving())
		ComputeAcceleratedVelocity();
	}

/*
	Recomputes the current motor velocity. Call this from within the main loop.
*/
void Motor::ComputeAcceleratedVelocity()
	{
	float accelerationCurve = AcceleratedVelocity();
	float decelerationCurve = DeceleratedVelocity();
	float computedSpeed = min(abs(accelerationCurve), abs(decelerationCurve));
	float constrainedSpeed = constrain(computedSpeed, minSpeed, configuration->maxSpeed);
	currentVelocity = constrainedSpeed * direction;
	stepGenerator->SetStepRate(constrainedSpeed);	// Step rate must be positive
	}
