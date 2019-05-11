// Motor.h

#ifndef _MicrosteppingMotor_h
#define _MicrosteppingMotor_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IStepSequencer.h"
#include "IStepGenerator.h"

struct MotorSettings
	{
	uint32_t maxPosition;				// limit of travel, in steps
	volatile int32_t currentPosition;	// the current position (potentially updated by ISR)
	uint16_t rampTimeMilliseconds;		// milliseconds to ramp from minSpeed to maxSpeed
	uint16_t maxSpeed;					// maximum number of steps per second
	bool directionReversed;				// If true, reverses the rotation direction with respect to the step position
	};

typedef void (*StopHandler) ();


class MicrosteppingMotor : public IStepSequencer
	{
	public:
		MicrosteppingMotor(uint8_t stepPin, uint8_t enablePin, uint8_t directionPin, IStepGenerator& stepper, MotorSettings& settings);
		virtual void Step(bool state) final;
		//void MoveAtVelocity(float stepsPerSecond);
		void EnergizeMotor();
		void ReleaseMotor();
		void registerStopHandler(StopHandler handler);
		void SetRampTime(uint16_t milliseconds);
		virtual void HardStop();
		virtual void SoftStop();
		virtual void Loop();
		void ComputeAcceleratedVelocity();
		virtual void MoveToPosition(int32_t position);
		void SetCurrentPosition(int32_t position);
		void SetLimitOfTravel(uint32_t limit);
		void SetMaximumSpeed(uint16_t speed);
		const float CurrentVelocity();
		virtual const int32_t CurrentPosition();
		const int32_t MidpointPosition();
		const int32_t LimitOfTravel();
		const uint16_t MaximumSpeed();
		const uint16_t MinimumSpeed();
		virtual bool IsMoving();
		virtual int8_t currentDirection();
		int32_t distanceToStop() const;

protected:
		MotorSettings *configuration;

	private:
		uint8_t stepPin, enablePin, directionPin;
		bool directionReversed;
		IStepGenerator *stepGenerator;
		int direction = +1;
		int32_t targetPosition{};
		unsigned long startTime{};
		float startVelocity{}, currentVelocity, targetVelocity{}, currentAcceleration{};
		float minSpeed;
		void InitializeHardware();
		float AcceleratedVelocity();
		float DeceleratedVelocity();
		float AccelerationFromRampTime();
		StopHandler stopHandler;
	};

// Motor Parameters (defaults)
#ifndef MICROSTEPS_PER_STEP
#define MICROSTEPS_PER_STEP			(16) 
#endif

#ifndef MIN_SPEED
#define MIN_SPEED					(250)	// Minimum speed that can be timed by the hardware timer
#endif

#ifndef MIN_RAMP_TIME
#define MIN_RAMP_TIME				(100)	// Minimum ramp up/down time in milliseconds
#endif

template <typename T> int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

#endif

