/*
 Name:		Timer.cpp
 Created:	2/5/2019 1:56:49 PM
 Author:	Tim Long, Tigra Astronomy
 Editor:	http://www.visualmicro.com
*/

#include "Timer.h"

Timer::Timer()
	{
	startedAt = 0;
	interval = 0;
	}

/*
 * Sets the timer interval and establishes the reference datum as the instant when this method was called.
 */
void Timer::SetDuration(unsigned long duration)
	{
	startedAt = millis();
	interval = duration;
	}

unsigned long Timer::Elapsed() const
	{
	if (!Enabled())
		return 0UL;
	return millis() - startedAt;
	}

unsigned long Timer::Remaining() const
	{
	return interval - Elapsed();
}

bool Timer::Expired() const
	{
	return Enabled() ? Elapsed() >= interval : false;
	}


bool Timer::Enabled() const
	{
	return interval > 0;
}

void Timer::Stop()
{
	interval = 0;
}



