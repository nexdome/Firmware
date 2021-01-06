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

#include <Arduino.h>
#include "MicrosteppingMotor.h"

// Configures the I/O pins and sets a safe starting state.
void MicrosteppingMotor::initializeHardware() const
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
	initializeHardware();
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
			hardStop();
			}
		}
	}

// Energizes the motor coils (applies holding torque) and prepares for stepping.
// Takes account of direction reversal.
void MicrosteppingMotor::energizeMotor() const
	{
	uint8_t forward = configuration->directionReversed ? HIGH : LOW;
	uint8_t backward = configuration->directionReversed ? LOW : HIGH;
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	digitalWrite(directionPin, direction >= 0 ? forward : backward);
	digitalWrite(enablePin, LOW);	// Active low, so energize the coils.
	}

// Disables the motor coils (releases holding torque).
void MicrosteppingMotor::releaseMotor()
	{
	digitalWrite(enablePin, HIGH);	// active low
	digitalWrite(stepPin, LOW);		// active high, so ensure we are not commanding a step.
	}

/*
 * Registers a method to be called whenever the motor stops.
 */
void MicrosteppingMotor::registerStopHandler(StopHandler handler)
	{
	this->stopHandler = handler;
	}

void MicrosteppingMotor::setRampTime(uint16_t milliseconds)
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
void MicrosteppingMotor::moveToPosition(int32_t position)
	{
	const int32_t deltaPosition = position - configuration->currentPosition;
	targetPosition = position;
	direction = sgn(deltaPosition);
	targetVelocity = configuration->maxSpeed * direction;
	currentAcceleration = accelerationFromRampTime() * direction;
	energizeMotor();
	startTime = millis();

	if (abs(currentVelocity) < minSpeed)
		{
		// Starting from rest
		startVelocity = minSpeed * direction;
		currentVelocity = startVelocity;
		stepGenerator->start(minSpeed, this);
		}
	else
		{
		// Starting with the motor already in motion
		startVelocity = currentVelocity;
		stepGenerator->setStepRate(abs(startVelocity));
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

void MicrosteppingMotor::setMaximumSpeed(uint16_t speed)
	{
	configuration->maxSpeed = speed;
	}

/*
	Gets the current motor velocity in steps per second.
*/
float MicrosteppingMotor::getCurrentVelocity() const
	{
	return currentVelocity;
	}

/*
	Gets the current motor position in steps.
*/
int32_t MicrosteppingMotor::getCurrentPosition()
	{
	return configuration->currentPosition;
	}

int32_t MicrosteppingMotor::midpointPosition() const
	{
	return configuration->maxPosition / 2;
	}

int32_t MicrosteppingMotor::limitOfTravel() const
	{
	return configuration->maxPosition;
	}

uint16_t MicrosteppingMotor::getMaximumSpeed()
	{
	return configuration->maxSpeed;
	}

uint16_t MicrosteppingMotor::getMinimumSpeed()
	{
	return minSpeed;
	}

bool MicrosteppingMotor::isMoving()
	{
	return currentVelocity != 0;
	}

/**
 * Gets the last direction of travel.
 * Returns +1 for travel in increasing step position, -1 for decreasing step position.
 * May return 0 if not moving, but isMoving() is the preferred method to check for motion.
 */
inline int8_t MicrosteppingMotor::getCurrentDirection() { return direction; }

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
float MicrosteppingMotor::accelerationFromRampTime()
	{
	const float rampTimeSeconds = float(configuration->rampTimeMilliseconds) / 1000.0;
	const float acceleration = float(configuration->maxSpeed) / rampTimeSeconds;
	return acceleration;
	}

/*
	Computes the theoretical accelerated velocity assuming uniform acceleration since start time.
	v = u + at
	u = startVelocity, a is acceleration, t is elapsed time since start
*/
float MicrosteppingMotor::getAcceleratedVelocity() const
	{
	const float elapsedTime = (millis() - startTime) / 1000.0;
	const float acceleratedVelocity = startVelocity + currentAcceleration * elapsedTime; // v = u + at
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
float MicrosteppingMotor::getDeceleratedVelocity() const
	{
	const auto current = int32_t(configuration->currentPosition);
	const auto target = int32_t(targetPosition);
	const int32_t deltaSteps = target - current;
	const uint32_t stepsToGo = abs(deltaSteps);
	const auto acceleration = fabs(currentAcceleration);
	const auto uSquared = minSpeed * minSpeed;
	const auto vSquared = uSquared + 2.0 * acceleration * stepsToGo;
	const auto speed = sqrt(vSquared);
	const auto velocity = speed * direction;
	return velocity;
	}

/*
	Brings the motor to an immediate hard stop.
*/
void MicrosteppingMotor::hardStop()
	{
	stepGenerator->stop();
	currentAcceleration = 0;
	currentVelocity = 0;
	direction = 0;
	if (!configuration->useHoldingTorque)
		releaseMotor();
	if (stopHandler != nullptr)
		stopHandler();
	}

/*
 * Decelerate to a stop in the shortest distance allowed by the current acceleration.
 */
void MicrosteppingMotor::SoftStop()
	{
	if (!isMoving()) return;
	const auto current = getCurrentPosition();
	const auto distance = distanceToStop();
	targetPosition = current + distance;
	}

void MicrosteppingMotor::loop()
	{
	if (isMoving())
		ComputeAcceleratedVelocity();
	}

/*
	Recomputes the current motor velocity. Call this from within the main loop.
*/
void MicrosteppingMotor::ComputeAcceleratedVelocity()
	{
	const float accelerationCurve = getAcceleratedVelocity();
	const float decelerationCurve = getDeceleratedVelocity();
	const float computedSpeed = min(abs(accelerationCurve), abs(decelerationCurve));
	const float constrainedSpeed = constrain(computedSpeed, minSpeed, configuration->maxSpeed);
	currentVelocity = constrainedSpeed * direction;
	stepGenerator->setStepRate(constrainedSpeed);	// Step rate must be positive
	}
