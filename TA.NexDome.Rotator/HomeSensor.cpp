/*
 * Provides interrupt processing for the home sensor.
 *
 * The home sensor synchronizes the dome rotation step position when it is triggered.
 * When rotating in the positive direction, we synchronise to the falling edge.
 * When rotating in the negative direction, we synchronise to the rising edge.
 * When not rotating, activity is ignored.
 *
 * Note: some fields have to be static because they are used during interrupts
 */

#include "NexDome.h"
#include "HomeSensor.h"

#pragma region static fields used within interrupt service routines
MicrosteppingMotor* HomeSensor::motor;
Home* HomeSensor::homeSettings;
uint8_t HomeSensor::sensorPin;
volatile HomingPhase HomeSensor::phase;
#pragma endregion

/*
 * Creates a new HomeSensor instance.
 * Note: sensorPin must correspond to a digital input pin that is valid for attaching interrupts.
 * Not all pins on all platforms support attaching interrupts.
 * Arduino Leonardo supports pins 0, 1, 2, 3, 7
 */
HomeSensor::HomeSensor(MicrosteppingMotor* stepper, Home* settings, const uint8_t sensorPin, CommandProcessor& processor)
	: commandProcessor(processor)
	{
	motor = stepper;
	HomeSensor::homeSettings = settings;
	HomeSensor::sensorPin = sensorPin;
	}


/*
 * Triggered as an interrupt whenever the home sensor pin changes state.
 * Synchronizes the current motor stop position to the calibrated home position.
 */
void HomeSensor::onHomeSensorChanged()
	{
	const auto state = digitalRead(sensorPin);
	if (state == 0 && phase==Detecting)
		foundHome();
	}

/*
 * Configures the hardware pin ready for use and attaches the interrupt.
 */
void HomeSensor::init()
	{
	pinMode(sensorPin, INPUT_PULLUP);
	setPhase(Idle);
	attachInterrupt(digitalPinToInterrupt(sensorPin), onHomeSensorChanged, CHANGE);
	}


/// <summary>
/// Indicates whether the dome is currently in the home position
/// (only valid after a successful homing operation and before and slews occur)
/// </summary>
bool HomeSensor::atHome()
	{
	return phase == AtHome;
	}

void HomeSensor::setPhase(HomingPhase newPhase)
	{
	phase = newPhase;
	Serial.print("Phase ");
	Serial.println(phase);
	}

/*
 * Rotates up to 2 full rotations clockwise while attempting to detect the home sensor.
 * Ignored if a homing operation is already in progress.
 */
void HomeSensor::findHome(int direction)
	{
	if (phase == Idle || phase == AtHome)
		{
		const auto distance = 2 * homeSettings->microstepsPerRotation;	// Allow 2 full rotations only
		setPhase(Detecting);
		motor->moveToPosition(distance);
		}
	}

/*
 * Stops a homing operation in progress.
 */
void HomeSensor::cancelHoming()
	{
	setPhase(Idle);
	if (motor->isMoving())
		motor->SoftStop();
	}

/*
 * Once the home sensor has been detected, we instruct the motor to soft-stop.
 * We also set the flag performPostHomeSlew.
 * At some point in the future, the onMotorStopped method will be called, which will
 * then initiate the final slew to return exactly to the home sensor position.
 */
void HomeSensor::foundHome()
	{
	setPhase(Stopping);
	motor->SetCurrentPosition(homeSettings->position);
	motor->SoftStop();
	}

/*
 * Handles the onMotorStopped event. Action depends on the homing phase.
 */
void HomeSensor::onMotorStopped() const
	{
	std::cout << "Hstop " << phase << std::endl;
	switch (phase)
		{
		case Reversing:
			setPhase(AtHome);
			break;
		case Stopping:
			setPhase(Reversing);
			const auto target = commandProcessor.targetStepPosition(homeSettings->position);
			motor->moveToPosition(target);
			break;
		default:
			setPhase(Idle);
			return;
		}
	}

bool HomeSensor::homingInProgress()
	{
	return !(phase == Idle || phase == AtHome);
	}
