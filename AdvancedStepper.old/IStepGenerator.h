#pragma once
#include "IStepSequencer.h"

class IStepGenerator
	{
	public:
		virtual void Start(float stepsPerSecond, IStepSequencer *sequencer) = 0;
		virtual void Stop() = 0;
		virtual void SetStepRate(float stepsPerSecond) = 0;
	};

