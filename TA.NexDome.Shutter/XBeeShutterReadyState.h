// XBeeShutterReadyState.h

#ifndef _XBEESHUTTERREADYSTATE_h
#define _XBEESHUTTERREADYSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "../TA.NexDome.Rotator/XBeeStateMachine.h"

class XBeeAssociatedState : public IXBeeState
	{
	String name() override { return "Ready"; }
	};

#endif

