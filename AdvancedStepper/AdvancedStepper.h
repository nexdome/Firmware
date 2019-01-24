/*
 Name:		AdvancedStepper.h
 Created:	1/23/2019 10:55:41 PM
 Author:	Tim
 Editor:	http://www.visualmicro.com
*/

#ifndef _AdvancedStepper_h
#define _AdvancedStepper_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define BitClear(target,position)	target &= ~(bit(position))
#define BitSet(target,position)		target |= bit(position)
#define BitToggle(target,position)	target ^= bit(position)

#include "IStepGenerator.h"
#include "IStepSequencer.h"
#include "CounterTimer1StepGenerator.h"
#include "MicrosteppingMotor.h"

#endif

