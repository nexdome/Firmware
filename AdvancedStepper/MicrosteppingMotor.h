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
		void energizeMotor() const;
		void releaseMotor();
		void registerStopHandler(StopHandler handler);
		void setRampTime(uint16_t milliseconds);
		virtual void hardStop();
		virtual void SoftStop();
		virtual void loop();
		void ComputeAcceleratedVelocity();
		virtual void moveToPosition(int32_t position);
		void SetCurrentPosition(int32_t position);
		void SetLimitOfTravel(uint32_t limit);
		void setMaximumSpeed(uint16_t speed);
		float getCurrentVelocity() const;
		virtual int32_t getCurrentPosition();
	int32_t midpointPosition() const;
	int32_t limitOfTravel() const;
	uint16_t getMaximumSpeed();
	uint16_t getMinimumSpeed();
		virtual bool isMoving();
		virtual int8_t getCurrentDirection();
		int32_t distanceToStop() const;

	protected:
		MotorSettings* configuration;

	private:
		uint8_t stepPin, enablePin, directionPin;
		IStepGenerator* stepGenerator;
		int direction = +1;
		int32_t targetPosition{};
		unsigned long startTime{};
		float startVelocity{}, currentVelocity, targetVelocity{}, currentAcceleration{};
		float minSpeed;
		void initializeHardware() const;
		float getAcceleratedVelocity() const;
		float getDeceleratedVelocity() const;
		float accelerationFromRampTime();
		StopHandler stopHandler;
	};

// Motor Parameters (defaults)

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

