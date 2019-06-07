// CounterTimer1.h

#ifndef _COUNTERTIMER1_h
#define _COUNTERTIMER1_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IStepGenerator.h"

#define Fmax	(F_CPU) 
#define Fmin	(F_CPU / 65535L)

class CounterTimer1StepGenerator : public IStepGenerator
	{
	 protected:
		 void Initialize();
		 uint16_t computeCountsFromStepsPerSecond(float frequency);

	 public:
		 CounterTimer1StepGenerator();
		 virtual void start(float stepsPerSecond, IStepSequencer *sequencer) final;
		 virtual void stop() final;
		 virtual void setStepRate(float stepsPerSecond) final;
		 static void timerCompareInterruptService();

	private:
		static IStepSequencer *activeSequencer;
		static uint16_t nextCompareValue;
		static bool stepState;
	};

#endif

