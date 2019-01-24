// CounterTimer1.h

#ifndef _COUNTERTIMER1_h
#define _COUNTERTIMER1_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IStepGenerator.h"

#define Fmax	(16000000)
#define Fmin	(244.1443503)

class CounterTimer1StepGenerator : public IStepGenerator
	{
	 protected:
		 void Initialize();
		 uint16_t ComputeCountsFromStepsPerSecond(float frequency);

	 public:
		 CounterTimer1StepGenerator();
		 virtual void Start(float stepsPerSecond, IStepSequencer *sequencer) final;
		 virtual void Stop() final;
		 virtual void SetStepRate(float stepsPerSecond) final;
		 static void TimerCompareInterruptService();

	private:
		static IStepSequencer *activeSequencer;
		static uint16_t nextCompareValue;
		static bool stepState;
	};

#endif

