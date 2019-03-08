/*
 Name:		Timer.cpp
 Created:	2/5/2019 1:56:49 PM
 Author:	Tim Long, Tigra Astronomy
 Editor:	http://www.visualmicro.com
*/

#include "Timer.h"
extern unsigned long millis();	// VMicro doesn't correctly detect this, so manually declare it.

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

unsigned long Timer::Elapsed()
	{
	if (!Enabled())
		return 0;
	unsigned long elapsed = millis() - startedAt;
	Serial.print(startedAt);
	Serial.print(',');
	Serial.print(interval);
	Serial.print(',');
	Serial.print(millis());
	Serial.print(',');
	Serial.println(elapsed);
	return elapsed;
	}

unsigned long Timer::Remaining()
{
	return interval - Elapsed();
}

bool Timer::Expired()
	{
	return Enabled() ? Elapsed() >= interval : false;
	}

/*
 * Repeats the timer interval but does not reset the reference datum.
 * This should be used when repeatedly timing the same interval, and it is
 * intended that the intervals be consistent _on average_.
 */
void Timer::Repeat()
	{
	startedAt += interval;
	}

bool Timer::Enabled()
{
	return interval > 0;
}

void Timer::Stop()
{
	interval = 0;
}



