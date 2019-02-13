/*
 Name:		Timer.h
 Created:	2/5/2019 1:56:49 PM
 Author:	Tim
 Editor:	http://www.visualmicro.com
*/

#ifndef _Timer_h
#define _Timer_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class Timer
	{
public:
	Timer();
	void SetDuration(unsigned long duration);
	bool Expired();
	unsigned long Elapsed();
	unsigned long Remaining();
	void Repeat();
private:
	unsigned long startedAt;
	unsigned long interval;
	};


#endif

