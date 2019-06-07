#pragma once
#include "IStepSequencer.h"

class IStepGenerator
	{
	public:
		virtual void start(float stepsPerSecond, IStepSequencer *sequencer) = 0;
		virtual void stop() = 0;
		virtual void setStepRate(float stepsPerSecond) = 0;
	};

