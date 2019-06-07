// MovingAverage.h

#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>

template <class SampleType>
class MovingAverage
	{
	private:
		std::vector<SampleType> samples;
		SampleType runningTotal;
		int numSamples, addIndex, removeIndex;
	public:
		void init(SampleType initialValue);
		MovingAverage<SampleType>();
		explicit MovingAverage<SampleType>(int sampleWindow);
		SampleType addSample(SampleType sample);
		SampleType average;
	};

// Implementation

template <class SampleType>
void MovingAverage<SampleType>::init(SampleType initialValue)
	{
	samples.clear();
	for (auto i = 0; i < numSamples; i++)
		{
		samples.push_back(initialValue);
		}
	runningTotal = initialValue * numSamples;
	average = initialValue;
	}

template<class SampleType>
MovingAverage<SampleType>::MovingAverage() : MovingAverage<SampleType>(5)
	{
	}

template<class SampleType>
MovingAverage<SampleType>::MovingAverage(int sampleWindow)
	{
	numSamples = sampleWindow;
	samples = std::vector<SampleType>();
	init(SampleType());
	}

template<class SampleType>
SampleType MovingAverage<SampleType>::addSample(SampleType sample)
	{
	runningTotal -= samples.front();
	runningTotal += sample;
	samples.erase(samples.begin());
	samples.push_back(sample);
	average = runningTotal / numSamples;
	return average;
	}
#endif

