// XBeeOnlineState.h

#ifndef _XBEEONLINESTATE_h
#define _XBEEONLINESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "XBeeStateMachine.h"

class XBeeOnlineState : public IXBeeState
	{
public:
	explicit XBeeOnlineState(XBeeStateMachine& machine) : IXBeeState(machine) {  }
	String name() override { return "online"; }
	};

#endif

